/*
	Copyright (C) 2009 - 2021
	by Tomasz Sniatowski <kailoran@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include <ctime>
#include <set>
#include <deque>
#include <functional>
#include <vector>
#include <string>

class config;

namespace mp {

/** This class represents a single stored chat message */
struct chat_message
{
	/** Create a chat message */
	chat_message(const std::time_t& timestamp,
				 const std::string& user,
				 const std::string& message);

	std::time_t timestamp;
	std::string user;
	std::string message;
};

/** this class memorizes a chat session. */
class chat_session
{
public:
	chat_session();

	void add_message(const std::time_t& timestamp,
					 const std::string& user,
					 const std::string& message);

	void add_message(const std::string& user, const std::string& message);

	const std::deque<chat_message>& history() const
	{
		return history_;
	}

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
	explicit room_info(const std::string& name);

	const std::string& name() const
	{
		return name_;
	}
	const std::set<std::string>& members() const
	{
		return members_;
	}
	bool is_member(const std::string& user) const;
	void add_member(const std::string& user);
	void remove_member(const std::string& user);
	void process_room_members(const config& data);

	const chat_session& log() const
	{
		return log_;
	}
	chat_session& log()
	{
		return log_;
	}

private:
	std::string name_;
	std::set<std::string> members_;
	chat_session log_;
};


/**
 * This class represents the information a client has about another player
 */
struct user_info
{
	explicit user_info(const config& c);

	void update_state(int selected_game_id);

	void update_relation();

	enum class user_relation {
		ME,
		FRIEND,
		NEUTRAL,
		IGNORED
	};

	enum class user_state {
		LOBBY,
		GAME,
		SEL_GAME
	};

	bool operator<(const user_info& b) const;

	std::string name;
	int forum_id;
	int game_id;
	user_relation relation;
	user_state state;
	bool registered;
	bool observing;
	bool moderator;
};

/**
 * This class represents the info a client has about a game on the server
 */
struct game_info
{
	game_info(const config& c, const std::vector<std::string>& installed_addons);

	bool can_join() const;
	bool can_observe() const;

	int id;
	std::string map_data;
	std::string name;
	std::string scenario;
	std::string type_marker;
	bool remote_scenario;
	std::string map_info;
	std::string map_size_info;
	std::string era;

	/** List of modification names and whether they're installed or not. */
	std::vector<std::pair<std::string, bool>> mod_info;

	std::string gold;
	std::string support;
	std::string xp;
	std::string vision;
	std::string status; // vacant slots or turn info
	std::string time_limit;
	std::size_t vacant_slots;

	unsigned int current_turn;
	bool reloaded;
	bool started;
	bool fog;
	bool shroud;
	bool observers;
	bool shuffle_sides;
	bool use_map_settings;
	bool private_replay;
	bool verified;
	bool password_required;
	bool have_era;
	bool have_all_mods;

	bool has_friends;
	bool has_ignored;

	enum class disp_status {
		CLEAN,
		NEW,
		UPDATED,
		DELETED
	};

	disp_status display_status;

	enum class addon_req { SATISFIED, NEED_DOWNLOAD, CANNOT_SATISFY };

	struct required_addon {
		std::string addon_id;
		addon_req outcome;
		std::string message;
	};

	std::vector<required_addon> required_addons;
	addon_req addons_outcome;

	addon_req check_addon_version_compatibility(const config& local_item, const config& game);

	const char* display_status_string() const;

	bool match_string_filter(const std::string& filter) const;
};

}
