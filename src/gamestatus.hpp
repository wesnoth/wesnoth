/* $Id$ */
/*
   Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
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

#include "mp_game_settings.hpp"
#include "random.hpp"
#include "simple_rng.hpp"
#include "map_location.hpp"
#include "variable.hpp"
#include "serialization/binary_or_text.hpp"
#include "boost/shared_ptr.hpp"
#include "game_end_exceptions.hpp"
#include "unit.hpp"

class scoped_wml_variable;
class team;
class gamemap;

namespace t_translation {
	struct t_match;
}

class team_builder;
typedef boost::shared_ptr<team_builder> team_builder_ptr;

struct wml_menu_item
{
	wml_menu_item(const std::string& id, const config* cfg=NULL);
	std::string name;
	const std::string event_id;
	std::string image;
	t_string description;
	bool needs_select;
	config show_if;
	config filter_location;
	config command;
};

class wmi_container{
public:
	wmi_container();
	explicit wmi_container(std::map<std::string, wml_menu_item*> wml_menu_items);
	explicit wmi_container(const wmi_container& container);

	std::map<std::string, wml_menu_item*>& get_menu_items() { return wml_menu_items_; };
	void clear_wmi();
	void to_config(config& cfg);
	void set_menu_items(const config& cfg);

	wml_menu_item*& get_item(const std::string id) { return wml_menu_items_[id]; };
private:
	std::map<std::string, wml_menu_item*> wml_menu_items_;
};

class carryover{
public:
	carryover()
		: add_ ()
		, color_()
		, current_player_()
		, gold_()
		, name_()
		, previous_recruits_()
		, recall_list_()
		, save_id_()
	{};
	// Turns config from a loaded savegame into carryover_info
	explicit carryover(const config& side);
	carryover(const team& t, const int gold, const bool add);
	~carryover(){};

	const std::string& get_save_id() const{ return save_id_; };
	void transfer_all_gold_to(config& side_cfg);
	void transfer_all_recruits_to(config& side_cfg);
	void transfer_all_recalls_to(config& side_cfg);
	//std::vector<unit>& get_recall_list() { return recall_list_; };
	void update_carryover(const team& t, const int gold, const bool add);
	void initialize_team(config& side_cfg);
	const std::string to_string();
	void to_config(config& cfg);
private:
	bool add_;
	std::string color_;
	std::string current_player_;
	int gold_;
	std::string name_;
	std::set<std::string> previous_recruits_;
	std::vector<unit> recall_list_;
	std::string save_id_;

	std::string get_recruits(bool erase=false);
};

class carryover_info{
public:
	carryover_info()
		: carryover_sides_()
		, end_level_()
		, variables_()
		, rng_()
	{}
	// Turns config from a loaded savegame into carryover_info
	explicit carryover_info(const config& cfg);
	~carryover_info(){};
	carryover* get_side(std::string save_id);
	std::vector<carryover>& get_all_sides();
	void add_side(const config& cfg);
	void add_side(const team& t, const int gold, const bool add);
	void set_end_level(const end_level_data& end_level) { end_level_ = end_level; }

	void transfer_from(const team& t, int carryover_gold);
	void transfer_all_to(config& side_cfg);

	void transfer_from(const game_data& gamedata);
	void transfer_to(config& level);

	void set_variables(const config& vars) { variables_ = vars; }
	const config& get_variables() const { return variables_; }

	const rand_rng::simple_rng& rng() const { return rng_; }
	rand_rng::simple_rng& rng() { return rng_; }

	const end_level_data& get_end_level() const;
	const config to_config();

	wmi_container wml_menu_items;
private:
	std::vector<carryover> carryover_sides_;
	end_level_data end_level_;
	config variables_;
	rand_rng::simple_rng rng_;
};

class game_data  : public variable_set  {
public:
	game_data();
	game_data(const config& level);
	game_data(const game_data& data);

	~game_data();

	std::vector<scoped_wml_variable*> scoped_variables;
	wmi_container wml_menu_items;

	const config& get_variables() const { return variables_; }
	void set_variables(const config& vars);

	// Variable access
	config::attribute_value &get_variable(const std::string &varname);
	virtual config::attribute_value get_variable_const(const std::string& varname) const;
	config& get_variable_cfg(const std::string& varname);

	void set_variable(const std::string& varname, const t_string& value);
	config& add_variable_cfg(const std::string& varname, const config& value=config());

	void clear_variable(const std::string& varname);
	void clear_variable_cfg(const std::string& varname); // Clears only the config children

	const rand_rng::simple_rng& rng() const { return rng_; }
	rand_rng::simple_rng& rng() { return rng_; }

	void set_vars(const config& cfg);

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
			, unit_map& units, bool snapshot, const config& starting_pos);

	//do first stage of team initialization (everything except unit placement)
	void build_team_stage_one(team_builder_ptr tb_ptr);

	//do second stage of team initialization (unit placement)
	void build_team_stage_two(team_builder_ptr tb_ptr);

	bool allow_end_turn() const { return can_end_turn_; }
	void set_allow_end_turn(bool value) { can_end_turn_ = value; }

	/** the last location where a select event fired. */
	map_location last_selected;

	void write_snapshot(config& cfg);
	void write_config(config_writer& out, bool write_variables);

	game_data& operator=(const game_data& info);
	game_data* operator=(const game_data* info);

private:
	rand_rng::simple_rng rng_;
	config variables_;
	mutable config temporaries_; // lengths of arrays, etc.
	const rand_rng::set_random_generator generator_setter_; /**< Make sure that rng is initialized first */
	friend struct variable_info;
	PHASE phase_;
	bool can_end_turn_;
};

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
	bool end_credits;                                /**< whether to show the standard credits at the end */
	std::string end_text;                            /**< end-of-campaign text */
	unsigned int end_text_duration;                  /**< for how long the end-of-campaign text is shown */
	std::string difficulty; /**< The difficulty level the game is being played on. */
};

class game_state
{
public:
	game_state();
	game_state(const game_state& state);
	game_state(const config& cfg, bool show_replay = false);

	~game_state(){};
	game_state& operator=(const game_state& state);

	//write the gamestate into a config object
	void write_snapshot(config& cfg) const;
	//write the config information into a stream (file)
	void write_config(config_writer& out, bool write_variables=true) const;

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

	/** The carryover information for all sides*/
	carryover_info carryover_sides;

private:
	game_classification classification_;
	mp_game_settings mp_settings_;
};


void write_players(game_state& gamestate, config& cfg, const bool use_snapshot=true, const bool merge_side = false);

void extract_summary_from_config(config& cfg_save, config& cfg_summary);

#endif
