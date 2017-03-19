/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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
 * Maintain game variables + misc.
 */

#include "game_data.hpp"

#include "log.hpp" //LOG_STREAM
#include "variable.hpp" //scoped_wml_variable

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

game_data::game_data(const config& level)
	: variable_set()
	, scoped_variables()
	, last_selected(map_location::null_location())
	, rng_(level)
	, variables_(level.child_or_empty("variables"))
	, phase_(INITIAL)
	, can_end_turn_(level["can_end_turn"].to_bool(true))
	, next_scenario_(level["next_scenario"])
{
}

game_data::game_data(const game_data& data)
	: variable_set() // variable set is just an interface.
	, scoped_variables(data.scoped_variables)
	, last_selected(data.last_selected)
	, rng_(data.rng_)
	, variables_(data.variables_)
	, phase_(data.phase_)
	, can_end_turn_(data.can_end_turn_)
	, next_scenario_(data.next_scenario_)
{
}
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
	std::vector<config> temp = {value};
	return get_variable_access_write(key).append_array(temp).front();
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

void game_data::write_snapshot(config& cfg) const
{
	cfg["next_scenario"] = next_scenario_;

	cfg["can_end_turn"] = can_end_turn_;

	cfg["random_seed"] = rng_.get_random_seed_str();
	cfg["random_calls"] = rng_.get_random_calls();

	cfg.add_child("variables", variables_);

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
