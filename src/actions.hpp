/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef ACTIONS_H_INCLUDED
#define ACTIONS_H_INCLUDED

class display;
class gamestatus;
class replay;

#include "global.hpp"

#include "map.hpp"
#include "unit.hpp"

#include <deque>
typedef std::map<gamemap::location,unit> units_map;

//this file defines various functions which implement different in-game
//events and commands.
bool can_recruit_on(const gamemap& map, const gamemap::location& leader, const gamemap::location loc);

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
		unit u, gamemap::location& recruit_location,
		display *disp=NULL, bool need_castle=true, bool full_movement=false);

//a structure which defines all the statistics for a potential
//battle that could take place.
struct battle_stats
{
	int attacker_hp, defender_hp;
	int chance_to_hit_attacker, chance_to_hit_defender;
	int damage_attacker_takes, damage_defender_takes;
	int amount_attacker_drains, amount_defender_drains;
	int ndefends, nattacks;
	int attack_with, defend_with;
	bool attacker_plague, defender_plague;
        std::string attacker_plague_type, defender_plague_type;
	bool attacker_slows, defender_slows;
	int rounds;
	bool defender_strikes_first;
	bool attacker_poisons, defender_poisons;
	bool attacker_stones, defender_stones;
};

struct battle_stats_strings
{
	std::string attack_name, defend_name;
	std::string attack_type, defend_type;
	std::string attack_special, defend_special;
	std::string range;
	std::string attack_icon, defend_icon;
	std::vector<std::string> attack_calculations, defend_calculations;
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
battle_stats evaluate_battle_stats(const gamemap& map,
                                   std::vector<team>& teams,
                                   const gamemap::location& attacker,
                                   const gamemap::location& defender,
                                   int attack_with,
                                   units_map& units,
                                   const gamestatus& state,
                                   const game_data& gamedata,
                                   gamemap::TERRAIN attacker_terrain_override = 0,
                                   battle_stats_strings *strings = NULL);


/* Computes the statistics of a battle between an attacker and a
 * defender unit. It is meant as a replacement of battle_stats.
 *
 * Public interface:
 *
 * So far, you initialize the object, call set_attacker_weapon(), set
 * set_defender_weapon() and compute_battle_stats() to obtain the statistics
 * of the battle. This interface is temporary and will soon be replaced
 * with a method that automatically selects the best defender weapon with
 * an heuristic function.
 */
class battle_context
{
public:
	// Structure describing the statistics of a unit involved in the battle.
	struct unit_stats
	{
		bool is_attacker;		// True if the unit is the attacker.
		bool is_poisoned;	  	// True if the unit is poisoned at the beginning of the battle.
		bool is_slowed;	    	// True if the unit is slowed at the beginning of the battle.
		bool slows;				// Attack slows opponent when it hits.
		bool drains;			// Attack drains opponent when it hits.
		bool stones;			// Attack turns opponent to stone when it hits.
		bool poisons;			// Attack poisons opponent when it hits.
		bool backstab_pos;		// True if the attacker is in *position* to backstab the defender (this is used to
								// determine whether to apply the backstab bonus in case the attacker has backstab).
		bool swarm;				// Attack has swarm special.
		bool berserk;			// Berserk special is used, either by this unit or his opponent.
		bool firststrike;		// Attack has firststrike special.

		int hp;					// Hitpoints of the unit at the beginning of the battle.
		int max_hp;	     		// Maximum hitpoints of the unit.
		int chance_to_hit;		// Effective chance to hit (all factors accounted for).
		int damage;   			// Effective damage of the weapon (all factors accounted for, except slow state).
		int slow_damage;		// Effective damage if unit is/becomes slowed.
		int num_blows;			// Effective number of blows, takes swarm into account.
		int swarm_min;			// Minimum number of blows with swarm (equal to num_blows if swarm isn't used).
		int swarm_max;			// Maximum number of blows with swarm (equal to num_blows if swarm isn't used).

		unit_stats(const unit &u, const gamemap::location& u_loc,
				   bool attacking, const attack_type *weapon,
				   const unit &opp, const gamemap::location& opp_loc,
				   const std::map<gamemap::location,unit>& units,
				   const std::vector<team>& teams, const gamestatus& status,
				   const gamemap& map);

		// This method dumps the statistics of a unit on stdout. Remove it eventually.
		void dump() const;
	};

	battle_context(const gamemap& map, const std::vector<team>& teams, const std::map<gamemap::location,unit>& units,
		       const gamestatus& status, const gamemap::location& attacker_loc, const gamemap::location& defender_loc)
			 : map_(map), teams_(teams), units_(units), status_(status),
			   attacker_loc_(attacker_loc), defender_loc_(defender_loc),
			   attacker_weapon_(NULL), defender_weapon_(NULL), attacker_stats_(NULL), defender_stats_(NULL) {}

	~battle_context() { delete attacker_stats_; delete defender_stats_; }

	// Set the attacker weapon.
	void set_attacker_weapon(const attack_type& weapon) { attacker_weapon_ = &weapon; }
	
	// Set the defender weapon.
	void set_defender_weapon(const attack_type& weapon) { defender_weapon_ = &weapon; }

	// This method computes the statistics of the units for the upcoming battle.
	std::pair<unit_stats *,unit_stats *> compute_unit_stats();

private:
	// Map (used to query terrain features that affect combat).
	const gamemap& map_;

	// Teams (used to compute backstab).
	const std::vector<team>& teams_;

	// This location-to-unit map describes the expected location of the
	// units. The attacker and defender positions must match the ones
	// specified in this map. It is used to compute leadership, backstab and
	// the time of day bonus. It is expected that the caller will obtain a
	// copy of the current location-to-unit map of the game and modify it to
	// place the units where they should be when the battle actually takes
	// place.
	const std::map<gamemap::location,unit>& units_;

	// The status of the game (used to compute the time of day bonus).
	const gamestatus& status_;

	const gamemap::location attacker_loc_, defender_loc_;
	const attack_type *attacker_weapon_, *defender_weapon_;

	// Statistics of the units: only valid after compute_battle_stats().
	unit_stats *attacker_stats_, *defender_stats_;
};


//attack: executes an attack.
void attack(display& gui, const gamemap& map,
            std::vector<team>& teams,
            gamemap::location attacker,
            gamemap::location defender,
            int attack_with,
            units_map& units,
            const gamestatus& state,
            const game_data& info,
			bool update_display = true);

//given the location of a village, will return the 0-based index of the team
//that currently owns it, and -1 if it is unowned.
int village_owner(const gamemap::location& loc, const std::vector<team>& teams);

//makes it so the village at the given location is owned by the given
//0-based team number. Returns true if getting the village triggered a mutating event
bool get_village(const gamemap::location& loc, std::vector<team>& teams,
               size_t team_num, const unit_map& units);
bool get_village(const gamemap::location& loc, std::vector<team>& teams,
               size_t team_num, const unit_map& units, int *time_bonus);


//given the 1-based side, will find the leader of that side,
//and return an iterator to the leader
unit_map::iterator find_leader(unit_map& units, int side);

unit_map::const_iterator find_leader(const unit_map& units, int side);

// Resets resting for all units on this side: should be called after calculate_healing().
// FIXME: Try moving this to unit::new_turn, then move it above calculate_healing().
void reset_resting(std::map<gamemap::location,unit>& units, unsigned int side);

//calculates healing for all units for the given side. Should be called
//at the beginning of a side's turn.
void calculate_healing(display& disp, const gamestatus& status, const gamemap& map,
                       units_map& units, unsigned int side,
					   const std::vector<team>& teams, bool update_display);

// Resets resting for all units on this side: should be called after calculate_healing().
// FIXME: Try moving this to unit::new_turn, then move it above calculate_healing().
void reset_resting(std::map<gamemap::location,unit>& units, unsigned int side);

//function which, given the location of a unit that is advancing, and the
//name of the unit it is advancing to, will return the advanced version of
//this unit. (with traits and items retained).
unit get_advanced_unit(const game_data& info,
                  units_map& units,
                  const gamemap::location& loc, const std::string& advance_to);

//function which will advance the unit at loc to 'advance_to'.
//note that 'loc' is not a reference, because if it were a reference, we couldn't
//safely pass in a reference to the item in the map that we're going to delete,
//since deletion would invalidate the reference.
void advance_unit(const game_data& info,
                  units_map& units,
                  gamemap::location loc, const std::string& advance_to);

//function which tests if the unit at loc is currently affected
//by leadership. (i.e. has a higher-level 'leadership' unit next to it).
//if it does, then the location of the leader unit will be returned, otherwise
//gamemap::location::null_location will be returned
//if 'bonus' is not NULL, the % bonus will be stored in it
gamemap::location under_leadership(const units_map& units,
                                   const gamemap::location& loc, int* bonus=NULL);

//checks to see if a side has won, and will throw an end_level_exception
//if one has. Will also remove control of villages from sides  with dead leaders
void check_victory(units_map& units,
                   std::vector<team>& teams);

//gets the time of day at a certain tile. Certain tiles may have a time of
//day that differs from 'the' time of day, if a unit that illuminates is
//in that tile or adjacent.
const time_of_day&  timeofday_at(const gamestatus& status,
                              const units_map& units,
                              const gamemap::location& loc,
			      const gamemap& map);

//returns the amount that a unit's damage should be multiplied by due to
//the current time of day.
int combat_modifier(const gamestatus& status,
			const units_map& units,
			const gamemap::location& loc,
			unit_type::ALIGNMENT alignment,
			const gamemap& map);

//structure which records information to be able to undo a movement
struct undo_action {
	undo_action(unit u,const std::vector<gamemap::location>& rt,int sm,int timebonus=0,int orig=-1)
		: route(rt), starting_moves(sm), original_village_owner(orig), recall_pos(-1), affected_unit(u), countdown_time_bonus(timebonus) {}
	undo_action(unit u,const gamemap::location& loc, int pos)
		: recall_loc(loc), recall_pos(pos), affected_unit(u) {}
	std::vector<gamemap::location> route;
	int starting_moves;
	int original_village_owner;
	gamemap::location recall_loc;
	int recall_pos;
	unit affected_unit;
	int countdown_time_bonus;
	bool is_recall() const { return recall_pos >= 0; }
};

typedef std::deque<undo_action> undo_list;

//function which moves a unit along the sequence of locations given by
//steps. If the unit cannot make it completely along the path this turn,
//a goto order will be set. If move_recorder is not NULL, the move will
//be recorded in it. If undos is not NULL, undo information will be added.
size_t move_unit(display* disp, const game_data& gamedata,
				const gamestatus& status, const gamemap& map,
				unit_map& units, std::vector<team>& teams,
				std::vector<gamemap::location> steps,
				replay* move_recorder, undo_list* undos,
				gamemap::location *next_unit = NULL,
				bool continue_move = false, bool should_clear_shroud=true);

//function which recalculates the fog
void recalculate_fog(const gamemap& map, const gamestatus& status,
		      const game_data& gamedata,
		      unit_map& units, std::vector<team>& teams, int team);

//function which will clear shroud away for the given 0-based team based on
//current unit positions. Returns true if some shroud is actually cleared away.
bool clear_shroud(display& disp, const gamestatus& status,
		const gamemap& map, const game_data& gamedata,
		unit_map& units, std::vector<team>& teams, int team);

//function to apply pending shroud changes in the undo stack.
//it needs tons of parameters because it calls clear_shroud(...) (see above)
void apply_shroud_changes(undo_list& undos, display* disp, const gamestatus& status, const gamemap& map,
	const game_data& gamedata, unit_map& units, std::vector<team>& teams, int team);

//will return true iff the unit at 'loc' has any possible moves it can do
//(including attacking etc).
bool unit_can_move(const gamemap::location& loc, const unit_map& units,
                   const gamemap& map, const std::vector<team>& teams);


namespace victory_conditions {
	void set_victory_when_enemies_defeated(bool on);
	bool victory_when_enemies_defeated();
}

//Function to check if an attack will satisfy the requirements for backstab
//given the location from which the attack will occur, the defending unit
//location, the list of units on the map and the list of teams.
//The defender and opposite units should be in place already. The
//attacking unit doesn't need to be, but if it isn't, an external check should
//be made to make sure the opposite unit isn't also the attacker.
bool backstab_check(const gamemap::location& attacker_loc,
	const gamemap::location& defender_loc,
	const units_map& units, const std::vector<team>& teams);

#endif
