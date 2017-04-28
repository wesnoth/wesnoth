/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "replay_helper.hpp"

#include <string>
#include <cassert>
#include "map/location.hpp"
#include "time_of_day.hpp"
#include "resources.hpp"
#include "play_controller.hpp"

config replay_helper::get_recruit(const std::string& type_id, const map_location& loc, const map_location& from)
{
	config val;
	val["type"] = type_id;
	loc.write(val);
	config& leader_position = val.add_child("from");
	from.write(leader_position);

	return val;
}

config replay_helper::get_recall(const std::string& unit_id, const map_location& loc, const map_location& from)
{

	config val;
	val["value"] = unit_id;
	loc.write(val);
	config& leader_position = val.add_child("from");
	from.write(leader_position);

	return val;
}

config replay_helper::get_disband(const std::string& unit_id)
{
	config val;

	val["value"] = unit_id;

	return val;
}


/**
 * Records a move that follows the provided @a steps.
 * This should be the steps to be taken this turn, ending in an
 * apparently-unoccupied (from the moving team's perspective) hex.
 */
config replay_helper::get_movement(const std::vector<map_location>& steps, bool skip_sighted, bool skip_ally_sighted)
{
	assert(!steps.empty());

	config move;
	if(skip_sighted)
	{
		//note, that skip_ally_sighted has no effect if skip_sighted is true
		move["skip_sighted"] = "all";
	}
	else if(skip_ally_sighted && !skip_sighted)
	{
		move["skip_sighted"] = "only_ally";
	}
	else
	{
		//leave it empty
	}
	write_locations(steps, move);

	return move;
}



config replay_helper::get_attack(const map_location& a, const map_location& b,
	int att_weapon, int def_weapon, const std::string& attacker_type_id,
	const std::string& defender_type_id, int attacker_lvl,
	int defender_lvl, const size_t turn, const time_of_day &t)
{

	config move, src, dst;
	a.write(src);
	b.write(dst);

	move.add_child("source",src);
	move.add_child("destination",dst);


	move["weapon"] = att_weapon;
	move["defender_weapon"] = def_weapon;
	move["attacker_type"] = attacker_type_id;
	move["defender_type"] = defender_type_id;
	move["attacker_lvl"] = attacker_lvl;
	move["defender_lvl"] = defender_lvl;
	move["turn"] = int(turn);
	move["tod"] = t.id;
	/*
	add_unit_checksum(a,current_);
	add_unit_checksum(b,current_);
	*/
	return move;
}

/**
 * Records that the player has toggled automatic shroud updates.
 */
config replay_helper::get_auto_shroud(bool turned_on)
{
	config child;
	child["active"] = turned_on;
	return child;
}

/**
 * Records that the player has manually updated fog/shroud.
 */
config replay_helper::get_update_shroud()
{
	return config();
}


config replay_helper::get_init_side()
{
	config init_side;
		init_side["side_number"] = resources::controller->current_side();
	return init_side;
}

config replay_helper::get_event(const std::string& name, const map_location& loc, const map_location*  last_select_loc)
{
	config ev;
	ev["raise"] = name;
	if(loc.valid()) {
		config& source = ev.add_child("source");
		loc.write(source);
	}
	if(last_select_loc != nullptr && last_select_loc->valid())
	{
		config& source = ev.add_child("last_select");
		last_select_loc->write(source);
	}
	return ev;
}

config replay_helper::get_lua_ai(const std::string& lua_code)
{
	config child;
	child["code"] = lua_code;
	return child;
}
