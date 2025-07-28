/*
	Copyright (C) 2020 - 2025
	by Steve Cotton <steve@octalot.co.uk>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "units/orb_status.hpp"
#include "preferences/preferences.hpp"

bool orb_status_helper::prefs_show_orb(orb_status os)
{
	switch(os) {
	case orb_status::unmoved:
		return prefs::get().show_unmoved_orb();
	case orb_status::moved:
		return prefs::get().show_moved_orb();
	case orb_status::disengaged:
		return prefs::get().show_disengaged_orb();
	case orb_status::partial:
		return prefs::get().show_partial_orb();
	case orb_status::allied:
		return prefs::get().show_ally_orb();
	case orb_status::enemy:
		return prefs::get().show_enemy_orb();
	default:
		assert(!"expected to handle all the enum values");
		return false;
	}
}

std::string orb_status_helper::get_orb_color(orb_status os)
{
	switch(os) {
	case orb_status::unmoved:
		return prefs::get().unmoved_color();
	case orb_status::moved:
		return prefs::get().moved_color();
	case orb_status::disengaged:
		[[fallthrough]]; // use partial_color() for any context that wants a single color
	case orb_status::partial:
		return prefs::get().partial_color();
	case orb_status::allied:
		return prefs::get().allied_color();
	case orb_status::enemy:
		return prefs::get().enemy_color();
	default:
		assert(!"expected to handle all the enum values");
		return {};
	}
}
