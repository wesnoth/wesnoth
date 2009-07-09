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
#include <deque>

/** This class represenst a single stored chat message */
struct chat_message
{
	/** Create a chat message */
	chat_message(const time_t& timestamp, const std::string& user, const std::string& message);

	/** Create a chat message, assume the time is "now" */
	chat_message(const std::string& user, const std::string& message);

	time_t timestamp;
	std::string user;
	std::string message;
};

/** this class memorizes a chat session. */
class chat_log
{
public:
	chat_log();

	void add_message(const time_t& timestamp, const std::string& user, const std::string& message);

	void add_message(const std::string& user, const std::string& message);

	const std::deque<chat_message>& history() const { return history_; }

	std::string assemble_text();

	void clear();
private:
	std::deque<chat_message> history_;
};

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
	void process_room_members(const config &data);

	const chat_log& log() const { return log_; }
	chat_log& log() { return log_; }

private:
	std::string name_;
	std::set<std::string> members_;
	chat_log log_;
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
	std::string scenario;
	bool remote_scenario;
	std::string map_info;
	std::string map_size_info;
	std::string era;
	std::string era_short;

	std::string gold;
	std::string xp;
	std::string vision;
	std::string status; //vacant slots or turn info
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

	void open_room(const std::string& name);
	void close_room(const std::string& name);
	bool has_room(const std::string& name) const;
	room_info* get_room(const std::string& name);
	const room_info* get_room(const std::string& name) const;

	user_info& get_user(const std::string& name);

	chat_log& get_whisper_log(const std::string& name);

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
	std::map<std::string, chat_log> whispers_;
};

#endif
