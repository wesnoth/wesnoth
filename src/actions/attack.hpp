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
class  gamemap;

#include "units/types.hpp"
#include "ai/lua/aspect_advancements.hpp"

#include <vector>


/// Calculates the number of blows resulting from swarm.
inline unsigned swarm_blows(unsigned min_blows, unsigned max_blows, unsigned hp,
                            unsigned max_hp)
{
	return hp >= max_hp          ? max_blows :
	       max_blows < min_blows ? min_blows - (min_blows-max_blows) * hp / max_hp :
	                               min_blows + (max_blows-min_blows) * hp / max_hp;
}


/** Structure describing the statistics of a unit involved in the battle. */
struct battle_context_unit_stats
{
	const attack_type *weapon;	/**< The weapon used by the unit to attack the opponent, or nullptr if there is none. */
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
	bool disable;           /**< Attack has disable special. */
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

	/** Used by AI for combat analysis */
	battle_context_unit_stats(const unit_type* u_type,
		   const attack_type* att_weapon, bool attacking,
		   const unit_type* opp_type, const attack_type* opp_weapon,
		   unsigned int opp_terrain_defense,
		   int lawful_bonus = 0);

	~battle_context_unit_stats()
	{}

	/// Calculates the number of blows we would have if we had @a new_hp
	// instead of the recorded hp.
	unsigned int calc_blows(unsigned new_hp) const
	{ return swarm_blows(swarm_min, swarm_max, new_hp, max_hp); }

#if defined(BENCHMARK) || defined(CHECK)
	/// Special constructor for the stand-alone version of attack_prediction.cpp.
	/// (This hardcodes some standard abilities for testing purposes.)
	battle_context_unit_stats(int dmg, int blows, int hitpoints, int maximum_hp,
	                          int hit_chance, bool drain, bool slows, bool slowed,
	                          bool berserk, bool first, bool do_swarm) :
		weapon(nullptr), attack_num(0), is_attacker(true), // These are not used in attack prediction.
		is_poisoned(false), is_slowed(slowed),
		slows(slows), drains(drain), petrifies(false), plagues(false),
		poisons(false), backstab_pos(false), swarm(do_swarm), firststrike(first), disable(false),
		experience(0), max_experience(50), level(1), // No units should advance in the attack prediction tests.
		rounds(berserk ? 30 : 1),
		hp(std::max<int>(0, hitpoints)), max_hp(std::max<int>(1, maximum_hp)),
		chance_to_hit(hit_chance),
		damage(std::max(0, dmg)), slow_damage(round_damage(damage, 1, 2)),
		drain_percent(drain ? 50 : 0), drain_constant(0),
		num_blows(do_swarm ? blows * hp / max_hp : blows),
		swarm_min(do_swarm ? 0 : blows), swarm_max(blows),
		plague_type()
	{
		if ( slowed )
			damage = slow_damage;
		if ( hp > max_hp )
			hp = max_hp; // Keeps the prob_matrix from going out of bounds.
	}
#endif
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
	               double aggression = 0.0, const combatant *prev_def = nullptr,
	               const unit* attacker_ptr=nullptr);

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
	const combatant &get_attacker_combatant(const combatant *prev_def = nullptr);
	const combatant &get_defender_combatant(const combatant *prev_def = nullptr);

	/** Given this harm_weight, is this attack better than that? */
	bool better_attack(class battle_context &that, double harm_weight);
	static bool better_combat(const combatant &us_a, const combatant &them_a,
						   const combatant &us_b, const combatant &them_b,
						   double harm_weight);

private:

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

/** Performs an attack, and advanced the units afterwards */
void attack_unit_and_advance(const map_location &attacker, const map_location &defender,
                 int attack_with, int defend_with, bool update_display = true,
				 const ai::unit_advancements_aspect& ai_advancement = ai::unit_advancements_aspect());

/**
	advances the unit at loc if it has enough experience, maximum 20 times.
	if the unit is on the currently active side, and that side is controlled by a human, a dialog pops up.
	if we are in a non mp game, and the side is controlled by a human then a dialog is shown too.
	if the side is controlled by an ai, and if ai_advancement is passed, then ai_advancement will be used.
	otherwise a random decision will be taken.

	this method is currently not used by unstore_unit, if we want to do that we'd need to allow more arguments (animate, fire_events).
*/
struct advance_unit_params
{
	advance_unit_params(const map_location& loc) : loc_(loc), ai_advancements_(nullptr), force_dialog_(false), fire_events_(true), animate_(true) {}
	advance_unit_params& ai_advancements(const ai::unit_advancements_aspect& value) {ai_advancements_ = &value; return *this;}
	advance_unit_params& force_dialog(bool value) {force_dialog_ = value; return *this;}
	advance_unit_params& fire_events(bool value) {fire_events_ = value; return *this;}
	advance_unit_params& animate(bool value) {animate_ = value; return *this;}
	friend void advance_unit_at(const advance_unit_params&);
private:
	const map_location& loc_;
	const ai::unit_advancements_aspect* ai_advancements_;
	bool force_dialog_;
	bool fire_events_;
	bool animate_;
};
void advance_unit_at(const advance_unit_params& params);
/**
 * Returns the advanced version of a unit (with traits and items retained).
 */
unit_ptr get_advanced_unit(const unit &u, const std::string &advance_to);

/**
 * Returns the AMLA-advanced version of a unit (with traits and items retained).
 */
unit_ptr get_amla_unit(const unit &u, const config &mod_option);

/**
 * Function which will advance the unit at @a loc to 'advance_to'.
 * (Unless mod_option is supplied, in which case an AMLA is performed.)
 * Note that 'loc' is not a reference, because if it were a reference,
 * we couldn't safely pass in a reference to the item in the map
 * that we're going to delete, since deletion would invalidate the reference.
 */
void advance_unit(map_location loc, const std::string &advance_to,
                  const bool &fire_event = true, const config * mod_option = nullptr);

/**
 * function which tests if the unit at loc is currently affected by leadership.
 * (i.e. has a higher-level 'leadership' unit next to it).
 * If it does, then the location of the leader unit will be returned,
 * Otherwise map_location::null_location() will be returned.
 * If 'bonus' is not nullptr, the % bonus will be stored in it.
 */
map_location under_leadership(const unit_map& units, const map_location& loc,
                              int* bonus=nullptr);

/**
 * Returns the amount that a unit's damage should be multiplied by
 * due to the current time of day.
 */
int combat_modifier(const unit_map & units, const gamemap & map, const map_location &loc, unit_type::ALIGNMENT alignment,
                    bool is_fearless);

/**
 * Returns the amount that a unit's damage should be multiplied by
 * due to a given lawful_bonus.
 */
int generic_combat_modifier(int lawful_bonus, unit_type::ALIGNMENT alignment,
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
