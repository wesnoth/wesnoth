/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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
 * Fighting.
 */

#include "attack.hpp"

#include "vision.hpp"

#include "../ai/lua/unit_advancements_aspect.hpp"
#include "../attack_prediction.hpp"
#include "../config_assign.hpp"
#include "../dialogs.hpp"
#include "../game_config.hpp"
#include "../game_display.hpp"
#include "../game_events/pump.hpp"
#include "../game_preferences.hpp"
#include "../gamestatus.hpp"
#include "../gettext.hpp"
#include "../log.hpp"
#include "../map.hpp"
#include "../mouse_handler_base.hpp"
#include "../play_controller.hpp"
#include "../random.hpp"
#include "../random_new.hpp"
#include "../replay.hpp"
#include "../resources.hpp"
#include "../statistics.hpp"
#include "../synced_checkup.hpp"
#include "../team.hpp"
#include "../tod_manager.hpp"
#include "../unit.hpp"
#include "../unit_abilities.hpp"
#include "../unit_display.hpp"
#include "../unit_helper.hpp"
#include "../unit_map.hpp"
#include "../whiteboard/manager.hpp"
#include "../wml_exception.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_config("config");
#define LOG_CF LOG_STREAM(info, log_config)


		/* battle_context_unit_stats */

battle_context_unit_stats::battle_context_unit_stats(const unit &u,
		const map_location& u_loc, int u_attack_num, bool attacking,
		const unit &opp, const map_location& opp_loc,
		const attack_type *opp_weapon, const unit_map& units) :
	weapon(NULL),
	attack_num(u_attack_num),
	is_attacker(attacking),
	is_poisoned(u.get_state(unit::STATE_POISONED)),
	is_slowed(u.get_state(unit::STATE_SLOWED)),
	slows(false),
	drains(false),
	petrifies(false),
	plagues(false),
	poisons(false),
	backstab_pos(false),
	swarm(false),
	firststrike(false),
	disable(false),
	experience(u.experience()),
	max_experience(u.max_experience()),
	level(u.level()),
	rounds(1),
	hp(0),
	max_hp(u.max_hitpoints()),
	chance_to_hit(0),
	damage(0),
	slow_damage(0),
	drain_percent(0),
	drain_constant(0),
	num_blows(0),
	swarm_min(0),
	swarm_max(0),
	plague_type()
{
	// Get the current state of the unit.
	if (attack_num >= 0) {
		weapon = &u.attacks()[attack_num];
	}
	if(u.hitpoints() < 0) {
		LOG_CF << "Unit with " << u.hitpoints() << " hitpoints found, set to 0 for damage calculations\n";
		hp = 0;
	} else if(u.hitpoints() > u.max_hitpoints()) {
		// If a unit has more hp than its maximum, the engine will fail
		// with an assertion failure due to accessing the prob_matrix
		// out of bounds.
		hp = u.max_hitpoints();
	} else {
		hp = u.hitpoints();
	}

	// Get the weapon characteristics, if any.
	if (weapon) {
		weapon->set_specials_context(u_loc, opp_loc, attacking, opp_weapon);
		if (opp_weapon)
			opp_weapon->set_specials_context(opp_loc, u_loc, !attacking, weapon);
		slows = weapon->get_special_bool("slow");
		drains = !opp.get_state("undrainable") && weapon->get_special_bool("drains");
		petrifies = weapon->get_special_bool("petrifies");
		poisons = !opp.get_state("unpoisonable") && weapon->get_special_bool("poison") && !opp.get_state(unit::STATE_POISONED);
		backstab_pos = is_attacker && backstab_check(u_loc, opp_loc, units, *resources::teams);
		rounds = weapon->get_specials("berserk").highest("value", 1).first;
		firststrike = weapon->get_special_bool("firststrike");
		{
			const int distance = distance_between(u_loc, opp_loc);
			const bool out_of_range = distance > weapon->max_range() || distance < weapon->min_range();
			disable = weapon->get_special_bool("disable") || out_of_range;
		}

		// Handle plague.
		unit_ability_list plague_specials = weapon->get_specials("plague");
		plagues = !opp.get_state("unplagueable") && !plague_specials.empty() &&
			strcmp(opp.undead_variation().c_str(), "null") && !resources::game_map->is_village(opp_loc);

		if (plagues) {
			plague_type = (*plague_specials.front().first)["type"].str();
			if (plague_type.empty())
				plague_type = u.type().base_id();
		}

		// Compute chance to hit.
		chance_to_hit = opp.defense_modifier(
			resources::game_map->get_terrain(opp_loc)) + weapon->accuracy() -
			(opp_weapon ? opp_weapon->parry() : 0);
		if(chance_to_hit > 100) {
			chance_to_hit = 100;
		}

		unit_ability_list cth_specials = weapon->get_specials("chance_to_hit");
		unit_abilities::effect cth_effects(cth_specials, chance_to_hit, backstab_pos);
		chance_to_hit = cth_effects.get_composite_value();

		// Compute base damage done with the weapon.
		int base_damage = weapon->modified_damage(backstab_pos);

		// Get the damage multiplier applied to the base damage of the weapon.
		int damage_multiplier = 100;
		// Time of day bonus.
		damage_multiplier += combat_modifier(u_loc, u.alignment(), u.is_fearless());
		// Leadership bonus.
		int leader_bonus = 0;
		if (under_leadership(units, u_loc, &leader_bonus).valid())
			damage_multiplier += leader_bonus;
		// Resistance modifier.
		damage_multiplier *= opp.damage_from(*weapon, !attacking, opp_loc);

		// Compute both the normal and slowed damage.
		damage = round_damage(base_damage, damage_multiplier, 10000);
		slow_damage = round_damage(base_damage, damage_multiplier, 20000);
		if (is_slowed)
			damage = slow_damage;

		// Compute drain amounts only if draining is possible.
		if(drains) {
			unit_ability_list drain_specials = weapon->get_specials("drains");

			// Compute the drain percent (with 50% as the base for backward compatibility)
			unit_abilities::effect drain_percent_effects(drain_specials, 50, backstab_pos);
			drain_percent = drain_percent_effects.get_composite_value();
		}

		// Add heal_on_hit (the drain constant)
		unit_ability_list heal_on_hit_specials = weapon->get_specials("heal_on_hit");
		unit_abilities::effect heal_on_hit_effects(heal_on_hit_specials, 0, backstab_pos);
		drain_constant += heal_on_hit_effects.get_composite_value();

		drains = drain_constant || drain_percent;

		// Compute the number of blows and handle swarm.
		weapon->modified_attacks(backstab_pos, swarm_min, swarm_max);
		swarm = swarm_min != swarm_max;
		num_blows = calc_blows(hp);
	}
}

battle_context_unit_stats::battle_context_unit_stats(const unit_type* u_type,
	   const attack_type* att_weapon, bool attacking,
	   const unit_type* opp_type,
	   const attack_type* opp_weapon,
	   unsigned int opp_terrain_defense,
	   int lawful_bonus) :
	weapon(att_weapon),
	attack_num(-2),  // This is and stays invalid. Always use weapon, when using this constructor.
	is_attacker(attacking),
	is_poisoned(false),
	is_slowed(false),
	slows(false),
	drains(false),
	petrifies(false),
	plagues(false),
	poisons(false),
	backstab_pos(false),
	swarm(false),
	firststrike(false),
	disable(false),
	experience(0),
	max_experience(0),
	level(0),
	rounds(1),
	hp(0),
	max_hp(0),
	chance_to_hit(0),
	damage(0),
	slow_damage(0),
	drain_percent(0),
	drain_constant(0),
	num_blows(0),
	swarm_min(0),
	swarm_max(0),
	plague_type()
{
	if (!u_type || !opp_type) {
		return;
	}

	// Get the current state of the unit.
	if (u_type->hitpoints() < 0) {
		hp = 0;
	} else {
		hp = u_type->hitpoints();
	}
	max_experience = u_type->experience_needed();
	level = (u_type->level());
	max_hp = (u_type->hitpoints());

	// Get the weapon characteristics, if any.
	if (weapon) {
		weapon->set_specials_context(map_location::null_location, attacking);
		if (opp_weapon) {
			opp_weapon->set_specials_context(map_location::null_location, !attacking);
		}
		slows = weapon->get_special_bool("slow");
		drains = !opp_type->musthave_status("undrainable") && weapon->get_special_bool("drains");
		petrifies = weapon->get_special_bool("petrifies");
		poisons = !opp_type->musthave_status("unpoisonable") && weapon->get_special_bool("poison");
		rounds = weapon->get_specials("berserk").highest("value", 1).first;
		firststrike = weapon->get_special_bool("firststrike");
		disable = weapon->get_special_bool("disable");

		unit_ability_list plague_specials = weapon->get_specials("plague");
		plagues = !opp_type->musthave_status("unplagueable") && !plague_specials.empty() &&
			strcmp(opp_type->undead_variation().c_str(), "null");

		if (plagues) {
			plague_type = (*plague_specials.front().first)["type"].str();
			if (plague_type.empty()) {
				plague_type = u_type->base_id();
			}
		}

		signed int cth = 100 - opp_terrain_defense + weapon->accuracy() -
				(opp_weapon ? opp_weapon->parry() : 0);
		cth = std::min(100, cth);
		cth = std::max(0, cth);
		chance_to_hit = cth;

		unit_ability_list cth_specials = weapon->get_specials("chance_to_hit");
		unit_abilities::effect cth_effects(cth_specials, chance_to_hit, backstab_pos);
		chance_to_hit = cth_effects.get_composite_value();

		int base_damage = weapon->modified_damage(backstab_pos);
		int damage_multiplier = 100;
		damage_multiplier += generic_combat_modifier(lawful_bonus, u_type->alignment(),
				u_type->musthave_status("fearless"));
		damage_multiplier *= opp_type->resistance_against(weapon->type(), !attacking);

		damage = round_damage(base_damage, damage_multiplier, 10000);
		slow_damage = round_damage(base_damage, damage_multiplier, 20000);

		if (drains) {
			unit_ability_list drain_specials = weapon->get_specials("drains");

			// Compute the drain percent (with 50% as the base for backward compatibility)
			unit_abilities::effect drain_percent_effects(drain_specials, 50, backstab_pos);
			drain_percent = drain_percent_effects.get_composite_value();
		}

		// Add heal_on_hit (the drain constant)
		unit_ability_list heal_on_hit_specials = weapon->get_specials("heal_on_hit");
		unit_abilities::effect heal_on_hit_effects(heal_on_hit_specials, 0, backstab_pos);
		drain_constant += heal_on_hit_effects.get_composite_value();

		drains = drain_constant || drain_percent;

		// Compute the number of blows and handle swarm.
		weapon->modified_attacks(backstab_pos, swarm_min, swarm_max);
		swarm = swarm_min != swarm_max;
		num_blows = calc_blows(hp);
	}
}

		/* battle_context */

battle_context::battle_context(const unit_map& units,
		const map_location& attacker_loc, const map_location& defender_loc,
		int attacker_weapon, int defender_weapon, double aggression,
		const combatant *prev_def, const unit* attacker_ptr) :
	attacker_stats_(NULL), defender_stats_(NULL), attacker_combatant_(NULL),
	defender_combatant_(NULL)
{
	const unit &attacker = attacker_ptr ? *attacker_ptr : *units.find(attacker_loc);
	const unit &defender = *units.find(defender_loc);
	const double harm_weight = 1.0 - aggression;

	if (attacker_weapon == -1 && attacker.attacks().size() == 1 && attacker.attacks()[0].attack_weight() > 0 )
		attacker_weapon = 0;

	if (attacker_weapon == -1) {
		attacker_weapon = choose_attacker_weapon(attacker, defender, units,
			attacker_loc, defender_loc,
				harm_weight, &defender_weapon, prev_def);
	} else if (defender_weapon == -1) {
		defender_weapon = choose_defender_weapon(attacker, defender, attacker_weapon,
			units, attacker_loc, defender_loc, prev_def);
	}

	// If those didn't have to generate statistics, do so now.
	if (!attacker_stats_) {
		const attack_type *adef = NULL;
		const attack_type *ddef = NULL;
		if (attacker_weapon >= 0) {
			VALIDATE(attacker_weapon < static_cast<int>(attacker.attacks().size()),
					_("An invalid attacker weapon got selected."));
			adef = &attacker.attacks()[attacker_weapon];
		}
		if (defender_weapon >= 0) {
			VALIDATE(defender_weapon < static_cast<int>(defender.attacks().size()),
					_("An invalid defender weapon got selected."));
			ddef = &defender.attacks()[defender_weapon];
		}
		assert(!defender_stats_ && !attacker_combatant_ && !defender_combatant_);
		attacker_stats_ = new battle_context_unit_stats(attacker, attacker_loc, attacker_weapon,
				true, defender, defender_loc, ddef, units);
		defender_stats_ = new battle_context_unit_stats(defender, defender_loc, defender_weapon, false,
				attacker, attacker_loc, adef, units);
	}

	// There have been various bugs where only one of these was set
	assert(attacker_stats_);
	assert(defender_stats_);
}

battle_context::battle_context(const battle_context_unit_stats &att,
                               const battle_context_unit_stats &def) :
	attacker_stats_(new battle_context_unit_stats(att)),
	defender_stats_(new battle_context_unit_stats(def)),
	attacker_combatant_(NULL),
	defender_combatant_(NULL)
{
}

battle_context::battle_context(const battle_context &other) :
	attacker_stats_(NULL), defender_stats_(NULL), attacker_combatant_(NULL),
	defender_combatant_(NULL)
{
	*this = other;
}

battle_context::~battle_context()
{
	delete attacker_stats_;
	delete defender_stats_;
	delete attacker_combatant_;
	delete defender_combatant_;
}

battle_context& battle_context::operator=(const battle_context &other)
{
	if (&other != this) {
		delete attacker_stats_;
		delete defender_stats_;
		delete attacker_combatant_;
		delete defender_combatant_;
		attacker_stats_ = new battle_context_unit_stats(*other.attacker_stats_);
		defender_stats_ = new battle_context_unit_stats(*other.defender_stats_);
		attacker_combatant_ = other.attacker_combatant_ ? new combatant(*other.attacker_combatant_, *attacker_stats_) : NULL;
		defender_combatant_ = other.defender_combatant_ ? new combatant(*other.defender_combatant_, *defender_stats_) : NULL;
	}
	return *this;
}

/** @todo FIXME: better to initialize combatant initially (move into
                 battle_context_unit_stats?), just do fight() when required. */
const combatant &battle_context::get_attacker_combatant(const combatant *prev_def)
{
	// We calculate this lazily, since AI doesn't always need it.
	if (!attacker_combatant_) {
		assert(!defender_combatant_);
		attacker_combatant_ = new combatant(*attacker_stats_);
		defender_combatant_ = new combatant(*defender_stats_, prev_def);
		attacker_combatant_->fight(*defender_combatant_);
	}
	return *attacker_combatant_;
}

const combatant &battle_context::get_defender_combatant(const combatant *prev_def)
{
	// We calculate this lazily, since AI doesn't always need it.
	if (!defender_combatant_) {
		assert(!attacker_combatant_);
		attacker_combatant_ = new combatant(*attacker_stats_);
		defender_combatant_ = new combatant(*defender_stats_, prev_def);
		attacker_combatant_->fight(*defender_combatant_);
	}
	return *defender_combatant_;
}

// Given this harm_weight, are we better than this other context?
bool battle_context::better_attack(class battle_context &that, double harm_weight)
{
	return better_combat(get_attacker_combatant(), get_defender_combatant(),
			that.get_attacker_combatant(), that.get_defender_combatant(), harm_weight);
}

// Does combat A give us a better result than combat B?
bool battle_context::better_combat(const combatant &us_a, const combatant &them_a,
		const combatant &us_b, const combatant &them_b, double harm_weight)
{
	double a, b;

	// Compare: P(we kill them) - P(they kill us).
	a = them_a.hp_dist[0] - us_a.hp_dist[0] * harm_weight;
	b = them_b.hp_dist[0] - us_b.hp_dist[0] * harm_weight;
	if (a - b < -0.01)
		return false;
	if (a - b > 0.01)
		return true;

	// Add poison to calculations
	double poison_a_us = (us_a.poisoned) * game_config::poison_amount;
	double poison_a_them = (them_a.poisoned) * game_config::poison_amount;
	double poison_b_us = (us_b.poisoned) * game_config::poison_amount;
	double poison_b_them = (them_b.poisoned) * game_config::poison_amount;
	// Compare: damage to them - damage to us (average_hp replaces -damage)
	a = (us_a.average_hp()-poison_a_us)*harm_weight - (them_a.average_hp()-poison_a_them);
	b = (us_b.average_hp()-poison_b_us)*harm_weight - (them_b.average_hp()-poison_b_them);
	if (a - b < -0.01)
		return false;
	if (a - b > 0.01)
		return true;

	// All else equal: go for most damage.
	return them_a.average_hp() < them_b.average_hp();
}

int battle_context::choose_attacker_weapon(const unit &attacker,
		const unit &defender, const unit_map& units,
		const map_location& attacker_loc, const map_location& defender_loc,
		double harm_weight, int *defender_weapon, const combatant *prev_def)
{
	std::vector<unsigned int> choices;

	// What options does attacker have?
	unsigned int i;
	for (i = 0; i < attacker.attacks().size(); ++i) {
		const attack_type &att = attacker.attacks()[i];
		if (att.attack_weight() > 0) {
			choices.push_back(i);
		}
	}
	if (choices.empty())
		return -1;
	if (choices.size() == 1) {
		*defender_weapon = choose_defender_weapon(attacker, defender, choices[0], units,
			attacker_loc, defender_loc, prev_def);
		const attack_type *def_weapon = *defender_weapon >= 0 ? &defender.attacks()[*defender_weapon] : NULL;
		attacker_stats_ = new battle_context_unit_stats(attacker, attacker_loc, choices[0],
						true, defender, defender_loc, def_weapon, units);
		if (attacker_stats_->disable) {
			delete attacker_stats_;
			attacker_stats_ = NULL;
			return -1;
		}
		const attack_type &att = attacker.attacks()[choices[0]];
		defender_stats_ = new battle_context_unit_stats(defender, defender_loc, *defender_weapon, false,
			attacker, attacker_loc, &att, units);
		return choices[0];
	}

	// Multiple options: simulate them, save best.
	battle_context_unit_stats *best_att_stats = NULL, *best_def_stats = NULL;
	combatant *best_att_comb = NULL, *best_def_comb = NULL;

	for (i = 0; i < choices.size(); ++i) {
		const attack_type &att = attacker.attacks()[choices[i]];
		int def_weapon = choose_defender_weapon(attacker, defender, choices[i], units,
			attacker_loc, defender_loc, prev_def);
		// If that didn't simulate, do so now.
		if (!attacker_combatant_) {
			const attack_type *def = NULL;
			if (def_weapon >= 0) {
				def = &defender.attacks()[def_weapon];
			}
			attacker_stats_ = new battle_context_unit_stats(attacker, attacker_loc, choices[i],
				true, defender, defender_loc, def, units);
			if (attacker_stats_->disable) {
				delete attacker_stats_;
				attacker_stats_ = NULL;
				continue;
			}
			defender_stats_ = new battle_context_unit_stats(defender, defender_loc, def_weapon, false,
				attacker, attacker_loc, &att, units);
			attacker_combatant_ = new combatant(*attacker_stats_);
			defender_combatant_ = new combatant(*defender_stats_, prev_def);
			attacker_combatant_->fight(*defender_combatant_);
		}
		if (!best_att_comb || better_combat(*attacker_combatant_, *defender_combatant_,
					*best_att_comb, *best_def_comb, harm_weight)) {
			delete best_att_comb;
			delete best_def_comb;
			delete best_att_stats;
			delete best_def_stats;
			best_att_comb = attacker_combatant_;
			best_def_comb = defender_combatant_;
			best_att_stats = attacker_stats_;
			best_def_stats = defender_stats_;
		} else {
			delete attacker_combatant_;
			delete defender_combatant_;
			delete attacker_stats_;
			delete defender_stats_;
		}
		attacker_combatant_ = NULL;
		defender_combatant_ = NULL;
		attacker_stats_ = NULL;
		defender_stats_ = NULL;
	}

	attacker_combatant_ = best_att_comb;
	defender_combatant_ = best_def_comb;
	attacker_stats_ = best_att_stats;
	defender_stats_ = best_def_stats;

	// These currently mean the same thing, but assumptions like that have been broken before
	if (!defender_stats_ || !attacker_stats_) {
		return -1;
	}
	*defender_weapon = defender_stats_->attack_num;
	return attacker_stats_->attack_num;
}

/** @todo FIXME: Hand previous defender unit in here. */
int battle_context::choose_defender_weapon(const unit &attacker,
		const unit &defender, unsigned attacker_weapon, const unit_map& units,
		const map_location& attacker_loc, const map_location& defender_loc,
		const combatant *prev_def)
{
	VALIDATE(attacker_weapon < attacker.attacks().size(),
			_("An invalid attacker weapon got selected."));
	const attack_type &att = attacker.attacks()[attacker_weapon];
	std::vector<unsigned int> choices;

	// What options does defender have?
	unsigned int i;
	for (i = 0; i < defender.attacks().size(); ++i) {
		const attack_type &def = defender.attacks()[i];
		if (def.range() == att.range() && def.defense_weight() > 0) {
			choices.push_back(i);
		}
	}
	if (choices.empty())
		return -1;
	if (choices.size() == 1) {
		const battle_context_unit_stats def_stats(defender, defender_loc,
				choices[0], false, attacker, attacker_loc, &att, units);
		return (def_stats.disable) ? -1 : choices[0];
	}

	// Multiple options:
	// First pass : get the best weight and the minimum simple rating for this weight.
	// simple rating = number of blows * damage per blows (resistance taken in account) * cth * weight
	// Eligible attacks for defense should have a simple rating greater or equal to this weight.

	int min_rating = 0;
	{
		double max_weight = 0.0;

		for (i = 0; i < choices.size(); ++i) {
			const attack_type &def = defender.attacks()[choices[i]];
			if (def.defense_weight() >= max_weight) {
				const battle_context_unit_stats def_stats(defender, defender_loc,
						choices[i], false, attacker, attacker_loc, &att, units);
				if (def_stats.disable) continue;
				max_weight = def.defense_weight();
				int rating = static_cast<int>(def_stats.num_blows * def_stats.damage *
						def_stats.chance_to_hit * def.defense_weight());
				if (def.defense_weight() > max_weight || rating < min_rating ) {
					min_rating = rating;
				}
			}
		}
	}

	// Multiple options: simulate them, save best.
	for (i = 0; i < choices.size(); ++i) {
		const attack_type &def = defender.attacks()[choices[i]];
		battle_context_unit_stats *att_stats = new battle_context_unit_stats(attacker, attacker_loc, attacker_weapon,
				true, defender, defender_loc, &def, units);
		battle_context_unit_stats *def_stats = new battle_context_unit_stats(defender, defender_loc, choices[i], false,
				attacker, attacker_loc, &att, units);
		if (def_stats->disable) {
			delete att_stats;
			delete def_stats;
			continue;
		}
		combatant *att_comb = new combatant(*att_stats);
		combatant *def_comb = new combatant(*def_stats, prev_def);
		att_comb->fight(*def_comb);

		int simple_rating = static_cast<int>(def_stats->num_blows *
				def_stats->damage * def_stats->chance_to_hit * def.defense_weight());

		if (simple_rating >= min_rating &&
				( !attacker_combatant_ || better_combat(*def_comb, *att_comb, *defender_combatant_, *attacker_combatant_, 1.0) )
		   ) {
			delete attacker_combatant_;
			delete defender_combatant_;
			delete attacker_stats_;
			delete defender_stats_;
			attacker_combatant_ = att_comb;
			defender_combatant_ = def_comb;
			attacker_stats_ = att_stats;
			defender_stats_ = def_stats;
		} else {
			delete att_comb;
			delete def_comb;
			delete att_stats;
			delete def_stats;
		}
	}

	return defender_stats_ ? defender_stats_->attack_num : -1;
}


namespace {
	void refresh_weapon_index(int& weap_index, std::string const& weap_id, std::vector<attack_type> const& attacks)
	{
		if(attacks.empty()) {
			//no attacks to choose from
			weap_index = -1;
			return;
		}
		if(weap_index >= 0 && weap_index < static_cast<int>(attacks.size()) && attacks[weap_index].id() == weap_id) {
			//the currently selected attack fits
			return;
		}
		if(!weap_id.empty()) {
			//lookup the weapon by id
			for(int i=0; i<static_cast<int>(attacks.size()); ++i) {
				if(attacks[i].id() == weap_id) {
					weap_index = i;
					return;
				}
			}
		}
		//lookup has failed
		weap_index = -1;
		return;
	}


	/** Helper class for performing an attack. */
	class attack
	{
	public:
		attack(const map_location &attacker, const map_location &defender,
			int attack_with, int defend_with, bool update_display = true);
		~attack();

		void perform();

	private:
		class attack_end_exception {};
		bool perform_hit(bool, statistics::attack_context &);
		void fire_event(const std::string& n);
		void refresh_bc();

		/** Structure holding unit info used in the attack action. */
		struct unit_info
		{
			const map_location loc_;
			int weapon_;
			unit_map &units_;
			size_t id_; /**< unit.underlying_id() */
			std::string weap_id_;
			int orig_attacks_;
			int n_attacks_; /**< Number of attacks left. */
			int cth_;
			int damage_;
			int xp_;

			unit_info(const map_location &loc, int weapon, unit_map &units);
			unit &get_unit();
			bool valid();

			std::string dump();
		};

		void unit_killed(unit_info &, unit_info &,
			const battle_context_unit_stats *&, const battle_context_unit_stats *&,
			bool);

		battle_context *bc_;
		const battle_context_unit_stats *a_stats_;
		const battle_context_unit_stats *d_stats_;

		int abs_n_attack_, abs_n_defend_;
		// update_att_fog_ is not used, other than making some code simpler.
		bool update_att_fog_, update_def_fog_, update_minimap_;

		unit_info a_, d_;
		unit_map &units_;
		std::ostringstream errbuf_;

		bool update_display_;
		bool OOS_error_;
	};


	attack::unit_info::unit_info(const map_location& loc, int weapon, unit_map& units) :
		loc_(loc),
		weapon_(weapon),
		units_(units),
		id_(),
		weap_id_(),
		orig_attacks_(0),
		n_attacks_(0),
		cth_(0),
		damage_(0),
		xp_(0)
	{
		unit_map::iterator i = units_.find(loc_);
		if (!i.valid()) return;
		id_ = i->underlying_id();
	}

	unit &attack::unit_info::get_unit()
	{
		unit_map::iterator i = units_.find(loc_);
		assert(i.valid() && i->underlying_id() == id_);
		return *i;
	}

	bool attack::unit_info::valid()
	{
		unit_map::iterator i = units_.find(loc_);
		return i.valid() && i->underlying_id() == id_;
	}

	std::string attack::unit_info::dump()
	{
		std::stringstream s;
		s << get_unit().type_id() << " (" << loc_.x + 1 << ',' << loc_.y + 1 << ')';
		return s.str();
	}


	attack::attack(const map_location &attacker, const map_location &defender,
			int attack_with, int defend_with, bool update_display) :
		bc_(NULL),
		a_stats_(NULL),
		d_stats_(NULL),
		abs_n_attack_(0),
		abs_n_defend_(0),
		update_att_fog_(false),
		update_def_fog_(false),
		update_minimap_(false),
		a_(attacker, attack_with, *resources::units),
		d_(defender, defend_with, *resources::units),
		units_(*resources::units),
		errbuf_(),
		update_display_(update_display),
		OOS_error_(false)
	{
	}

	attack::~attack()
	{
		delete bc_;
	}

	void attack::fire_event(const std::string& n)
	{
		LOG_NG << "firing " << n << " event\n";
		//prepare the event data for weapon filtering
		config ev_data;
		config& a_weapon_cfg = ev_data.add_child("first");
		config& d_weapon_cfg = ev_data.add_child("second");
		if(a_stats_->weapon != NULL && a_.valid()) {
			a_weapon_cfg = a_stats_->weapon->get_cfg();
		}
		if(d_stats_->weapon != NULL && d_.valid()) {
			d_weapon_cfg = d_stats_->weapon->get_cfg();
		}
		if(a_weapon_cfg["name"].empty()) {
			a_weapon_cfg["name"] = "none";
		}
		if(d_weapon_cfg["name"].empty()) {
			d_weapon_cfg["name"] = "none";
		}
		if(n == "attack_end") {
			// We want to fire attack_end event in any case! Even if one of units was removed by WML
			game_events::fire(n, a_.loc_, d_.loc_, ev_data);
			return;
		}
		const int defender_side = d_.get_unit().side();
		game_events::fire(n, game_events::entity_location(a_.loc_, a_.id_),
			game_events::entity_location(d_.loc_, d_.id_), ev_data);

		// The event could have killed either the attacker or
		// defender, so we have to make sure they still exist
		refresh_bc();
		if(!a_.valid() || !d_.valid() || !(*resources::teams)[a_.get_unit().side() - 1].is_enemy(d_.get_unit().side())) {
			actions::recalculate_fog(defender_side);
			if (update_display_){
				resources::screen->redraw_minimap();
				resources::screen->draw(true, true);
			}
			fire_event("attack_end");
			throw attack_end_exception();
		}
	}

	void attack::refresh_bc()
	{
		// Fix index of weapons
		if (a_.valid()) {
			refresh_weapon_index(a_.weapon_, a_.weap_id_, a_.get_unit().attacks());
		}
		if (d_.valid()) {
			refresh_weapon_index(d_.weapon_, d_.weap_id_, d_.get_unit().attacks());
		}
		if(!a_.valid() || !d_.valid()) {
			// Fix pointer to weapons
			const_cast<battle_context_unit_stats*>(a_stats_)->weapon =
				a_.valid() && a_.weapon_ >= 0
					? &a_.get_unit().attacks()[a_.weapon_] : NULL;

			const_cast<battle_context_unit_stats*>(d_stats_)->weapon =
				d_.valid() && d_.weapon_ >= 0
					? &d_.get_unit().attacks()[d_.weapon_] : NULL;

			return;
		}

		*bc_ =	battle_context(units_, a_.loc_, d_.loc_, a_.weapon_, d_.weapon_);
		a_stats_ = &bc_->get_attacker_stats();
		d_stats_ = &bc_->get_defender_stats();
		a_.cth_ = a_stats_->chance_to_hit;
		d_.cth_ = d_stats_->chance_to_hit;
		a_.damage_ = a_stats_->damage;
		d_.damage_ = d_stats_->damage;
	}

	bool attack::perform_hit(bool attacker_turn, statistics::attack_context &stats)
	{
		unit_info
			&attacker = *(attacker_turn ? &a_ : &d_),
			&defender = *(attacker_turn ? &d_ : &a_);
		const battle_context_unit_stats
			*&attacker_stats = *(attacker_turn ? &a_stats_ : &d_stats_),
			*&defender_stats = *(attacker_turn ? &d_stats_ : &a_stats_);
		int &abs_n = *(attacker_turn ? &abs_n_attack_ : &abs_n_defend_);
		bool &update_fog = *(attacker_turn ? &update_def_fog_ : &update_att_fog_);

		int ran_num = random_new::generator->next_random();
		bool hits = (ran_num % 100) < attacker.cth_;

		int damage = 0;
		if (hits) {
			damage = attacker.damage_;
			resources::gamedata->get_variable("damage_inflicted") = damage;
		}

		
		// Make sure that if we're serializing a game here,
		// we got the same results as the game did originally.
		const config local_results = config_of("chance", attacker.cth_)("hits", hits)("damage", damage);
		config replay_results;
		bool equals_replay = checkup_instance->local_checkup(local_results, replay_results);
		if (!equals_replay)
		{
			
			int results_chance = replay_results["chance"];
			bool results_hits = replay_results["hits"].to_bool();
			int results_damage = replay_results["damage"];
			/*
			errbuf_ << "SYNC: In attack " << a_.dump() << " vs " << d_.dump() 
				<< " replay data differs from local calculated data:"
				<< " chance to hit in data source: " << results_chance 
				<< " chance to hit in calculated:  " << attacker.cth_
				<< " chance to hit in data source: " << results_chance 
				<< " chance to hit in calculated:  " << attacker.cth_
				;
			
				attacker.cth_ = results_chance;
				hits = results_hits;
				damage = results_damage;

				OOS_error_ = true;
				*/
			if (results_chance != attacker.cth_)
			{
				errbuf_ << "SYNC: In attack " << a_.dump() << " vs " << d_.dump()
					<< ": chance to hit is inconsistent. Data source: "
					<< results_chance << "; Calculation: " << attacker.cth_
					<< " (over-riding game calculations with data source results)\n";
				attacker.cth_ = results_chance;
				OOS_error_ = true;
			}

			if (results_hits != hits)
			{
				errbuf_ << "SYNC: In attack " << a_.dump() << " vs " << d_.dump()
					<< ": the data source says the hit was "
					<< (results_hits ? "successful" : "unsuccessful")
					<< ", while in-game calculations say the hit was "
					<< (hits ? "successful" : "unsuccessful")
					<< " random number: " << ran_num << " = "
					<< (ran_num % 100) << "/" << results_chance
					<< " (over-riding game calculations with data source results)\n";
				hits = results_hits;
				OOS_error_ = true;
			}

			if (results_damage != damage)
			{
				errbuf_ << "SYNC: In attack " << a_.dump() << " vs " << d_.dump()
					<< ": the data source says the hit did " << results_damage
					<< " damage, while in-game calculations show the hit doing "
					<< damage
					<< " damage (over-riding game calculations with data source results)\n";
				damage = results_damage;
				OOS_error_ = true;
			}
		}

		int damage_done = std::min<int>(defender.get_unit().hitpoints(), attacker.damage_);

		int drains_damage = 0;
		if (hits && attacker_stats->drains) {
			drains_damage = damage_done * attacker_stats->drain_percent / 100 + attacker_stats->drain_constant;
			// don't drain so much that the attacker gets more than his maximum hitpoints
			drains_damage = std::min<int>(drains_damage, attacker.get_unit().max_hitpoints() - attacker.get_unit().hitpoints());
			// if drain is negative, don't allow drain to kill the attacker
			drains_damage = std::max<int>(drains_damage, 1 - attacker.get_unit().hitpoints());
		}

		if (update_display_)
		{
			std::ostringstream float_text;
			if (hits)
			{
				const unit &defender_unit = defender.get_unit();
				if (attacker_stats->poisons && !defender_unit.get_state(unit::STATE_POISONED)) {
					float_text << (defender_unit.gender() == unit_race::FEMALE ?
						_("female^poisoned") : _("poisoned")) << '\n';
				}

				if (attacker_stats->slows && !defender_unit.get_state(unit::STATE_SLOWED)) {
					float_text << (defender_unit.gender() == unit_race::FEMALE ?
						_("female^slowed") : _("slowed")) << '\n';
				}

				if (attacker_stats->petrifies) {
					float_text << (defender_unit.gender() == unit_race::FEMALE ?
						_("female^petrified") : _("petrified")) << '\n';
				}
			}

			unit_display::unit_attack(attacker.loc_, defender.loc_, damage,
				*attacker_stats->weapon, defender_stats->weapon,
				abs_n, float_text.str(), drains_damage, "");
		}

		bool dies = defender.get_unit().take_hit(damage);
		LOG_NG << "defender took " << damage << (dies ? " and died\n" : "\n");
		if (attacker_turn) {
			stats.attack_result(hits
				? (dies ? statistics::attack_context::KILLS : statistics::attack_context::HITS)
				: statistics::attack_context::MISSES, damage_done, drains_damage);
		} else {
			stats.defend_result(hits
				? (dies ? statistics::attack_context::KILLS : statistics::attack_context::HITS)
				: statistics::attack_context::MISSES, damage_done, drains_damage);
		}

		
		replay_results.clear();
		// there was also a attribute cfg["unit_hit"] which was never used so i deleted.
		equals_replay = checkup_instance->local_checkup(config_of("dies", dies), replay_results);
		if (!equals_replay)
		{
			bool results_dies = replay_results["dies"].to_bool();

				errbuf_ << "SYNC: In attack " << a_.dump() << " vs " << d_.dump()
					<< ": the data source says the "
					<< (attacker_turn ? "defender" : "attacker") << ' '
					<< (results_dies ? "perished" : "survived")
					<< " while in-game calculations show it "
					<< (dies ? "perished" : "survived")
					<< " (over-riding game calculations with data source results)\n";
				dies = results_dies;
				// Set hitpoints to 0 so later checks don't invalidate the death.
				if (results_dies) defender.get_unit().set_hitpoints(0);
				OOS_error_ = true;
		}

		if (hits)
		{
			try {
				fire_event(attacker_turn ? "attacker_hits" : "defender_hits");
			} catch (attack_end_exception) {
				refresh_bc();
				return false;
			}
		}
		else
		{
			try {
				fire_event(attacker_turn ? "attacker_misses" : "defender_misses");
			} catch (attack_end_exception) {
				refresh_bc();
				return false;
			}
		}
		refresh_bc();

		bool attacker_dies = false;
		if (drains_damage > 0) {
			attacker.get_unit().heal(drains_damage);
		} else if(drains_damage < 0) {
			attacker_dies = attacker.get_unit().take_hit(-drains_damage);
		}

		if (dies) {
			unit_killed(attacker, defender, attacker_stats, defender_stats, false);
			update_fog = true;
		}
		if (attacker_dies) {
			unit_killed(defender, attacker, defender_stats, attacker_stats, true);
			*(attacker_turn ? &update_att_fog_ : &update_def_fog_) = true;
		}

		if(dies) {
			update_minimap_ = true;
			return false;
		}

		if (hits)
		{
			unit &defender_unit = defender.get_unit();
			if (attacker_stats->poisons && !defender_unit.get_state(unit::STATE_POISONED)) {
				defender_unit.set_state(unit::STATE_POISONED, true);
				LOG_NG << "defender poisoned\n";
			}

			if (attacker_stats->slows && !defender_unit.get_state(unit::STATE_SLOWED)) {
				defender_unit.set_state(unit::STATE_SLOWED, true);
				update_fog = true;
				defender.damage_ = defender_stats->slow_damage;
				LOG_NG << "defender slowed\n";
			}

			// If the defender is petrified, the fight stops immediately
			if (attacker_stats->petrifies) {
				defender_unit.set_state(unit::STATE_PETRIFIED, true);
				update_fog = true;
				attacker.n_attacks_ = 0;
				defender.n_attacks_ = -1; // Petrified.
				game_events::fire("petrified", defender.loc_, attacker.loc_);
				refresh_bc();
			}
		}

		// Delay until here so that poison and slow go through
		if(attacker_dies) {
			update_minimap_ = true;
			return false;
		}

		--attacker.n_attacks_;
		return true;
	}

	void attack::unit_killed(unit_info& attacker, unit_info& defender,
		const battle_context_unit_stats *&attacker_stats,
		const battle_context_unit_stats *&defender_stats,
		bool drain_killed)
	{
		attacker.xp_ = game_config::kill_xp(defender.get_unit().level());
		defender.xp_ = 0;
		resources::screen->invalidate(attacker.loc_);

		game_events::entity_location death_loc(defender.loc_, defender.id_);
		game_events::entity_location attacker_loc(attacker.loc_, attacker.id_);
		std::string undead_variation = defender.get_unit().undead_variation();
		fire_event("attack_end");
		refresh_bc();

		// get weapon info for last_breath and die events
		config dat;
		config a_weapon_cfg = attacker_stats->weapon && attacker.valid() ?
			attacker_stats->weapon->get_cfg() : config();
		config d_weapon_cfg = defender_stats->weapon && defender.valid() ?
			defender_stats->weapon->get_cfg() : config();
		if (a_weapon_cfg["name"].empty())
			a_weapon_cfg["name"] = "none";
		if (d_weapon_cfg["name"].empty())
			d_weapon_cfg["name"] = "none";
		dat.add_child("first",  d_weapon_cfg);
		dat.add_child("second", a_weapon_cfg);

		game_events::fire("last breath", death_loc, attacker_loc, dat);
		refresh_bc();

		if (!defender.valid() || defender.get_unit().hitpoints() > 0) {
			// WML has invalidated the dying unit, abort
			return;
		}

		if (!attacker.valid()) {
			unit_display::unit_die(defender.loc_, defender.get_unit(),
				NULL, defender_stats->weapon);
		} else {
			unit_display::unit_die(defender.loc_, defender.get_unit(),
				attacker_stats->weapon, defender_stats->weapon,
				attacker.loc_, &attacker.get_unit());
		}

		game_events::fire("die", death_loc, attacker_loc, dat);
		refresh_bc();

		if (!defender.valid() || defender.get_unit().hitpoints() > 0) {
			// WML has invalidated the dying unit, abort
			return;
		}

		units_.erase(defender.loc_);

		if (attacker.valid() && attacker_stats->plagues && !drain_killed)
		{
			// plague units make new units on the target hex
			LOG_NG << "trying to reanimate " << attacker_stats->plague_type << '\n';
			const unit_type *reanimator =
				unit_types.find(attacker_stats->plague_type);
			if (reanimator)
			{
				LOG_NG << "found unit type:" << reanimator->id() << '\n';
				unit newunit(*reanimator, attacker.get_unit().side(),
					true, unit_race::MALE);
				newunit.set_attacks(0);
				newunit.set_movement(0, true);
				// Apply variation
				if (undead_variation != "null")
				{
					config mod;
					config &variation = mod.add_child("effect");
					variation["apply_to"] = "variation";
					variation["name"] = undead_variation;
					newunit.add_modification("variation",mod);
					newunit.heal_all();
				}
				units_.add(death_loc, newunit);
				preferences::encountered_units().insert(newunit.type_id());
				if (update_display_) {
					resources::screen->invalidate(death_loc);
				}
			}
		}
		else
		{
			LOG_NG << "unit not reanimated\n";
		}
	}

	void attack::perform()
	{
		// Stop the user from issuing any commands while the units are fighting
		const events::command_disabler disable_commands;

		if(!a_.valid() || !d_.valid()) {
			return;
		}

		// no attack weapon => stop here and don't attack
		if (a_.weapon_ < 0) {
			a_.get_unit().set_attacks(a_.get_unit().attacks_left()-1);
			a_.get_unit().set_movement(-1, true);
			return;
		}

		a_.get_unit().set_attacks(a_.get_unit().attacks_left()-1);
		VALIDATE(a_.weapon_ < static_cast<int>(a_.get_unit().attacks().size()),
			_("An invalid attacker weapon got selected."));
		a_.get_unit().set_movement(a_.get_unit().movement_left() -
			a_.get_unit().attacks()[a_.weapon_].movement_used(), true);
		a_.get_unit().set_state(unit::STATE_NOT_MOVED,false);
		a_.get_unit().set_resting(false);
		d_.get_unit().set_resting(false);

		// If the attacker was invisible, she isn't anymore!
		a_.get_unit().set_state(unit::STATE_UNCOVERED, true);

		bc_ = new battle_context(units_, a_.loc_, d_.loc_, a_.weapon_, d_.weapon_);
		a_stats_ = &bc_->get_attacker_stats();
		d_stats_ = &bc_->get_defender_stats();
		if(a_stats_->weapon) {
			a_.weap_id_ = a_stats_->weapon->id();
		}
		if(d_stats_->weapon) {
			d_.weap_id_ = d_stats_->weapon->id();
		}

		try {
			fire_event("attack");
		} catch (attack_end_exception) {
			return;
		}
		refresh_bc();

		DBG_NG << "getting attack statistics\n";
		statistics::attack_context attack_stats(a_.get_unit(), d_.get_unit(), a_stats_->chance_to_hit, d_stats_->chance_to_hit);

		{
			// Calculate stats for battle
			combatant attacker(bc_->get_attacker_stats());
			combatant defender(bc_->get_defender_stats());
			attacker.fight(defender,false);
			const double attacker_inflict = static_cast<double>(d_.get_unit().hitpoints()) - defender.average_hp();
			const double defender_inflict = static_cast<double>(a_.get_unit().hitpoints()) - attacker.average_hp();

			attack_stats.attack_expected_damage(attacker_inflict,defender_inflict);
		}

		a_.orig_attacks_ = a_stats_->num_blows;
		d_.orig_attacks_ = d_stats_->num_blows;
		a_.n_attacks_ = a_.orig_attacks_;
		d_.n_attacks_ = d_.orig_attacks_;
		a_.xp_ = d_.get_unit().level();
		d_.xp_ = a_.get_unit().level();

		bool defender_strikes_first = (d_stats_->firststrike && !a_stats_->firststrike);
		unsigned int rounds = std::max<unsigned int>(a_stats_->rounds, d_stats_->rounds) - 1;
		const int defender_side = d_.get_unit().side();

		LOG_NG << "Fight: (" << a_.loc_ << ") vs (" << d_.loc_ << ") ATT: " <<
			      a_stats_->weapon->name() << " " << a_stats_->damage << "-" <<
			      a_stats_->num_blows << "(" << a_stats_->chance_to_hit << "%) vs DEF: " <<
			      (d_stats_->weapon ? d_stats_->weapon->name() : "none") << " " <<
			      d_stats_->damage << "-" << d_stats_->num_blows <<
			      "(" << d_stats_->chance_to_hit << "%)" <<
			      (defender_strikes_first ? " defender first-strike" : "") << "\n";

		// Play the pre-fight animation
		unit_display::unit_draw_weapon(a_.loc_,a_.get_unit(),a_stats_->weapon,d_stats_->weapon,d_.loc_,&d_.get_unit());

		for (;;)
		{
			DBG_NG << "start of attack loop...\n";
			++abs_n_attack_;

			if (a_.n_attacks_ > 0 && !defender_strikes_first) {
				if (!perform_hit(true, attack_stats)) break;
			}

			// If the defender got to strike first, they use it up here.
			defender_strikes_first = false;
			++abs_n_defend_;

			if (d_.n_attacks_ > 0) {
				if (!perform_hit(false, attack_stats)) break;
			}

			// Continue the fight to death; if one of the units got petrified,
			// either n_attacks or n_defends is -1
			if(rounds > 0 && d_.n_attacks_ == 0 && a_.n_attacks_ == 0) {
				a_.n_attacks_ = a_.orig_attacks_;
				d_.n_attacks_ = d_.orig_attacks_;
				--rounds;
				defender_strikes_first = (d_stats_->firststrike && ! a_stats_->firststrike);
			}

			if (a_.n_attacks_ <= 0 && d_.n_attacks_ <= 0) {
				fire_event("attack_end");
				refresh_bc();
				break;
			}
		}

		if ( update_def_fog_ )
		{
			actions::recalculate_fog(defender_side);
		}

		// TODO: if we knew the viewing team, we could skip this display update
		if (update_minimap_ && update_display_) {
			resources::screen->redraw_minimap();
		}

		if (a_.valid()) {
			unit &u = a_.get_unit();
			u.set_standing();
			u.set_experience(u.experience() + a_.xp_);
		}

		if (d_.valid()) {
			unit &u = d_.get_unit();
			u.set_standing();
			u.set_experience(u.experience() + d_.xp_);
		}

		unit_display::unit_sheath_weapon(a_.loc_,a_.valid()?&a_.get_unit():NULL,a_stats_->weapon,
				d_stats_->weapon,d_.loc_,d_.valid()?&d_.get_unit():NULL);

		if (update_display_){
			resources::screen->invalidate_unit();
			resources::screen->invalidate(a_.loc_);
			resources::screen->invalidate(d_.loc_);
			resources::screen->draw(true, true);
		}

		if(OOS_error_) {
			replay::process_error(errbuf_.str());
		}
	}

} //end anonymous namespace


void attack_unit(const map_location &attacker, const map_location &defender,
	int attack_with, int defend_with, bool update_display)
{
	attack dummy(attacker, defender, attack_with, defend_with, update_display);
	dummy.perform();
}



namespace
{
	class unit_advancement_choice : public mp_sync::user_choice
	{
	public:
		unit_advancement_choice(const map_location& loc, int total_opt, int side_num, const ai::unit_advancements_aspect& ai_advancement, bool force_dialog)
			: loc_ (loc), nb_options_(total_opt), side_num_(side_num), ai_advancement_(ai_advancement), force_dialog_(force_dialog)
		{	
		}
		
		virtual ~unit_advancement_choice() 
		{
		}

		virtual config query_user(int /*side*/) const
		{
			//the 'side' parameter might differ from side_num_- 
			int res = 0;
			team t = (*resources::teams)[side_num_ - 1];
			//i wonder how this got included here ? 
			bool is_mp = network::nconnections() != 0;
			bool is_current_side = resources::controller->current_side() == side_num_;
			//note, that the advancements for networked sides are also determined on the current playing side.
			
			//to make mp games equal we only allow selecting advancements to the current side.
			//otherwise we'd give an unfair advantage to the side that hosts ai sides if units advance during ai turns.
			if(force_dialog_ || (t.is_human() && (is_current_side || !is_mp)))
			{
				res = dialogs::advance_unit_dialog(loc_); 
			}
			else if(t.is_ai() || t.is_network_ai() || t.is_empty() || t.is_idle())
			{
				res = rand() % nb_options_;

				//if ai_advancement_ is the default advancement the following code will 
				//have no effect because get_advancements returns an empty list.
				unit_map::iterator u = resources::units->find(loc_);
				const std::vector<std::string>& options = u->advances_to();
				const std::vector<std::string>& allowed = ai_advancement_.get_advancements(u);

				for(std::vector<std::string>::const_iterator a = options.begin(); a != options.end(); ++a) {
					if (std::find(allowed.begin(), allowed.end(), *a) != allowed.end()){
						res = a - options.begin();
						break;
					}
				}

			}
			else
			{
				//we are in the situation, that the unit is owned by a human, but he's not allowed to do this decision.
				//becasue it's a mp game and it's not his turn.
				//note that it doens't matter wether we call random_new::generator->next_random() or rand().
				res = random_new::generator->next_random() % nb_options_;
			}
			LOG_NG << "unit at position " << loc_ << "choose advancement number " << res << "\n";
			config retv;
			retv["value"] = res;
			return retv;

		}
		virtual config random_choice(int /*side*/) const 
		{
			config retv;
			retv["value"] = 0;
			return retv;
		}
	private:
		const map_location loc_;
		int nb_options_;
		int side_num_;
		const ai::unit_advancements_aspect& ai_advancement_;
		bool force_dialog_;
	};
}

/*
advances the unit and stores data in the replay (or reads data from replay).
*/
void advance_unit_at(const map_location& loc, const ai::unit_advancements_aspect ai_advancement, bool force_dialog)
{
	//i just don't want infinite loops...
	// the 20 is picked rather randomly.
	for(int advacment_number = 0; advacment_number < 20; advacment_number++)
	{
		unit_map::iterator u = resources::units->find(loc);
		//this implies u.valid()
		if(!unit_helper::will_certainly_advance(u)) {
			return;
		}
		
		//we don't want to let side 1 decide it during start/prestart.
		int side_for = resources::gamedata->phase() == game_data::PLAY ? 0: u->side();
		config selected = mp_sync::get_user_choice("choose",
			unit_advancement_choice(loc, unit_helper::number_of_possible_advances(*u),u->side(), ai_advancement, force_dialog), side_for); 
		//calls actions::advance_unit.
		dialogs::animate_unit_advancement(loc, selected["value"], true, true);
		u = resources::units->find(loc);
		// level 10 unit gives 80 XP and the highest mainline is level 5
		if (u.valid() && u->experience() > 80) 
		{
			ERR_NG << "Unit has too many (" << u->experience() << ") XP left; cascade leveling goes on still.\n";
		}
	}
	ERR_NG << "unit at " << loc << "tried to adcance more than 20 times\n";
}


void attack_unit_and_advance(const map_location &attacker, const map_location &defender,
                 int attack_with, int defend_with, bool update_display, 
				 const ai::unit_advancements_aspect& ai_advancement)
{	try
	{
		attack_unit(attacker, defender, attack_with, defend_with, update_display);
	}
	catch(end_level_exception&)
	{

		unit_map::const_iterator atku = resources::units->find(attacker);
		
		if (atku != resources::units->end()) {
			advance_unit_at(attacker, ai_advancement);
		}

		unit_map::const_iterator defu = resources::units->find(defender);
		if (defu != resources::units->end()) {
			advance_unit_at(defender, ai_advancement);
		}
		throw;
	}
	unit_map::const_iterator atku = resources::units->find(attacker);
	if (atku != resources::units->end()) {
		advance_unit_at(attacker, ai_advancement);
	}

	unit_map::const_iterator defu = resources::units->find(defender);
	if (defu != resources::units->end()) {
		advance_unit_at(defender, ai_advancement);
	}
}


unit get_advanced_unit(const unit &u, const std::string& advance_to)
{
	const unit_type *new_type = unit_types.find(advance_to);
	if (!new_type) {
		throw game::game_error("Could not find the unit being advanced"
			" to: " + advance_to);
	}
	unit new_unit(u);
	new_unit.set_experience(new_unit.experience() - new_unit.max_experience());
	new_unit.advance_to(*new_type);
	new_unit.heal_all();
	new_unit.set_state(unit::STATE_POISONED, false);
	new_unit.set_state(unit::STATE_SLOWED, false);
	new_unit.set_state(unit::STATE_PETRIFIED, false);
	new_unit.set_user_end_turn(false);
	new_unit.set_hidden(false);
	return new_unit;
}


/**
 * Returns the AMLA-advanced version of a unit (with traits and items retained).
 */
unit get_amla_unit(const unit &u, const config &mod_option)
{
	unit amla_unit(u);
	amla_unit.set_experience(amla_unit.experience() - amla_unit.max_experience());
	amla_unit.add_modification("advance", mod_option);
	return amla_unit;
}


void advance_unit(map_location loc, const std::string &advance_to,
                  const bool &fire_event, const config * mod_option)
{
	unit_map::unit_iterator u = resources::units->find(loc);
	if(!u.valid()) {
		return;
	}
	// original_type is not a reference, since the unit may disappear at any moment.
	std::string original_type = u->type_id();

	// "advance" event.
	if(fire_event)
	{
		LOG_NG << "Firing advance event at " << loc <<".\n";
		game_events::fire("advance",loc);

		if (!u.valid() || u->experience() < u->max_experience() ||
			u->type_id() != original_type)
		{
			LOG_NG << "WML has invalidated the advancing unit. Aborting.\n";
			return;
		}
		// In case WML moved the unit:
		loc = u->get_location();
	}

	// This is not normally necessary, but if a unit loses power when leveling
	// (e.g. loses "jamming" or ambush), it could be discovered as a result of
	// the advancement.
	std::vector<int> not_seeing = actions::get_sides_not_seeing(*u);

	// Create the advanced unit.
	bool use_amla = mod_option != NULL;
	unit new_unit = use_amla ? get_amla_unit(*u, *mod_option) :
	                           get_advanced_unit(*u, advance_to);
	if ( !use_amla )
	{
		statistics::advance_unit(new_unit);
		preferences::encountered_units().insert(new_unit.type_id());
		LOG_CF << "Added '" << new_unit.type_id() << "' to the encountered units.\n";
	}
	u = resources::units->replace(loc, new_unit).first;

	// Update fog/shroud.
	actions::shroud_clearer clearer;
	clearer.clear_unit(loc, new_unit);

	// "post_advance" event.
	if(fire_event)
	{
		LOG_NG << "Firing post_advance event at " << loc << ".\n";
		game_events::fire("post_advance",loc);
	}

	// "sighted" event(s).
	clearer.fire_events();
	if ( u.valid() )
		actions::actor_sighted(*u, &not_seeing);

	resources::whiteboard->on_gamestate_change();
}

map_location under_leadership(const unit_map& units, const map_location& loc,
                              int* bonus)
{
	const unit_map::const_iterator un = units.find(loc);
	if(un == units.end()) {
		return map_location::null_location;
	}
	unit_ability_list abil = un->get_abilities("leadership");
	if(bonus) {
		*bonus = abil.highest("value").first;
	}
	return abil.highest("value").second;
}

int combat_modifier(const map_location &loc, unit_type::ALIGNMENT alignment,
                    bool is_fearless)
{
	const tod_manager & tod_m = *resources::tod_manager;
	int lawful_bonus = tod_m.get_illuminated_time_of_day(loc).lawful_bonus;
	return generic_combat_modifier(lawful_bonus, alignment, is_fearless);
}

int generic_combat_modifier(int lawful_bonus, unit_type::ALIGNMENT alignment,
                            bool is_fearless) {
	int bonus;
	switch(alignment) {
		case unit_type::LAWFUL:
			bonus = lawful_bonus;
			break;
		case unit_type::NEUTRAL:
			bonus = 0;
			break;
		case unit_type::CHAOTIC:
			bonus = -lawful_bonus;
			break;
		case unit_type::LIMINAL:
			bonus = -abs(lawful_bonus);
			break;
		default:
			bonus = 0;
	}

	if(is_fearless)
		bonus = std::max<int>(bonus, 0);

	return bonus;
}


bool backstab_check(const map_location& attacker_loc,
                    const map_location& defender_loc,
                    const unit_map& units, const std::vector<team>& teams)
{
	const unit_map::const_iterator defender = units.find(defender_loc);
	if(defender == units.end()) return false; // No defender

	map_location adj[6];
	get_adjacent_tiles(defender_loc, adj);
	int i;
	for(i = 0; i != 6; ++i) {
		if(adj[i] == attacker_loc)
			break;
	}
	if(i >= 6) return false;  // Attack not from adjacent location

	const unit_map::const_iterator opp =
		units.find(adj[(i+3)%6]);
	if(opp == units.end()) return false; // No opposite unit
	if (opp->incapacitated()) return false;
	if (size_t(defender->side() - 1) >= teams.size() || size_t(opp->side() - 1) >= teams.size())
		return true; // If sides aren't valid teams, then they are enemies
	if (teams[defender->side() - 1].is_enemy(opp->side()))
		return true; // Defender and opposite are enemies
	return false; // Defender and opposite are friends
}

