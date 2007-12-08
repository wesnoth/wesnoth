/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file gamestatus.hpp
//!

#ifndef GAME_STATUS_HPP_INCLUDED
#define GAME_STATUS_HPP_INCLUDED

#include "filesystem.hpp"
#include "team.hpp"
#include "unit.hpp"
#include "variable.hpp"

#include <time.h>
#include <string>
#include <vector>

class scoped_wml_variable;

//! Object which defines the current time of day.
struct time_of_day
{
	explicit time_of_day(const config& cfg);
	void write(config& cfg) const;

	// The % bonus lawful units receive.
	// Chaotic units will receive -lawful_bonus.
	int lawful_bonus;
	int bonus_modified;

	// The image to be displayed in the game status.
	std::string image;
	t_string name;
	std::string id;

	// The image that is to be laid over all images
	// while this time of day lasts.
	std::string image_mask;

	// The colour modifications that should be made
	// to the game board to reflect the time of day.
	int red, green, blue;

	//! List of "ambient" sounds associated with this time_of_day,
	//! Played at the beginning of turn.
	std::string sounds;
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

//! Information on a particular player of the game. 
struct player_info
{
	player_info() : 
		name(),
		gold(-1) ,
		gold_add(false),
		available_units(),
		can_recruit()
	{}

	std::string name;                  //!< Stores the current_player name 
	int gold;                          //!< Amount of gold the player has saved 
	bool gold_add;                     //!< Amount of gold is added to the 
	                                   //!< starting gold, if not it uses the highest
	                                   //!< of the two. 
	std::vector<unit> available_units; //!< Units the player may recall 
	std::set<std::string> can_recruit; //!< Units the player has the ability to recruit 
};

class game_state : public variable_set
{
public:
	game_state() : 
		label(),
		version(),
		campaign_type(),
		campaign_define(),
		campaign_xtra_defines(),
		campaign(),
		abbrev(),
		scenario(),
		next_scenario(),
		completion(),
		players(),
		scoped_variables(),
		wml_menu_items(),
		difficulty("NORMAL"), 
		replay_data(),
		starting_pos(),
		snapshot(),
		last_selected(gamemap::location::null_location), 
		variables(),
		temporaries()
		{}

	game_state(const game_state& state);
	game_state(const game_data& data, const config& cfg);

	~game_state();
	game_state& operator=(const game_state& state);

	std::string label;                               //!< Name of the game (e.g. name of save file).
	std::string version;                             //!< Version game was created with.
	std::string campaign_type;                       //!< Type of the game - campaign, multiplayer etc.

	std::string campaign_define;                     //!< If there is a define the campaign uses to customize data
	std::vector<std::string> campaign_xtra_defines;  //!< more customization of data

	std::string campaign;                            //!< the campaign being played
	std::string abbrev;	                             //!< the campaign abbreviation
	std::string scenario;                            //!< the scenario being played
	std::string next_scenario;                       //!< the scenario coming next (for campaigns)
	std::string completion;                          //!< running. victory, or defeat

	//! Information about campaign players who carry resources
	//! from previous levels, indexed by a string identifier
	// (which is the leader name by default, but can be set
	// with the "id" attribute of the "side" tag).
	std::map<std::string, player_info> players;

	//! Return the Nth player, or NULL if no such player exists.
	player_info* get_player(const std::string& id);

	std::vector<scoped_wml_variable*> scoped_variables;
	std::map<std::string, wml_menu_item*> wml_menu_items;

	const config& get_variables() const { return variables; }
	void set_variables(const config& vars);

	void set_menu_items(const config::child_list& menu_items);

	// Variable access

	t_string& get_variable(const std::string& varname);
	virtual const t_string& get_variable_const(const std::string& varname) const;
	config& get_variable_cfg(const std::string& varname);

	void set_variable(const std::string& varname, const t_string& value);
	config& add_variable_cfg(const std::string& varname, const config& value=config());

	void clear_variable(const std::string& varname);
	void clear_variable_cfg(const std::string& varname); // Clears only the config children

	std::string difficulty; //!< The difficulty level the game is being played on.

	//! If the game is saved mid-level, we have a series of replay steps
	//! to take the game up to the position it was saved at.
	config replay_data;

	//! Saved starting state of the game.
	//! For multiplayer games, the position the game started in
	//! may be different to the scenario,
	config starting_pos;

	//! Snapshot of the game's current contents.
	//! i.e. unless the player selects to view a replay,
	//! the game's settings are read in from this object.
	config snapshot;

	//! the last location where a select event fired.
	gamemap::location last_selected;
private:
	config variables;
	mutable config temporaries; // lengths of arrays, etc.
	friend struct variable_info;
};


//! Contains the global status of the game.
//! Namely the current turn, the number of turns, and the time of day.
class gamestatus
{
public:
	gamestatus(const config& time_cfg, int num_turns, game_state* s_o_g = NULL);
	void write(config& cfg) const;

	time_of_day get_time_of_day() const;
	time_of_day get_previous_time_of_day() const;
	time_of_day get_time_of_day(int illuminated, const gamemap::location& loc) const;
	time_of_day get_time_of_day(int illuminated, const gamemap::location& loc, int n_turn) const;
	bool set_time_of_day(int);
	size_t turn() const;
	int number_of_turns() const;
	void modify_turns(const std::string& mod);
	void add_turns(int num);

	//! Function to move to the next turn.
	//! Returns true iff time has expired.
	bool next_turn();

	static bool is_start_ToD(const std::string&);

	//! @todo FIXME: since gamestatus may be constructed with NULL game_state* (by default),
	//! you should not rely on this function to return the current game_state.
	const game_state& sog() const{return(*state_of_game_);}

	std::vector<team> *teams;

private:
	void set_start_ToD(config&, game_state*);
	time_of_day get_time_of_day_turn(int nturn) const;
	void next_time_of_day();

	std::vector<time_of_day> times_;

	struct area_time_of_day {
		area_time_of_day() :
			xsrc(),
			ysrc(),
			times(),
			hexes()
			{}

		std::string xsrc, ysrc;
		std::vector<time_of_day> times;
		std::set<gamemap::location> hexes;
	};

	std::vector<area_time_of_day> areas_;

	size_t turn_;
	int numTurns_;
	int currentTime_;
	const game_state* state_of_game_;
};

//! Holds all the data needed to start a scenario.
//! I.e. this is the object serialized to disk when saving/loading a game.
//! It is also the object which needs to be created to start a new game.
struct save_info {
	save_info(const std::string& n, time_t t) : name(n), time_modified(t) {}
	std::string name;
	time_t time_modified;
};

//! Get a list of available saves.
std::vector<save_info> get_saves_list(const std::string *dir = NULL);

enum WRITE_GAME_MODE { WRITE_SNAPSHOT_ONLY, WRITE_FULL_GAME };

void read_save_file(const std::string& name, config& cfg, std::string* error_log);

void write_game(const game_state& gamestate, config& cfg, WRITE_GAME_MODE mode=WRITE_FULL_GAME);
void write_game(config_writer &out, const game_state& gamestate, WRITE_GAME_MODE mode=WRITE_FULL_GAME);

//! Returns true iff there is already a savegame with that name.
bool save_game_exists(const std::string & name);

//! Throws game::save_game_failed
scoped_ostream open_save_game(const std::string &label);
void finish_save_game(config_writer &out, const game_state& gamestate, const std::string &label);

//! Load/Save games.
void load_game(const game_data& data, const std::string& name, game_state& gamestate, std::string* error_log);
void load_game_summary(const std::string& name, config& cfg_summary, std::string* error_log);
//! Throws gamestatus::save_game_failed
void save_game(const game_state& gamestate);

//! Delete a savegame.
void delete_game(const std::string& name);

config& save_summary(const std::string& save);

void write_save_index();

void replace_underbar2space(std::string &name);

#endif
