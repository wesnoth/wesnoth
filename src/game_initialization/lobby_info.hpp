/*
   Copyright (C) 2009 - 2017 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "config.hpp"
#include "game_initialization/lobby_data.hpp"

#include <boost/dynamic_bitset.hpp>

#include <memory>

namespace mp
{
/**
 * This class represents the collective information the client has
 * about the players and games on the server
 */
class lobby_info
{
public:
	explicit lobby_info(const config& game_config, const std::vector<std::string>& installed_addons);

	typedef std::map<int, game_info> game_info_map;

	/** Process a full game list. Current info is discarded. */
	void process_gamelist(const config& data);

	/**
	 * Process a gamelist diff.
	 *
	 * @param data            Raw game list data, usually received from the MP server.
	 *
	 * @returns               True on success, false on failure (e.g. when the diff did
	 *                        not apply correctly).
	 */
	bool process_gamelist_diff(const config& data);

	void sync_games_display_status();

	/**
	 * Generates a new vector of game pointers from the ID/game map.
	 * The games will be referenced in ascending order by ID.
	 */
	void make_games_vector();

	/** Returns the raw game list config data. */
	const config& gamelist() const
	{
		return gamelist_;
	}

	using game_filter_func = std::function<bool(const game_info&)>;

	/** Adds a new filter function to be considered when @ref apply_game_filter is called. */
	void add_game_filter(game_filter_func func)
	{
		game_filters_.push_back(func);
	}

	/** Clears all game filter functions. */
	void clear_game_filter()
	{
		game_filters_.clear();
	}

	/** Sets whether the result of each game filter should be inverted. */
	void set_game_filter_invert(bool value)
	{
		game_filter_invert_ = value;
	}

	/** Generates a new list of games that match the current filter functions and inversion setting. */
	void apply_game_filter();

	/** Returns info on a game with the given game ID. */
	game_info* get_game_by_id(int id);

	/** Const overload of @ref get_game_by_id. */
	const game_info* get_game_by_id(int id) const;

	/**
	 * Sorts the user list by the given parameters.
	 *
	 * @param by_name         Whether to sort users alphabetically by name.
	 * @param by_relation     Whether to sort users by their relation to each other (ie,
	 *                        display friends before blocked users).
	 */
	void sort_users(bool by_name, bool by_relation);

	/** Open a new chat room with the given name. */
	void open_room(const std::string& name);

	/** Close the chat room with the given name. */
	void close_room(const std::string& name);

	/** Returns whether a room with the given name has been opened. */
	bool has_room(const std::string& name) const;

	/** Returns info on room with the given name, or nullptr if it doesn't exist. */
	room_info* get_room(const std::string& name);

	/** Const overload of @ref get_room. */
	const room_info* get_room(const std::string& name) const;

	/** Returns info on the user with the given name, or nullptr if they don't eixst. */
	user_info* get_user(const std::string& name);

	chat_session& get_whisper_log(const std::string& name)
	{
		return whispers_[name];
	}

	void update_user_statuses(int game_id, const room_info* room);

	const std::vector<room_info>& rooms() const
	{
		return rooms_;
	}

	const std::vector<game_info*>& games() const
	{
		return games_;
	}

	const boost::dynamic_bitset<>& games_visibility() const
	{
		return games_visibility_;
	}

	const std::vector<game_info*>& games_filtered() const
	{
		return games_filtered_;
	}

	const std::vector<user_info>& users() const
	{
		return users_;
	}

	const std::vector<user_info*>& users_sorted() const
	{
		return users_sorted_;
	}

private:
	void process_userlist();

	const config& game_config_;

	const std::vector<std::string>& installed_addons_;

	config gamelist_;

	bool gamelist_initialized_;

	std::vector<room_info> rooms_;

	game_info_map games_by_id_;

	std::vector<game_info*> games_;
	std::vector<game_info*> games_filtered_;

	std::vector<user_info> users_;
	std::vector<user_info*> users_sorted_;

	std::map<std::string, chat_session> whispers_;

	std::vector<game_filter_func> game_filters_;

	bool game_filter_invert_;

	boost::dynamic_bitset<> games_visibility_;
};

enum notify_mode {
	NOTIFY_NONE,
	NOTIFY_MESSAGE,
	NOTIFY_MESSAGE_OTHER_WINDOW,
	NOTIFY_SERVER_MESSAGE,
	NOTIFY_OWN_NICK,
	NOTIFY_FRIEND_MESSAGE,
	NOTIFY_WHISPER,
	NOTIFY_WHISPER_OTHER_WINDOW,
	NOTIFY_LOBBY_JOIN,
	NOTIFY_LOBBY_QUIT,
	NOTIFY_COUNT
};

void do_notify(notify_mode mode, const std::string& sender, const std::string& message);
inline void do_notify(notify_mode mode)
{
	do_notify(mode, "", "");
}
}
