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

#include "lobby_preferences.hpp"
#include "game_preferences.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"

namespace preferences {

bool lobby_sounds()
{
	return preferences::get("lobby_sounds", true);
}

void set_lobby_sounds(bool v)
{
	preferences::set("lobby_sounds", v);
}

bool sort_list()
{
	return preferences::get("sort_list", true);
}

void _set_sort_list(bool sort)
{
	preferences::set("sort_list", sort);
}

bool iconize_list()
{
	return preferences::get("iconize_list", true);
}

void _set_iconize_list(bool sort)
{
	preferences::set("iconize_list", sort);
}


bool whisper_friends_only()
{
	return preferences::get("lobby_whisper_friends_only", false);
}

bool auto_open_whisper_windows()
{
	return preferences::get("lobby_auto_open_whisper_windows", true);
}

bool playerlist_sort_relation()
{
	return preferences::get("lobby_playerlist_sort_relation", true);
}

void set_playerlist_sort_relation(bool v)
{
	return preferences::set("lobby_playerlist_sort_relation", v);
}

bool playerlist_sort_name()
{
	return preferences::get("lobby_playerlist_sort_name", true);
}

void set_playerlist_sort_name(bool v)
{
	return preferences::set("lobby_playerlist_sort_name", v);
}

bool playerlist_group_players()
{
	return preferences::get("lobby_playerlist_group_players", true);
}

void set_playerlist_group_players(bool v)
{
	return preferences::set("lobby_playerlist_group_players", v);
}

} //end namespace preferences
