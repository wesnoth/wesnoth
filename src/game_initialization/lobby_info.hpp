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

namespace mp {

/**
 * This class represents the collective information the client has
 * about the players and games on the server
 */
class lobby_info
{
public:
	explicit lobby_info(const config& game_config, const std::vector<std::string>& installed_addons);

	~lobby_info();

	void delete_games();

	typedef std::map<int, game_info*> game_info_map;

	using game_filter_func = std::function<bool(const game_info&)>;

	/**
	 * Process a full gamelist. Current info is discarded.
	 */
	void process_gamelist(const config& data);

	/**
	 * Process a gamelist diff.
	 * @return true on success, false on failure (e.g. when the
	 * diff did not apply correctly)
	 */
	bool process_gamelist_diff(const config& data);

	void sync_games_display_status();

	void make_games_vector();

	const config& gamelist() const
	{
		return gamelist_;
	}

	void clear_game_filter();
	void add_game_filter(game_filter_func func);
	void set_game_filter_invert(bool value);
	void apply_game_filter();

	game_info* get_game_by_id(int id);
	const game_info* get_game_by_id(int id) const;

	void sort_users(bool by_name, bool by_relation);

	void open_room(const std::string& name);
	void close_room(const std::string& name);
	bool has_room(const std::string& name) const;
	room_info* get_room(const std::string& name);
	const room_info* get_room(const std::string& name) const;

	user_info* get_user(const std::string& name);

	chat_session& get_whisper_log(const std::string& name);

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
	const std::vector<game_info*>& games_filtered() const;
	const std::vector<user_info>& users() const
	{
		return users_;
	}
	const std::vector<user_info*>& users_sorted() const;

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

void do_notify(notify_mode mode, const std::string & sender, const std::string & message);
inline void do_notify(notify_mode mode) { do_notify(mode, "", ""); }

}
