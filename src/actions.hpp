/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef ACTIONS_H_INCLUDED
#define ACTIONS_H_INCLUDED

#include "display.hpp"
#include "gamestatus.hpp"
#include "map.hpp"
#include "replay.hpp"
#include "unit.hpp"
#include "unit_types.hpp"

#include <deque>
#include <map>
#include <string>

//this file defines various functions which implement different in-game
//events and commands.

//recruit_unit: function which recruits a unit into the game. A
//copy of u will be created and inserted as the new recruited unit.
//If need_castle is true, then the new unit must be on the same castle
//as the leader of the team is on the keep of.
//
//If preferred_location is in a valid location, it will be used, otherwise
//a valid location will be arbitrarily chosen. If disp is not NULL, the
//new unit will be faded in.
//
//If the unit cannot be recruited, then a human-readable message
//describing why not will be returned. On success, the return string is empty
std::string recruit_unit(const gamemap& map, int team, unit_map& units,
						 unit& u, gamemap::location preferred_location,
                         display *disp=NULL, bool need_castle=true, bool full_movement=false);

//a structure which defines all the statistics for a potential
//battle that could take place.
struct battle_stats
{
	std::string attack_name, defend_name;
	std::string attack_type, defend_type;
	std::string attack_special, defend_special;
	std::string range;
	std::string attack_icon, defend_icon;
	int chance_to_hit_attacker, chance_to_hit_defender;
	int damage_attacker_takes, damage_defender_takes;
	int amount_attacker_drains, amount_defender_drains;
	int ndefends, nattacks;
	int attack_with, defend_with;
	bool attacker_plague, defender_plague;
};

//evaluate_battle_stats: a function which, if given an attacker
//and defender, and the number of a weapon to use, will report
//the statistics if that battle were to take place.
//
//attacker_terrain_override allows a different terrain to the
//one currently stood on by the attacker to be used in calculating
//the statistics. This is useful if one wants to look at the
//statistics if an attacker were to attack from one of several
//different locations.
//
//if include_strings is false, then none of the strings in
//battle_stats will be populated, and the function will run
//substantially faster.
battle_stats evaluate_battle_stats(
                   const gamemap& map,
                   const gamemap::location& attacker,
                   const gamemap::location& defender,
				   int attack_with,
				   std::map<gamemap::location,unit>& units,
				   const gamestatus& state,
				   const game_data& info,
				   gamemap::TERRAIN attacker_terrain_override=0,
				   bool include_strings=true);

//attack: executes an attack.
void attack(display& gui, const gamemap& map,
            const gamemap::location& attacker,
            const gamemap::location& defender,
            int attack_with,
            std::map<gamemap::location,unit>& units,
            const gamestatus& state,
            const game_data& info, bool player_is_attacker);

//given the location of a village, will return the 0-based index of the team
//that currently owns it, and -1 if it is unowned.
int tower_owner(const gamemap::location& loc, std::vector<team>& teams);

//makes it so the tower at the given location is owned by the given
//0-based team number
void get_tower(const gamemap::location& loc, std::vector<team>& teams,
               int team_num);

//given the 1-based side, will find the leader of that side,
//and return an iterator to the leader
std::map<gamemap::location,unit>::iterator
   find_leader(std::map<gamemap::location,unit>& units, int side);

//calculates healing for all units for the given side. Should be called
//at the beginning of a side's turn.
void calculate_healing(display& disp, const gamemap& map,
                       std::map<gamemap::location,unit>& units, int side);

//function which, given the location of a unit that is advancing, and the
//name of the unit it is advancing to, will return the advanced version of
//this unit. (with traits and items retained).
unit get_advanced_unit(const game_data& info,
                  std::map<gamemap::location,unit>& units,
                  const gamemap::location& loc, const std::string& advance_to);

//function which will advance the unit at loc to 'advance_to'.
//note that 'loc' is not a reference, because if it were a reference, we couldn't
//safely pass in a reference to the item in the map that we're going to delete,
//since deletion would invalidate the reference.
void advance_unit(const game_data& info,
                  std::map<gamemap::location,unit>& units,
                  gamemap::location loc, const std::string& advance_to);

//function which returns true iff the unit at loc is currently affected
//by leadership. (i.e. has a higher-level 'leadership' unit next to it)
bool under_leadership(const std::map<gamemap::location,unit>& units,
                      const gamemap::location& loc);

//checks to see if a side has won, and will throw an end_level_exception
//if one has.
void check_victory(std::map<gamemap::location,unit>& units,
                   const std::vector<team>& teams);

//gets the time of day at a certain tile. Certain tiles may have a time of
//day that differs from 'the' time of day, if a unit that illuminates is
//in that tile or adjacent.
const time_of_day&  timeofday_at(const gamestatus& status,
                              const std::map<gamemap::location,unit>& units,
                              const gamemap::location& loc);

//returns the amount that a unit's damage should be multiplied by due to
//the current time of day.
int combat_modifier(const gamestatus& status,
                    const std::map<gamemap::location,unit>& units,
					const gamemap::location& loc,
					unit_type::ALIGNMENT alignment);

//structure which records information to be able to undo a movement
struct undo_action {
	undo_action(const std::vector<gamemap::location>& rt,int sm,int orig=-1)
	       : route(rt), starting_moves(sm), original_village_owner(orig) {}
	std::vector<gamemap::location> route;
	int starting_moves;
	int original_village_owner;
};

typedef std::deque<undo_action> undo_list;

//function which moves a unit along the sequence of locations given by
//steps. If the unit cannot make it completely along the path this turn,
//a goto order will be set. If move_recorder is not NULL, the move will
//be recorded in it. If undos is not NULL, undo information will be added.
size_t move_unit(display* disp, const game_data& gamedata, const gamemap& map,
                 unit_map& units, std::vector<team>& teams,
                 const std::vector<gamemap::location>& steps,
                 replay* move_recorder, undo_list* undos);

//function which will clear shroud away for the given 0-based team based on
//current unit positions. Returns true if some shroud is actually cleared away.
bool clear_shroud(display& disp, const gamemap& map, const game_data& gamedata,
                  const unit_map& units, std::vector<team>& teams, int team);

//will return true iff the unit at 'loc' has any possible moves it can do
//(including attacking etc).
bool unit_can_move(const gamemap::location& loc, const unit_map& units,
                   const gamemap& map, const std::vector<team>& teams);

#endif
