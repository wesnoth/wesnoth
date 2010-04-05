/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file gamestatus.hpp */

#ifndef GAME_STATUS_HPP_INCLUDED
#define GAME_STATUS_HPP_INCLUDED

#include "mp_game_settings.hpp"
#include "random.hpp"
#include "simple_rng.hpp"
#include "team.hpp"
#include "variable.hpp"
#include "savegame_config.hpp"
#include "serialization/binary_or_text.hpp"

#include <time.h>
#include <string>
#include <vector>

class scoped_wml_variable;

//meta information of the game
class game_classification : public savegame::savegame_config
{
public:
	game_classification();
	game_classification(const config& cfg);
	game_classification(const game_classification& gc);

	config to_config() const;

	std::string label;                               /**< Name of the game (e.g. name of save file). */
	std::string parent;                              /**< Parent of the game (for save-threading purposes). */
	std::string version;                             /**< Version game was created with. */
	std::string campaign_type;                       /**< Type of the game - campaign, multiplayer etc. */
	std::string campaign_define;                     /**< If there is a define the campaign uses to customize data */
	std::vector<std::string> campaign_xtra_defines;  /**< more customization of data */

	std::string campaign;                            /**< the campaign being played */
	std::string history;                             /**< ancestral IDs */
	std::string abbrev;                              /**< the campaign abbreviation */
	std::string scenario;                            /**< the scenario being played */
	std::string next_scenario;                       /**< the scenario coming next (for campaigns) */
	std::string completion;                          /**< running. victory, or defeat */
	std::string end_text;                            /**< end-of-campaign text */
	unsigned int end_text_duration;                  /**< for how long the end-of-campaign text is shown */
	std::string difficulty; /**< The difficulty level the game is being played on. */
};

struct wml_menu_item
{
	wml_menu_item(const std::string& id, const config* cfg=NULL);
	std::string name;
	std::string image;
	t_string description;
	bool needs_select;
	config show_if;
	config filter_location;
	config command;
};

class game_state : public variable_set
{
public:
	game_state();
	game_state(const game_state& state);
	game_state(const config& cfg, bool show_replay = false);

	~game_state();
	game_state& operator=(const game_state& state);

	std::vector<scoped_wml_variable*> scoped_variables;
	std::map<std::string, wml_menu_item*> wml_menu_items;

	const config& get_variables() const { return variables; }
	void set_variables(const config& vars);

	void set_menu_items(const config::const_child_itors &menu_items);

	//write the gamestate into a config object
	void write_snapshot(config& cfg) const;
	//write the config information into a stream (file)
	void write_config(config_writer& out, bool write_variables=true) const;

	// Variable access

	config::proxy_string get_variable(const std::string &varname);
	virtual t_string get_variable_const(const std::string& varname) const;
	config& get_variable_cfg(const std::string& varname);

	void set_variable(const std::string& varname, const t_string& value);
	config& add_variable_cfg(const std::string& varname, const config& value=config());

	void clear_variable(const std::string& varname);
	void clear_variable_cfg(const std::string& varname); // Clears only the config children

	const rand_rng::simple_rng& rng() const { return rng_; }
	rand_rng::simple_rng& rng() { return rng_; }

	//create and populate a team from a config
	void build_team(const config& side_cfg, std::string save_id
			, std::vector<team>& teams, const config& level, gamemap& map
			, unit_map& units, bool snapshot);

	game_classification& classification() { return classification_; }
	const game_classification& classification() const { return classification_; } //FIXME: const getter to allow use from const gamestatus::sog() (see ai.cpp:344) - remove after merge?

	/** Multiplayer parameters for this game */
	mp_game_settings& mp_settings() { return mp_settings_; }
	const mp_game_settings& mp_settings() const { return mp_settings_; }

	/**
	 * If the game is saved mid-level, we have a series of replay steps
	 * to take the game up to the position it was saved at.
	 */
	config replay_data;

	/**
	 * Saved starting state of the game.
	 *
	 * For multiplayer games, the position the game started in may be different
	 * to the scenario.
	 */
	config starting_pos;

	/**
	 * Snapshot of the game's current contents.
	 *
	 * i.e. unless the player selects to view a replay, the game's settings are
	 * read in from this object.
	 */
	config snapshot;

	/** the last location where a select event fired. */
	map_location last_selected;

private:
	rand_rng::simple_rng rng_ ;
	config variables;
	mutable config temporaries; // lengths of arrays, etc.
	const rand_rng::set_random_generator generator_setter; /**< Make sure that rng is initialized first */
	friend struct variable_info;
	game_classification classification_;
	mp_game_settings mp_settings_;
};


std::string generate_game_uuid();

void write_players(game_state& gamestate, config& cfg, const bool use_snapshot=true, const bool merge_side = false);

void extract_summary_from_config(config& cfg_save, config& cfg_summary);

#endif
