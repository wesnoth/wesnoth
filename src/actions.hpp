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

std::string recruit_unit(const gamemap& map, int team,
                         std::map<gamemap::location,unit>& units,
						 unit& unit, gamemap::location preferred_location,
                         display *disp=NULL, bool need_castle=true);

struct battle_stats
{
	std::string attack_name, defend_name;
	std::string attack_type, defend_type;
	std::string attack_special, defend_special;
	std::string range;
	double chance_to_hit_attacker, chance_to_hit_defender;
	int damage_attacker_takes, damage_defender_takes;
	int amount_attacker_drains, amount_defender_drains;
	int ndefends, nattacks;
	int attack_with, defend_with;
	bool attacker_plague, defender_plague;
};

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

void attack(display& gui, const gamemap& map,
            const gamemap::location& attacker,
            const gamemap::location& defender,
            int attack_with,
            std::map<gamemap::location,unit>& units,
            const gamestatus& state,
            const game_data& info, bool player_is_attacker);

int tower_owner(const gamemap::location& loc, std::vector<team>& teams);

void get_tower(const gamemap::location& loc, std::vector<team>& teams,
               int team_num);

std::map<gamemap::location,unit>::iterator
   find_leader(std::map<gamemap::location,unit>& units, int side);

void calculate_healing(display& disp, const gamemap& map,
                       std::map<gamemap::location,unit>& units, int side);

unit get_advanced_unit(const game_data& info,
                  std::map<gamemap::location,unit>& units,
                  const gamemap::location& loc, const std::string& advance_to);

void advance_unit(const game_data& info,
                  std::map<gamemap::location,unit>& units,
                  const gamemap::location& loc, const std::string& advance_to);

bool under_leadership(const std::map<gamemap::location,unit>& units,
                      const gamemap::location& loc);

void check_victory(std::map<gamemap::location,unit>& units,
                   const std::vector<team>& teams);

//gets the time of day at a certain tile. Certain tiles may have a time of
//day that differs from 'the' time of day, if a unit that brings light is there
const time_of_day&  timeofday_at(const gamestatus& status,
                              const std::map<gamemap::location,unit>& units,
                              const gamemap::location& loc);

double combat_modifier(const gamestatus& status,
                       const std::map<gamemap::location,unit>& units,
					   const gamemap::location& loc,
					   unit_type::ALIGNMENT alignment);

struct undo_action {
	undo_action(const std::vector<gamemap::location>& rt,int sm,int orig=-1)
	       : route(rt), starting_moves(sm), original_village_owner(orig) {}
	std::vector<gamemap::location> route;
	int starting_moves;
	int original_village_owner;
};

typedef std::deque<undo_action> undo_list;

size_t move_unit(display* disp, const gamemap& map,
                 unit_map& units, std::vector<team>& teams,
                 const std::vector<gamemap::location>& steps,
                 replay* move_recorder, undo_list* undos);

bool clear_shroud(display& disp, const gamemap& map, const game_data& gamedata,
                  const unit_map& units, std::vector<team>& teams, int team);

#endif
