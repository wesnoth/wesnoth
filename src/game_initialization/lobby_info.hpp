/*
	Copyright (C) 2009 - 2025
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

#include "config.hpp"
#include "game_initialization/lobby_data.hpp"

#include <boost/dynamic_bitset.hpp>


namespace mp
{
/**
 * This class represents the collective information the client has
 * about the players and games on the server
 */
class lobby_info
{
public:
	lobby_info();

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

	/**
	 * Updates the game pointer list and returns a second stage cleanup function to be
	 * called after any actions have been done using the pointer list.
	 */
	std::function<void()> begin_state_sync();

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
	void clear_game_filters()
	{
		game_filters_.clear();
	}

	/** Sets whether the result of each game filter should be inverted. */
	void set_game_filter_invert(std::function<bool(bool)> value)
	{
		game_filter_invert_ = value;
	}

	/** Returns whether the game would be visible after the game filters are applied */
	bool is_game_visible(const game_info&);

	/** Generates a new list of games that match the current filter functions and inversion setting. */
	void apply_game_filter();

	/** Returns info on a game with the given game ID. */
	game_info* get_game_by_id(int id);

	/** Const overload of @ref get_game_by_id. */
	const game_info* get_game_by_id(int id) const;

	/** Returns info on the user with the given name, or nullptr if they don't eixst. */
	user_info* get_user(const std::string& name);

	const std::vector<game_info*>& games() const
	{
		return games_;
	}

	const boost::dynamic_bitset<>& games_visibility() const
	{
		return games_visibility_;
	}

	const std::vector<user_info>& users() const
	{
		return users_;
	}

	std::vector<user_info>& users()
	{
		return users_;
	}

	bool gamelist_initialized() const
	{
		return gamelist_initialized_;
	}

	void refresh_installed_addons_cache();

private:
	bool process_gamelist_diff_impl(const config& data);

	void process_userlist();

	/**
	 * Generates a new vector of game pointers from the ID/game map.
	 * The games will be referenced in ascending order by ID.
	 */
	void make_games_vector();

	std::vector<std::string> installed_addons_;

	config gamelist_;

	bool gamelist_initialized_;

	game_info_map games_by_id_;

	std::vector<game_info*> games_;

	std::vector<user_info> users_;

	std::vector<game_filter_func> game_filters_;

	std::function<bool(bool)> game_filter_invert_;

	boost::dynamic_bitset<> games_visibility_;
};

enum class notify_mode {
	none,
	message,
	message_other_window,
	server_message,
	own_nick,
	friend_message,
	whisper,
	whisper_other_window,
	lobby_join,
	lobby_quit,
	game_created
};

void do_notify(notify_mode mode, const std::string& sender = "", const std::string& message = "");
}
