/*
   Copyright (C) 2014 - 2016 by Chris Beck <render787@gmail.com>
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
#include "log.hpp"

#include "config.hpp"
#include "display_context.hpp"
#include "filter_context.hpp"
#include "game_data.hpp"
#include "make_enum.hpp"
#include "map.hpp"
#include "map_location.hpp"
#include "scripting/game_lua_kernel.hpp" //Needed for lua kernel
#include "side_filter.hpp"
#include "team.hpp"
#include "terrain_filter.hpp"
#include "tod_manager.hpp"
#include "unit.hpp"
#include "unit_formula_manager.hpp"
#include "unit_map.hpp"
#include "unit_types.hpp"
#include "variable.hpp" // needed for vconfig, scoped unit
#include "wml_exception.hpp" // needed for FAIL

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility/in_place_factory.hpp> //needed for boost::in_place to initialize optionals

#include <vector>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define DBG_CF LOG_STREAM(debug, log_config)

///Defined out of line to prevent including unit at unit_filter.hpp
bool unit_filter::matches(const unit & u) const {
	return matches (u, u.get_location());
}

bool unit_filter::matches(const unit & u, const map_location & loc, const unit & u2) const {
	return impl_->matches(u,loc,&u2);
}

bool unit_filter::matches(const unit & u, const unit & u2) const {
	return matches(u, u.get_location(), u2);
}

//bool unit_filter::matches(const unit & /*u*/, const map_location & /*loc*/) const {
//	assert(false && "called match against a pure abstract unit_filter! this indicates a programmer error, this function must be overrided");
//	return false;
//}


/// Forward declare the "construct" method which constructs an appropriate filter impl
static boost::shared_ptr<unit_filter_abstract_impl> construct(const vconfig & vcfg, const filter_context & fc, bool flat_tod);

/// Null unit filter is built when the input config is null
class null_unit_filter_impl : public unit_filter_abstract_impl {
public:
	null_unit_filter_impl(const filter_context & fc) : fc_(fc) {}
	virtual bool matches(const unit & /*u*/, const map_location & /*loc*/, const unit *) const {
		return true;
	}
	virtual std::vector<const unit *> all_matches_on_map() const {
		std::vector<const unit *> ret;
		BOOST_FOREACH(const unit & u, fc_.get_disp_context().units()) {
			ret.push_back(&u);
		}
		return ret;
	}

	virtual unit_const_ptr first_match_on_map() const {
		return fc_.get_disp_context().units().begin().get_shared_ptr();
	}


	virtual ~null_unit_filter_impl() {}

private:
	const filter_context & fc_;
};

/// This enum helps to evaluate conditional filters
namespace conditional {
	MAKE_ENUM (TYPE,
		(AND, "and")
		(OR, "or")
		(NOT, "not")
	)
}


/// The basic unit filter gives a generic implementation of the match fcn
class basic_unit_filter_impl : public unit_filter_abstract_impl {
public:
	basic_unit_filter_impl(const vconfig & vcfg, const filter_context & fc, bool flat_tod)
		: fc_(fc)
		, vcfg(vcfg)
		, use_flat_tod_(flat_tod)
		, cond_children_()
		, cond_child_types_()
	{
		// Handle [and], [or], and [not] with in-order precedence
		vconfig::all_children_iterator cond = vcfg.ordered_begin();
		vconfig::all_children_iterator cond_end = vcfg.ordered_end();
		while(cond != cond_end)
		{
			const std::string& cond_name = cond.get_key();
			conditional::TYPE type;
			if(type.parse(cond_name)) {
				const vconfig& cond_filter = cond.get_child();

				cond_children_.push_back(unit_filter(cond_filter, &fc_, use_flat_tod_));
				cond_child_types_.push_back(type);			
			}
			else {
				static const int NUM_VALID_TAGS = 5;
				static const std::string valid_tags[NUM_VALID_TAGS] = {
					"filter_vision",
					"filter_adjacent",
					"filter_location",
					"filter_side",
					"filter_wml",
				};
				static const std::string* const valid_tags_end = valid_tags + NUM_VALID_TAGS;

				if (std::find(valid_tags, valid_tags_end, cond_name) == valid_tags_end){
					std::stringstream errmsg;
					errmsg << "encountered a child [" << cond_name << "] of a standard unit filter, it is being ignored";
					DBG_CF << errmsg.str() << std::endl; //FAIL( errmsg.str() );
				}

			}
			++cond;
		}
		this->vcfg.make_safe();
	}

	virtual bool matches(const unit & u, const map_location & loc, const unit * u2) const;
	virtual std::vector<const unit *> all_matches_on_map() const;
	virtual unit_const_ptr first_match_on_map() const;

	virtual ~basic_unit_filter_impl() {}
private:
	const filter_context & fc_;
	const vconfig vcfg;
	bool use_flat_tod_;

	std::vector<unit_filter> cond_children_;
	std::vector<conditional::TYPE> cond_child_types_;

	bool internal_matches_filter(const unit & u, const map_location & loc) const;
};

/** "Factory" method which constructs an appropriate implementation
 *
 */

static boost::shared_ptr<unit_filter_abstract_impl> construct(const vconfig & vcfg, const filter_context & fc, bool flat_tod)
{
	if (vcfg.null()) {
		return boost::make_shared<null_unit_filter_impl> (fc);
	}
	return boost::make_shared<basic_unit_filter_impl>(vcfg, fc, flat_tod);
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
	impl_ = construct(vcfg, *fc, flat_tod);
}

/** Begin implementations of filter impl's
 */

bool basic_unit_filter_impl::matches(const unit & u, const map_location& loc, const unit * u2) const
{
	bool matches = true;

	if(loc.valid()) {
		scoped_xy_unit auto_store("this_unit", loc.x, loc.y, fc_.get_disp_context().units());
		if (u2) {
			const map_location& loc2 = u2->get_location();
			scoped_xy_unit auto_store("other_unit", loc2.x, loc2.y, fc_.get_disp_context().units());
			matches = internal_matches_filter(u, loc);
		} else {
			matches = internal_matches_filter(u, loc);
		}
	} else {
		// If loc is invalid, then this is a recall list unit (already been scoped)
		matches = internal_matches_filter(u, loc);
	}

	// Handle [and], [or], and [not] with in-order precedence
	for (size_t i = 0; i < cond_children_.size(); i++) {
		switch (cond_child_types_[i].v) {
			case conditional::TYPE::AND:
				matches = matches && cond_children_[i].matches(u,loc);
				break;
			case conditional::TYPE::OR:
				matches = matches || cond_children_[i].matches(u,loc);
				break;
			case conditional::TYPE::NOT:
				matches = matches && !cond_children_[i].matches(u,loc);
		}
	}
	return matches;
}

bool basic_unit_filter_impl::internal_matches_filter(const unit & u, const map_location& loc) const
{
	if (!vcfg["name"].blank() && vcfg["name"].t_str() != u.name()) {
		return false;
	}

	if (!vcfg["id"].empty()) {
		std::vector<std::string> id_list = utils::split(vcfg["id"]);
		if (std::find(id_list.begin(), id_list.end(), u.id()) == id_list.end()) {
			return false;
		}
	}

	// Allow 'speaker' as an alternative to id, since people use it so often
	if (!vcfg["speaker"].blank() && vcfg["speaker"].str() != u.id()) {
		return false;
	}

	if (vcfg.has_child("filter_location")) {
		if (vcfg.count_children("filter_location") > 1) {
			FAIL("Encountered multiple [filter_location] children of a standard unit filter. "
				 "This is not currently supported and in all versions of wesnoth would have "
				 "resulted in the later children being ignored. You must use [and] or similar "
				 "to achieve the desired result.");
		}
		terrain_filter filt(vcfg.child("filter_location"), &fc_, use_flat_tod_);
		if (!filt.match(loc)) {
			return false;
		}
	}

	if(vcfg.has_child("filter_side")) {
		if (vcfg.count_children("filter_side") > 1) {
			FAIL("Encountered multiple [filter_side] children of a standard unit filter. "
				 "This is not currently supported and in all versions of wesnoth would have "
				 "resulted in the later children being ignored. You must use [and] or similar "
				 "to achieve the desired result.");
		}
		side_filter filt(vcfg.child("filter_side"), &fc_);
		if(!filt.match(u.side()))
			return false;
	}

	// Also allow filtering on location ranges outside of the location filter
	if (!vcfg["x"].blank() || !vcfg["y"].blank()){
		if(vcfg["x"] == "recall" && vcfg["y"] == "recall") {
			//locations on the map are considered to not be on a recall list
			if (fc_.get_disp_context().map().on_board(loc))
			{
				return false;
			}
		} else if(vcfg["x"].empty() && vcfg["y"].empty()) {
			return false;
		} else if(!loc.matches_range(vcfg["x"], vcfg["y"])) {
			return false;
		}
	}

	// The type could be a comma separated list of types
	if (!vcfg["type"].empty()) {
		std::vector<std::string> types = utils::split(vcfg["type"]);
		if (std::find(types.begin(), types.end(), u.type_id()) == types.end()) {
			return false;
		}
	}

	// The variation_type could be a comma separated list of types
	if (!vcfg["variation"].empty())
	{
		std::vector<std::string> types = utils::split(vcfg["variation"]);
		if (std::find(types.begin(), types.end(), u.variation()) == types.end()) {
			return false;
		}
	}

	// The has_variation_type could be a comma separated list of types
	if (!vcfg["has_variation"].empty())
	{
		bool match = false;
		// If this unit is a variation itself then search in the base unit's variations.
		const unit_type* const type = u.variation().empty() ? &u.type() : unit_types.find(u.type().base_id());
		assert(type);

		BOOST_FOREACH(const std::string& variation_id, utils::split(vcfg["has_variation"])) {
			if (type->has_variation(variation_id)) {
				match = true;
				break;
			}
		}
		if (!match) return false;
	}

	if (!vcfg["ability"].empty())
	{
		bool match = false;

		BOOST_FOREACH(const std::string& ability_id, utils::split(vcfg["ability"])) {
			if (u.has_ability_by_id(ability_id)) {
				match = true;
				break;
			}
		}
		if (!match) return false;
	}

	if (!vcfg["race"].empty()) {
		std::vector<std::string> races = utils::split(vcfg["race"]);
		if (std::find(races.begin(), races.end(), u.race()->id()) == races.end()) {
			return false;
		}
	}

	if (!vcfg["gender"].blank() && string_gender(vcfg["gender"]) != u.gender()) {
		return false;
	}

	if (!vcfg["side"].empty() && vcfg["side"].to_int(-999) != u.side()) {
		std::vector<std::string> sides = utils::split(vcfg["side"]);
		const std::string u_side = str_cast(u.side());
		if (std::find(sides.begin(), sides.end(), u_side) == sides.end()) {
			return false;
		}
	}

	// handle statuses list
	if (!vcfg["status"].empty()) {
		bool status_found = false;
		std::map<std::string, std::string> states_map = u.get_states();

		BOOST_FOREACH (const std::string status, utils::split(vcfg["status"])) {
			if (states_map[status] == "yes") {
				status_found = true;
				break;
			}
		}

		if(!status_found) {
			return false;
		}
	}

	if (!vcfg["has_weapon"].blank()) {
		std::string weapon = vcfg["has_weapon"];
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

	if (!vcfg["role"].blank() && vcfg["role"].str() != u.get_role()) {
		return false;
	}

	if (!vcfg["ai_special"].blank() && ((vcfg["ai_special"].str() == "guardian") != u.get_state(unit::STATE_GUARDIAN))) {
		return false;
	}

	if (!vcfg["canrecruit"].blank() && vcfg["canrecruit"].to_bool() != u.can_recruit()) {
		return false;
	}

	if (!vcfg["recall_cost"].blank() && vcfg["recall_cost"].to_int(-1) != u.recall_cost()) {
		return false;
	}

	if (!vcfg["level"].blank() && vcfg["level"].to_int(-1) != u.level()) {
		return false;
	}

	if (!vcfg["defense"].blank() && vcfg["defense"].to_int(-1) != u.defense_modifier(fc_.get_disp_context().map().get_terrain(loc))) {
		return false;
	}

	if (!vcfg["movement_cost"].blank() && vcfg["movement_cost"].to_int(-1) != u.movement_cost(fc_.get_disp_context().map().get_terrain(loc))) {
		return false;
	}

	// Now start with the new WML based comparison.
	// If a key is in the unit and in the filter, they should match
	// filter only => not for us
	// unit only => not filtered
	config unit_cfg; // No point in serializing the unit once for each [filter_wml]!
	BOOST_FOREACH(const vconfig& wmlcfg, vcfg.get_children("filter_wml")) {
			config fwml = wmlcfg.get_parsed_config();
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

	BOOST_FOREACH(const vconfig& vision, vcfg.get_children("filter_vision")) {
		std::set<int> viewers;

		// Use standard side filter
		side_filter ssf(vision, &fc_);
		std::vector<int> sides = ssf.get_teams();
		viewers.insert(sides.begin(), sides.end());

		bool found = false;
		BOOST_FOREACH (const int viewer, viewers) {
			bool fogged = fc_.get_disp_context().teams()[viewer - 1].fogged(loc);
			bool hiding = u.invisible(loc/*, false(?) */);
			bool unit_hidden = fogged || hiding;
			if (vision["visible"].to_bool(true) != unit_hidden) {
				found = true;
				break;
			}
		}
		if (!found) {return false;}
	}

	if (vcfg.has_child("filter_adjacent")) {
		const unit_map& units = fc_.get_disp_context().units();
		map_location adjacent[6];
		get_adjacent_tiles(loc, adjacent);

		BOOST_FOREACH(const vconfig& adj_cfg, vcfg.get_children("filter_adjacent")) {
			int match_count=0;
			unit_filter filt(adj_cfg, &fc_, use_flat_tod_);

			config::attribute_value i_adjacent = adj_cfg["adjacent"];
			std::vector<map_location::DIRECTION> dirs;
			if (i_adjacent.blank()) {
				dirs = map_location::default_dirs();
			} else {
				dirs = map_location::parse_directions(i_adjacent);
			}

			std::vector<map_location::DIRECTION>::const_iterator j, j_end = dirs.end();
			for (j = dirs.begin(); j != j_end; ++j) {
				unit_map::const_iterator unit_itor = units.find(adjacent[*j]);
				if (unit_itor == units.end() || !filt(*unit_itor, u)) {
					continue;
				}
				boost::optional<bool> is_enemy;
				if (!adj_cfg["is_enemy"].blank()) {
					is_enemy = adj_cfg["is_enemy"].to_bool();
				}
				if (!is_enemy || *is_enemy ==
				    fc_.get_disp_context().teams()[u.side() - 1].is_enemy(unit_itor->side())) {
					++match_count;
				}
			}

			static std::vector<std::pair<int,int> > default_counts = utils::parse_ranges("1-6");
			config::attribute_value i_count = adj_cfg["count"];
			if(!in_ranges(match_count, !i_count.blank() ? utils::parse_ranges(i_count) : default_counts)) {
				return false;
			}
		}
	}

	if (!vcfg["find_in"].blank()) {
		// Allow filtering by searching a stored variable of units
		if (const game_data * gd = fc_.get_game_data()) {
			try
			{
				variable_access_const vi = gd->get_variable_access_read(vcfg["find_in"]);
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
	}
	if (!vcfg["formula"].blank()) {
		if (!u.formula_manager().matches_filter(vcfg["formula"], loc, u)) {
			return false;
		}
	}

	if (!vcfg["lua_function"].blank()) {
		if (game_lua_kernel * lk = fc_.get_lua_kernel()) {
			bool b = lk->run_filter(vcfg["lua_function"].str().c_str(), u);
			if (!b) return false;
		}
	}

	return true;
}

std::vector<const unit *> basic_unit_filter_impl::all_matches_on_map() const {
	std::vector<const unit *> ret;
	BOOST_FOREACH(const unit & u, fc_.get_disp_context().units()) {
		if (matches(u, u.get_location(), NULL)) {
			ret.push_back(&u);
		}
	}
	return ret;
}

unit_const_ptr basic_unit_filter_impl::first_match_on_map() const {
	const unit_map & units = fc_.get_disp_context().units();
	for(unit_map::const_iterator u = units.begin(); u != units.end(); u++) {
		if (matches(*u,u->get_location(),NULL)) {
			return u.get_shared_ptr();
		}
	}
	return unit_const_ptr();
}
