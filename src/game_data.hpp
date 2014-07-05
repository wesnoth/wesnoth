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

/** @file */

#ifndef GAME_STATUS_HPP_INCLUDED
#define GAME_STATUS_HPP_INCLUDED

#include "config.hpp"
#include "game_end_exceptions.hpp"
#include "game_events/wmi_container.hpp"
#include "map_location.hpp"
#include "simple_rng.hpp"
#include "variable_info.hpp"
#include <boost/shared_ptr.hpp>

class config_writer;
class game_display;
class gamemap;
class scoped_wml_variable;
class t_string;
class team;
class unit_map;

namespace t_translation {
	struct t_match;
}
extern int sdfasf;
class team_builder;
typedef boost::shared_ptr<team_builder> team_builder_ptr;

class game_data  : public variable_set  {
public:
	game_data();
	explicit game_data(const config& level);
	game_data(const game_data& data);

	std::vector<scoped_wml_variable*> scoped_variables;

	const config& get_variables() const { return variables_; }

	// Variable access
	config::attribute_value &get_variable(const std::string &varname);
	virtual config::attribute_value get_variable_const(const std::string& varname) const;
	config& get_variable_cfg(const std::string& varname);

	void set_variable(const std::string& varname, const t_string& value);
	config& add_variable_cfg(const std::string& varname, const config& value=config());

	void activate_scope_variable(std::string var_name) const;

	// returns a variable_access that cannot be used to change the game variables
	variable_access_const get_variable_access_read(const std::string& varname) const
	{
		assert(this != NULL);
		activate_scope_variable(varname);
		return variable_access_const(varname, variables_);
	}

	// returns a variable_access that cannot be used to change the game variables
	variable_access_create get_variable_access_write(const std::string& varname)
	{
		assert(this != NULL);
		activate_scope_variable(varname);
		return variable_access_create(varname, variables_);
	}



	void clear_variable(const std::string& varname);
	void clear_variable_cfg(const std::string& varname); // Clears only the config children

	game_events::wmi_container& get_wml_menu_items() { return wml_menu_items_; }

	const rand_rng::simple_rng& rng() const { return rng_; }
	rand_rng::simple_rng& rng() { return rng_; }

	enum PHASE {
		INITIAL,
		PRELOAD,
		PRESTART,
		START,
		PLAY
	};
	PHASE phase() const { return phase_; }
	void set_phase(PHASE phase) { phase_ = phase; }

	//create an object responsible for creating and populating a team from a config
	team_builder_ptr create_team_builder(const config& side_cfg, std::string save_id
			, std::vector<team>& teams, const config& level, gamemap& map
			, unit_map& units, const config& starting_pos);

	//do first stage of team initialization (everything except unit placement)
	void build_team_stage_one(team_builder_ptr tb_ptr);

	//do second stage of team initialization (unit placement)
	void build_team_stage_two(team_builder_ptr tb_ptr);

	bool allow_end_turn() const { return can_end_turn_; }
	void set_allow_end_turn(bool value) { can_end_turn_ = value; }

	/** the last location where a select event fired. */
	map_location last_selected;

	void write_snapshot(config& cfg) const ;
	void write_config(config_writer& out);

	const std::string& next_scenario() const { return next_scenario_; }
	void set_next_scenario(const std::string& next_scenario) { next_scenario_ = next_scenario; }

	game_data& operator=(const game_data& info);
	game_data* operator=(const game_data* info);

private:

	
	///Used to delete variables.
	variable_access_throw get_variable_access_throw(const std::string& varname)
	{
		assert(this != NULL);
		activate_scope_variable(varname);
		return variable_access_throw(varname, variables_);
	}

	game_events::wmi_container wml_menu_items_;
	rand_rng::simple_rng rng_;
	config variables_;
	PHASE phase_;
	bool can_end_turn_;
	std::string scenario_;                            /**< the scenario being played */
	std::string next_scenario_;                       /**< the scenario coming next (for campaigns) */
};

#endif
