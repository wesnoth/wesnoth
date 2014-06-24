/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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
 * Maintain status of a game, load&save games.
 */

#include "global.hpp"

#include "gamestatus.hpp"

#include "carryover.hpp"
#include "filesystem.hpp"
#include "formula_string_utils.hpp"
#include "game_config.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map.hpp"
#include "recall_list_manager.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "serialization/binary_or_text.hpp"
#include "statistics.hpp"
#include "team.hpp"
#include "teambuilder.hpp"
#include "unit.hpp"
#include "unit_id.hpp"
#include "wesconfig.h"
#include "wml_exception.hpp"
#include "variable.hpp"
#include "pathfind/pathfind.hpp"
#include "whiteboard/side_actions.hpp"
#include "sound.hpp"
#include "soundsource.hpp"
#include "map_label.hpp"
#include "unit.hpp"
#include "unit_map.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#ifndef _MSC_VER
#include <sys/time.h>
#endif

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)


/// The default difficulty setting for campaigns.
const std::string DEFAULT_DIFFICULTY("NORMAL");

game_data::game_data()
		: scoped_variables()
		, last_selected(map_location::null_location())
		, wml_menu_items_()
		, rng_()
		, variables_()
		, temporaries_()
		, phase_(INITIAL)
		, can_end_turn_(true)
		, scenario_()
		, next_scenario_()
		{}

game_data::game_data(const config& level)
		: scoped_variables()
		, last_selected(map_location::null_location())
		, wml_menu_items_()
		, rng_(level)
		, variables_(level.child_or_empty("variables"))
		, temporaries_()
		, phase_(INITIAL)
		, can_end_turn_(level["can_end_turn"].to_bool(true))
		, scenario_(level["id"])
		, next_scenario_(level["next_scenario"])
{
	wml_menu_items_.set_menu_items(level);
}

game_data::game_data(const game_data& data)
		: variable_set() // Not sure why empty, copied from old code
		, scoped_variables(data.scoped_variables)
		, last_selected(data.last_selected)
		, wml_menu_items_(data.wml_menu_items_)
		, rng_(data.rng_)
		, variables_(data.variables_)
		, temporaries_()
		, phase_(data.phase_)
		, can_end_turn_(data.can_end_turn_)
		, scenario_(data.scenario_)
		, next_scenario_(data.next_scenario_)
{}

config::attribute_value &game_data::get_variable(const std::string& key)
{
	return variable_info(key, true, variable_info::TYPE_SCALAR).as_scalar();
}

config::attribute_value game_data::get_variable_const(const std::string &key) const
{
	variable_info to_get(key, false, variable_info::TYPE_SCALAR);
	if (!to_get.is_valid)
	{
		config::attribute_value &to_return = temporaries_[key];
		if (key.size() > 7 && key.substr(key.size() - 7) == ".length") {
			// length is a special attribute, so guarantee its correctness
			to_return = 0;
		}
		return to_return;
	}
	return to_get.as_scalar();
}

config& game_data::get_variable_cfg(const std::string& key)
{
	return variable_info(key, true, variable_info::TYPE_CONTAINER).as_container();
}

void game_data::set_variable(const std::string& key, const t_string& value)
{
	get_variable(key) = value;
}

config& game_data::add_variable_cfg(const std::string& key, const config& value)
{
	variable_info to_add(key, true, variable_info::TYPE_ARRAY);
	return to_add.vars->add_child(to_add.key, value);
}

void game_data::clear_variable_cfg(const std::string& varname)
{
	variable_info to_clear(varname, false, variable_info::TYPE_CONTAINER);
	if(!to_clear.is_valid) return;
	if(to_clear.explicit_index) {
		to_clear.vars->remove_child(to_clear.key, to_clear.index);
	} else {
		to_clear.vars->clear_children(to_clear.key);
	}
}

void game_data::clear_variable(const std::string& varname)
{
	variable_info to_clear(varname, false);
	if(!to_clear.is_valid) return;
	if(to_clear.explicit_index) {
		to_clear.vars->remove_child(to_clear.key, to_clear.index);
	} else {
		to_clear.vars->clear_children(to_clear.key);
		to_clear.vars->remove_attribute(to_clear.key);
	}
}

void game_data::write_snapshot(config& cfg) const {
	cfg["scenario"] = scenario_;
	cfg["next_scenario"] = next_scenario_;

	cfg["can_end_turn"] = can_end_turn_;

	cfg["random_seed"] = rng_.get_random_seed();
	cfg["random_calls"] = rng_.get_random_calls();

	cfg.add_child("variables", variables_);

	wml_menu_items_.to_config(cfg);
}

void game_data::write_config(config_writer& out){
	out.write_key_val("scenario", scenario_);
	out.write_key_val("next_scenario", next_scenario_);

	out.write_key_val("random_seed", lexical_cast<std::string>(rng_.get_random_seed()));
	out.write_key_val("random_calls", lexical_cast<std::string>(rng_.get_random_calls()));
	out.write_child("variables", variables_);

	config cfg;
	wml_menu_items_.to_config(cfg);
	out.write_child("menu_item", cfg);
}

team_builder_ptr game_data::create_team_builder(const config& side_cfg,
					 std::string save_id, std::vector<team>& teams,
					 const config& level, gamemap& map, unit_map& units,
					 const config& starting_pos)
{
	return team_builder_ptr(new team_builder(side_cfg, save_id, teams, level, map, units, starting_pos));
}

void game_data::build_team_stage_one(team_builder_ptr tb_ptr)
{
	tb_ptr->build_team_stage_one();
}

void game_data::build_team_stage_two(team_builder_ptr tb_ptr)
{
	tb_ptr->build_team_stage_two();
}

game_data& game_data::operator=(const game_data& info)
{
	// Use copy constructor to make sure we are coherent
	if (this != &info) {
		this->~game_data();
		new (this) game_data(info) ;
	}
	return *this ;
}

game_data* game_data::operator=(const game_data* info)
{
	// Use copy constructor to make sure we are coherent
	if (this != info) {
		this->~game_data();
		new (this) game_data(*info) ;
	}
	return this ;
}
