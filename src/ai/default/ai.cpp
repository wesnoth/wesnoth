/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Artificial intelligence - The computer commands the enemy.
 */

#include "ai.hpp"

#include "ai/actions.hpp"
#include "ai/manager.hpp"
#include "ai/formula/ai.hpp"

#include "array.hpp"
#include "game_board.hpp"
#include "game_events/manager.hpp"
#include "game_events/pump.hpp"
#include "game_classification.hpp"
#include "log.hpp"
#include "mouse_handler_base.hpp"
#include "recall_list_manager.hpp"
#include "resources.hpp"
#include "terrain/filter.hpp"
#include "units/unit.hpp"
#include "units/udisplay.hpp"
#include "wml_exception.hpp"

#include "pathfind/pathfind.hpp"

#include <boost/foreach.hpp>

#include <iterator>
#include <algorithm>
#include <fstream>

static lg::log_domain log_ai("ai/general");
#define DBG_AI LOG_STREAM(debug, log_ai)
#define LOG_AI LOG_STREAM(info, log_ai)
#define WRN_AI LOG_STREAM(warn, log_ai)
#define ERR_AI LOG_STREAM(err, log_ai)

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

idle_ai::idle_ai(readwrite_context &context, const config& /*cfg*/)
	: recursion_counter_(context.get_recursion_count())
{
	init_readwrite_context_proxy(context);
}

std::string idle_ai::describe_self() const
{
	return "[idle_ai]";
}


void idle_ai::new_turn()
{
}


void idle_ai::switch_side(side_number side)
{
	set_side(side);
}


config idle_ai::to_config() const
{
	return config();
}


int idle_ai::get_recursion_count() const
{
	return recursion_counter_.get_count();
}


void idle_ai::play_turn()
{
	resources::game_events->pump().fire("ai turn");
}


#ifdef _MSC_VER
#pragma warning(pop)
#endif

variant attack_analysis::get_value(const std::string& key) const
{
	using namespace game_logic;
	if(key == "target") {
		return variant(new location_callable(target));
	} else if(key == "movements") {
		std::vector<variant> res;
		for(size_t n = 0; n != movements.size(); ++n) {
			map_formula_callable* item = new map_formula_callable(NULL);
			item->add("src", variant(new location_callable(movements[n].first)));
			item->add("dst", variant(new location_callable(movements[n].second)));
			res.push_back(variant(item));
		}

		return variant(&res);
	} else if(key == "units") {
		std::vector<variant> res;
		for(size_t n = 0; n != movements.size(); ++n) {
			res.push_back(variant(new location_callable(movements[n].first)));
		}

		return variant(&res);
	} else if(key == "target_value") {
		return variant(static_cast<int>(target_value*1000));
	} else if(key == "avg_losses") {
		return variant(static_cast<int>(avg_losses*1000));
	} else if(key == "chance_to_kill") {
		return variant(static_cast<int>(chance_to_kill*100));
	} else if(key == "avg_damage_inflicted") {
		return variant(static_cast<int>(avg_damage_inflicted));
	} else if(key == "target_starting_damage") {
		return variant(target_starting_damage);
	} else if(key == "avg_damage_taken") {
		return variant(static_cast<int>(avg_damage_taken));
	} else if(key == "resources_used") {
		return variant(static_cast<int>(resources_used));
	} else if(key == "terrain_quality") {
		return variant(static_cast<int>(terrain_quality));
	} else if(key == "alternative_terrain_quality") {
		return variant(static_cast<int>(alternative_terrain_quality));
	} else if(key == "vulnerability") {
		return variant(static_cast<int>(vulnerability));
	} else if(key == "support") {
		return variant(static_cast<int>(support));
	} else if(key == "leader_threat") {
		return variant(leader_threat);
	} else if(key == "uses_leader") {
		return variant(uses_leader);
	} else if(key == "is_surrounded") {
		return variant(is_surrounded);
	} else {
		return variant();
	}
}

void attack_analysis::get_inputs(std::vector<game_logic::formula_input>* inputs) const
{
	using namespace game_logic;
	inputs->push_back(formula_input("target", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("movements", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("units", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("target_value", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("avg_losses", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("chance_to_kill", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("avg_damage_inflicted", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("target_starting_damage", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("avg_damage_taken", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("resources_used", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("terrain_quality", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("alternative_terrain_quality", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("vulnerability", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("support", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("leader_threat", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("uses_leader", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("is_surrounded", FORMULA_READ_ONLY));
}

} //end of namespace ai

