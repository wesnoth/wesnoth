/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef DIALOGS_H_INCLUDED
#define DIALOGS_H_INCLUDED

class attack_type;
class config;
class display;
class game_display;
class unit;
class unit_map;
class unit_type;
class terrain_type;
class twesnothd_connection;
#include "map/location.hpp"
#include "construct_dialog.hpp"
#include "units/ptr.hpp"
#include "ai/lua/aspect_advancements.hpp"

#include <boost/shared_ptr.hpp>
namespace dialogs {


/**
 * Lets the user to select a unit advancement. This should always be used
 * from WML events, advance_unit can only be used safely for normal levels.
 */
int advance_unit_dialog(const map_location &loc);

/**
 * Actually levels a unit up. This is the other part of the low-level
 * interface to the advancing code (along with advance_unit_dialog). This needs
 * to be used to implement advances from any nonstandard situation. It does
 * not add a replay.
 */
bool animate_unit_advancement(const map_location &loc, size_t choice, const bool &fire_event = true, const bool animate = true);

void show_objectives(const std::string& scenarioname, const std::string &objectives);

void show_unit_list(display& gui);

bool network_receive_dialog(CVideo& video, const std::string& msg, config& cfg, twesnothd_connection& wesnothd_connection);
std::unique_ptr<twesnothd_connection> network_connect_dialog(CVideo& video, const std::string& msg, const std::string& hostname, int port);

} //end namespace dialogs

#endif
