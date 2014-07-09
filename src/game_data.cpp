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

#include "game_data.hpp"

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

#include <boost/assign.hpp>
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
		, phase_(data.phase_)
		, can_end_turn_(data.can_end_turn_)
		, scenario_(data.scenario_)
		, next_scenario_(data.next_scenario_)
{}
//throws
config::attribute_value &game_data::get_variable(const std::string& key)
{
	return get_variable_access_write(key).as_scalar();
}

config::attribute_value game_data::get_variable_const(const std::string &key) const
{
	try
	{
		return get_variable_access_read(key).as_scalar();
	}
	catch(const invalid_variablename_exception&)
	{
		return config::attribute_value();
	}
}
//throws
config& game_data::get_variable_cfg(const std::string& key)
{
	return get_variable_access_write(key).as_container();
}

void game_data::set_variable(const std::string& key, const t_string& value)
{
	try
	{
		get_variable(key) = value;
	}
	catch(const invalid_variablename_exception&)
	{
		ERR_NG << "variable " << key << "cannot be set to " << value << std::endl;
	}
}
//throws
config& game_data::add_variable_cfg(const std::string& key, const config& value)
{
	std::vector<config> temp = boost::assign::list_of(value);
	return *get_variable_access_write(key).append_array(temp).first;
}

void game_data::clear_variable_cfg(const std::string& varname)
{
	try
	{
		get_variable_access_throw(varname).clear(true);
	}
	catch(const invalid_variablename_exception&)
	{
		//variable doesn't exist, nothing to delete
	}
}

void game_data::clear_variable(const std::string& varname)
{
	try
	{
		get_variable_access_throw(varname).clear(false);
	}
	catch(const invalid_variablename_exception&)
	{
		//variable doesn't exist, nothing to delete
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
					 std::vector<team>& teams,
					 const config& level, gamemap& map)
{
	return team_builder_ptr(new team_builder(side_cfg, teams, level, map, *this));
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

namespace {
	bool recursive_activation = false;

} // end anonymous namespace

void game_data::activate_scope_variable(std::string var_name) const
{
	
	if(recursive_activation)
		return;
	const std::string::iterator itor = std::find(var_name.begin(),var_name.end(),'.');
	if(itor != var_name.end()) {
		var_name.erase(itor, var_name.end());
	}
	std::vector<scoped_wml_variable*>::const_reverse_iterator rit;
	for(
		rit = scoped_variables.rbegin(); 
		rit != scoped_variables.rend(); 
	++rit) {
		if((**rit).name() == var_name) {
			recursive_activation = true;
			if(!(**rit).activated()) {
				(**rit).activate();
			}
			recursive_activation = false;
			break;
		}
	}
}
