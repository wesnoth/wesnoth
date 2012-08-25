/*
   Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Various functions that implement attacks and attack calculations.
 * Unit advancements are also included, as they usually occur as a
 * result of combat.
 */

#ifndef ACTIONS_ATTACK_H_INCLUDED
#define ACTIONS_ATTACK_H_INCLUDED

struct combatant;
struct map_location;
class  team;
class  unit;
class  unit_map;

#include "../unit_types.hpp"

#include <vector>


/** Structure describing the statistics of a unit involved in the battle. */
struct battle_context_unit_stats
{
	const attack_type *weapon;	/**< The weapon used by the unit to attack the opponent, or NULL if there is none. */
	int attack_num;			/**< Index into unit->attacks() or -1 for none. */
	bool is_attacker;		/**< True if the unit is the attacker. */
	bool is_poisoned;		/**< True if the unit is poisoned at the beginning of the battle. */
	bool is_slowed;			/**< True if the unit is slowed at the beginning of the battle. */
	bool slows;				/**< Attack slows opponent when it hits. */
	bool drains;			/**< Attack drains opponent when it hits. */
	bool petrifies;			/**< Attack petrifies opponent when it hits. */
	bool plagues;			/**< Attack turns opponent into a zombie when fatal. */
	bool poisons;			/**< Attack poisons opponent when it hits. */
	bool backstab_pos;		/**<
		                         * True if the attacker is in *position* to backstab the defender (this is used to
		                         * determine whether to apply the backstab bonus in case the attacker has backstab).
		                         */
	bool swarm;				/**< Attack has swarm special. */
	bool firststrike;		/**< Attack has firststrike special. */
	unsigned int experience, max_experience;
	unsigned int level;

	unsigned int rounds;	/**< Berserk special can force us to fight more than one round. */
	unsigned int hp;		/**< Hitpoints of the unit at the beginning of the battle. */
	unsigned int max_hp;	/**< Maximum hitpoints of the unit. */
	unsigned int chance_to_hit;	/**< Effective chance to hit as a percentage (all factors accounted for). */
	int damage;				/**< Effective damage of the weapon (all factors accounted for). */
	int slow_damage;		/**< Effective damage if unit becomes slowed (== damage, if already slowed) */
	int drain_percent;		/**< Percentage of damage recovered as health */
	int drain_constant;		/**< Base HP drained regardless of damage dealt */
	unsigned int num_blows;	/**< Effective number of blows, takes swarm into account. */
	unsigned int swarm_min;	/**< Minimum number of blows with swarm (equal to num_blows if swarm isn't used). */
	unsigned int swarm_max;	/**< Maximum number of blows with swarm (equal to num_blows if swarm isn't used). */

	std::string plague_type; /**< The plague type used by the attack, if any. */

	battle_context_unit_stats(const unit &u, const map_location& u_loc,
		   int u_attack_num, bool attacking,
		   const unit &opp, const map_location& opp_loc,
		   const attack_type *opp_weapon,
		   const unit_map& units);
	~battle_context_unit_stats();

	/** Dumps the statistics of a unit on stdout. Remove it eventually. */
	void dump() const;
};

/** Computes the statistics of a battle between an attacker and a defender unit. */
class battle_context
{
public:

	/**
	 * If no attacker_weapon is given, we select the best one,
	 * based on harm_weight (1.0 means 1 hp lost counters 1 hp damage,
	 * 0.0 means we ignore harm weight).
	 * prev_def is for predicting multiple attacks against a defender.
	 */
	battle_context(const unit_map &units,
	               const map_location& attacker_loc, const map_location& defender_loc,
	               int attacker_weapon = -1, int defender_weapon = -1,
	               double aggression = 0.0, const combatant *prev_def = NULL,
	               const unit* attacker_ptr=NULL);

	/** Used by the AI which caches battle_context_unit_stats */
	battle_context(const battle_context_unit_stats &att, const battle_context_unit_stats &def);

	battle_context(const battle_context &other);
	~battle_context();

	battle_context& operator=(const battle_context &other);

	/** This method returns the statistics of the attacker. */
	const battle_context_unit_stats& get_attacker_stats() const { return *attacker_stats_; }

	/** This method returns the statistics of the defender. */
	const battle_context_unit_stats& get_defender_stats() const { return *defender_stats_; }

	/** Get the simulation results. */
	const combatant &get_attacker_combatant(const combatant *prev_def = NULL);
	const combatant &get_defender_combatant(const combatant *prev_def = NULL);

	/** Given this harm_weight, is this attack better than that? */
	bool better_attack(class battle_context &that, double harm_weight);

private:
	bool better_combat(const combatant &us_a, const combatant &them_a,
					   const combatant &us_b, const combatant &them_b,
					   double harm_weight);

	int choose_attacker_weapon(const unit &attacker, const unit &defender,
	                           const unit_map& units,
	                           const map_location& attacker_loc, const map_location& defender_loc,
	                           double harm_weight, int *defender_weapon,
	                           const combatant *prev_def);

	int choose_defender_weapon(const unit &attacker, const unit &defender,
	                           unsigned attacker_weapon, const unit_map& units,
							   const map_location& attacker_loc,
	                           const map_location& defender_loc,
	                           const combatant *prev_def);

	/** Statistics of the units. */
	battle_context_unit_stats *attacker_stats_, *defender_stats_;

	/** Outcome of simulated fight. */
	combatant *attacker_combatant_, *defender_combatant_;
};

/** Performs an attack. */
void attack_unit(const map_location &attacker, const map_location &defender,
                 int attack_with, int defend_with, bool update_display = true);

/**
 * Returns the advanced version of unit (with traits and items retained).
 */
unit get_advanced_unit(const unit &u, const std::string &advance_to);

/**
 * Function which will advance the unit at @a loc to 'advance_to'.
 * Note that 'loc' is not a reference, because if it were a reference,
 * we couldn't safely pass in a reference to the item in the map
 * that we're going to delete, since deletion would invalidate the reference.
 */
void advance_unit(map_location loc, const std::string &advance_to, const bool &fire_event = true);

/**
 * function which tests if the unit at loc is currently affected by leadership.
 * (i.e. has a higher-level 'leadership' unit next to it).
 * If it does, then the location of the leader unit will be returned,
 * Otherwise map_location::null_location will be returned.
 * If 'bonus' is not NULL, the % bonus will be stored in it.
 */
map_location under_leadership(const unit_map& units, const map_location& loc,
                              int* bonus=NULL);

/**
 * Returns the amount that a unit's damage should be multiplied by
 * due to the current time of day.
 */
int combat_modifier(const map_location &loc, unit_type::ALIGNMENT alignment,
                    bool is_fearless);

/**
 * Function to check if an attack will satisfy the requirements for backstab.
 * Input:
 * - the location from which the attack will occur,
 * - the defending unit location,
 * - the list of units on the map and
 * - the list of teams.
 * The defender and opposite units should be in place already.
 * The attacking unit doesn't need to be, but if it isn't,
 * an external check should be made to make sure the opposite unit
 * isn't also the attacker.
 */
bool backstab_check(const map_location& attacker_loc,
                    const map_location& defender_loc,
                    const unit_map& units, const std::vector<team>& teams);

#endif
