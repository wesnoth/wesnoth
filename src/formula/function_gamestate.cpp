/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "formula/function_gamestate.hpp"
#include "formula/callable_objects.hpp"

#include "resources.hpp"
#include "game_board.hpp"
#include "map/map.hpp"
#include "pathutils.hpp"
#include "units/types.hpp"
#include "units/unit.hpp"

namespace wfl {

namespace gamestate {

DEFINE_WFL_FUNCTION(adjacent_locs, 1, 1)
{
	const map_location loc = args()[0]->evaluate(variables, add_debug_info(fdb, 0, "adjacent_locs:location")).convert_to<location_callable>()->loc();
	map_location adj[6];
	get_adjacent_tiles(loc, adj);

	std::vector<variant> v;
	for(int n = 0; n != 6; ++n) {
		if(resources::gameboard->map().on_board(adj[n])) {
			v.emplace_back(std::make_shared<location_callable>(adj[n]));
		}
	}

	return variant(v);
}

DEFINE_WFL_FUNCTION(locations_in_radius, 2, 2)
{
	const map_location loc = args()[0]->evaluate(variables, fdb).convert_to<location_callable>()->loc();

	int range = args()[1]->evaluate(variables, fdb).as_int();

	if(range < 0) {
		return variant();
	}

	if(!range) {
		return variant(std::make_shared<location_callable>(loc));
	}

	std::vector<map_location> res;

	get_tiles_in_radius(loc, range, res);

	std::vector<variant> v;
	v.reserve(res.size() + 1);
	v.emplace_back(std::make_shared<location_callable>(loc));

	for(size_t n = 0; n != res.size(); ++n) {
		if(resources::gameboard->map().on_board(res[n])) {
			v.emplace_back(std::make_shared<location_callable>(res[n]));
		}
	}

	return variant(v);
}

DEFINE_WFL_FUNCTION(get_unit_type, 1, 1)
{
	const std::string type = args()[0]->evaluate(variables, add_debug_info(fdb, 0, "get_unit_type:name")).as_string();

	const unit_type *ut = unit_types.find(type);
	if(ut) {
		return variant(std::make_shared<unit_type_callable>(*ut));
	}

	return variant();
}

DEFINE_WFL_FUNCTION(unit_at, 1, 1)
{
	variant loc_var = args()[0]->evaluate(variables, add_debug_info(fdb, 0, "unit_at:location"));
	if(loc_var.is_null()) {
		return variant();
	}
	auto loc = loc_var.convert_to<location_callable>();
	const unit_map::const_iterator i = resources::gameboard->units().find(loc->loc());
	if(i != resources::gameboard->units().end()) {
		return variant(std::make_shared<unit_callable>(*i));
	} else {
		return variant();
	}
}

DEFINE_WFL_FUNCTION(defense_on, 2, 2)
{
	variant u = args()[0]->evaluate(variables, add_debug_info(fdb, 0, "defense_on:unit"));
	variant loc_var = args()[1]->evaluate(variables, add_debug_info(fdb, 1, "defense_on:location"));
	if(u.is_null() || loc_var.is_null()) {
		return variant();
	}

	auto u_call = u.try_convert<unit_callable>();
	auto u_type = u.try_convert<unit_type_callable>();
	const map_location& loc = loc_var.convert_to<location_callable>()->loc();

	if(u_call) {
		const unit& un = u_call->get_unit();

		if(un.total_movement() < un.movement_cost((resources::gameboard->map())[loc]))
			return variant();

		if(!resources::gameboard->map().on_board(loc)) {
			return variant();
		}

		return variant(100 - un.defense_modifier((resources::gameboard->map())[loc]));
	}

	if(u_type) {
		const unit_type& un = u_type->get_unit_type();

		if(un.movement() < un.movement_type().movement_cost((resources::gameboard->map())[loc]))
			return variant();

		if(!resources::gameboard->map().on_board(loc)) {
			return variant();
		}

		return variant(100 - un.movement_type().defense_modifier((resources::gameboard->map())[loc]));
	}

	return variant();
}

DEFINE_WFL_FUNCTION(chance_to_hit, 2, 2)
{
	variant u = args()[0]->evaluate(variables, add_debug_info(fdb, 0, "chance_to_hit:unit"));
	variant loc_var = args()[1]->evaluate(variables, add_debug_info(fdb, 1, "chance_to_hit:location"));
	if(u.is_null() || loc_var.is_null()) {
		return variant();
	}

	auto u_call = u.try_convert<unit_callable>();
	auto u_type = u.try_convert<unit_type_callable>();
	const map_location& loc = loc_var.convert_to<location_callable>()->loc();

	if(u_call) {
		const unit& un = u_call->get_unit();

		if(!resources::gameboard->map().on_board(loc)) {
			return variant();
		}

		return variant(un.defense_modifier((resources::gameboard->map())[loc]));
	}

	if(u_type) {
		const unit_type& un = u_type->get_unit_type();

		if(!resources::gameboard->map().on_board(loc)) {
			return variant();
		}

		return variant(un.movement_type().defense_modifier((resources::gameboard->map())[loc]));
	}

	return variant();
}

DEFINE_WFL_FUNCTION(movement_cost, 2, 2)
{
	variant u = args()[0]->evaluate(variables, add_debug_info(fdb, 0, "movement_cost:unit"));
	variant loc_var = args()[1]->evaluate(variables, add_debug_info(fdb, 0, "movement_cost:location"));
	if(u.is_null() || loc_var.is_null()) {
		return variant();
	}
	//we can pass to this function either unit_callable or unit_type callable
	auto u_call = u.try_convert<unit_callable>();
	auto u_type = u.try_convert<unit_type_callable>();
	const map_location& loc = loc_var.convert_to<location_callable>()->loc();

	if(u_call) {
		const unit& un = u_call->get_unit();

		if(!resources::gameboard->map().on_board(loc)) {
			return variant();
		}

		return variant(un.movement_cost((resources::gameboard->map())[loc]));
	}

	if(u_type) {
		const unit_type& un = u_type->get_unit_type();

		if(!resources::gameboard->map().on_board(loc)) {
			return variant();
		}

		return variant(un.movement_type().movement_cost((resources::gameboard->map())[loc]));
	}

	return variant();
}

DEFINE_WFL_FUNCTION(enemy_of, 2, 2)
{
	variant self_v = args()[0]->evaluate(variables, add_debug_info(fdb, 0, "enemy_of:self"));
	variant other_v = args()[1]->evaluate(variables, add_debug_info(fdb, 1, "enemy_of:other"));
	int self, other;

	if(auto uc = self_v.try_convert<unit_callable>()) {
		// For some obscure, bizarre reason, the unit callable returns a 0-indexed side. :|
		self = uc->get_value("side").as_int() + 1;
	} else if(auto tc = self_v.try_convert<team_callable>()) {
		self = tc->get_value("side").as_int();
	} else {
		self = self_v.as_int();
	}

	if(auto uc = other_v.try_convert<unit_callable>()) {
		// For some obscure, bizarre reason, the unit callable returns a 0-indexed side. :|
		other = uc->get_value("side").as_int() + 1;
	} else if(auto tc = other_v.try_convert<team_callable>()) {
		other = tc->get_value("side").as_int();
	} else {
		other = other_v.as_int();
	}

	int num_teams = resources::gameboard->teams().size();
	if(self < 1 || self > num_teams || other < 1 || other > num_teams) {
		return variant(0);
	}
	return variant(resources::gameboard->get_team(self).is_enemy(other) ? 1 : 0);
}

} // namespace gamestate

gamestate_function_symbol_table::gamestate_function_symbol_table(std::shared_ptr<function_symbol_table> parent) : function_symbol_table(parent) {
	using namespace gamestate;
	function_symbol_table& functions_table = *this;
	DECLARE_WFL_FUNCTION(get_unit_type);
	DECLARE_WFL_FUNCTION(unit_at);
	DECLARE_WFL_FUNCTION(defense_on);
	DECLARE_WFL_FUNCTION(chance_to_hit);
	DECLARE_WFL_FUNCTION(movement_cost);
	DECLARE_WFL_FUNCTION(adjacent_locs); // This is deliberately duplicated here; this form excludes off-map locations, while the core form does not
	DECLARE_WFL_FUNCTION(locations_in_radius);
	DECLARE_WFL_FUNCTION(enemy_of);
}

}
