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
#include "game_data.hpp"
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
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>

///Defined out of line to prevent including unit at unit_filter.hpp
bool unit_filter::matches(const unit & u) const {
	return matches (u, u.get_location());
}

//bool unit_filter::matches(const unit & /*u*/, const map_location & /*loc*/) const {
//	assert(false && "called match against a pure abstract unit_filter! this indicates a programmer error, this function must be overrided");
//	return false;
//}

/// Null unit filter is built when the input config is null
class null_unit_filter_impl : public unit_filter_abstract_impl {
public:
	null_unit_filter_impl() {}
	virtual bool matches(const unit & /*u*/, const map_location & /*loc*/) const {
		return true;
	}

	~null_unit_filter_impl() {}
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
		: vcfg_(vcfg)
		, fc_(fc)
		, use_flat_tod_(flat_tod)
	{
		// Handle [and], [or], and [not] with in-order precedence
		vconfig::all_children_iterator cond = vcfg_.ordered_begin();
		vconfig::all_children_iterator cond_end = vcfg_.ordered_end();
		while(cond != cond_end)
		{
			try {
				const std::string& cond_name = cond.get_key();
				conditional::TYPE type = conditional::string_to_TYPE(cond_name); // throws bad_enum_cast if we don't get a string match with any enum

				const vconfig& cond_filter = cond.get_child();

				cond_children_.push_back(new basic_unit_filter_impl(cond_filter, fc_, use_flat_tod_));
				cond_child_types_.push_back(type);
			} catch (bad_enum_cast &) {} //ignore tags that aren't conditionals

			++cond;
		}
	}

	virtual bool matches(const unit & u, const map_location & loc) const;

	~basic_unit_filter_impl() {}
private:
	const vconfig vcfg_;
	const filter_context & fc_;
	bool use_flat_tod_;

	boost::ptr_vector<unit_filter_abstract_impl> cond_children_;
	std::vector<conditional::TYPE> cond_child_types_;

	bool internal_matches_filter(const unit & u, const map_location & loc) const;
};

/** Ctor of unit filter
 *  unit_filter::unit_filter acts as a factory, selecting the appropriate implementation class
 */
unit_filter::unit_filter(const vconfig & vcfg, const filter_context * fc, bool flat_tod)
{
	if (!fc) {
		assert(false && "attempt to instantiate a unit filter with a null filter context!");
	}
	if (vcfg.null()) {
		impl_.reset(new null_unit_filter_impl());
	}
	impl_.reset(new basic_unit_filter_impl(vcfg, *fc, flat_tod));
	//TODO: Add more efficient implementations for special cases
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
	config::attribute_value cfg_name = vcfg_["name"];
	if (!cfg_name.blank() && cfg_name.str() != u.name()) {
		return false;
	}

	const config::attribute_value cfg_id = vcfg_["id"];
	if (!cfg_id.blank()) {
		const std::string& id = cfg_id;
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
	config::attribute_value cfg_speaker = vcfg_["speaker"];
	if (!cfg_speaker.blank() && cfg_speaker.str() != u.id()) {
		return false;
	}

	if(vcfg_.has_child("filter_location")) {
		const vconfig& t_cfg = vcfg_.child("filter_location");
		terrain_filter t_filter(t_cfg, &fc_, use_flat_tod_);
		if(!t_filter.match(loc)) {
			return false;
		}
	}

	const vconfig& filter_side = vcfg_.child("filter_side");
	if(!filter_side.null()) {
		side_filter s_filter(filter_side, &fc_);
		if(!s_filter.match(u.side()))
			return false;
	}

	// Also allow filtering on location ranges outside of the location filter
	config::attribute_value cfg_x = vcfg_["x"];
	config::attribute_value cfg_y = vcfg_["y"];
	if (!cfg_x.blank() || !cfg_y.blank()){
		if(cfg_x == "recall" && cfg_y == "recall") {
			//locations on the map are considered to not be on a recall list
			if (fc_.get_disp_context().map().on_board(loc))
			{
				return false;
			}
		} else if(cfg_x.empty() && cfg_y.empty()) {
			return false;
		} else if(!loc.matches_range(cfg_x, cfg_y)) {
			return false;
		}
	}

	// The type could be a comma separated list of types
	config::attribute_value cfg_type = vcfg_["type"];
	if (!cfg_type.blank())
	{
		const std::string type_ids = cfg_type.str();
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
	config::attribute_value cfg_variation_type = vcfg_["variation"];
	if (!cfg_variation_type.blank())
	{
		const std::string type_ids = cfg_variation_type.str();
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
	config::attribute_value cfg_has_variation_type = vcfg_["has_variation"];
	if (!cfg_has_variation_type.blank())
	{
		const std::string& var_ids  = cfg_has_variation_type.str();
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

	config::attribute_value cfg_ability = vcfg_["ability"];
	if (!cfg_ability.blank())
	{
		std::string ability = cfg_ability;
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

	config::attribute_value cfg_race = vcfg_["race"];
	if (!cfg_race.blank()) {
		std::string race = cfg_race;

		if(race != u.race()->id()) {
			const std::vector<std::string>& vals = utils::split(race);
			if(std::find(vals.begin(), vals.end(), u.race()->id()) == vals.end()) {
				return false;
			}
		}
	}

	config::attribute_value cfg_gender = vcfg_["gender"];
	if (!cfg_gender.blank() && string_gender(cfg_gender) != u.gender()) {
		return false;
	}

	config::attribute_value cfg_side = vcfg_["side"];
	if (!cfg_side.blank() && cfg_side.to_int() != u.side()) {
		std::string side = cfg_side;
		if ( side.find(',') == std::string::npos ) {
			return false;
		}
		std::vector<std::string> vals = utils::split(side);
		if (std::find(vals.begin(), vals.end(), str_cast(u.side())) == vals.end()) {
			return false;
		}
	}

	config::attribute_value cfg_has_weapon = vcfg_["has_weapon"];
	if (!cfg_has_weapon.blank()) {
		std::string weapon = cfg_has_weapon;
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

	config::attribute_value cfg_role = vcfg_["role"];
	if (!cfg_role.blank() && cfg_role.str() != u.get_role()) {
		return false;
	}

	config::attribute_value cfg_ai_special = vcfg_["ai_special"];
	if (!cfg_ai_special.blank() && ((cfg_ai_special.str() == "guardian")  != u.get_state(unit::STATE_GUARDIAN))) {
		return false;
	}

	config::attribute_value cfg_canrecruit = vcfg_["canrecruit"];
	if (!cfg_canrecruit.blank() && cfg_canrecruit.to_bool() != u.can_recruit()) {
		return false;
	}

	config::attribute_value cfg_recall_cost = vcfg_["recall_cost"];
	if (!cfg_recall_cost.blank() && cfg_recall_cost.to_int(-1) != u.recall_cost()) {
		return false;
	}

	config::attribute_value cfg_level = vcfg_["level"];
	if (!cfg_level.blank() && cfg_level.to_int(-1) != u.level()) {
		return false;
	}

	config::attribute_value cfg_defense = vcfg_["defense"];
	if (!cfg_defense.blank() && cfg_defense.to_int(-1) != u.defense_modifier(fc_.get_disp_context().map().get_terrain(loc))) {
		return false;
	}

	config::attribute_value cfg_movement = vcfg_["movement_cost"];
	if (!cfg_movement.blank() && cfg_movement.to_int(-1) != u.movement_cost(fc_.get_disp_context().map().get_terrain(loc))) {
		return false;
	}

	// Now start with the new WML based comparison.
	// If a key is in the unit and in the filter, they should match
	// filter only => not for us
	// unit only => not filtered
	const vconfig::child_list& wmlcfgs = vcfg_.get_children("filter_wml");
	if (!wmlcfgs.empty()) {
		config unit_cfg;
		for (unsigned i = 0; i < wmlcfgs.size(); ++i)
		{
			config fwml = wmlcfgs[i].get_parsed_config();
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

	if (vcfg_.has_child("filter_vision")) {
		const vconfig::child_list& vis_filt = vcfg_.get_children("filter_vision");
		vconfig::child_list::const_iterator i, i_end = vis_filt.end();
		for (i = vis_filt.begin(); i != i_end; ++i) {
			bool visible = (*i)["visible"].to_bool(true);
			std::set<int> viewers;
			// Use standard side filter
			side_filter ssf(*i, &fc_);
			std::vector<int> sides = ssf.get_teams();
			viewers.insert(sides.begin(), sides.end());
			if (viewers.empty()) {
				return false;
			}
			std::set<int>::const_iterator viewer, viewer_end = viewers.end();
			for (viewer = viewers.begin(); viewer != viewer_end; ++viewer) {
				bool fogged = fc_.get_disp_context().teams()[*viewer - 1].fogged(loc);
				bool hiding = u.invisible(loc/*, false(?) */);
				bool unit_hidden = fogged || hiding;
				if (visible == unit_hidden) return false;
			}
		}
	}

	if (vcfg_.has_child("filter_adjacent")) {
		const unit_map& units = fc_.get_disp_context().units();
		map_location adjacent[6];
		get_adjacent_tiles(loc, adjacent);
		vconfig::child_list::const_iterator i, i_end;
		const vconfig::child_list& adj_filt = vcfg_.get_children("filter_adjacent");
		for (i = adj_filt.begin(), i_end = adj_filt.end(); i != i_end; ++i) {
			int match_count=0;
			config::attribute_value i_adjacent = (*i)["adjacent"];
			std::vector<map_location::DIRECTION> dirs = !i_adjacent.blank() ?
				map_location::parse_directions(i_adjacent) : map_location::default_dirs();
			std::vector<map_location::DIRECTION>::const_iterator j, j_end = dirs.end();
			for (j = dirs.begin(); j != j_end; ++j) {
				unit_map::const_iterator unit_itor = units.find(adjacent[*j]);
				if (unit_itor == units.end()
				|| !unit_filter(*i, &fc_, use_flat_tod_).matches(*unit_itor)) {
					continue;
				}
				config::attribute_value i_is_enemy = (*i)["is_enemy"];
				if (i_is_enemy.blank() || i_is_enemy.to_bool() ==
				    fc_.get_disp_context().teams()[u.side() - 1].is_enemy(unit_itor->side())) {
					++match_count;
				}
			}
			static std::vector<std::pair<int,int> > default_counts = utils::parse_ranges("1-6");
			config::attribute_value i_count = (*i)["count"];
			std::vector<std::pair<int,int> > counts = !i_count.blank()
				? utils::parse_ranges(i_count) : default_counts;
			if(!in_ranges(match_count, counts)) {
				return false;
			}
		}
	}

	config::attribute_value cfg_find_in = vcfg_["find_in"];
	if (!cfg_find_in.blank()) {
		// Allow filtering by searching a stored variable of units
		try
		{
			variable_access_const vi = resources::gamedata->get_variable_access_read(cfg_find_in);
			bool found_id = false;
			BOOST_FOREACH(const config& c, vi.as_array())
			{
				if(c["id"] == u.id())
					found_id = true;
			}
			if(!found_id)
			{
				return false;
			}
		}
		catch(const invalid_variablename_exception&)
		{
			return false;
		}
	}
	config::attribute_value cfg_formula = vcfg_["formula"];
	if (!cfg_formula.blank()) {
		if (!u.formula_manager().matches_filter(cfg_formula, loc, u)) {
			return false;
		}
	}

	config::attribute_value cfg_lua_function = vcfg_["lua_function"];
	if (!cfg_lua_function.blank()) {
		bool b = resources::lua_kernel->run_filter(cfg_lua_function.str().c_str(), u);
		if (!b) return false;
	}

	return true;
}
