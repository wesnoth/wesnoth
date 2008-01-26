/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file actions.hpp
//! Various functions which implement in-game events and commands.

#ifndef ACTIONS_H_INCLUDED
#define ACTIONS_H_INCLUDED

class display;
class gamestatus;
class game_display;
class replay;
struct combatant;
class unit;

class attack_type;
class team;
class game_data;

#include "global.hpp"
#include "map.hpp"
#include "unit.hpp"
#include "unit_map.hpp"

#include <deque>
#include <sstream>

#define RECRUIT_POS -2

bool can_recruit_on(const gamemap& map, const gamemap::location& leader, const gamemap::location loc);

struct end_level_exception;

//! Function which recruits a unit into the game.
// A copy of u will be created and inserted as the new recruited unit.
// If need_castle is true, then the new unit must be on the same castle
// as the leader of the team is on the keep of.
//
// If preferred_location is in a valid location, it will be used,
// otherwise a valid location will be arbitrarily chosen.
// If disp is not NULL, the new unit will be faded in.
//
// If the unit cannot be recruited, then a human-readable message
// describing the reason will be returned.
// On success, the return string is empty.
std::string recruit_unit(const gamemap& map, const int side, unit_map& units,
		unit u, gamemap::location& recruit_location,const bool show=false,
		const bool need_castle=true, const bool full_movement=false,
		const bool wml_recall=false);

//! Computes the statistics of a battle between an attacker and a defender unit.
class battle_context
{
public:
	//! Structure describing the statistics of a unit involved in the battle.
	struct unit_stats
	{
		const attack_type *weapon;	// The weapon used by the unit to attack the opponent, or NULL if there is none.
		int attack_num;			// Index into unit->attacks() or -1 for none.
		bool is_attacker;		// True if the unit is the attacker.
		bool is_poisoned;		// True if the unit is poisoned at the beginning of the battle.
		bool is_slowed;			// True if the unit is slowed at the beginning of the battle.
		bool slows;				// Attack slows opponent when it hits.
		bool drains;			// Attack drains opponent when it hits.
		bool stones;			// Attack turns opponent to stone when it hits.
		bool plagues;			// Attack turns opponent into a zombie when fatal.
		bool poisons;			// Attack poisons opponent when it hits.
		bool backstab_pos;		// True if the attacker is in *position* to backstab the defender (this is used to
								// determine whether to apply the backstab bonus in case the attacker has backstab).
		bool swarm;				// Attack has swarm special.
		bool firststrike;		// Attack has firststrike special.

		unsigned int rounds;	// Berserk special can force us to fight more than one round.
		unsigned int hp;		// Hitpoints of the unit at the beginning of the battle.
		unsigned int max_hp;	// Maximum hitpoints of the unit.
		unsigned int chance_to_hit;	// Effective chance to hit as a percentage (all factors accounted for).
		int damage;				// Effective damage of the weapon (all factors accounted for).
		int slow_damage;		// Effective damage if unit becomes slowed (== damage, if already slowed)
		unsigned int num_blows;	// Effective number of blows, takes swarm into account.
		unsigned int swarm_min;	// Minimum number of blows with swarm (equal to num_blows if swarm isn't used).
		unsigned int swarm_max;	// Maximum number of blows with swarm (equal to num_blows if swarm isn't used).

		std::string plague_type; // The plague type used by the attack, if any.

		unit_stats(const unit &u, const gamemap::location& u_loc,
				   int u_attack_num, bool attacking,
				   const unit &opp, const gamemap::location& opp_loc,
				   const attack_type *opp_weapon,
				   const unit_map& units,
				   const std::vector<team>& teams,
				   const gamestatus& status, const gamemap& map, const game_data& gamedata);
		~unit_stats();

		//! Dumps the statistics of a unit on stdout. Remove it eventually.
		void dump() const;
	};

	// If no attacker_weapon is given, we select the best one,
	// based on harm_weight (1.0 means 1 hp lost counters 1 hp damage,
	// 0.0 means we ignore harm weight).
	// prev_def is for predicting multiple attacks against a defender.
	battle_context(const gamemap& map, const std::vector<team>& teams, const unit_map& units,
				   const gamestatus& status, const game_data& gamedata,
				   const gamemap::location& attacker_loc, const gamemap::location& defender_loc,
				   int attacker_weapon = -1, int defender_weapon = -1, double aggression = 0.0, const combatant *prev_def = NULL);

	// Used by the AI which caches unit_stats
	battle_context(const unit_stats &att, const unit_stats &def);

	battle_context(const battle_context &other);
	~battle_context();

	battle_context& operator=(const battle_context &other);

	//! This method returns the statistics of the attacker.
	const unit_stats& get_attacker_stats() const { return *attacker_stats_; }

	//! This method returns the statistics of the defender.
	const unit_stats& get_defender_stats() const { return *defender_stats_; }

	//! Get the simulation results.
	const combatant &get_attacker_combatant(const combatant *prev_def = NULL);
	const combatant &get_defender_combatant(const combatant *prev_def = NULL);

	//! Given this harm_weight, is this attack better than that?
	bool better_attack(class battle_context &that, double harm_weight);

private:
	bool better_combat(const combatant &us_a, const combatant &them_a,
					   const combatant &us_b, const combatant &them_b,
					   double harm_weight);

	int choose_attacker_weapon(const unit &attacker, const unit &defender,
								const gamemap& map, const std::vector<team>& teams, const unit_map& units,
								const gamestatus& status, const game_data& gamedata,
								const gamemap::location& attacker_loc, const gamemap::location& defender_loc,
								double harm_weight, int *defender_weapon, const combatant *prev_def);

	int choose_defender_weapon(const unit &attacker, const unit &defender, unsigned attacker_weapon,
							   const gamemap& map, const std::vector<team>& teams, const unit_map& units,
							   const gamestatus& status, const game_data& gamedata,
							   const gamemap::location& attacker_loc, const gamemap::location& defender_loc, const combatant *prev_def);

	// Statistics of the units.
	unit_stats *attacker_stats_, *defender_stats_;

	// Outcome of simulated fight.
	combatant *attacker_combatant_, *defender_combatant_;
};

//! Executes an attack.
class attack {
	public:
	    attack(game_display& gui, const gamemap& map,
            std::vector<team>& teams,
            gamemap::location attacker,
            gamemap::location defender,
            int attack_with,
            int defend_with,
            unit_map& units,
            const gamestatus& state,
            const game_data& info,
			bool update_display = true);
		~attack();
	private:
		class attack_end_exception {};
		void fire_event(const std::string& n);
		void refresh_bc();
		game_display& gui_;
		const gamemap& map_;
		std::vector<team>& teams_;
		gamemap::location attacker_;
		gamemap::location defender_;
		int attack_with_;
		int defend_with_;
		unit_map& units_;
		const gamestatus& state_;
		const game_data& info_;
		unit_map::iterator a_,d_; // attacker and defender
		std::string a_id_, d_id_;
		std::stringstream errbuf_;
		battle_context* bc_;
		const battle_context::unit_stats* a_stats_;
		const battle_context::unit_stats* d_stats_;
		int orig_attacks_,orig_defends_;
		int n_attacks_,n_defends_;
		int attacker_cth_,defender_cth_;
		int attacker_damage_,defender_damage_;
		int attackerxp_,defenderxp_;

		bool update_display_;
		bool OOS_error_;
		end_level_exception* delayed_exception;

};

//! Given the location of a village, will return the 0-based index
//! of the team that currently owns it, and -1 if it is unowned.
int village_owner(const gamemap::location& loc, const std::vector<team>& teams);

//! Makes it so the village at the given location
//! is owned by the given 0-based team number.
//! Returns true if getting the village triggered a mutating event.
bool get_village(const gamemap::location& loc, game_display& disp,
                 std::vector<team>& teams, size_t team_num,
                 const unit_map& units, int *time_bonus = NULL);

//! Given the 1-based side, will find the leader of that side,
//! and return an iterator to the leader
unit_map::iterator find_leader(unit_map& units, int side);

unit_map::const_iterator find_leader(const unit_map& units, int side);

//! Resets resting for all units on this side: should be called after calculate_healing().
//! @todo FIXME: Try moving this to unit::new_turn, then move it above calculate_healing().
void reset_resting(unit_map& units, unsigned int side);

//! Calculates healing for all units for the given side.
//! Should be called at the beginning of a side's turn.
void calculate_healing(game_display& disp, const gamemap& map,
                       unit_map& units, unsigned int side,
					   const std::vector<team>& teams, bool update_display);

//! Function which, given the location of a unit that is advancing,
//! and the name of the unit it is advancing to,
//! Will return the advanced version of this unit.
//! (with traits and items retained).
unit get_advanced_unit(const game_data& info,
                  unit_map& units,
                  const gamemap::location& loc, const std::string& advance_to);

//! Function which will advance the unit at loc to 'advance_to'.
//  Note that 'loc' is not a reference, because if it were a reference,
//  we couldn't safely pass in a reference to the item in the map
//  that we're going to delete, since deletion would invalidate the reference.
void advance_unit(const game_data& info,
                  unit_map& units,
                  gamemap::location loc, const std::string& advance_to);

//! function which tests if the unit at loc is currently affected by leadership.
//! (i.e. has a higher-level 'leadership' unit next to it).
//! If it does, then the location of the leader unit will be returned,
//! Otherwise gamemap::location::null_location will be returned.
//! If 'bonus' is not NULL, the % bonus will be stored in it.
gamemap::location under_leadership(const unit_map& units,
                                   const gamemap::location& loc, int* bonus=NULL);

//! Checks to see if a side has won, and will throw
//! an end_level_exception if one has.
//! Will also remove control of villages from sides with dead leaders.
void check_victory(unit_map& units, std::vector<team>& teams, display& disp);

//! Gets the time of day at a certain tile.
//! Certain tiles may have a time of day that differs
//! from 'the' time of day, if a unit that illuminates
//! is in that tile or adjacent.
time_of_day timeofday_at(const gamestatus& status,
                              const unit_map& units,
                              const gamemap::location& loc,
			      const gamemap& map);

//! Returns the amount that a unit's damage should be multiplied by
//! due to the current time of day.
int combat_modifier(const gamestatus& status,
			const unit_map& units,
			const gamemap::location& loc,
			unit_type::ALIGNMENT alignment,
			bool is_fearless,
			const gamemap& map);

//! Records information to be able to undo a movement.
struct undo_action {
	undo_action(unit u, 
		const std::vector<gamemap::location>& rt, 
		int sm, int timebonus = 0, int orig = -1) :
			route(rt), 
			starting_moves(sm), 
			original_village_owner(orig),
			recall_loc(),
			recall_pos(-1), 
			affected_unit(u), 
			countdown_time_bonus(timebonus) 
			{}

	undo_action(const unit& u, const gamemap::location& loc, const int pos) :
		route(),
		starting_moves(),
		original_village_owner(),
		recall_loc(loc), 
		recall_pos(pos), 
		affected_unit(u), 
		countdown_time_bonus(1) 
		{}

	std::vector<gamemap::location> route;
	int starting_moves;
	int original_village_owner;
	gamemap::location recall_loc;
	int recall_pos; // set to RECRUIT_POS for an undo-able recruit
	unit affected_unit;
	int countdown_time_bonus;
	bool is_recall() const { return recall_pos >= 0; }
	bool is_recruit() const { return recall_pos == RECRUIT_POS; }
};

typedef std::deque<undo_action> undo_list;

//! function which moves a unit along the sequence of locations given by steps.
//! If the unit cannot make it completely along the path this turn,
//! a goto order will be set.
//! If move_recorder is not NULL, the move will be recorded in it.
//! If undos is not NULL, undo information will be added.
size_t move_unit(game_display* disp, const game_data& gamedata,
				const gamestatus& status, const gamemap& map,
				unit_map& units, std::vector<team>& teams,
				std::vector<gamemap::location> steps,
				replay* move_recorder, undo_list* undos,
				gamemap::location *next_unit = NULL,
				bool continue_move = false, bool should_clear_shroud=true);

//! Function which recalculates the fog.
void recalculate_fog(const gamemap& map, const gamestatus& status,
		      const game_data& gamedata,
		      unit_map& units, std::vector<team>& teams, int team);

//! Function which will clear shroud away for the given 0-based team
//! based on current unit positions.
//! Returns true if some shroud is actually cleared away.
bool clear_shroud(game_display& disp, const gamestatus& status,
		const gamemap& map, const game_data& gamedata,
		unit_map& units, std::vector<team>& teams, int team);

//! Function to apply pending shroud changes in the undo stack.
//! It needs tons of parameters because it calls clear_shroud(...) (see above)
void apply_shroud_changes(undo_list& undos, game_display* disp, const gamestatus& status, const gamemap& map,
	const game_data& gamedata, unit_map& units, std::vector<team>& teams, int team);

//! Will return true iff the unit at 'loc' has any possible moves
//! it can do (including attacking etc).
bool unit_can_move(const gamemap::location& loc, const unit_map& units,
                   const gamemap& map, const std::vector<team>& teams);


namespace victory_conditions {
	void set_victory_when_enemies_defeated(const bool on);
	void set_carryover_percentage(const int percentage);
	void set_carryover_add(const bool add);
}

//! Function to check if an attack will satisfy the requirements for backstab.
//! Input:
//! - the location from which the attack will occur,
//! - the defending unit location,
//! - the list of units on the map and
//! - the list of teams.
//! The defender and opposite units should be in place already.
//! The attacking unit doesn't need to be, but if it isn't,
//! an external check should be made to make sure the opposite unit
//! isn't also the attacker.
bool backstab_check(const gamemap::location& attacker_loc,
	const gamemap::location& defender_loc,
	const unit_map& units, const std::vector<team>& teams);

#endif
