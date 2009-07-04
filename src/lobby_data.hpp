/* $Id$ */
/*
   Copyright (C) 2009 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef INC_LOBBY_DATA
#define INC_LOBBY_DATA

#include "config.hpp"
#include "sdl_utils.hpp"
#include <set>
#include <map>

/**
 * This class represents the information a client has about a room
 */
class room_info
{
public:
	room_info(const std::string& name);

	const std::string& name() const { return name_; }
	bool is_member(const std::string& user) const;
	void add_member(const std::string& user);
	void remove_member(const std::string& user);
private:
	const std::string& name_;
	std::set<std::string> members_;
};


/**
 * This class represents the information a client has about another player
 */
struct user_info
{
	user_info();
	user_info(const config& c);

	void update_state(const std::string& selected_game_id, const room_info* current_room = NULL);

	enum user_relation { ME, FRIEND, NEUTRAL, IGNORED };
	enum user_state    { LOBBY, SEL_ROOM, GAME, SEL_GAME };

	bool operator> (const user_info& b) const;

	std::string name;
	std::string game_id;
	std::string location;
	user_relation relation;
	user_state state;
	bool registered;
};

/**
 * This class represents the info a client has about a game on the server
 */
struct game_info
{
	game_info();
	game_info(const config& c, const config& game_config);

	surface mini_map;
	std::string id;
	std::string map_data;
	std::string name;
	std::string map_info;
	std::string map_info_size;
	std::string gold;
	std::string xp;
	std::string vision;
	std::string status;
	std::string time_limit;
	size_t vacant_slots;
	unsigned int current_turn;
	bool reloaded;
	bool started;
	bool fog;
	bool shroud;
	bool observers;
	bool use_map_settings;
	bool verified;
	bool password_required;
	bool have_era;
};

/**
 * This class represents the collective information the client has
 * about the players and games on the server
 */
class lobby_info
{
public:
	lobby_info(const config& game_config);

	void process_gamelist(const config &data);
	bool process_gamelist_diff(const config &data);

	const config& gamelist() const { return gamelist_; }
	const std::vector<room_info>& rooms() const { return rooms_; }
	const std::vector<game_info>& games() const { return games_; }
	const std::vector<user_info>& users() const { return users_; }
private:
	void parse_gamelist();

	const config& game_config_;
	config gamelist_;
	bool gamelist_initialized_;
	std::vector<room_info> rooms_;
	std::vector<game_info> games_;
	std::vector<user_info> users_;
};

#endif
