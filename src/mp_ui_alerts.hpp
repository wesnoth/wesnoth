/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * This namespace provides handlers which play the sounds / notificaitons
 * for various mp server events, depending on the preference configuration.
 */

#pragma once

#include<string>
#include<vector>

namespace mp_ui_alerts {

	// Functions called when such an event occurs
	void player_joins(bool is_lobby);
	void player_leaves(bool is_lobby);
	void public_message(bool is_lobby, const std::string & sender, const std::string & message);
	void friend_message(bool is_lobby, const std::string & sender, const std::string & message);
	void private_message(bool is_lobby, const std::string & sender, const std::string & message);
	void server_message(bool is_lobby, const std::string & sender, const std::string & message);
	void ready_for_start();
	void game_has_begun();

	void turn_changed(const std::string & player);

	// Functions to calculate what the default preference should be
	bool get_def_pref_sound(const std::string &);
	bool get_def_pref_notif(const std::string &);
	bool get_def_pref_lobby(const std::string &);

	// Note, this list of items must match those ids defined in data/gui/dialogs/mp_alerts_options.cfg
	extern const std::vector<std::string> items;
}
