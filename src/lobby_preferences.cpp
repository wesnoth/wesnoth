/* $Id$ */
/*
   Copyright (C) 2009 by Tomasz Sniatowski <kailoran@gmail.com>
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
	return utils::string_bool(preferences::get("lobby_sounds"), true);
}

void set_lobby_sounds(bool v)
{
	preferences::set("lobby_sounds", v ? "yes" : "no");
}

bool sort_list()
{
	return utils::string_bool(preferences::get("sort_list"), true);
}

void _set_sort_list(bool sort)
{
	preferences::set("sort_list", sort ? "yes" : "no");
}

bool iconize_list()
{
	return utils::string_bool(preferences::get("iconize_list"), true);
}

void _set_iconize_list(bool sort)
{
	preferences::set("iconize_list", sort ? "yes" : "no");
}


bool whisper_friends_only()
{
	return utils::string_bool(preferences::get("lobby_whisper_friends_only"));
}

bool auto_open_whisper_windows()
{
	return utils::string_bool(preferences::get("lobby_auto_open_whisper_windows"), true);
}

bool playerlist_sort_relation()
{
	return utils::string_bool(preferences::get("lobby_playerlist_sort_relation"), true);
}

void set_playerlist_sort_relation(bool v)
{
	return preferences::set("lobby_playerlist_sort_relation", lexical_cast<std::string>(v));
}

bool playerlist_sort_name()
{
	return utils::string_bool(preferences::get("lobby_playerlist_sort_name"), true);
}

void set_playerlist_sort_name(bool v)
{
	return preferences::set("lobby_playerlist_sort_name", lexical_cast<std::string>(v));
}

bool playerlist_group_players()
{
	return utils::string_bool(preferences::get("lobby_playerlist_group_players"), true);
}

void set_playerlist_group_players(bool v)
{
	return preferences::set("lobby_playerlist_group_players", lexical_cast<std::string>(v));
}


bool filter_lobby()
{
	return utils::string_bool(preferences::get("filter_lobby"), false);
}

void set_filter_lobby(bool value)
{
	preferences::set("filter_lobby", value ? "yes" : "no");
}

bool fi_invert()
{
	return utils::string_bool(preferences::get("fi_invert"), false);
}

void set_fi_invert(bool value)
{
	preferences::set("fi_invert", value ? "yes" : "no");
}

bool fi_vacant_slots()
{
	return utils::string_bool(preferences::get("fi_vacant_slots"), false);
}

void set_fi_vacant_slots(bool value)
{
	preferences::set("fi_vacant_slots", value ? "yes" : "no");
}

bool fi_friends_in_game()
{
	return utils::string_bool(preferences::get("fi_friends_in_game"), false);
}

void set_fi_friends_in_game(bool value)
{
	preferences::set("fi_friends_in_game", value ? "yes" : "no");
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
