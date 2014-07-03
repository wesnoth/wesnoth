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

namespace { bool internal_matches_filter(const vconfig& filter, const unit & u, const map_location& loc, const filter_context * fc, bool use_flat_tod); }

namespace unit_filter {

bool matches_filter(const vconfig& filter, const unit & u, const filter_context * fc, bool use_flat_tod)
{ return matches_filter(filter, u, u.get_location(), fc, use_flat_tod); }

bool matches_filter(const vconfig& cfg, const unit & u, const map_location& loc, const filter_context * fc, bool use_flat_tod)
{
	bool matches = true;

	if(loc.valid()) {
		assert(fc != NULL);
		scoped_xy_unit auto_store("this_unit", loc.x, loc.y, fc->get_disp_context().units());
		matches = internal_matches_filter(cfg, u, loc, fc, use_flat_tod);
	} else {
		// If loc is invalid, then this is a recall list unit (already been scoped)
		matches = internal_matches_filter(cfg, u, loc, fc, use_flat_tod);
	}

	// Handle [and], [or], and [not] with in-order precedence
	vconfig::all_children_iterator cond = cfg.ordered_begin();
	vconfig::all_children_iterator cond_end = cfg.ordered_end();
	while(cond != cond_end)
	{

		const std::string& cond_name = cond.get_key();
		const vconfig& cond_filter = cond.get_child();

		// Handle [and]
		if(cond_name == "and") {
			matches = matches && matches_filter(cond_filter,u, loc, fc, use_flat_tod);
		}
		// Handle [or]
		else if(cond_name == "or") {
			matches = matches || matches_filter(cond_filter,u, loc, fc,use_flat_tod);
		}
		// Handle [not]
		else if(cond_name == "not") {
			matches = matches && !matches_filter(cond_filter,u, loc, fc,use_flat_tod);
		}

		++cond;
	}
	return matches;
}

} //end namespace unit_filter

namespace { //begin anonymous namespace

bool internal_matches_filter(const vconfig& cfg, const unit & u, const map_location& loc, const filter_context * fc, bool use_flat_tod)
{
	config::attribute_value cfg_name = cfg["name"];
	if (!cfg_name.blank() && cfg_name.str() != u.name()) {
		return false;
	}

	const config::attribute_value cfg_id = cfg["id"];
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
	config::attribute_value cfg_speaker = cfg["speaker"];
	if (!cfg_speaker.blank() && cfg_speaker.str() != u.id()) {
		return false;
	}

	if(cfg.has_child("filter_location")) {
		assert(fc != NULL);
		const vconfig& t_cfg = cfg.child("filter_location");
		terrain_filter t_filter(t_cfg, fc, use_flat_tod);
		if(!t_filter.match(loc)) {
			return false;
		}
	}

	const vconfig& filter_side = cfg.child("filter_side");
	if(!filter_side.null()) {
		side_filter s_filter(filter_side, fc);
		if(!s_filter.match(u.side()))
			return false;
	}

	// Also allow filtering on location ranges outside of the location filter
	config::attribute_value cfg_x = cfg["x"];
	config::attribute_value cfg_y = cfg["y"];
	if (!cfg_x.blank() || !cfg_y.blank()){
		if(cfg_x == "recall" && cfg_y == "recall") {
			//locations on the map are considered to not be on a recall list
			if ((!fc && loc.valid()) ||
			    (fc && fc->get_disp_context().map().on_board(loc)))
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
	config::attribute_value cfg_type = cfg["type"];
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
	config::attribute_value cfg_variation_type = cfg["variation"];
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
	config::attribute_value cfg_has_variation_type = cfg["has_variation"];
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

	config::attribute_value cfg_ability = cfg["ability"];
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

	config::attribute_value cfg_race = cfg["race"];
	if (!cfg_race.blank()) {
		std::string race = cfg_race;

		if(race != u.race()->id()) {
			const std::vector<std::string>& vals = utils::split(race);
			if(std::find(vals.begin(), vals.end(), u.race()->id()) == vals.end()) {
				return false;
			}
		}
	}

	config::attribute_value cfg_gender = cfg["gender"];
	if (!cfg_gender.blank() && string_gender(cfg_gender) != u.gender()) {
		return false;
	}

	config::attribute_value cfg_side = cfg["side"];
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

	config::attribute_value cfg_has_weapon = cfg["has_weapon"];
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

	config::attribute_value cfg_role = cfg["role"];
	if (!cfg_role.blank() && cfg_role.str() != u.get_role()) {
		return false;
	}

	config::attribute_value cfg_ai_special = cfg["ai_special"];
	if (!cfg_ai_special.blank() && ((cfg_ai_special.str() == "guardian")  != u.get_state(unit::STATE_GUARDIAN))) {
		return false;
	}

	config::attribute_value cfg_canrecruit = cfg["canrecruit"];
	if (!cfg_canrecruit.blank() && cfg_canrecruit.to_bool() != u.can_recruit()) {
		return false;
	}

	config::attribute_value cfg_recall_cost = cfg["recall_cost"];
	if (!cfg_recall_cost.blank() && cfg_recall_cost.to_int(-1) != u.recall_cost()) {
		return false;
	}

	config::attribute_value cfg_level = cfg["level"];
	if (!cfg_level.blank() && cfg_level.to_int(-1) != u.level()) {
		return false;
	}

	config::attribute_value cfg_defense = cfg["defense"];
	if (!cfg_defense.blank() && cfg_defense.to_int(-1) != u.defense_modifier(fc->get_disp_context().map().get_terrain(loc))) {
		return false;
	}

	config::attribute_value cfg_movement = cfg["movement_cost"];
	if (!cfg_movement.blank() && cfg_movement.to_int(-1) != u.movement_cost(fc->get_disp_context().map().get_terrain(loc))) {
		return false;
	}

	// Now start with the new WML based comparison.
	// If a key is in the unit and in the filter, they should match
	// filter only => not for us
	// unit only => not filtered
	const vconfig::child_list& wmlcfgs = cfg.get_children("filter_wml");
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

	if (cfg.has_child("filter_vision")) {
		const vconfig::child_list& vis_filt = cfg.get_children("filter_vision");
		vconfig::child_list::const_iterator i, i_end = vis_filt.end();
		for (i = vis_filt.begin(); i != i_end; ++i) {
			bool visible = (*i)["visible"].to_bool(true);
			std::set<int> viewers;
			// Use standard side filter
			side_filter ssf(*i, fc);
			std::vector<int> sides = ssf.get_teams();
			viewers.insert(sides.begin(), sides.end());
			if (viewers.empty()) {
				return false;
			}
			std::set<int>::const_iterator viewer, viewer_end = viewers.end();
			for (viewer = viewers.begin(); viewer != viewer_end; ++viewer) {
				bool fogged = fc->get_disp_context().teams()[*viewer - 1].fogged(loc);
				bool hiding = u.invisible(loc/*, false(?) */);
				bool unit_hidden = fogged || hiding;
				if (visible == unit_hidden) return false;
			}
		}
	}

	if (cfg.has_child("filter_adjacent")) {
		assert(fc);
		const unit_map& units = fc->get_disp_context().units();
		map_location adjacent[6];
		get_adjacent_tiles(loc, adjacent);
		vconfig::child_list::const_iterator i, i_end;
		const vconfig::child_list& adj_filt = cfg.get_children("filter_adjacent");
		for (i = adj_filt.begin(), i_end = adj_filt.end(); i != i_end; ++i) {
			int match_count=0;
			config::attribute_value i_adjacent = (*i)["adjacent"];
			std::vector<map_location::DIRECTION> dirs = !i_adjacent.blank() ?
				map_location::parse_directions(i_adjacent) : map_location::default_dirs();
			std::vector<map_location::DIRECTION>::const_iterator j, j_end = dirs.end();
			for (j = dirs.begin(); j != j_end; ++j) {
				unit_map::const_iterator unit_itor = units.find(adjacent[*j]);
				if (unit_itor == units.end()
				|| !unit_filter::matches_filter(*i, *unit_itor, unit_itor->get_location(), fc, use_flat_tod)) {
					continue;
				}
				config::attribute_value i_is_enemy = (*i)["is_enemy"];
				if (i_is_enemy.blank() || i_is_enemy.to_bool() ==
				    fc->get_disp_context().teams()[u.side() - 1].is_enemy(unit_itor->side())) {
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

	config::attribute_value cfg_find_in = cfg["find_in"];
	if (!cfg_find_in.blank()) {
		// Allow filtering by searching a stored variable of units
		variable_info vi(cfg_find_in, false, variable_info::TYPE_CONTAINER);
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
	config::attribute_value cfg_formula = cfg["formula"];
	if (!cfg_formula.blank()) {
		if (!u.formula_manager().matches_filter(cfg_formula, loc, u)) {
			return false;
		}
	}

	config::attribute_value cfg_lua_function = cfg["lua_function"];
	if (!cfg_lua_function.blank()) {
		bool b = resources::lua_kernel->run_filter(cfg_lua_function.str().c_str(), u);
		if (!b) return false;
	}

	return true;
}

} //end anonymous namespace
