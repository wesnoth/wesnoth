/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
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

#ifndef INCL_MP_UI_SOUNDS_HPP_
#define INCL_MP_UI_SOUNDS_HPP_

#include<string>
#include<vector>

namespace mp_ui_sounds {

	// Functions called when such an event occurs
	void player_enters(bool is_lobby);
	void player_leaves(bool is_lobby);
	void public_message(bool is_lobby);
	void friend_message(bool is_lobby);
	void private_message(bool is_lobby);
	void server_message(bool is_lobby);
	void ready_for_start();
	void game_has_begun();

	// Functions to calculate what the default preference should be
	bool get_def_pref_sound(const std::string &);
	bool get_def_pref_notif(const std::string &);
	bool get_def_pref_lobby(const std::string &);

	// Note, this list of items must match those ids defined in data/gui/dialogs/lobby_sounds_options.cfg
	extern const std::vector<std::string> items;
}

#endif
