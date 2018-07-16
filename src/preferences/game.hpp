/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

class game_board;
class gamemap;
class team;
class unit_map;

#include "preferences/general.hpp"
#include "game_config.hpp"

#include "serialization/compression.hpp"

#include <set>
#include <vector>

namespace preferences {

class acquaintance;

	struct manager
	{
		manager();
		~manager();

		base_manager base;
	};

	bool is_authenticated();
	void parse_admin_authentication(const std::string& sender, const std::string& message);

	/**
	 * Used to reset is_authenticated flag after disconnecting.
	 *
	 * @todo FIXME: is_authenticated shouldn't be a preferences function. Also,
	 *              the name is misleading.
	 */
	struct admin_authentication_reset
	{
		/**
		 * Default constructor, defined out of line to work around a warning in
		 * gcc 4.5.2
		 */
		admin_authentication_reset();
		~admin_authentication_reset();
	};

	bool parse_should_show_lobby_join(const std::string& sender, const std::string& message);
	int lobby_joins();
	void _set_lobby_joins(int show);
	enum LOBBY_JOINS { SHOW_NONE, SHOW_FRIENDS, SHOW_ALL };

	const std::map<std::string, acquaintance> & get_acquaintances();
	std::map<std::string, std::string> get_acquaintances_nice(const std::string& filter);
	std::pair<preferences::acquaintance*, bool> add_acquaintance(const std::string& nick, const std::string& mode, const std::string& notes);
	void add_completed_campaign(const std::string &campaign_id, const std::string &difficulty_level);
	bool remove_acquaintance(const std::string& nick);
	bool is_friend(const std::string& nick);
	bool is_ignored(const std::string& nick);
	bool is_campaign_completed(const std::string& campaign_id);
	bool is_campaign_completed(const std::string& campaign_id, const std::string &difficulty_level);

	const std::vector<game_config::server_info>& server_list();

	std::string network_host();
	void set_network_host(const std::string& host);

	std::string campaign_server();
	void set_campaign_server(const std::string& host);

	bool turn_dialog();
	void set_turn_dialog(bool ison);

	bool enable_whiteboard_mode_on_start();
	void set_enable_whiteboard_mode_on_start(bool value);

	bool hide_whiteboard();
	void set_hide_whiteboard(bool value);

	bool show_combat();

	bool allow_observers();
	void set_allow_observers(bool value);

	bool registered_users_only();
	void set_registered_users_only(bool value);

	bool shuffle_sides();
	void set_shuffle_sides(bool value);

	std::string random_faction_mode();
	void set_random_faction_mode(const std::string & value);

	bool use_map_settings();
	void set_use_map_settings(bool value);

	int mp_server_warning_disabled();
	void set_mp_server_warning_disabled(int value);

	void set_mp_server_program_name(const std::string&);
	std::string get_mp_server_program_name();

	bool random_start_time();
	void set_random_start_time(bool value);

	bool fog();
	void set_fog(bool value);

	bool shroud();
	void set_shroud(bool value);

	int turns();
	void set_turns(int value);

	const config& options();
	void set_options(const config& values);

	bool skip_mp_replay();
	void set_skip_mp_replay(bool value);

	bool blindfold_replay();
	void set_blindfold_replay(bool value);

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

	int village_support();
	void set_village_support(int value);

	int xp_modifier();
	void set_xp_modifier(int value);

	std::string era();
	void set_era(const std::string& value);

	std::string level();
	void set_level(const std::string& value);
	int level_type();
	void set_level_type(int value);

	const std::vector<std::string>& modifications(bool mp=true);
	void set_modifications(const std::vector<std::string>& value, bool mp=true);

	bool skip_ai_moves();
	void set_skip_ai_moves(bool value);

	void set_show_side_colors(bool value);
	bool show_side_colors();

	bool save_replays();
	void set_save_replays(bool value);

	bool delete_saves();
	void set_delete_saves(bool value);

	void set_ask_delete_saves(bool value);
	bool ask_delete_saves();

    void set_interrupt_when_ally_sighted(bool value);
    bool interrupt_when_ally_sighted();

	void set_autosavemax(int value);
	int autosavemax();

	const int INFINITE_AUTO_SAVES = 61;

	bool show_floating_labels();
	void set_show_floating_labels(bool value);

	bool message_private();
	void set_message_private(bool value);

	// Multiplayer functions
	std::string get_chat_timestamp(const std::time_t& t);
	bool chat_timestamping();
	void set_chat_timestamping(bool value);

	int chat_lines();
	void set_chat_lines(int lines);

	int chat_message_aging();
	void set_chat_message_aging(const int aging);

	bool show_all_units_in_help();
	void set_show_all_units_in_help(bool value);

	compression::format save_compression_format();

	std::set<std::string> &encountered_units();
	std::set<t_translation::terrain_code> &encountered_terrains();

	std::string custom_command();
	void set_custom_command(const std::string& command);

	std::vector<std::string>* get_history(const std::string& id);

	void set_theme(const std::string& theme);
	std::string theme();

	// Ask for end turn confirmation
	bool yellow_confirm();
	bool green_confirm();
	bool confirm_no_moves();

	// Add all recruitable units as encountered so that information
	// about them are displayed to the user in the help system.
	void encounter_recruitable_units(const std::vector<team>& teams);
	// Add all units that exist at the start to the encountered units so
	// that information about them are displayed to the user in the help
	// system.
	void encounter_start_units(const unit_map& units);
	// Add all units that are recallable as encountered units.
	void encounter_recallable_units(std::vector<team>& teams);
	// Add all terrains on the map as encountered terrains.
	void encounter_map_terrain(const gamemap& map);

	// Calls all of the above functions on the current game board
	void encounter_all_content(const game_board & gb);

class acquaintance {
public:

	acquaintance()
	{
	}

	explicit acquaintance(const config& cfg)
	{
		load_from_config(cfg);
	}

	acquaintance(
			  const std::string& nick
			, const std::string& status
			, const std::string& notes)
		: nick_(nick)
		, status_(status)
		, notes_(notes)
	{

	}

	void load_from_config(const config& cfg);

	const std::string& get_nick() const { return nick_; }
	const std::string& get_status() const { return status_; }
	const std::string& get_notes() const { return notes_; }

	void save(config& cfg);

private:

	/** acquaintance's MP nick */
	std::string nick_;

	/**status (e.g., "friend", "ignore") */
	std::string status_;

	/** notes on the acquaintance */
	std::string notes_;

};

}
