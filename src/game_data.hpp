/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef GAME_DATA_HPP_INCLUDED
#define GAME_DATA_HPP_INCLUDED

#include "config.hpp"
#include "game_end_exceptions.hpp"
#include "map_location.hpp"
#include "mt_rng.hpp"
#include "variable_info.hpp"

class scoped_wml_variable;
class t_string;

class game_data  : public variable_set  {
public:
	game_data();
	explicit game_data(const config& level);
	game_data(const game_data& data);

	std::vector<scoped_wml_variable*> scoped_variables;

	const config& get_variables() const { return variables_; }
	/// throws invalid_variablename_exception if varname is no valid variable name.
	config::attribute_value &get_variable(const std::string &varname);
	/// returns a blank attribute value if varname is no valid variable name.
	virtual config::attribute_value get_variable_const(const std::string& varname) const;
	/// throws invalid_variablename_exception if varname is no valid variable name.
	config& get_variable_cfg(const std::string& varname);
	/// does nothing if varname is no valid variable name.
	void set_variable(const std::string& varname, const t_string& value);
	/// throws invalid_variablename_exception if varname is no valid variable name.
	config& add_variable_cfg(const std::string& varname, const config& value=config());
	/// returns a variable_access that cannot be used to change the game variables
	variable_access_const get_variable_access_read(const std::string& varname) const
	{
		activate_scope_variable(varname);
		return variable_access_const(varname, variables_);
	}
	/// returns a variable_access that can be used to change the game variables
	variable_access_create get_variable_access_write(const std::string& varname)
	{
		activate_scope_variable(varname);
		return variable_access_create(varname, variables_);
	}
	/// Clears attributes config children
	/// does nothing if varname is no valid variable name.
	void clear_variable(const std::string& varname);
	/// Clears only the config children
	/// does nothing if varname is no valid variable name.
	void clear_variable_cfg(const std::string& varname); 

	const rand_rng::mt_rng& rng() const { return rng_; }
	rand_rng::mt_rng& rng() { return rng_; }

	enum PHASE {
		INITIAL,
		PRELOAD,
		PRESTART,
		START,
		PLAY
	};

	PHASE phase() const { return phase_; }
	void set_phase(PHASE phase) { phase_ = phase; }

	bool allow_end_turn() const { return can_end_turn_; }
	void set_allow_end_turn(bool value) { can_end_turn_ = value; }

	/** the last location where a select event fired. Used by wml menu items with needs_select=yes*/
	map_location last_selected;

	void write_snapshot(config& cfg) const;

	const std::string& next_scenario() const { return next_scenario_; }
	void set_next_scenario(const std::string& next_scenario) { next_scenario_ = next_scenario; }

private:
	void activate_scope_variable(std::string var_name) const;
	///Used to delete variables.
	variable_access_throw get_variable_access_throw(const std::string& varname)
	{
		activate_scope_variable(varname);
		return variable_access_throw(varname, variables_);
	}

	rand_rng::mt_rng rng_;
	config variables_;
	PHASE phase_;
	bool can_end_turn_;
	std::string next_scenario_;                       /**< the scenario coming next (for campaigns) */
};

#endif
