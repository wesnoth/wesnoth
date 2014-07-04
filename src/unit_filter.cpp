/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "unit_filter.hpp"
#include "global.hpp"

#include "config.hpp"
#include "display_context.hpp"
#include "filter_context.hpp"
#include "make_enum.hpp"
#include "map_location.hpp"
#include "resources.hpp" //Needed for lua kernel pointer
#include "scripting/lua.hpp" //Needed for lua kernel
#include "side_filter.hpp"
#include "team.hpp"
#include "terrain_filter.hpp"
#include "unit.hpp"
#include "unit_formula_manager.hpp"
#include "unit_map.hpp"
#include "unit_types.hpp"
#include "variable.hpp" // needed for vconfig, scoped unit

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/utility/in_place_factory.hpp> //needed for boost::in_place to initialize optionals

#include <vector>

///Defined out of line to prevent including unit at unit_filter.hpp
bool unit_filter::matches(const unit & u) const {
	return matches (u, u.get_location());
}

//bool unit_filter::matches(const unit & /*u*/, const map_location & /*loc*/) const {
//	assert(false && "called match against a pure abstract unit_filter! this indicates a programmer error, this function must be overrided");
//	return false;
//}


/// Forward declare the "construct" method which constructs an appropriate filter impl
static unit_filter_abstract_impl * construct(const vconfig & vcfg, const filter_context & fc, bool flat_tod);

/// Null unit filter is built when the input config is null
class null_unit_filter_impl : public unit_filter_abstract_impl {
public:
	null_unit_filter_impl() {}
	virtual bool matches(const unit & /*u*/, const map_location & /*loc*/) const {
		return true;
	}

	virtual ~null_unit_filter_impl() {}
};

/// This enum helps to evaluate conditional filters
namespace conditional {
	MAKE_ENUM (TYPE,
		(AND, "and")
		(OR, "or")
		(NOT, "not")
	)
	MAKE_ENUM_STREAM_OPS1(TYPE)

	static TYPE warning_suppressor = string_to_TYPE_default("foo", NOT);
}

/// The basic unit filter gives a generic implementation of the match fcn
class basic_unit_filter_impl : public unit_filter_abstract_impl {
public:
	basic_unit_filter_impl(const vconfig & vcfg, const filter_context & fc, bool flat_tod)
		: fc_(fc)
		, use_flat_tod_(flat_tod)
		, cond_children_()
		, cond_child_types_()
		, cfg_name_(vcfg["name"])
		, cfg_id_(vcfg["id"])
		, cfg_speaker_(vcfg["speaker"])
		, cfg_filter_loc_(vcfg.has_child("filter_location") ? new terrain_filter(vconfig(vcfg.child("filter_location")), &fc_, use_flat_tod_) : NULL)
		, cfg_filter_side_(vcfg.has_child("filter_side") ? new side_filter(vconfig(vcfg.child("filter_side")), &fc_) : NULL) //Note that it would be better to use boost optional here but it is apparently not possible to do in an initialization list using boost::none, because when using ? the types must match, and side_filter is non-copyable
		, cfg_x_(vcfg["x"])
		, cfg_y_(vcfg["y"])
		, cfg_type_(vcfg["type"])
		, cfg_variation_type_(vcfg["variation"])
		, cfg_has_variation_type_(vcfg["has_variation"])
		, cfg_ability_(vcfg["ability"])
		, cfg_race_(vcfg["race"])
		, cfg_gender_(vcfg["gender"])
		, cfg_side_(vcfg["side"])
		, cfg_has_weapon_(vcfg["has_weapon"])
		, cfg_role_(vcfg["role"])
		, cfg_ai_special_(vcfg["ai_special"])
		, cfg_canrecruit_(vcfg["canrecruit"])
		, cfg_recall_cost_(vcfg["recall_cost"])
		, cfg_level_(vcfg["level"])
		, cfg_defense_(vcfg["defense"])
		, cfg_movement_(vcfg["movement_cost"])
		, wmlcfgs_(vcfg.get_children("filter_wml"))
		, vision_filters_viewers_lists_()
		, vision_filters_visible_attr_()
		, filter_adj_filters_()
		, filter_adj_is_enemy_()
		, filter_adj_dirs_()
		, filter_adj_counts_()
		, cfg_find_in_(vcfg["find_in"])
		, cfg_formula_(vcfg["formula"])
		, cfg_lua_function_(vcfg["lua_function"])

	{
		// Handle [and], [or], and [not] with in-order precedence
		vconfig::all_children_iterator cond = vcfg.ordered_begin();
		vconfig::all_children_iterator cond_end = vcfg.ordered_end();
		while(cond != cond_end)
		{
			const std::string& cond_name = cond.get_key();

			try {
				conditional::TYPE type = conditional::string_to_TYPE(cond_name); // throws bad_enum_cast if we don't get a string match with any enum

				const vconfig& cond_filter = cond.get_child();

				cond_children_.push_back(new unit_filter(cond_filter, &fc_, use_flat_tod_));
				cond_child_types_.push_back(type);
			} catch (bad_enum_cast &) { // this means it isn't a conditional filter tag

				//while we are here, process filter_vision tags and filter_adjacent
				if (cond_name == "filter_vision")
				{
					const vconfig& f = cond.get_child();
					vision_filters_visible_attr_.push_back(f["visible"].to_bool(true));

					std::set<int> viewers;

					// Use standard side filter
					side_filter ssf(f, &fc_);
					std::vector<int> sides = ssf.get_teams();
					viewers.insert(sides.begin(), sides.end());

					vision_filters_viewers_lists_.push_back(viewers);
				} else if (cond_name == "filter_adjacent") {
					const vconfig& f = cond.get_child();
					filter_adj_filters_.push_back(new unit_filter(f, &fc_, use_flat_tod_));

					config::attribute_value i_adjacent = f["adjacent"];
					filter_adj_dirs_.push_back(!i_adjacent.blank() ? map_location::parse_directions(i_adjacent) : map_location::default_dirs());

					config::attribute_value i_is_enemy = f["is_enemy"];
					if (i_is_enemy.blank()) {
						filter_adj_is_enemy_.push_back(boost::none);
					} else {
						filter_adj_is_enemy_.push_back(i_is_enemy.to_bool());
					}
					static std::vector<std::pair<int,int> > default_counts = utils::parse_ranges("1-6");
					config::attribute_value i_count = f["count"];
					filter_adj_counts_.push_back(!i_count.blank() ? utils::parse_ranges(i_count) : default_counts);
				}
			}
			++cond;
		}
	}

	virtual bool matches(const unit & u, const map_location & loc) const;

	virtual ~basic_unit_filter_impl() {}
private:
	const filter_context & fc_;
	bool use_flat_tod_;

	boost::ptr_vector<unit_filter> cond_children_;
	std::vector<conditional::TYPE> cond_child_types_;

	const config::attribute_value cfg_name_;
	const config::attribute_value cfg_id_;
	const config::attribute_value cfg_speaker_;
	boost::scoped_ptr<terrain_filter> cfg_filter_loc_;
	boost::scoped_ptr<side_filter> cfg_filter_side_;
	const config::attribute_value cfg_x_;
	const config::attribute_value cfg_y_;
	const config::attribute_value cfg_type_;
	const config::attribute_value cfg_variation_type_;
	const config::attribute_value cfg_has_variation_type_;
	const config::attribute_value cfg_ability_;
	const config::attribute_value cfg_race_;
	const config::attribute_value cfg_gender_;
	const config::attribute_value cfg_side_;
	const config::attribute_value cfg_has_weapon_;
	const config::attribute_value cfg_role_;
	const config::attribute_value cfg_ai_special_;
	const config::attribute_value cfg_canrecruit_;
	const config::attribute_value cfg_recall_cost_;
	const config::attribute_value cfg_level_;
	const config::attribute_value cfg_defense_;
	const config::attribute_value cfg_movement_;

	const vconfig::child_list wmlcfgs_;

	std::vector<std::set<int> > vision_filters_viewers_lists_;
	std::vector<bool> vision_filters_visible_attr_;

	boost::ptr_vector<unit_filter> filter_adj_filters_;
	std::vector<boost::optional<bool> > filter_adj_is_enemy_;
	std::vector<std::vector<map_location::DIRECTION> > filter_adj_dirs_;
	std::vector<std::vector<std::pair<int,int> > > filter_adj_counts_;

	const config::attribute_value cfg_find_in_;
	const config::attribute_value cfg_formula_;
	const config::attribute_value cfg_lua_function_;

	bool internal_matches_filter(const unit & u, const map_location & loc) const;
};

/** "Factory" method which constructs an appropriate implementation
 *
 */

static unit_filter_abstract_impl * construct(const vconfig & vcfg, const filter_context & fc, bool flat_tod)
{
	if (vcfg.null()) {
		return new null_unit_filter_impl();
	}
	return new basic_unit_filter_impl(vcfg, fc, flat_tod);
	//TODO: Add more efficient implementations for special cases
}

/** Ctor of unit filter
 *  unit_filter::unit_filter acts as a factory, selecting the appropriate implementation class
 */
unit_filter::unit_filter(const vconfig & vcfg, const filter_context * fc, bool flat_tod)
{
	if (!fc) {
		assert(false && "attempt to instantiate a unit filter with a null filter context!");
	}
	impl_.reset(construct(vcfg, *fc, flat_tod));
}

/** Begin implementations of filter impl's
 */

bool basic_unit_filter_impl::matches(const unit & u, const map_location& loc) const
{
	bool matches = true;

	if(loc.valid()) {
		scoped_xy_unit auto_store("this_unit", loc.x, loc.y, fc_.get_disp_context().units());
		matches = internal_matches_filter(u, loc);
	} else {
		// If loc is invalid, then this is a recall list unit (already been scoped)
		matches = internal_matches_filter(u, loc);
	}

	// Handle [and], [or], and [not] with in-order precedence
	for (size_t i = 0; i < cond_children_.size(); i++) {
		switch (cond_child_types_[i]) {
			case conditional::AND:
				matches = matches && cond_children_[i].matches(u,loc);
				break;
			case conditional::OR:
				matches = matches || cond_children_[i].matches(u,loc);
				break;
			case conditional::NOT:
				matches = matches && !cond_children_[i].matches(u,loc);
		}
	}
	return matches;
}

bool basic_unit_filter_impl::internal_matches_filter(const unit & u, const map_location& loc) const
{
	if (!cfg_name_.blank() && cfg_name_.str() != u.name()) {
		return false;
	}

	if (!cfg_id_.blank()) {
		const std::string& id = cfg_id_;
		const std::string& this_id = u.id();

		if (id == this_id) {
		}
		else if ( id.find(',') == std::string::npos ){
			return false;
		}
		else {
			const std::vector<std::string>& ids = utils::split(id);
			if (std::find(ids.begin(), ids.end(), this_id) == ids.end()) {
				return false;
			}
		}
	}

	// Allow 'speaker' as an alternative to id, since people use it so often
	if (!cfg_speaker_.blank() && cfg_speaker_.str() != u.id()) {
		return false;
	}

	if(cfg_filter_loc_) {
		if(!cfg_filter_loc_->match(loc)) {
			return false;
		}
	}

	if(cfg_filter_side_) {
		if(!cfg_filter_side_->match(u.side()))
			return false;
	}

	// Also allow filtering on location ranges outside of the location filter
	if (!cfg_x_.blank() || !cfg_y_.blank()){
		if(cfg_x_ == "recall" && cfg_y_ == "recall") {
			//locations on the map are considered to not be on a recall list
			if (fc_.get_disp_context().map().on_board(loc))
			{
				return false;
			}
		} else if(cfg_x_.empty() && cfg_y_.empty()) {
			return false;
		} else if(!loc.matches_range(cfg_x_, cfg_y_)) {
			return false;
		}
	}

	// The type could be a comma separated list of types
	if (!cfg_type_.blank())
	{
		const std::string type_ids = cfg_type_.str();
		const std::string& this_type = u.type_id();

		// We only do the full CSV search if we find a comma in there,
		// and if the subsequence is found within the main sequence.
		// This is because doing the full CSV split is expensive.
		if ( type_ids == this_type ) {
			// pass
		} else if ( type_ids.find(',') != std::string::npos  &&
		            type_ids.find(this_type) != std::string::npos ) {
			const std::vector<std::string>& vals = utils::split(type_ids);

			if(std::find(vals.begin(),vals.end(),this_type) == vals.end()) {
				return false;
			}
		} else {
			return false;
		}
	}

	// The variation_type could be a comma separated list of types
	if (!cfg_variation_type_.blank())
	{
		const std::string type_ids = cfg_variation_type_.str();
		const std::string& this_type = u.variation();

		// We only do the full CSV search if we find a comma in there,
		// and if the subsequence is found within the main sequence.
		// This is because doing the full CSV split is expensive.
		if ( type_ids == this_type ) {
			// pass
		} else if ( type_ids.find(',') != std::string::npos  &&
				type_ids.find(this_type) != std::string::npos ) {
			const std::vector<std::string>& vals = utils::split(type_ids);

			if(std::find(vals.begin(),vals.end(),this_type) == vals.end()) {
				return false;
			}
		} else {
			return false;
		}
	}

	// The has_variation_type could be a comma separated list of types
	if (!cfg_has_variation_type_.blank())
	{
		const std::string& var_ids  = cfg_has_variation_type_.str();
		const std::string& this_var = u.variation();

		if ( var_ids == this_var ) {
			// pass
		} else {

			bool match = false;
			const std::vector<std::string>& variation_types = utils::split(var_ids);
			// If this unit is a variation itself then search in the base unit's variations.
			const unit_type* const type = this_var.empty() ? &u.type() : unit_types.find(u.type().base_id());
			assert(type);

			BOOST_FOREACH(const std::string& variation_id, variation_types) {
				if (type->has_variation(variation_id)) {
					match = true;
					break;
				}
			}
			if (!match) return false;
		}
	}

	if (!cfg_ability_.blank())
	{
		std::string ability = cfg_ability_;
		if(u.has_ability_by_id(ability)) {
			// pass
		} else if ( ability.find(',') != std::string::npos ) {
			const std::vector<std::string>& vals = utils::split(ability);
			bool has_ability = false;
			for(std::vector<std::string>::const_iterator this_ability = vals.begin(); this_ability != vals.end(); ++this_ability) {
				if(u.has_ability_by_id(*this_ability)) {
					has_ability = true;
					break;
				}
			}
			if(!has_ability) {
				return false;
			}
		} else {
			return false;
		}
	}

	if (!cfg_race_.blank()) {
		std::string race = cfg_race_;

		if(race != u.race()->id()) {
			const std::vector<std::string>& vals = utils::split(race);
			if(std::find(vals.begin(), vals.end(), u.race()->id()) == vals.end()) {
				return false;
			}
		}
	}

	if (!cfg_gender_.blank() && string_gender(cfg_gender_) != u.gender()) {
		return false;
	}

	if (!cfg_side_.blank() && cfg_side_.to_int() != u.side()) {
		std::string side = cfg_side_;
		if ( side.find(',') == std::string::npos ) {
			return false;
		}
		std::vector<std::string> vals = utils::split(side);
		if (std::find(vals.begin(), vals.end(), str_cast(u.side())) == vals.end()) {
			return false;
		}
	}

	if (!cfg_has_weapon_.blank()) {
		std::string weapon = cfg_has_weapon_;
		bool has_weapon = false;
		const std::vector<attack_type>& attacks = u.attacks();
		for(std::vector<attack_type>::const_iterator i = attacks.begin();
			i != attacks.end(); ++i) {
			if(i->id() == weapon) {
				has_weapon = true;
				break;
			}
		}
		if(!has_weapon) {
			return false;
		}
	}

	if (!cfg_role_.blank() && cfg_role_.str() != u.get_role()) {
		return false;
	}

	if (!cfg_ai_special_.blank() && ((cfg_ai_special_.str() == "guardian")  != u.get_state(unit::STATE_GUARDIAN))) {
		return false;
	}

	if (!cfg_canrecruit_.blank() && cfg_canrecruit_.to_bool() != u.can_recruit()) {
		return false;
	}

	if (!cfg_recall_cost_.blank() && cfg_recall_cost_.to_int(-1) != u.recall_cost()) {
		return false;
	}

	if (!cfg_level_.blank() && cfg_level_.to_int(-1) != u.level()) {
		return false;
	}

	if (!cfg_defense_.blank() && cfg_defense_.to_int(-1) != u.defense_modifier(fc_.get_disp_context().map().get_terrain(loc))) {
		return false;
	}

	if (!cfg_movement_.blank() && cfg_movement_.to_int(-1) != u.movement_cost(fc_.get_disp_context().map().get_terrain(loc))) {
		return false;
	}

	// Now start with the new WML based comparison.
	// If a key is in the unit and in the filter, they should match
	// filter only => not for us
	// unit only => not filtered
	if (!wmlcfgs_.empty()) {
		config unit_cfg;
		for (unsigned i = 0; i < wmlcfgs_.size(); ++i)
		{
			config fwml = wmlcfgs_[i].get_parsed_config();
			/* Check if the filter only cares about variables.
			   If so, no need to serialize the whole unit. */
			config::const_attr_itors ai = fwml.attribute_range();
			config::all_children_itors ci = fwml.all_children_range();
			if (std::distance(ai.first, ai.second) == 0 &&
			    std::distance(ci.first, ci.second) == 1 &&
			    ci.first->key == "variables") {
				if (!u.variables().matches(ci.first->cfg))
					return false;
			} else {
				if (unit_cfg.empty())
					u.write(unit_cfg);
				if (!unit_cfg.matches(fwml))
					return false;
			}
		}
	}

	assert(vision_filters_viewers_lists_.size() == vision_filters_visible_attr_.size());
	for (size_t i = 0; i < vision_filters_viewers_lists_.size(); i++) {
		const std::set<int> & viewers = vision_filters_viewers_lists_[i];
		if (viewers.empty()) {
			return false;
		}
		std::set<int>::const_iterator viewer, viewer_end = viewers.end();
		for (viewer = viewers.begin(); viewer != viewer_end; ++viewer) {
			bool fogged = fc_.get_disp_context().teams()[*viewer - 1].fogged(loc);
			bool hiding = u.invisible(loc/*, false(?) */);
			bool unit_hidden = fogged || hiding;
			if (vision_filters_visible_attr_[i] == unit_hidden) return false;
		}
	}

	assert(filter_adj_filters_.size() == filter_adj_is_enemy_.size());
	assert(filter_adj_filters_.size() == filter_adj_dirs_.size());
	assert(filter_adj_filters_.size() == filter_adj_counts_.size());
	if (filter_adj_filters_.size() > 0) {
		const unit_map& units = fc_.get_disp_context().units();
		map_location adjacent[6];
		get_adjacent_tiles(loc, adjacent);

		for (size_t i = 0; i < filter_adj_filters_.size(); i++) {
			int match_count=0;
			const std::vector<map_location::DIRECTION> & dirs = filter_adj_dirs_[i];

			std::vector<map_location::DIRECTION>::const_iterator j, j_end = dirs.end();
			for (j = dirs.begin(); j != j_end; ++j) {
				unit_map::const_iterator unit_itor = units.find(adjacent[*j]);
				if (unit_itor == units.end() || !filter_adj_filters_[i](*unit_itor)) {
					continue;
				}
				if (!filter_adj_is_enemy_[i] || *filter_adj_is_enemy_[i] ==
				    fc_.get_disp_context().teams()[u.side() - 1].is_enemy(unit_itor->side())) {
					++match_count;
				}
			}

			if(!in_ranges(match_count, filter_adj_counts_[i])) {
				return false;
			}
		}
	}

	if (!cfg_find_in_.blank()) {
		// Allow filtering by searching a stored variable of units
		variable_info vi(cfg_find_in_, false, variable_info::TYPE_CONTAINER);
		if(!vi.is_valid) return false;
		if(vi.explicit_index) {
			config::const_child_iterator i = vi.vars->child_range(vi.key).first;
			std::advance(i, vi.index);
			if ((*i)["id"] != u.id()) {
				return false;
			}
		} else {
			if (!vi.vars->find_child(vi.key, "id", u.id()))
				return false;
		}
	}
	if (!cfg_formula_.blank()) {
		if (!u.formula_manager().matches_filter(cfg_formula_, loc, u)) {
			return false;
		}
	}

	if (!cfg_lua_function_.blank()) {
		bool b = resources::lua_kernel->run_filter(cfg_lua_function_.str().c_str(), u);
		if (!b) return false;
	}

	return true;
}
