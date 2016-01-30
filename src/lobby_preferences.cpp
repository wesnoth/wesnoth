/*
   Copyright (C) 2009 - 2016 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "lobby_preferences.hpp"
#include "game_preferences.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"

namespace preferences {

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

bool filter_lobby()
{
	return preferences::get("filter_lobby", false);
}

void set_filter_lobby(bool value)
{
	preferences::set("filter_lobby", value);
}

bool fi_invert()
{
	return preferences::get("fi_invert", false);
}

void set_fi_invert(bool value)
{
	preferences::set("fi_invert", value);
}

bool fi_vacant_slots()
{
	return preferences::get("fi_vacant_slots", false);
}

void set_fi_vacant_slots(bool value)
{
	preferences::set("fi_vacant_slots", value);
}

bool fi_friends_in_game()
{
	return preferences::get("fi_friends_in_game", false);
}

void set_fi_friends_in_game(bool value)
{
	preferences::set("fi_friends_in_game", value);
}

std::string fi_text()
{
	return preferences::get("fi_text");
}

void set_fi_text(const std::string& search_string)
{
	preferences::set("fi_text", search_string);
}

} //end namespace preferences
