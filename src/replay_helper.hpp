/*
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef REPLAY_HELPER_H_INCLUDED
#define REPLAY_HELPER_H_INCLUDED


#include "config.hpp"
#include <string>
struct map_location;
struct time_of_day;

class replay_helper
{
public:
	static config get_recruit(const std::string& type_id, const map_location& loc, const map_location& from);

	static config get_recall(const std::string& unit_id, const map_location& loc, const map_location& from);

	static config get_disband(const std::string& unit_id);
	//TODO: add some additional checkup (unit checksum) here.
	static config get_movement(const std::vector<map_location>& steps, bool skipsighed, bool skip_ally_sighted);

	static config get_attack(const map_location& a, const map_location& b,
		int att_weapon, int def_weapon, const std::string& attacker_type_id,
		const std::string& defender_type_id, int attacker_lvl,
		int defender_lvl, const size_t turn, const time_of_day &t);

	static config get_auto_shroud(bool turned_on);

	static config get_update_shroud();

	static config get_init_side();
	/*
		
	*/
	static config get_event(const std::string& name, const map_location& loc, const map_location*  last_select_loc);

	static config get_lua_ai(const std::string& lua_code);

};

#endif
