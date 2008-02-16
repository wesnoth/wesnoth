/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef GAME_PREFERENCES_HPP_INCLUDED
#define GAME_PREFERENCES_HPP_INCLUDED

class gamemap;
class game_state;
class team;
class unit_map;

#include "game_config.hpp"
#include "preferences.hpp"
#include "terrain_translation.hpp"

#include <string>
#include <utility>
#include <set>

namespace preferences {

	struct manager
	{
		manager();
		~manager();

		base_manager base;
	};

	bool adjust_gamma();
	void _set_adjust_gamma(bool val);

	int gamma();
	void _set_gamma(int gamma);

	bool show_lobby_join(const std::string& sender, const std::string& message);
	int lobby_joins();
	void _set_lobby_joins(int show);
	enum { SHOW_NONE, SHOW_FRIENDS, SHOW_ALL };

	std::string get_friends();
	std::string get_ignores();
	bool add_friend(const std::string nick);
	bool add_ignore(const std::string nick);
	void remove_friend(const std::string nick);
	void remove_ignore(const std::string nick);
	void clear_friends();
	void clear_ignores();
	bool is_friend(const std::string nick);
	bool is_ignored(const std::string nick);
	
	bool sort_list();
	void _set_sort_list(bool show);

	bool iconize_list();
	void _set_iconize_list(bool show);

	const std::vector<game_config::server_info>& server_list();

	const std::string network_host();
	void set_network_host(const std::string& host);
	
	const unsigned int get_ping_timeout();
	void set_ping_timeout(unsigned int timeout);

	const std::string campaign_server();
	void set_campaign_server(const std::string& host);

	const std::string login();
	void set_login(const std::string& username);

	bool mouse_scroll_enabled();
	void enable_mouse_scroll(bool value);

	bool turn_dialog();
	void set_turn_dialog(bool ison);

	bool show_combat();

	bool allow_observers();
	void set_allow_observers(bool value);

	bool use_map_settings();
	void set_use_map_settings(bool value);

	bool random_start_time();
	void set_random_start_time(bool value);

	bool fog();
	void set_fog(bool value);

	bool shroud();
	void set_shroud(bool value);

	int turns();
	void set_turns(int value);

	bool skip_mp_replay();
	void set_skip_mp_replay(bool value);

	bool countdown();
	void set_countdown(bool value);
	int countdown_init_time();
	void set_countdown_init_time(int value);
	int countdown_turn_bonus();
	void set_countdown_turn_bonus(int value);
	int countdown_reservoir_time();
	void set_countdown_reservoir_time(int value);
	int countdown_action_bonus();
	void set_countdown_action_bonus(int value);

	int village_gold();
	void set_village_gold(int value);

	int xp_modifier();
	void set_xp_modifier(int value);

	int era();
	void set_era(int value);

	int map();
	void set_map(int value);

	bool show_ai_moves();
	void set_show_ai_moves(bool value);

	void set_show_side_colours(bool value);
	bool show_side_colours();

	bool save_replays();
	void set_save_replays(bool value);

	bool delete_saves();
	void set_delete_saves(bool value);

	void set_ask_delete_saves(bool value);
	bool ask_delete_saves();

	void set_autosavemax(int value);
	int autosavemax();

	const int INFINITE_AUTO_SAVES = 61;

	bool show_floating_labels();
	void set_show_floating_labels(bool value);

	bool message_private();
	void set_message_private(bool value);

	bool show_tip_of_day();

	bool show_haloes();
	void set_show_haloes(bool value);


	bool flip_time();
	void set_flip_time(bool value);

	bool upload_log();
	void set_upload_log(bool value);
	const std::string upload_id();

	// Multiplayer functions
	std::string get_chat_timestamp(const time_t& t);
	bool chat_timestamping();
	void set_chat_timestamping(bool value);

	int chat_lines();
	void set_chat_lines(int lines);

	bool compress_saves();

	std::set<std::string> &encountered_units();
	std::set<t_translation::t_terrain> &encountered_terrains();

	std::string client_type();

	std::string clock_format();

	void set_theme(const std::string& theme);
	const std::string theme();

	bool compare_resolutions(const std::pair<int,int>& lhs, const std::pair<int,int>& rhs);

	// Ask for end turn confirmation
	bool yellow_confirm();
	bool green_confirm();
	bool confirm_no_moves();

	// Add all recruitable units as encountered so that information
	// about them are displayed to the user in the help system.
	void encounter_recruitable_units(std::vector<team>& teams);
	// Add all units that exist at the start to the encountered units so
	// that information about them are displayed to the user in the help
	// system.
	void encounter_start_units(unit_map& units);
	// Add all units that are recallable as encountred units.
	void encounter_recallable_units(game_state& gamestate);
	// Add all terrains on the map as encountered terrains.
	void encounter_map_terrain(gamemap& map);
}

#endif
