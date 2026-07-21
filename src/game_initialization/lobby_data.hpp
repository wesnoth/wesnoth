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

#include <vector>
#include <string>

class config;

namespace mp {

/**
 * This class represents the information a client has about another player
 */
struct user_info
{
	explicit user_info(const config& c);

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

	user_state get_state(int selected_game_id) const;
	user_relation get_relation() const;

	std::string name;
	int forum_id;
	int game_id;
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
	std::string scenario_id;
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
	// TODO: what does it do? it doesn't seem to be used.
	bool verified;
	bool password_required;
	bool have_era;
	bool have_all_mods;

	bool has_friends;
	bool has_ignored;
	bool auto_hosted;
	bool game_preset;

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
