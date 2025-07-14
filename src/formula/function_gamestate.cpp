/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "actions/attack.hpp"
#include "filesystem.hpp"
#include "game_board.hpp"
#include "map/label.hpp"
#include "map/map.hpp"
#include "pathutils.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "tod_manager.hpp"
#include "units/types.hpp"
#include "units/unit.hpp"

#include <utility>

namespace wfl {

namespace gamestate {

DEFINE_WFL_FUNCTION(run_file, 1, 1)
{
	variant var = args()[0]->evaluate(variables, add_debug_info(fdb, 0, "run_file:file"));

	// NOTE: get_wml_location also filters file path to ensure it doesn't contain things like "../../top/secret"
	auto path = filesystem::get_wml_location(var.as_string());
	if(!path) {
		return variant();
	}

	std::string formula_string = filesystem::read_file(path.value());
	gamestate_function_symbol_table symbols;

	auto parsed_formula = formula::create_optional_formula(formula_string, &symbols);
	return formula::evaluate(parsed_formula, variables, add_debug_info(fdb, -1, "run_file:formula_from_file"));
}

DEFINE_WFL_FUNCTION(debug_label, 2, 2)
{
	variant var0 = args()[0]->evaluate(variables, fdb);
	variant var1 = args()[1]->evaluate(variables, fdb);

	if(game_config::debug) {
		const team& team = resources::controller->current_team();
		const map_location loc = var0.convert_to<location_callable>()->loc();
		const std::string text = var1.is_string() ? var1.as_string() : var1.to_debug_string();

		display::get_singleton()->labels().set_label(
			loc, text, team.side(), team.team_name(), team::get_side_color(team.side()));
	}

	return variant(std::vector{var0, var1});
}

DEFINE_WFL_FUNCTION(adjacent_locs, 1, 1)
{
	const map_location loc = args()[0]->evaluate(variables, add_debug_info(fdb, 0, "adjacent_locs:location")).convert_to<location_callable>()->loc();

	std::vector<variant> v;
	for(const map_location& adj : get_adjacent_tiles(loc)) {
		if(resources::gameboard->map().on_board(adj)) {
			v.emplace_back(std::make_shared<location_callable>(adj));
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

	for(std::size_t n = 0; n != res.size(); ++n) {
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

	const auto& tdata = resources::gameboard->map().tdata();
	t_translation::terrain_code ter;

	if(loc_var.is_string()) {
		ter = t_translation::read_terrain_code(loc_var.as_string());
	} else if(auto tc = loc_var.try_convert<terrain_callable>()) {
		const std::string id = tc->get_value("id").as_string();
		auto iter = std::find_if(tdata->map().begin(), tdata->map().end(), [id](const std::pair<t_translation::terrain_code, terrain_type>& p) {
			return id == p.second.id();
		});
		if(iter == tdata->map().end()) {
			return variant();
		}
		ter = iter->first;
	} else if(auto loc = loc_var.try_convert<location_callable>()) {
		if(!resources::gameboard->map().on_board(loc->loc())) {
			return variant();
		}
		ter = resources::gameboard->map().get_terrain(loc->loc());
	} else {
		return variant();
	}

	if(u_call) {
		const unit& un = u_call->get_unit();

		return variant(100 - un.defense_modifier(ter));
	}

	if(u_type) {
		const unit_type& un = u_type->get_unit_type();

		return variant(100 - un.movement_type().defense_modifier(ter));
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

	const auto& tdata = resources::gameboard->map().tdata();
	t_translation::terrain_code ter;

	if(loc_var.is_string()) {
		ter = t_translation::read_terrain_code(loc_var.as_string());
	} else if(auto tc = loc_var.try_convert<terrain_callable>()) {
		const std::string id = tc->get_value("id").as_string();
		auto iter = std::find_if(tdata->map().begin(), tdata->map().end(), [id](const std::pair<t_translation::terrain_code, terrain_type>& p) {
			return id == p.second.id();
		});
		if(iter == tdata->map().end()) {
			return variant();
		}
		ter = iter->first;
	} else if(auto loc = loc_var.try_convert<location_callable>()) {
		if(!resources::gameboard->map().on_board(loc->loc())) {
			return variant();
		}
		ter = resources::gameboard->map().get_terrain(loc->loc());
	} else {
		return variant();
	}

	if(u_call) {
		const unit& un = u_call->get_unit();

		return variant(un.defense_modifier((ter)));
	}

	if(u_type) {
		const unit_type& un = u_type->get_unit_type();

		return variant(un.movement_type().defense_modifier((ter)));
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

	const auto& tdata = resources::gameboard->map().tdata();
	t_translation::terrain_code ter;

	if(loc_var.is_string()) {
		ter = t_translation::read_terrain_code(loc_var.as_string());
	} else if(auto tc = loc_var.try_convert<terrain_callable>()) {
		const std::string id = tc->get_value("id").as_string();
		auto iter = std::find_if(tdata->map().begin(), tdata->map().end(), [id](const std::pair<t_translation::terrain_code, terrain_type>& p) {
			return id == p.second.id();
		});
		if(iter == tdata->map().end()) {
			return variant();
		}
		ter = iter->first;
	} else if(auto loc = loc_var.try_convert<location_callable>()) {
		if(!resources::gameboard->map().on_board(loc->loc())) {
			return variant();
		}
		ter = resources::gameboard->map().get_terrain(loc->loc());
	} else {
		return variant();
	}

	if(u_call) {
		const unit& un = u_call->get_unit();

		return variant(un.movement_cost(ter));
	}

	if(u_type) {
		const unit_type& un = u_type->get_unit_type();

		return variant(un.movement_type().movement_cost(ter));
	}

	return variant();
}

DEFINE_WFL_FUNCTION(vision_cost, 2, 2)
{
	variant u = args()[0]->evaluate(variables, add_debug_info(fdb, 0, "vision_cost:unit"));
	variant loc_var = args()[1]->evaluate(variables, add_debug_info(fdb, 0, "vision_cost:location"));
	if(u.is_null() || loc_var.is_null()) {
		return variant();
	}
	//we can pass to this function either unit_callable or unit_type callable
	auto u_call = u.try_convert<unit_callable>();
	auto u_type = u.try_convert<unit_type_callable>();

	const auto& tdata = resources::gameboard->map().tdata();
	t_translation::terrain_code ter;

	if(loc_var.is_string()) {
		ter = t_translation::read_terrain_code(loc_var.as_string());
	} else if(auto tc = loc_var.try_convert<terrain_callable>()) {
		const std::string id = tc->get_value("id").as_string();
		auto iter = std::find_if(tdata->map().begin(), tdata->map().end(), [id](const std::pair<t_translation::terrain_code, terrain_type>& p) {
			return id == p.second.id();
		});
		if(iter == tdata->map().end()) {
			return variant();
		}
		ter = iter->first;
	} else if(auto loc = loc_var.try_convert<location_callable>()) {
		if(!resources::gameboard->map().on_board(loc->loc())) {
			return variant();
		}
		ter = resources::gameboard->map().get_terrain(loc->loc());
	} else {
		return variant();
	}

	if(u_call) {
		const unit& un = u_call->get_unit();

		return variant(un.vision_cost(ter));
	}

	if(u_type) {
		const unit_type& un = u_type->get_unit_type();

		return variant(un.movement_type().vision_cost(ter));
	}

	return variant();
}

DEFINE_WFL_FUNCTION(jamming_cost, 2, 2)
{
	variant u = args()[0]->evaluate(variables, add_debug_info(fdb, 0, "jamming_cost:unit"));
	variant loc_var = args()[1]->evaluate(variables, add_debug_info(fdb, 0, "jamming_cost:location"));
	if(u.is_null() || loc_var.is_null()) {
		return variant();
	}
	//we can pass to this function either unit_callable or unit_type callable
	auto u_call = u.try_convert<unit_callable>();
	auto u_type = u.try_convert<unit_type_callable>();

	const auto& tdata = resources::gameboard->map().tdata();
	t_translation::terrain_code ter;

	if(loc_var.is_string()) {
		ter = t_translation::read_terrain_code(loc_var.as_string());
	} else if(auto tc = loc_var.try_convert<terrain_callable>()) {
		const std::string id = tc->get_value("id").as_string();
		auto iter = std::find_if(tdata->map().begin(), tdata->map().end(), [id](const std::pair<t_translation::terrain_code, terrain_type>& p) {
			return id == p.second.id();
		});
		if(iter == tdata->map().end()) {
			return variant();
		}
		ter = iter->first;
	} else if(auto loc = loc_var.try_convert<location_callable>()) {
		if(!resources::gameboard->map().on_board(loc->loc())) {
			return variant();
		}
		ter = resources::gameboard->map().get_terrain(loc->loc());
	} else {
		return variant();
	}

	if(u_call) {
		const unit& un = u_call->get_unit();

		return variant(un.jamming_cost(ter));
	}

	if(u_type) {
		const unit_type& un = u_type->get_unit_type();

		return variant(un.movement_type().jamming_cost(ter));
	}

	return variant();
}

DEFINE_WFL_FUNCTION(enemy_of, 2, 2)
{
	variant self_v = args()[0]->evaluate(variables, add_debug_info(fdb, 0, "enemy_of:self"));
	variant other_v = args()[1]->evaluate(variables, add_debug_info(fdb, 1, "enemy_of:other"));
	int self, other;

	if(auto uc = self_v.try_convert<unit_callable>()) {
		self = uc->get_value("side_number").as_int();
	} else if(auto tc = self_v.try_convert<team_callable>()) {
		self = tc->get_value("side_number").as_int();
	} else {
		self = self_v.as_int();
	}

	if(auto uc = other_v.try_convert<unit_callable>()) {
		other = uc->get_value("side_number").as_int();
	} else if(auto tc = other_v.try_convert<team_callable>()) {
		other = tc->get_value("side_number").as_int();
	} else {
		other = other_v.as_int();
	}

	int num_teams = resources::gameboard->teams().size();
	if(self < 1 || self > num_teams || other < 1 || other > num_teams) {
		return variant(0);
	}
	return variant(resources::gameboard->get_team(self).is_enemy(other) ? 1 : 0);
}

DEFINE_WFL_FUNCTION(resistance_on, 3, 4)
{
	variant u = args()[0]->evaluate(variables, add_debug_info(fdb, 0, "resistance_on:unit"));
	variant loc_var = args()[1]->evaluate(variables, add_debug_info(fdb, 1, "resistance_on:location"));
	if(u.is_null() || loc_var.is_null()) {
		return variant();
	}
	std::string type = args()[2]->evaluate(variables, add_debug_info(fdb, 2, "resistance_on:type")).as_string();
	bool attacker = args().size() > 3 ? args()[3]->evaluate(variables, add_debug_info(fdb, 3, "resistance_on:attacker")).as_bool() : false;
	const map_location& loc = loc_var.convert_to<location_callable>()->loc();

	if(auto u_call = u.try_convert<unit_callable>()) {
		const unit& un = u_call->get_unit();

		return variant(100 - un.resistance_against(type, attacker, loc));
	}

	return variant();
}

DEFINE_WFL_FUNCTION(tod_bonus, 0, 2)
{
	map_location loc;
	int turn = resources::controller->turn();
	if(args().size() > 0) {
		variant loc_arg = args()[0]->evaluate(variables, add_debug_info(fdb, 0, "tod_bonus:loc"));
		if(auto p = loc_arg.try_convert<location_callable>()) {
			loc = p->loc();
		} else return variant();

		if(args().size() > 1) {
			variant turn_arg = args()[1]->evaluate(variables, add_debug_info(fdb, 0, "tod_bonus:turn"));
			if(turn_arg.is_int()) {
				turn = turn_arg.as_int();
			} else if(!turn_arg.is_null()) {
				return variant();
			}
		}
	}
	int bonus = resources::tod_manager->get_illuminated_time_of_day(resources::gameboard->units(), resources::gameboard->map(), loc, turn).lawful_bonus;
	return variant(bonus);
}

DEFINE_WFL_FUNCTION(base_tod_bonus, 0, 2)
{
	map_location loc;
	int turn = resources::controller->turn();
	if(args().size() > 0) {
		variant loc_arg = args()[0]->evaluate(variables, add_debug_info(fdb, 0, "tod_bonus:loc"));
		if(auto p = loc_arg.try_convert<location_callable>()) {
			loc = p->loc();
		} else return variant();

		if(args().size() > 1) {
			variant turn_arg = args()[1]->evaluate(variables, add_debug_info(fdb, 0, "tod_bonus:turn"));
			if(turn_arg.is_int()) {
				turn = turn_arg.as_int();
			} else if(!turn_arg.is_null()) {
				return variant();
			}
		}
	}
	int bonus = resources::tod_manager->get_time_of_day(loc, turn).lawful_bonus;
	return variant(bonus);
}

DEFINE_WFL_FUNCTION(unit_tod_modifier, 1, 2)
{
	const unit& unit = args()[0]
		->evaluate(variables, add_debug_info(fdb, 0, "unit_tod_modifier:unit"))
		.convert_to<unit_callable>()
		->get_unit();

	const map_location loc = args().size() == 2
		? args()[1]
			->evaluate(variables, add_debug_info(fdb, 1, "unit_tod_modifier:location"))
			.convert_to<location_callable>()
			->loc()
		: unit.get_location();

	return variant(combat_modifier(
		resources::gameboard->units(), resources::gameboard->map(), loc, unit.alignment(), unit.is_fearless()));
}

DEFINE_WFL_FUNCTION(is_shrouded, 2, 2)
{
	variant var0 = args()[0]->evaluate(variables, fdb);
	variant var1 = args()[1]->evaluate(variables, fdb);

	try {
		const map_location loc = var0.convert_to<location_callable>()->loc();
		return variant(resources::gameboard->get_team(var1.as_int()).shrouded(loc));

	} catch(const std::out_of_range&) {
		return variant();
	}
}

DEFINE_WFL_FUNCTION(is_fogged, 2, 2)
{
	variant var0 = args()[0]->evaluate(variables, fdb);
	variant var1 = args()[1]->evaluate(variables, fdb);

	try {
		const map_location loc = var0.convert_to<location_callable>()->loc();
		return variant(resources::gameboard->get_team(var1.as_int()).fogged(loc));

	} catch(const std::out_of_range&) {
		return variant();
	}
}

} // namespace gamestate

gamestate_function_symbol_table::gamestate_function_symbol_table(const std::shared_ptr<function_symbol_table>& parent) : function_symbol_table(parent) {
	using namespace gamestate;
	function_symbol_table& functions_table = *this;
	DECLARE_WFL_FUNCTION(run_file);
	DECLARE_WFL_FUNCTION(debug_label);
	DECLARE_WFL_FUNCTION(get_unit_type);
	DECLARE_WFL_FUNCTION(unit_at);
	DECLARE_WFL_FUNCTION(resistance_on);
	DECLARE_WFL_FUNCTION(defense_on);
	DECLARE_WFL_FUNCTION(chance_to_hit);
	DECLARE_WFL_FUNCTION(movement_cost);
	DECLARE_WFL_FUNCTION(vision_cost);
	DECLARE_WFL_FUNCTION(jamming_cost);
	DECLARE_WFL_FUNCTION(adjacent_locs); // This is deliberately duplicated here; this form excludes off-map locations, while the core form does not
	DECLARE_WFL_FUNCTION(locations_in_radius);
	DECLARE_WFL_FUNCTION(enemy_of);
	DECLARE_WFL_FUNCTION(tod_bonus);
	DECLARE_WFL_FUNCTION(base_tod_bonus);
	DECLARE_WFL_FUNCTION(unit_tod_modifier);
	DECLARE_WFL_FUNCTION(is_shrouded);
	DECLARE_WFL_FUNCTION(is_fogged);
}

}
