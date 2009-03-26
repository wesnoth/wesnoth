/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
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

#include "filesystem.hpp"
#include "random.hpp"
#include "team.hpp"
#include "time_of_day.hpp"
#include "unit.hpp"
#include "variable.hpp"

#include <time.h>
#include <string>
#include <vector>


class scoped_wml_variable;

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

/** Information on a particular player of the game. */
struct player_info
{
	player_info() :
		name(),
		gold(-1) ,
		gold_add(false),
		available_units(),
		can_recruit()
	{}

	std::string name;                  /**< Stores the current_player name */
	int gold;                          /**< Amount of gold the player has saved */
	bool gold_add;                     /**<
										* Amount of gold is added to the
										* starting gold, if not it uses the
										* highest of the two.
										*/
	std::vector<unit> available_units; /**< Units the player may recall */
	std::set<std::string> can_recruit; /**< Units the player has the ability to recruit */

	void debug();
};

class game_state : public variable_set
{
public:
	game_state();
	game_state(const game_state& state);
	game_state(const config& cfg, bool show_replay = false);

	~game_state();
	game_state& operator=(const game_state& state);
	std::string label;                               /**< Name of the game (e.g. name of save file). */
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

	/**
	 * Information about campaign players who carry resources from previous
	 * levels, indexed by a string identifier (which is the leader name by
	 * default, but can be set with the "id" attribute of the "side" tag).
	 */
	std::map<std::string, player_info> players;

	/** Return the Nth player, or NULL if no such player exists. */
	player_info* get_player(const std::string& id);

	/**
	 * Loads the recall list.
	 *
	 * @param players      Reference to the players section to load.
	 */
	void load_recall_list(const config::const_child_itors &players);

	std::vector<scoped_wml_variable*> scoped_variables;
	std::map<std::string, wml_menu_item*> wml_menu_items;

	const config& get_variables() const { return variables; }
	void set_variables(const config& vars);

	void set_menu_items(const config::const_child_itors &menu_items);

	//write the gamestate into a config object
	void write_snapshot(config& cfg) const;

	// Variable access

	t_string& get_variable(const std::string& varname);
	virtual const t_string& get_variable_const(const std::string& varname) const;
	config& get_variable_cfg(const std::string& varname);
	variable_info::array_range get_variable_cfgs(const std::string& varname);

	void set_variable(const std::string& varname, const t_string& value);
	config& add_variable_cfg(const std::string& varname, const config& value=config());

	void clear_variable(const std::string& varname);
	void clear_variable_cfg(const std::string& varname); // Clears only the config children

        const rand_rng::simple_rng& rng() const { return rng_; }
        rand_rng::simple_rng& rng() { return rng_; }

	std::string difficulty; /**< The difficulty level the game is being played on. */

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
};

/**
 * Contains the global status of the game.
 *
 * Namely the current turn, the number of turns, and the time of day.
 */
class gamestatus
{
public:
	/**
	 * Reads turns and time information from parameters.
	 *
	 * It sets random starting ToD and current_tod to config.
	 */
	gamestatus(const config& time_cfg, int num_turns, game_state* s_o_g = NULL);
	void write(config& cfg) const;

	/** Returns time of day object for current turn. */
	time_of_day get_time_of_day() const;
	time_of_day get_previous_time_of_day() const;
	time_of_day get_time_of_day(int illuminated, const map_location& loc) const;

	/**
	 * Returns time of day object in the turn.
	 *
	 * It first tries to look for specified. If no area time specified in
	 * location, it returns global time.
	 */
	time_of_day get_time_of_day(int illuminated, const map_location& loc, int n_turn) const;

	/**
	 * Sets global time of day in this turn.
	 *
	 * Time is a number between 0 and n-1, where n is number of ToDs.
	 */
	bool set_time_of_day(int newTime);
	size_t turn() const;
	int number_of_turns() const;
	void modify_turns(const std::string& mod);
	void add_turns(int num);

	/** Dynamically change the current turn number. */
	void set_turn(unsigned int num);

	/**
	 * Function to move to the next turn.
	 *
	 * @returns                   True if time has not expired.
	 */
	bool next_turn();

	static bool is_start_ToD(const std::string&);

	/**
	 * Adds a new local time area from config, making it follow its own
	 * time-of-day sequence.
	 *
	 * @param cfg                 Config object containing x,y range/list of
	 *                            locations and desired [time] information.
	 */
	void add_time_area(const config& cfg);

	/**
	 * Adds a new local time area from a set of locations, making those
	 * follow a different time-of-day sequence.
	 *
	 * @param id                  Identifier string to associate this time area
	 *                            with.
	 * @param locs                Set of locations to be affected.
	 * @param time_cfg            Config object containing [time] information.
	 */
	void add_time_area(const std::string& id, const std::set<map_location>& locs,
	                   const config& time_cfg);

	/**
	 * Removes a time area from config, making it follow the scenario's
	 * normal time-of-day sequence.
	 *
	 * @param id                  Identifier of time_area to remove. Supply an
	 *                            empty one to remove all local time areas.
	 */
	void remove_time_area(const std::string& id);

	/**
	 * @todo FIXME: since gamestatus may be constructed with NULL game_state* (by default),
	 * you should not rely on this function to return the current game_state.
	 */
	const game_state& sog() const{return(*state_of_game_);}

	std::vector<team> *teams;

private:
	void set_start_ToD(config&, game_state*);

	/**
	 * Returns time of day object in the turn.
	 *
	 * Correct time is calculated from current time.
	 */
	time_of_day get_time_of_day_turn(int nturn) const;
	void next_time_of_day();

	std::vector<time_of_day> times_;

	struct area_time_of_day {
		area_time_of_day() :
			xsrc(),
			ysrc(),
			id(),
			times(),
			hexes()
			{}

		std::string xsrc, ysrc;
		std::string id;
		std::vector<time_of_day> times;
		std::set<map_location> hexes;
	};

	std::vector<area_time_of_day> areas_;

	size_t turn_;
	int numTurns_;
	int currentTime_;
	const game_state* state_of_game_;
};

std::string generate_game_uuid();

/**
 * Holds all the data needed to start a scenario.
 *
 * I.e. this is the object serialized to disk when saving/loading a game.
 * It is also the object which needs to be created to start a new game.
 */
struct save_info {
	save_info(const std::string& n, time_t t) : name(n), time_modified(t) {}
	std::string name;
	time_t time_modified;
};

/** Get a list of available saves. */
std::vector<save_info> get_saves_list(const std::string* dir = NULL, const std::string* filter = NULL);

enum WRITE_GAME_MODE { WRITE_SNAPSHOT_ONLY, WRITE_FULL_GAME };

void read_save_file(const std::string& name, config& cfg, std::string* error_log);

void write_players(game_state& gamestate, config& cfg);
void write_game(config_writer &out, const game_state& gamestate, WRITE_GAME_MODE mode=WRITE_FULL_GAME);

/** Returns true iff there is already a savegame with that name. */
bool save_game_exists(const std::string & name);

/** Throws game::save_game_failed. */
scoped_ostream open_save_game(const std::string &label);
void finish_save_game(config_writer &out, const game_state& gamestate, const std::string &label);

/** Load/Save games. */
void load_game(const std::string& name, game_state& gamestate, std::string* error_log);
void load_game_summary(const std::string& name, config& cfg_summary, std::string* error_log);

/** Throws gamestatus::save_game_failed. */
void save_game(const game_state& gamestate);

/** Delete a savegame. */
void delete_game(const std::string& name);

config& save_summary(std::string save);

void write_save_index();

void replace_underbar2space(std::string &name);

#endif
