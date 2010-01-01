/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef LOBBY_PREFERENCES_HPP_INCLUDED
#define LOBBY_PREFERENCES_HPP_INCLUDED

#include <string>

namespace preferences {

	bool lobby_sounds();
	void set_lobby_sounds(bool v);

	bool sort_list();
	void _set_sort_list(bool show);

	bool iconize_list();
	void _set_iconize_list(bool show);

	bool whisper_friends_only();
	void set_whisper_friends_only(bool v);

	bool auto_open_whisper_windows();
	void set_auto_open_whisper_windows(bool v);

	bool playerlist_sort_relation();
	void set_playerlist_sort_relation(bool v);

	bool playerlist_sort_name();
	void set_playerlist_sort_name(bool v);

	bool playerlist_group_players();
	void set_playerlist_group_players(bool v);
} //end namespace preferences


#endif
