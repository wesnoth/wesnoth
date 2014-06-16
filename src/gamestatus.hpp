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
#include "make_enum.hpp"
#include "map_location.hpp"
#include "mp_game_settings.hpp"
#include "simple_rng.hpp"

#include <boost/shared_ptr.hpp>

class config_writer;
class game_display;
class gamemap;
class scoped_wml_variable;
class t_string;
class team;
class unit_map;

// Defined later in this header:
class game_data;

namespace t_translation {
	struct t_match;
}

class team_builder;
typedef boost::shared_ptr<team_builder> team_builder_ptr;


/// The default difficulty setting for campaigns.
extern const std::string DEFAULT_DIFFICULTY;



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
	game_events::wmi_container wml_menu_items_;
	rand_rng::simple_rng rng_;
	config variables_;
	mutable config temporaries_; // lengths of arrays, etc.
	friend struct variable_info;
	PHASE phase_;
	bool can_end_turn_;
	std::string scenario_;                            /**< the scenario being played */
	std::string next_scenario_;                       /**< the scenario coming next (for campaigns) */
};

//meta information of the game
class game_classification : public savegame::savegame_config
{
public:
	game_classification();
	explicit game_classification(const config& cfg);
	game_classification(const game_classification& gc);

	config to_config() const;

	std::string label;                               /**< Name of the game (e.g. name of save file). */
	std::string version;                             /**< Version game was created with. */
	MAKE_ENUM (CAMPAIGN_TYPE,			 /**< Type of the game - campaign, multiplayer etc. */
		(SCENARIO, 	"scenario")
		(MULTIPLAYER, 	"multiplayer")
		(TEST,		"test")
		(TUTORIAL,	"tutorial")
	)
	CAMPAIGN_TYPE campaign_type;
	std::string campaign_define;                     /**< If there is a define the campaign uses to customize data */
	std::vector<std::string> campaign_xtra_defines;  /**< more customization of data */

	std::string campaign;                            /**< the campaign being played */

	std::string abbrev;                              /**< the campaign abbreviation */
//	std::string scenario;                            /**< the scenario being played */
//	std::string next_scenario;                       /**< the scenario coming next (for campaigns) */
	std::string completion;                          /**< running. victory, or defeat */
	bool end_credits;                                /**< whether to show the standard credits at the end */
	std::string end_text;                            /**< end-of-campaign text */
	unsigned int end_text_duration;                  /**< for how long the end-of-campaign text is shown */
	std::string difficulty; /**< The difficulty level the game is being played on. */
	std::string random_mode;
};
MAKE_ENUM_STREAM_OPS2(game_classification, CAMPAIGN_TYPE)

#endif
