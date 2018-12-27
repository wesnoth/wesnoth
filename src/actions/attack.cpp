/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "actions/attack.hpp"

#include "actions/advancement.hpp"
#include "actions/vision.hpp"

#include "ai/lua/aspect_advancements.hpp"
#include "game_config.hpp"
#include "game_data.hpp"
#include "game_events/pump.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "mouse_handler_base.hpp"
#include "play_controller.hpp"
#include "preferences/game.hpp"
#include "random.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "statistics.hpp"
#include "synced_checkup.hpp"
#include "synced_user_choice.hpp"
#include "team.hpp"
#include "tod_manager.hpp"
#include "units/abilities.hpp"
#include "units/animation_component.hpp"
#include "units/helper.hpp"
#include "units/filter.hpp"
#include "units/map.hpp"
#include "units/udisplay.hpp"
#include "units/unit.hpp"
#include "whiteboard/manager.hpp"
#include "wml_exception.hpp"

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(err, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_attack("engine/attack");
#define DBG_AT LOG_STREAM(debug, log_attack)
#define LOG_AT LOG_STREAM(info, log_attack)
#define WRN_AT LOG_STREAM(err, log_attack)
#define ERR_AT LOG_STREAM(err, log_attack)

static lg::log_domain log_config("config");
#define LOG_CF LOG_STREAM(info, log_config)

// ==================================================================================
// BATTLE CONTEXT UNIT STATS
// ==================================================================================

battle_context_unit_stats::battle_context_unit_stats(const unit& u,
		const map_location& u_loc,
		int u_attack_num,
		bool attacking,
		const unit& opp,
		const map_location& opp_loc,
		const_attack_ptr opp_weapon,
		const unit_map& units)
	: weapon(nullptr)
	, attack_num(u_attack_num)
	, is_attacker(attacking)
	, is_poisoned(u.get_state(unit::STATE_POISONED))
	, is_slowed(u.get_state(unit::STATE_SLOWED))
	, slows(false)
	, drains(false)
	, petrifies(false)
	, plagues(false)
	, poisons(false)
	, backstab_pos(false)
	, swarm(false)
	, firststrike(false)
	, disable(false)
	, experience(u.experience())
	, max_experience(u.max_experience())
	, level(u.level())
	, rounds(1)
	, hp(0)
	, max_hp(u.max_hitpoints())
	, chance_to_hit(0)
	, damage(0)
	, slow_damage(0)
	, drain_percent(0)
	, drain_constant(0)
	, num_blows(0)
	, swarm_min(0)
	, swarm_max(0)
	, plague_type()
{
	// Get the current state of the unit.
	if(attack_num >= 0) {
		weapon = u.attacks()[attack_num].shared_from_this();
	}

	if(u.hitpoints() < 0) {
		LOG_CF << "Unit with " << u.hitpoints() << " hitpoints found, set to 0 for damage calculations\n";
		hp = 0;
	} else if(u.hitpoints() > u.max_hitpoints()) {
		// If a unit has more hp than its maximum, the engine will fail with an
		// assertion failure due to accessing the prob_matrix out of bounds.
		hp = u.max_hitpoints();
	} else {
		hp = u.hitpoints();
	}

	// Exit if no weapon.
	if(!weapon) {
		return;
	}

	// Get the weapon characteristics as appropriate.
	auto ctx = weapon->specials_context(&u, &opp, u_loc, opp_loc, attacking, opp_weapon);
	boost::optional<decltype(ctx)> opp_ctx;

	if(opp_weapon) {
		opp_ctx.emplace(opp_weapon->specials_context(&opp, &u, opp_loc, u_loc, !attacking, weapon));
	}

	slows = weapon->bool_ability("slow");
	drains = !opp.get_state("undrainable") && weapon->bool_ability("drains");
	petrifies = weapon->bool_ability("petrifies");
	poisons = !opp.get_state("unpoisonable") && weapon->bool_ability("poison") && !opp.get_state(unit::STATE_POISONED);
	backstab_pos = is_attacker && backstab_check(u_loc, opp_loc, units, resources::gameboard->teams());
	rounds = weapon->get_specials("berserk").highest("value", 1).first;
	if(weapon->combat_ability("berserk", 1).second) {
		rounds = weapon->combat_ability("berserk", 1).first;
	}
	firststrike = weapon->bool_ability("firststrike");

	{
		const int distance = distance_between(u_loc, opp_loc);
		const bool out_of_range = distance > weapon->max_range() || distance < weapon->min_range();
		disable = weapon->get_special_bool("disable") || out_of_range;
	}

	// Handle plague.
	unit_ability_list plague_specials = weapon->get_specials("plague");
	plagues = !opp.get_state("unplagueable") && !plague_specials.empty() &&
		opp.undead_variation() != "null" && !resources::gameboard->map().is_village(opp_loc);

	if(plagues) {
		plague_type = (*plague_specials.front().first)["type"].str();

		if(plague_type.empty()) {
			plague_type = u.type().base_id();
		}
	}

	// Compute chance to hit.
	signed int cth = opp.defense_modifier(resources::gameboard->map().get_terrain(opp_loc)) + weapon->accuracy()
		- (opp_weapon ? opp_weapon->parry() : 0);

	cth = utils::clamp(cth, 0, 100);

	unit_ability_list cth_specials = weapon->get_specials("chance_to_hit");
	unit_abilities::effect cth_effects(cth_specials, cth, backstab_pos);
	cth = cth_effects.get_composite_value();

	cth = utils::clamp(cth, 0, 100);

	cth = weapon->combat_ability("chance_to_hit", cth, backstab_pos).first;

	if(opp.get_state("invulnerable")) {
		cth = 0;
	}

	chance_to_hit = utils::clamp(cth, 0, 100);

	// Compute base damage done with the weapon.
	int base_damage = weapon->modified_damage(backstab_pos);

	// Get the damage multiplier applied to the base damage of the weapon.
	int damage_multiplier = 100;

	// Time of day bonus.
	damage_multiplier += combat_modifier(
			resources::gameboard->units(), resources::gameboard->map(), u_loc, u.alignment(), u.is_fearless());

	// Leadership bonus.
	int leader_bonus = under_leadership(u, u_loc, weapon, opp_weapon);
	if(leader_bonus != 0) {
		damage_multiplier += leader_bonus;
	}

	// Resistance modifier.
	damage_multiplier *= opp.damage_from(*weapon, !attacking, opp_loc, opp_weapon);

	// Compute both the normal and slowed damage.
	damage = round_damage(base_damage, damage_multiplier, 10000);
	slow_damage = round_damage(base_damage, damage_multiplier, 20000);

	if(is_slowed) {
		damage = slow_damage;
	}

	// Compute drain amounts only if draining is possible.
	if(drains) {
		if (weapon->get_special_bool("drains")) {
			unit_ability_list drain_specials = weapon->get_specials("drains");
			// Compute the drain percent (with 50% as the base for backward compatibility)
			unit_abilities::effect drain_percent_effects(drain_specials, 50, backstab_pos);
			drain_percent = drain_percent_effects.get_composite_value();
		}
		if (weapon->combat_ability("drains", 50, backstab_pos).second) {
			drain_percent = weapon->combat_ability("drains", 50, backstab_pos).first;
		}
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

battle_context_unit_stats::battle_context_unit_stats(const unit_type* u_type,
		const_attack_ptr att_weapon,
		bool attacking,
		const unit_type* opp_type,
		const_attack_ptr opp_weapon,
		unsigned int opp_terrain_defense,
		int lawful_bonus)
	: weapon(att_weapon)
	, attack_num(-2) // This is and stays invalid. Always use weapon when using this constructor.
	, is_attacker(attacking)
	, is_poisoned(false)
	, is_slowed(false)
	, slows(false)
	, drains(false)
	, petrifies(false)
	, plagues(false)
	, poisons(false)
	, backstab_pos(false)
	, swarm(false)
	, firststrike(false)
	, disable(false)
	, experience(0)
	, max_experience(0)
	, level(0)
	, rounds(1)
	, hp(0)
	, max_hp(0)
	, chance_to_hit(0)
	, damage(0)
	, slow_damage(0)
	, drain_percent(0)
	, drain_constant(0)
	, num_blows(0)
	, swarm_min(0)
	, swarm_max(0)
	, plague_type()
{
	if(!u_type || !opp_type) {
		return;
	}

	// Get the current state of the unit.
	if(u_type->hitpoints() < 0) {
		hp = 0;
	} else {
		hp = u_type->hitpoints();
	}

	max_experience = u_type->experience_needed();
	level = (u_type->level());
	max_hp = (u_type->hitpoints());

	// Exit if no weapon.
	if(!weapon) {
		return;
	}

	// Get the weapon characteristics as appropriate.
	auto ctx = weapon->specials_context(*u_type, map_location::null_location(), attacking);
	boost::optional<decltype(ctx)> opp_ctx;

	if(opp_weapon) {
		opp_ctx.emplace(opp_weapon->specials_context(*opp_type, map_location::null_location(), !attacking));
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
		opp_type->undead_variation() != "null";

	if(plagues) {
		plague_type = (*plague_specials.front().first)["type"].str();
		if(plague_type.empty()) {
			plague_type = u_type->base_id();
		}
	}

	signed int cth = 100 - opp_terrain_defense + weapon->accuracy() - (opp_weapon ? opp_weapon->parry() : 0);
	cth = utils::clamp(cth, 0, 100);

	unit_ability_list cth_specials = weapon->get_specials("chance_to_hit");
	unit_abilities::effect cth_effects(cth_specials, cth, backstab_pos);
	cth = cth_effects.get_composite_value();

	chance_to_hit = utils::clamp(cth, 0, 100);

	int base_damage = weapon->modified_damage(backstab_pos);
	int damage_multiplier = 100;
	damage_multiplier
			+= generic_combat_modifier(lawful_bonus, u_type->alignment(), u_type->musthave_status("fearless"), 0);
	damage_multiplier *= opp_type->resistance_against(weapon->type(), !attacking);

	damage = round_damage(base_damage, damage_multiplier, 10000);
	slow_damage = round_damage(base_damage, damage_multiplier, 20000);

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


// ==================================================================================
// BATTLE CONTEXT
// ==================================================================================

battle_context::battle_context(
		const unit& attacker,
		const map_location& a_loc,
		int a_wep_index,
		const unit& defender,
		const map_location& d_loc,
		int d_wep_index,
		const unit_map& units)
	: attacker_stats_()
	, defender_stats_()
	, attacker_combatant_()
	, defender_combatant_()
{
	size_t a_wep_uindex = static_cast<size_t>(a_wep_index);
	size_t d_wep_uindex = static_cast<size_t>(d_wep_index);

	const_attack_ptr a_wep(a_wep_uindex < attacker.attacks().size() ? attacker.attacks()[a_wep_index].shared_from_this() : nullptr);
	const_attack_ptr d_wep(d_wep_uindex < defender.attacks().size() ? defender.attacks()[d_wep_index].shared_from_this() : nullptr);
	
	attacker_stats_.reset(new battle_context_unit_stats(attacker, a_loc, a_wep_index, true , defender, d_loc, d_wep, units));
	defender_stats_.reset(new battle_context_unit_stats(defender, d_loc, d_wep_index, false, attacker, a_loc, a_wep, units));
}

void battle_context::simulate(const combatant* prev_def)
{
	assert((attacker_combatant_.get() != nullptr) == (defender_combatant_.get() != nullptr));
	assert(attacker_stats_);
	assert(defender_stats_);
	if(!attacker_combatant_) {
		attacker_combatant_.reset(new combatant(*attacker_stats_));
		defender_combatant_.reset(new combatant(*defender_stats_, prev_def));
		attacker_combatant_->fight(*defender_combatant_);
	}
}

// more like a factory method than a constructor, always calls one of the other constructors.
battle_context::battle_context(const unit_map& units,
		const map_location& attacker_loc,
		const map_location& defender_loc,
		int attacker_weapon,
		int defender_weapon,
		double aggression,
		const combatant* prev_def,
		const unit* attacker_ptr,
		const unit* defender_ptr)
	: attacker_stats_(nullptr)
	, defender_stats_(nullptr)
	, attacker_combatant_(nullptr)
	, defender_combatant_(nullptr)
{
	//TODO: maybe check before dereferencing units.find(attacker_loc),units.find(defender_loc) ?
	const unit& attacker = attacker_ptr ? *attacker_ptr : *units.find(attacker_loc);
	const unit& defender = defender_ptr ? *defender_ptr : *units.find(defender_loc);
	const double harm_weight = 1.0 - aggression;

	if(attacker_weapon == -1) {
		*this = choose_attacker_weapon(
			attacker, defender, units, attacker_loc, defender_loc, harm_weight, prev_def
		);
	}
	else if(defender_weapon == -1) {
		*this = choose_defender_weapon(
			attacker, defender, attacker_weapon, units, attacker_loc, defender_loc, prev_def
		);
	}
	else {
		*this = battle_context(attacker, attacker_loc, attacker_weapon, defender, defender_loc, defender_weapon, units);
	}

	assert(attacker_stats_);
	assert(defender_stats_);
}

battle_context::battle_context(const battle_context_unit_stats& att, const battle_context_unit_stats& def)
	: attacker_stats_(new battle_context_unit_stats(att))
	, defender_stats_(new battle_context_unit_stats(def))
	, attacker_combatant_(nullptr)
	, defender_combatant_(nullptr)
{
}


/** @todo FIXME: better to initialize combatant initially (move into
				 battle_context_unit_stats?), just do fight() when required. */
const combatant& battle_context::get_attacker_combatant(const combatant* prev_def)
{
	// We calculate this lazily, since AI doesn't always need it.
	simulate(prev_def);
	return *attacker_combatant_;
}

const combatant& battle_context::get_defender_combatant(const combatant* prev_def)
{
	// We calculate this lazily, since AI doesn't always need it.
	simulate(prev_def);
	return *defender_combatant_;
}

// Given this harm_weight, are we better than that other context?
bool battle_context::better_attack(class battle_context& that, double harm_weight)
{
	return better_combat(
		get_attacker_combatant(),
		get_defender_combatant(),
		that.get_attacker_combatant(),
		that.get_defender_combatant(),
		harm_weight
	);
}

// Given this harm_weight, are we better than that other context?
bool battle_context::better_defense(class battle_context& that, double harm_weight)
{
	return better_combat(
		get_defender_combatant(),
		get_attacker_combatant(),
		that.get_defender_combatant(),
		that.get_attacker_combatant(),
		harm_weight
	);
}

// Does combat A give us a better result than combat B?
bool battle_context::better_combat(const combatant& us_a,
		const combatant& them_a,
		const combatant& us_b,
		const combatant& them_b,
		double harm_weight)
{
	double a, b;

	// Compare: P(we kill them) - P(they kill us).
	a = them_a.hp_dist[0] - us_a.hp_dist[0] * harm_weight;
	b = them_b.hp_dist[0] - us_b.hp_dist[0] * harm_weight;

	if(a - b < -0.01) {
		return false;
	}

	if(a - b > 0.01) {
		return true;
	}

	// Add poison to calculations
	double poison_a_us = (us_a.poisoned) * game_config::poison_amount;
	double poison_a_them = (them_a.poisoned) * game_config::poison_amount;
	double poison_b_us = (us_b.poisoned) * game_config::poison_amount;
	double poison_b_them = (them_b.poisoned) * game_config::poison_amount;

	// Compare: damage to them - damage to us (average_hp replaces -damage)
	a = (us_a.average_hp() - poison_a_us) * harm_weight - (them_a.average_hp() - poison_a_them);
	b = (us_b.average_hp() - poison_b_us) * harm_weight - (them_b.average_hp() - poison_b_them);

	if(a - b < -0.01) {
		return false;
	}

	if(a - b > 0.01) {
		return true;
	}

	// All else equal: go for most damage.
	return them_a.average_hp() < them_b.average_hp();
}

battle_context battle_context::choose_attacker_weapon(const unit& attacker,
		const unit& defender,
		const unit_map& units,
		const map_location& attacker_loc,
		const map_location& defender_loc,
		double harm_weight,
		const combatant* prev_def)
{
	log_scope2(log_attack, "choose_attacker_weapon");
	std::vector<battle_context> choices;

	// What options does attacker have?
	for(size_t i = 0; i < attacker.attacks().size(); ++i) {
		const attack_type& att = attacker.attacks()[i];

		if(att.attack_weight() <= 0) {
			continue;
		}
		battle_context bc = choose_defender_weapon(attacker, defender, i, units, attacker_loc, defender_loc, prev_def);
		//choose_defender_weapon will always choose the weapon that disabels the attackers weapon if possible.
		if(bc.attacker_stats_->disable) {
			continue;
		}
		choices.emplace_back(std::move(bc));
	}

	if(choices.empty()) {
		return battle_context(attacker, attacker_loc, -1, defender, defender_loc, -1, units);
	}

	if(choices.size() == 1) {
		return std::move(choices[0]);
	}

	// Multiple options: simulate them, save best.
	battle_context* best_choice = nullptr;
	for(auto& choice : choices) {
		// If choose_defender_weapon didn't simulate, do so now.
		choice.simulate(prev_def);

		if(!best_choice || choice.better_attack(*best_choice, harm_weight)) {
			best_choice = &choice;
		}
	}

	if(best_choice) {
		return std::move(*best_choice);
	}
	else {
		return battle_context(attacker, attacker_loc, -1, defender, defender_loc, -1, units);
	}
}

/** @todo FIXME: Hand previous defender unit in here. */
battle_context battle_context::choose_defender_weapon(const unit& attacker,
		const unit& defender,
		unsigned attacker_weapon,
		const unit_map& units,
		const map_location& attacker_loc,
		const map_location& defender_loc,
		const combatant* prev_def)
{
	log_scope2(log_attack, "choose_defender_weapon");
	VALIDATE(attacker_weapon < attacker.attacks().size(), _("An invalid attacker weapon got selected."));

	const attack_type& att = attacker.attacks()[attacker_weapon];
	auto no_weapon = [&]() { return battle_context(attacker, attacker_loc, attacker_weapon, defender, defender_loc, -1, units); };
	std::vector<battle_context> choices;

	// What options does defender have?
	for(size_t i = 0; i < defender.attacks().size(); ++i) {
		const attack_type& def = defender.attacks()[i];
		if(def.range() != att.range() || def.defense_weight() <= 0) {
			//no need to calculate the battle_context here.
			continue;
		}
		battle_context bc(attacker, attacker_loc, attacker_weapon, defender, defender_loc, i, units);

		if(bc.defender_stats_->disable) {
			continue;
		}
		if(bc.attacker_stats_->disable) {
			//the defenders attack disables the attakers attack: always choose this one.
			return bc;
		}
		choices.emplace_back(std::move(bc));
	}

	if(choices.empty()) {
		return no_weapon();
	}

	if(choices.size() == 1) {
		//only one usable weapon, don't simulate
		return std::move(choices[0]);
	}

	// Multiple options:
	// First pass : get the best weight and the minimum simple rating for this weight.
	// simple rating = number of blows * damage per blows (resistance taken in account) * cth * weight
	// Eligible attacks for defense should have a simple rating greater or equal to this weight.

	int min_rating = 0;
	{
		double max_weight = 0.0;

		for(const auto& choice : choices) {
			const attack_type& def = defender.attacks()[choice.defender_stats_->attack_num];

			if(def.defense_weight() >= max_weight) {
				const battle_context_unit_stats& def_stats = *choice.defender_stats_;

				max_weight = def.defense_weight();
				int rating = static_cast<int>(
						def_stats.num_blows * def_stats.damage * def_stats.chance_to_hit * def.defense_weight());

				if(def.defense_weight() > max_weight || rating < min_rating) {
					min_rating = rating;
				}
			}
		}
	}

	battle_context* best_choice = nullptr;
	// Multiple options: simulate them, save best.
	for(auto& choice : choices) {
		const attack_type& def = defender.attacks()[choice.defender_stats_->attack_num];

		choice.simulate(prev_def);


		int simple_rating = static_cast<int>(
				choice.defender_stats_->num_blows * choice.defender_stats_->damage * choice.defender_stats_->chance_to_hit * def.defense_weight());

		//FIXME: make sure there is no mostake in the better_combat call-
		if(simple_rating >= min_rating && (!best_choice || choice.better_defense(*best_choice, 1.0))) {
			best_choice = &choice;
		}
	}

	return best_choice ? std::move(*best_choice) : no_weapon();
}


// ==================================================================================
// HELPERS
// ==================================================================================

namespace
{
void refresh_weapon_index(int& weap_index, const std::string& weap_id, attack_itors attacks)
{
	// No attacks to choose from.
	if(attacks.empty()) {
		weap_index = -1;
		return;
	}

	// The currently selected attack fits.
	if(weap_index >= 0 && weap_index < static_cast<int>(attacks.size()) && attacks[weap_index].id() == weap_id) {
		return;
	}

	// Look up the weapon by id.
	if(!weap_id.empty()) {
		for(int i = 0; i < static_cast<int>(attacks.size()); ++i) {
			if(attacks[i].id() == weap_id) {
				weap_index = i;
				return;
			}
		}
	}

	// Lookup has failed.
	weap_index = -1;
	return;
}

/** Helper class for performing an attack. */
class attack
{
public:
	attack(const map_location& attacker,
			const map_location& defender,
			int attack_with,
			int defend_with,
			bool update_display = true);

	void perform();

private:
	class attack_end_exception
	{
	};

	bool perform_hit(bool, statistics::attack_context&);
	void fire_event(const std::string& n);
	void refresh_bc();

	/** Structure holding unit info used in the attack action. */
	struct unit_info
	{
		const map_location loc_;
		int weapon_;
		unit_map& units_;
		std::size_t id_; /**< unit.underlying_id() */
		std::string weap_id_;
		int orig_attacks_;
		int n_attacks_; /**< Number of attacks left. */
		int cth_;
		int damage_;
		int xp_;

		unit_info(const map_location& loc, int weapon, unit_map& units);
		unit& get_unit();
		bool valid();

		std::string dump();
	};

	/**
	 * Used in perform_hit to confirm a replay is in sync.
	 * Check OOS_error_ after this method, true if error detected.
	 */
	void check_replay_attack_result(bool&, int, int&, config, unit_info&);

	void unit_killed(
			unit_info&, unit_info&, const battle_context_unit_stats*&, const battle_context_unit_stats*&, bool);

	std::unique_ptr<battle_context> bc_;

	const battle_context_unit_stats* a_stats_;
	const battle_context_unit_stats* d_stats_;

	int abs_n_attack_, abs_n_defend_;
	// update_att_fog_ is not used, other than making some code simpler.
	bool update_att_fog_, update_def_fog_, update_minimap_;

	unit_info a_, d_;
	unit_map& units_;
	std::ostringstream errbuf_;

	bool update_display_;
	bool OOS_error_;

	bool use_prng_;

	std::vector<bool> prng_attacker_;
	std::vector<bool> prng_defender_;
};

attack::unit_info::unit_info(const map_location& loc, int weapon, unit_map& units)
	: loc_(loc)
	, weapon_(weapon)
	, units_(units)
	, id_()
	, weap_id_()
	, orig_attacks_(0)
	, n_attacks_(0)
	, cth_(0)
	, damage_(0)
	, xp_(0)
{
	unit_map::iterator i = units_.find(loc_);
	if(!i.valid()) {
		return;
	}

	id_ = i->underlying_id();
}

unit& attack::unit_info::get_unit()
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
	s << get_unit().type_id() << " (" << loc_.wml_x() << ',' << loc_.wml_y() << ')';
	return s.str();
}

attack::attack(const map_location& attacker,
		const map_location& defender,
		int attack_with,
		int defend_with,
		bool update_display)
	: bc_(nullptr)
	, a_stats_(nullptr)
	, d_stats_(nullptr)
	, abs_n_attack_(0)
	, abs_n_defend_(0)
	, update_att_fog_(false)
	, update_def_fog_(false)
	, update_minimap_(false)
	, a_(attacker, attack_with, resources::gameboard->units())
	, d_(defender, defend_with, resources::gameboard->units())
	, units_(resources::gameboard->units())
	, errbuf_()
	, update_display_(update_display)
	, OOS_error_(false)

	//new experimental prng mode.
	, use_prng_(preferences::get("use_prng") == "yes" && randomness::generator->is_networked() == false)
{
	if(use_prng_) {
		std::cerr << "Using experimental PRNG for combat\n";
	}
}

void attack::fire_event(const std::string& n)
{
	LOG_NG << "attack: firing '" << n << "' event\n";

	// prepare the event data for weapon filtering
	config ev_data;
	config& a_weapon_cfg = ev_data.add_child("first");
	config& d_weapon_cfg = ev_data.add_child("second");

	// Need these to ensure weapon filters work correctly
	boost::optional<attack_type::specials_context_t> a_ctx, d_ctx;

	if(a_stats_->weapon != nullptr && a_.valid()) {
		if(d_stats_->weapon != nullptr && d_.valid()) {
			a_ctx.emplace(a_stats_->weapon->specials_context(nullptr, nullptr, a_.loc_, d_.loc_, true, d_stats_->weapon));
		} else {
			a_ctx.emplace(a_stats_->weapon->specials_context(nullptr, a_.loc_, true));
		}
		a_stats_->weapon->write(a_weapon_cfg);
	}

	if(d_stats_->weapon != nullptr && d_.valid()) {
		if(a_stats_->weapon != nullptr && a_.valid()) {
			d_ctx.emplace(d_stats_->weapon->specials_context(nullptr, nullptr, d_.loc_, a_.loc_, false, a_stats_->weapon));
		} else {
			d_ctx.emplace(d_stats_->weapon->specials_context(nullptr, d_.loc_, false));
		}
		d_stats_->weapon->write(d_weapon_cfg);
	}

	if(a_weapon_cfg["name"].empty()) {
		a_weapon_cfg["name"] = "none";
	}

	if(d_weapon_cfg["name"].empty()) {
		d_weapon_cfg["name"] = "none";
	}

	if(n == "attack_end") {
		// We want to fire attack_end event in any case! Even if one of units was removed by WML.
		resources::game_events->pump().fire(n, a_.loc_, d_.loc_, ev_data);
		return;
	}

	// damage_inflicted is set in these two events.
	// TODO: should we set this value from unit_info::damage, or continue using the WML variable?
	if(n == "attacker_hits" || n == "defender_hits") {
		ev_data["damage_inflicted"] = resources::gamedata->get_variable("damage_inflicted");
	}

	const int defender_side = d_.get_unit().side();

	bool wml_aborted;
	std::tie(std::ignore, wml_aborted) = resources::game_events->pump().fire(n,
		game_events::entity_location(a_.loc_, a_.id_),
		game_events::entity_location(d_.loc_, d_.id_), ev_data);

	// The event could have killed either the attacker or
	// defender, so we have to make sure they still exist.
	refresh_bc();

	if(wml_aborted || !a_.valid() || !d_.valid()
		|| !resources::gameboard->get_team(a_.get_unit().side()).is_enemy(d_.get_unit().side())
	) {
		actions::recalculate_fog(defender_side);

		if(update_display_) {
			display::get_singleton()->redraw_minimap();
		}

		fire_event("attack_end");
		throw attack_end_exception();
	}
}

void attack::refresh_bc()
{
	// Fix index of weapons.
	if(a_.valid()) {
		refresh_weapon_index(a_.weapon_, a_.weap_id_, a_.get_unit().attacks());
	}

	if(d_.valid()) {
		refresh_weapon_index(d_.weapon_, d_.weap_id_, d_.get_unit().attacks());
	}

	if(!a_.valid() || !d_.valid()) {
		// Fix pointer to weapons.
		const_cast<battle_context_unit_stats*>(a_stats_)->weapon
				= a_.valid() && a_.weapon_ >= 0 ? a_.get_unit().attacks()[a_.weapon_].shared_from_this() : nullptr;

		const_cast<battle_context_unit_stats*>(d_stats_)->weapon
				= d_.valid() && d_.weapon_ >= 0 ? d_.get_unit().attacks()[d_.weapon_].shared_from_this() : nullptr;

		return;
	}

	bc_.reset(new battle_context(units_, a_.loc_, d_.loc_, a_.weapon_, d_.weapon_));

	a_stats_ = &bc_->get_attacker_stats();
	d_stats_ = &bc_->get_defender_stats();

	a_.cth_ = a_stats_->chance_to_hit;
	d_.cth_ = d_stats_->chance_to_hit;
	a_.damage_ = a_stats_->damage;
	d_.damage_ = d_stats_->damage;
}

bool attack::perform_hit(bool attacker_turn, statistics::attack_context& stats)
{
	unit_info& attacker = attacker_turn ? a_ : d_;
	unit_info& defender = attacker_turn ? d_ : a_;

	// NOTE: we need to use a reference-to-pointer here so a_stats_ and d_stats_ can be
	// modified without. Using a pointer directly would render them invalid when that happened.
	const battle_context_unit_stats*& attacker_stats = attacker_turn ? a_stats_ : d_stats_;
	const battle_context_unit_stats*& defender_stats = attacker_turn ? d_stats_ : a_stats_;

	int& abs_n = attacker_turn ? abs_n_attack_ : abs_n_defend_;
	bool& update_fog = attacker_turn ? update_def_fog_ : update_att_fog_;

	int ran_num;
        
	if(use_prng_) {

		std::vector<bool>& prng_seq = attacker_turn ? prng_attacker_ : prng_defender_;

		if(prng_seq.empty()) {
			const int ntotal = attacker.cth_*attacker.n_attacks_;
			int num_hits = ntotal/100;
			const int additional_hit_chance = ntotal%100;
			if(additional_hit_chance > 0 && randomness::generator->get_random_int(0, 99) < additional_hit_chance) {
				++num_hits;
			}

			std::vector<int> indexes;
			for(int i = 0; i != attacker.n_attacks_; ++i) {
				prng_seq.push_back(false);
				indexes.push_back(i);
			}

			for(int i = 0; i != num_hits; ++i) {
				int n = randomness::generator->get_random_int(0, static_cast<int>(indexes.size())-1);
				prng_seq[indexes[n]] = true;
				indexes.erase(indexes.begin() + n);
			}
		}

		bool does_hit = prng_seq.back();
		prng_seq.pop_back();
		ran_num = does_hit ? 0 : 99;
	} else {
		ran_num = randomness::generator->get_random_int(0, 99);
	}
	bool hits = (ran_num < attacker.cth_);

	int damage = 0;
	if(hits) {
		damage = attacker.damage_;
		resources::gamedata->get_variable("damage_inflicted") = damage;
	}

	// Make sure that if we're serializing a game here,
	// we got the same results as the game did originally.
	const config local_results {"chance", attacker.cth_, "hits", hits, "damage", damage};

	config replay_results;
	bool equals_replay = checkup_instance->local_checkup(local_results, replay_results);

	if(!equals_replay) {
		check_replay_attack_result(hits, ran_num, damage, replay_results, attacker);
	}

	// can do no more damage than the defender has hitpoints
	int damage_done = std::min<int>(defender.get_unit().hitpoints(), attacker.damage_);

	// expected damage = damage potential * chance to hit (as a percentage)
	double expected_damage = damage_done * attacker.cth_ * 0.01;

	if(attacker_turn) {
		stats.attack_expected_damage(expected_damage, 0);
	} else {
		stats.attack_expected_damage(0, expected_damage);
	}

	int drains_damage = 0;
	if(hits && attacker_stats->drains) {
		drains_damage = damage_done * attacker_stats->drain_percent / 100 + attacker_stats->drain_constant;

		// don't drain so much that the attacker gets more than his maximum hitpoints
		drains_damage =
			std::min<int>(drains_damage, attacker.get_unit().max_hitpoints() - attacker.get_unit().hitpoints());

		// if drain is negative, don't allow drain to kill the attacker
		drains_damage = std::max<int>(drains_damage, 1 - attacker.get_unit().hitpoints());
	}

	if(update_display_) {
		std::ostringstream float_text;
		std::vector<std::string> extra_hit_sounds;

		if(hits) {
			const unit& defender_unit = defender.get_unit();
			if(attacker_stats->poisons && !defender_unit.get_state(unit::STATE_POISONED)) {
				float_text << (defender_unit.gender() == unit_race::FEMALE ? _("female^poisoned") : _("poisoned"))
						   << '\n';

				extra_hit_sounds.push_back(game_config::sounds::status::poisoned);
			}

			if(attacker_stats->slows && !defender_unit.get_state(unit::STATE_SLOWED)) {
				float_text << (defender_unit.gender() == unit_race::FEMALE ? _("female^slowed") : _("slowed")) << '\n';

				extra_hit_sounds.push_back(game_config::sounds::status::slowed);
			}

			if(attacker_stats->petrifies) {
				float_text << (defender_unit.gender() == unit_race::FEMALE ? _("female^petrified") : _("petrified"))
						   << '\n';

				extra_hit_sounds.push_back(game_config::sounds::status::petrified);
			}
		}

		unit_display::unit_attack(
			game_display::get_singleton(),
			*resources::gameboard,
			attacker.loc_, defender.loc_,
			damage,
			*attacker_stats->weapon, defender_stats->weapon,
			abs_n, float_text.str(), drains_damage, "",
			&extra_hit_sounds
		);
	}

	bool dies = defender.get_unit().take_hit(damage);
	LOG_NG << "defender took " << damage << (dies ? " and died\n" : "\n");

	if(attacker_turn) {
		stats.attack_result(hits
			? (dies
				? statistics::attack_context::KILLS
				: statistics::attack_context::HITS)
			: statistics::attack_context::MISSES, damage_done, drains_damage
		);
	} else {
		stats.defend_result(hits
			? (dies
				? statistics::attack_context::KILLS
				: statistics::attack_context::HITS)
			: statistics::attack_context::MISSES, damage_done, drains_damage
		);
	}

	replay_results.clear();

	// There was also a attribute cfg["unit_hit"] which was never used so i deleted.
	equals_replay = checkup_instance->local_checkup(config{"dies", dies}, replay_results);

	if(!equals_replay) {
		bool results_dies = replay_results["dies"].to_bool();

		errbuf_ << "SYNC: In attack " << a_.dump() << " vs " << d_.dump() << ": the data source says the "
				<< (attacker_turn ? "defender" : "attacker") << ' ' << (results_dies ? "perished" : "survived")
				<< " while in-game calculations show it " << (dies ? "perished" : "survived")
				<< " (over-riding game calculations with data source results)\n";

		dies = results_dies;

		// Set hitpoints to 0 so later checks don't invalidate the death.
		if(results_dies) {
			defender.get_unit().set_hitpoints(0);
		}

		OOS_error_ = true;
	}

	if(hits) {
		try {
			fire_event(attacker_turn ? "attacker_hits" : "defender_hits");
		} catch(const attack_end_exception&) {
			refresh_bc();
			return false;
		}
	} else {
		try {
			fire_event(attacker_turn ? "attacker_misses" : "defender_misses");
		} catch(const attack_end_exception&) {
			refresh_bc();
			return false;
		}
	}

	refresh_bc();

	bool attacker_dies = false;

	if(drains_damage > 0) {
		attacker.get_unit().heal(drains_damage);
	} else if(drains_damage < 0) {
		attacker_dies = attacker.get_unit().take_hit(-drains_damage);
	}

	if(dies) {
		unit_killed(attacker, defender, attacker_stats, defender_stats, false);
		update_fog = true;
	}

	if(attacker_dies) {
		unit_killed(defender, attacker, defender_stats, attacker_stats, true);
		(attacker_turn ? update_att_fog_ : update_def_fog_) = true;
	}

	if(dies) {
		update_minimap_ = true;
		return false;
	}

	if(hits) {
		unit& defender_unit = defender.get_unit();

		if(attacker_stats->poisons && !defender_unit.get_state(unit::STATE_POISONED)) {
			defender_unit.set_state(unit::STATE_POISONED, true);
			LOG_NG << "defender poisoned\n";
		}

		if(attacker_stats->slows && !defender_unit.get_state(unit::STATE_SLOWED)) {
			defender_unit.set_state(unit::STATE_SLOWED, true);
			update_fog = true;
			defender.damage_ = defender_stats->slow_damage;
			LOG_NG << "defender slowed\n";
		}

		// If the defender is petrified, the fight stops immediately
		if(attacker_stats->petrifies) {
			defender_unit.set_state(unit::STATE_PETRIFIED, true);
			update_fog = true;
			attacker.n_attacks_ = 0;
			defender.n_attacks_ = -1; // Petrified.
			resources::game_events->pump().fire("petrified", defender.loc_, attacker.loc_);
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

void attack::unit_killed(unit_info& attacker,
		unit_info& defender,
		const battle_context_unit_stats*& attacker_stats,
		const battle_context_unit_stats*& defender_stats,
		bool drain_killed)
{
	attacker.xp_ = game_config::kill_xp(defender.get_unit().level());
	defender.xp_ = 0;

	display::get_singleton()->invalidate(attacker.loc_);

	game_events::entity_location death_loc(defender.loc_, defender.id_);
	game_events::entity_location attacker_loc(attacker.loc_, attacker.id_);

	std::string undead_variation = defender.get_unit().undead_variation();

	fire_event("attack_end");
	refresh_bc();

	// Get weapon info for last_breath and die events.
	config dat;
	config a_weapon_cfg = attacker_stats->weapon && attacker.valid() ? attacker_stats->weapon->to_config() : config();
	config d_weapon_cfg = defender_stats->weapon && defender.valid() ? defender_stats->weapon->to_config() : config();

	if(a_weapon_cfg["name"].empty()) {
		a_weapon_cfg["name"] = "none";
	}

	if(d_weapon_cfg["name"].empty()) {
		d_weapon_cfg["name"] = "none";
	}

	dat.add_child("first", d_weapon_cfg);
	dat.add_child("second", a_weapon_cfg);

	resources::game_events->pump().fire("last_breath", death_loc, attacker_loc, dat);
	refresh_bc();

	// WML has invalidated the dying unit, abort.
	if(!defender.valid() || defender.get_unit().hitpoints() > 0) {
		return;
	}

	if(!attacker.valid()) {
		unit_display::unit_die(
			defender.loc_,
			defender.get_unit(),
			nullptr,
			defender_stats->weapon
		);
	} else {
		unit_display::unit_die(
			defender.loc_,
			defender.get_unit(),
			attacker_stats->weapon,
			defender_stats->weapon,
			attacker.loc_,
			&attacker.get_unit()
		);
	}

	resources::game_events->pump().fire("die", death_loc, attacker_loc, dat);
	refresh_bc();

	if(!defender.valid() || defender.get_unit().hitpoints() > 0) {
		// WML has invalidated the dying unit, abort
		return;
	}

	units_.erase(defender.loc_);
	resources::whiteboard->on_kill_unit();

	// Plague units make new units on the target hex.
	if(attacker.valid() && attacker_stats->plagues && !drain_killed) {
		LOG_NG << "trying to reanimate " << attacker_stats->plague_type << '\n';

		if(const unit_type* reanimator = unit_types.find(attacker_stats->plague_type)) {
			LOG_NG << "found unit type:" << reanimator->id() << '\n';

			unit_ptr newunit = unit::create(*reanimator, attacker.get_unit().side(), true, unit_race::MALE);
			newunit->set_attacks(0);
			newunit->set_movement(0, true);
			newunit->set_facing(map_location::get_opposite_dir(attacker.get_unit().facing()));

			// Apply variation
			if(undead_variation != "null") {
				config mod;
				config& variation = mod.add_child("effect");
				variation["apply_to"] = "variation";
				variation["name"] = undead_variation;
				newunit->add_modification("variation", mod);
				newunit->heal_fully();
			}

			newunit->set_location(death_loc);
			units_.insert(newunit);

			game_events::entity_location reanim_loc(defender.loc_, newunit->underlying_id());
			resources::game_events->pump().fire("unit_placed", reanim_loc);

			preferences::encountered_units().insert(newunit->type_id());

			if(update_display_) {
				display::get_singleton()->invalidate(death_loc);
			}
		}
	} else {
		LOG_NG << "unit not reanimated\n";
	}
}

void attack::perform()
{
	// Stop the user from issuing any commands while the units are fighting.
	const events::command_disabler disable_commands;

	if(!a_.valid() || !d_.valid()) {
		return;
	}

	// no attack weapon => stop here and don't attack
	if(a_.weapon_ < 0) {
		a_.get_unit().set_attacks(a_.get_unit().attacks_left() - 1);
		a_.get_unit().set_movement(-1, true);
		return;
	}

	if(a_.get_unit().attacks_left() <= 0) {
		LOG_NG << "attack::perform(): not enough ap.\n";
		return;
	}

	a_.get_unit().set_facing(a_.loc_.get_relative_dir(d_.loc_));
	d_.get_unit().set_facing(d_.loc_.get_relative_dir(a_.loc_));

	a_.get_unit().set_attacks(a_.get_unit().attacks_left() - 1);

	VALIDATE(a_.weapon_ < static_cast<int>(a_.get_unit().attacks().size()),
			_("An invalid attacker weapon got selected."));

	a_.get_unit().set_movement(a_.get_unit().movement_left() - a_.get_unit().attacks()[a_.weapon_].movement_used(), true);
	a_.get_unit().set_state(unit::STATE_NOT_MOVED, false);
	a_.get_unit().set_resting(false);
	d_.get_unit().set_resting(false);

	// If the attacker was invisible, she isn't anymore!
	a_.get_unit().set_state(unit::STATE_UNCOVERED, true);

	bc_.reset(new battle_context(units_, a_.loc_, d_.loc_, a_.weapon_, d_.weapon_));

	a_stats_ = &bc_->get_attacker_stats();
	d_stats_ = &bc_->get_defender_stats();

	if(a_stats_->disable) {
		LOG_NG << "attack::perform(): tried to attack with a disabled attack.\n";
		return;
	}

	if(a_stats_->weapon) {
		a_.weap_id_ = a_stats_->weapon->id();
	}

	if(d_stats_->weapon) {
		d_.weap_id_ = d_stats_->weapon->id();
	}

	try {
		fire_event("attack");
	} catch(const attack_end_exception&) {
		return;
	}

	refresh_bc();

	DBG_NG << "getting attack statistics\n";
	statistics::attack_context attack_stats(
			a_.get_unit(), d_.get_unit(), a_stats_->chance_to_hit, d_stats_->chance_to_hit);

	a_.orig_attacks_ = a_stats_->num_blows;
	d_.orig_attacks_ = d_stats_->num_blows;
	a_.n_attacks_ = a_.orig_attacks_;
	d_.n_attacks_ = d_.orig_attacks_;
	a_.xp_ = game_config::combat_xp(d_.get_unit().level());
	d_.xp_ = game_config::combat_xp(a_.get_unit().level());

	bool defender_strikes_first = (d_stats_->firststrike && !a_stats_->firststrike);
	unsigned int rounds = std::max<unsigned int>(a_stats_->rounds, d_stats_->rounds) - 1;
	const int defender_side = d_.get_unit().side();

	LOG_NG << "Fight: (" << a_.loc_ << ") vs (" << d_.loc_ << ") ATT: " << a_stats_->weapon->name() << " "
		   << a_stats_->damage << "-" << a_stats_->num_blows << "(" << a_stats_->chance_to_hit
		   << "%) vs DEF: " << (d_stats_->weapon ? d_stats_->weapon->name() : "none") << " " << d_stats_->damage << "-"
		   << d_stats_->num_blows << "(" << d_stats_->chance_to_hit << "%)"
		   << (defender_strikes_first ? " defender first-strike" : "") << "\n";

	// Play the pre-fight animation
	unit_display::unit_draw_weapon(a_.loc_, a_.get_unit(), a_stats_->weapon, d_stats_->weapon, d_.loc_, &d_.get_unit());

	for(;;) {
		DBG_NG << "start of attack loop...\n";
		++abs_n_attack_;

		if(a_.n_attacks_ > 0 && !defender_strikes_first) {
			if(!perform_hit(true, attack_stats)) {
				DBG_NG << "broke from attack loop on attacker turn\n";
				break;
			}
		}

		// If the defender got to strike first, they use it up here.
		defender_strikes_first = false;
		++abs_n_defend_;

		if(d_.n_attacks_ > 0) {
			if(!perform_hit(false, attack_stats)) {
				DBG_NG << "broke from attack loop on defender turn\n";
				break;
			}
		}

		// Continue the fight to death; if one of the units got petrified,
		// either n_attacks or n_defends is -1
		if(rounds > 0 && d_.n_attacks_ == 0 && a_.n_attacks_ == 0) {
			a_.n_attacks_ = a_.orig_attacks_;
			d_.n_attacks_ = d_.orig_attacks_;
			--rounds;
			defender_strikes_first = (d_stats_->firststrike && !a_stats_->firststrike);
		}

		if(a_.n_attacks_ <= 0 && d_.n_attacks_ <= 0) {
			fire_event("attack_end");
			refresh_bc();
			break;
		}
	}

	// Set by attacker_hits and defender_hits events.
	resources::gamedata->clear_variable("damage_inflicted");

	if(update_def_fog_) {
		actions::recalculate_fog(defender_side);
	}

	// TODO: if we knew the viewing team, we could skip this display update
	if(update_minimap_ && update_display_) {
		display::get_singleton()->redraw_minimap();
	}

	if(a_.valid()) {
		unit& u = a_.get_unit();
		u.anim_comp().set_standing();
		u.set_experience(u.experience() + a_.xp_);
	}

	if(d_.valid()) {
		unit& u = d_.get_unit();
		u.anim_comp().set_standing();
		u.set_experience(u.experience() + d_.xp_);
	}

	unit_display::unit_sheath_weapon(a_.loc_, a_.valid() ? &a_.get_unit() : nullptr, a_stats_->weapon, d_stats_->weapon,
			d_.loc_, d_.valid() ? &d_.get_unit() : nullptr);

	if(update_display_) {
		game_display::get_singleton()->invalidate_unit();
		display::get_singleton()->invalidate(a_.loc_);
		display::get_singleton()->invalidate(d_.loc_);
	}

	if(OOS_error_) {
		replay::process_error(errbuf_.str());
	}
}

void attack::check_replay_attack_result(
		bool& hits, int ran_num, int& damage, config replay_results, unit_info& attacker)
{
	int results_chance = replay_results["chance"];
	bool results_hits = replay_results["hits"].to_bool();
	int results_damage = replay_results["damage"];

#if 0
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
#endif

	if(results_chance != attacker.cth_) {
		errbuf_ << "SYNC: In attack " << a_.dump() << " vs " << d_.dump()
				<< ": chance to hit is inconsistent. Data source: " << results_chance
				<< "; Calculation: " << attacker.cth_ << " (over-riding game calculations with data source results)\n";
		attacker.cth_ = results_chance;
		OOS_error_ = true;
	}

	if(results_hits != hits) {
		errbuf_ << "SYNC: In attack " << a_.dump() << " vs " << d_.dump() << ": the data source says the hit was "
				<< (results_hits ? "successful" : "unsuccessful") << ", while in-game calculations say the hit was "
				<< (hits ? "successful" : "unsuccessful") << " random number: " << ran_num << " = " << (ran_num % 100)
				<< "/" << results_chance << " (over-riding game calculations with data source results)\n";
		hits = results_hits;
		OOS_error_ = true;
	}

	if(results_damage != damage) {
		errbuf_ << "SYNC: In attack " << a_.dump() << " vs " << d_.dump() << ": the data source says the hit did "
				<< results_damage << " damage, while in-game calculations show the hit doing " << damage
				<< " damage (over-riding game calculations with data source results)\n";
		damage = results_damage;
		OOS_error_ = true;
	}
}
} // end anonymous namespace


// ==================================================================================
// FREE-STANDING FUNCTIONS
// ==================================================================================

void attack_unit(const map_location& attacker,
		const map_location& defender,
		int attack_with,
		int defend_with,
		bool update_display)
{
	attack dummy(attacker, defender, attack_with, defend_with, update_display);
	dummy.perform();
}

void attack_unit_and_advance(const map_location& attacker,
		const map_location& defender,
		int attack_with,
		int defend_with,
		bool update_display,
		const ai::unit_advancements_aspect& ai_advancement)
{
	attack_unit(attacker, defender, attack_with, defend_with, update_display);

	unit_map::const_iterator atku = resources::gameboard->units().find(attacker);
	if(atku != resources::gameboard->units().end()) {
		advance_unit_at(advance_unit_params(attacker).ai_advancements(ai_advancement));
	}

	unit_map::const_iterator defu = resources::gameboard->units().find(defender);
	if(defu != resources::gameboard->units().end()) {
		advance_unit_at(advance_unit_params(defender).ai_advancements(ai_advancement));
	}
}

int under_leadership(const unit &u, const map_location& loc, const_attack_ptr weapon, const_attack_ptr opp_weapon)
{
	unit_ability_list abil = u.get_abilities("leadership", loc, weapon, opp_weapon);
	unit_abilities::effect leader_effect(abil, 0, false);
	return leader_effect.get_composite_value();
}

//begin of weapon emulates function.

bool unit::abilities_filter_matches(const config& cfg, bool attacker, int res) const
{
	if(!(cfg["active_on"].empty() || (attacker && cfg["active_on"] == "offense") || (!attacker && cfg["active_on"] == "defense"))) {
		return false;
	}

	if(!unit_abilities::filter_base_matches(cfg, res)) {
		return false;
	}

	return true;
}

//functions for emulate weapon specials.
//filter opponent and affect self/opponent/both option.
bool unit::ability_filter_fighter(const std::string& ability, const std::string& filter_attacker , const config& cfg, const map_location& loc) const
{
	const config &filter = cfg.child(filter_attacker);
	if(!filter) {
		return true;
	}
	return unit_filter(vconfig(filter)).set_use_flat_tod(ability == "illuminates").matches(*this, loc);
}

static bool ability_apply_filter(const unit_map::const_iterator un, const unit_map::const_iterator up, const std::string& ability, const config& cfg, const map_location& loc, const map_location& opp_loc, bool attacker )
{
	if(!up->ability_filter_fighter(ability, "filter_opponent", cfg, opp_loc)){
		return true;
	}
	if((attacker && !un->ability_filter_fighter(ability, "filter_attacker", cfg, loc)) || (!attacker && !up->ability_filter_fighter(ability, "filter_attacker", cfg, opp_loc))){
		return true;
	}
	if((!attacker && !un->ability_filter_fighter(ability, "filter_defender", cfg, loc)) || (attacker && !up->ability_filter_fighter(ability, "filter_defender", cfg, opp_loc))){
		return true;
	}
	return false;
}

bool leadership_affects_self(const std::string& ability,const unit_map& units, const map_location& loc, bool attacker, const_attack_ptr weapon,const_attack_ptr opp_weapon)
{
	const unit_map::const_iterator un = units.find(loc);
	if(un == units.end()) {
		return false;
	}

	unit_ability_list abil = un->get_abilities(ability, weapon, opp_weapon);
	for(unit_ability_list::iterator i = abil.begin(); i != abil.end();) {
		const std::string& apply_to = (*i->first)["apply_to"];
		if(apply_to.empty() || apply_to == "both" || apply_to == "self") {
			return true;
		}
		if(attacker && apply_to == "attacker") {
			return true;
		}
		if(!attacker && apply_to == "defender") {
			return true;
		}
		++i;
	}
	return false;
}

bool leadership_affects_opponent(const std::string& ability,const unit_map& units, const map_location& loc, bool attacker, const_attack_ptr weapon,const_attack_ptr opp_weapon)
{
	const unit_map::const_iterator un = units.find(loc);
	if(un == units.end()) {
		return false;
	}

	unit_ability_list abil = un->get_abilities(ability, weapon, opp_weapon);
	for(unit_ability_list::iterator i = abil.begin(); i != abil.end();) {
		const std::string& apply_to = (*i->first)["apply_to"];
		if(apply_to == "both" || apply_to == "opponent") {
			return true;
		}
		if(attacker && apply_to == "defender") {
			return true;
		}
		if(!attacker && apply_to == "attacker") {
			return true;
		}
		++i;
	}
	return false;
}

//sub function for emulate chance_to_hit,damage drains and attacks special.
std::pair<int, bool> ability_leadership(const std::string& ability,const unit_map& units, const map_location& loc, const map_location& opp_loc, bool attacker, int abil_value, bool backstab_pos, const_attack_ptr weapon, const_attack_ptr opp_weapon)
{
	const unit_map::const_iterator un = units.find(loc);
	const unit_map::const_iterator up = units.find(opp_loc);
	if(un == units.end()) {
		return {abil_value, false};
	}

	unit_ability_list abil = un->get_abilities(ability, weapon, opp_weapon);
	for(unit_ability_list::iterator i = abil.begin(); i != abil.end();) {
		const config &filter = (*i->first).child("filter_opponent");
		const config &filter_attacker = (*i->first).child("filter_attacker");
		const config &filter_defender = (*i->first).child("filter_defender");
		bool show_result = false;
		if(up == units.end() && !filter && !filter_attacker && !filter_defender) {
			show_result = un->abilities_filter_matches(*i->first, attacker, abil_value);
		} else if(up == units.end() && (filter || filter_attacker || filter_defender)) {
			return {abil_value, false};
		} else {
			show_result = !(!un->abilities_filter_matches(*i->first, attacker, abil_value) || ability_apply_filter(un, up, ability, *i->first, loc, opp_loc, attacker));
		}

		if(!show_result) {
			i = abil.erase(i);
		} else {
			++i;
		}
	}

	if(!abil.empty()) {
		unit_abilities::effect leader_effect(abil, abil_value, backstab_pos);
		return {leader_effect.get_composite_value(), true};
	}
	return {abil_value, false};
}

//sub function for wmulate boolean special(slow, poison...)
bool bool_leadership(const std::string& ability,const unit_map& units, const map_location& loc, const map_location& opp_loc, bool attacker, const_attack_ptr weapon, const_attack_ptr opp_weapon)
{
	const unit_map::const_iterator un = units.find(loc);
	const unit_map::const_iterator up = units.find(opp_loc);
	if(un == units.end() || up == units.end()) {
		return false;
	}

	unit_ability_list abil = un->get_abilities(ability, weapon, opp_weapon);
	for(unit_ability_list::iterator i = abil.begin(); i != abil.end();) {
		const std::string& active_on = (*i->first)["active_on"];
		if(!(active_on.empty() || (attacker && active_on == "offense") || (!attacker && active_on == "defense")) || ability_apply_filter(un, up, ability, *i->first, loc, opp_loc, attacker)) {
			i = abil.erase(i);
		} else {
			++i;
		}
	}
	if(!abil.empty()) {
			return true;
	}
	return false;
}

//emulate boolean special for self/adjacent and/or opponent.
bool attack_type::bool_ability(const std::string& ability) const
{
	bool abil_bool= get_special_bool(ability);
	const unit_map& units = display::get_singleton()->get_units();

	if(leadership_affects_self(ability, units, self_loc_, is_attacker_, shared_from_this(), other_attack_)) {
		abil_bool = get_special_bool(ability) || bool_leadership(ability, units, self_loc_, other_loc_, is_attacker_, shared_from_this(), other_attack_);
	}

	if(leadership_affects_opponent(ability, units, other_loc_, !is_attacker_, other_attack_, shared_from_this())) {
		abil_bool = get_special_bool(ability) || bool_leadership(ability, units, other_loc_, self_loc_, !is_attacker_, other_attack_, shared_from_this());
	}
	return abil_bool;
}

//emulate numerical special for self/adjacent and/or opponent.
std::pair<int, bool> attack_type::combat_ability(const std::string& ability, int abil_value, bool backstab_pos) const
{
	const unit_map& units = display::get_singleton()->get_units();

	if(leadership_affects_self(ability, units, self_loc_, is_attacker_, shared_from_this(), other_attack_)) {
		return ability_leadership(ability, units, self_loc_, other_loc_, is_attacker_, abil_value, backstab_pos, shared_from_this(), other_attack_);
	}

	if(leadership_affects_opponent(ability, units, other_loc_, !is_attacker_, other_attack_, shared_from_this())) {
		return ability_leadership(ability, units, other_loc_,self_loc_, !is_attacker_, abil_value, backstab_pos, other_attack_, shared_from_this());
	}
	return {abil_value, false};
}
//end of emulate weapon special functions.

int combat_modifier(const unit_map& units,
		const gamemap& map,
		const map_location& loc,
		unit_type::ALIGNMENT alignment,
		bool is_fearless)
{
	const tod_manager& tod_m = *resources::tod_manager;
	const time_of_day& effective_tod = tod_m.get_illuminated_time_of_day(units, map, loc);
	return combat_modifier(effective_tod, alignment, is_fearless);
}

int combat_modifier(const time_of_day& effective_tod,
		unit_type::ALIGNMENT alignment,
		bool is_fearless)
{
	const tod_manager& tod_m = *resources::tod_manager;
	const int lawful_bonus = effective_tod.lawful_bonus;
	return generic_combat_modifier(lawful_bonus, alignment, is_fearless, tod_m.get_max_liminal_bonus());
}

int generic_combat_modifier(int lawful_bonus, unit_type::ALIGNMENT alignment, bool is_fearless, int max_liminal_bonus)
{
	int bonus;

	switch(alignment.v) {
	case unit_type::ALIGNMENT::LAWFUL:
		bonus = lawful_bonus;
		break;
	case unit_type::ALIGNMENT::NEUTRAL:
		bonus = 0;
		break;
	case unit_type::ALIGNMENT::CHAOTIC:
		bonus = -lawful_bonus;
		break;
	case unit_type::ALIGNMENT::LIMINAL:
		bonus = std::max(0, max_liminal_bonus-std::abs(lawful_bonus));
		break;
	default:
		bonus = 0;
	}

	if(is_fearless) {
		bonus = std::max<int>(bonus, 0);
	}

	return bonus;
}

bool backstab_check(const map_location& attacker_loc,
		const map_location& defender_loc,
		const unit_map& units,
		const std::vector<team>& teams)
{
	const unit_map::const_iterator defender = units.find(defender_loc);
	if(defender == units.end()) {
		return false; // No defender
	}

	adjacent_loc_array_t adj;
	get_adjacent_tiles(defender_loc, adj.data());

	unsigned i;

	for(i = 0; i < adj.size(); ++i) {
		if(adj[i] == attacker_loc) {
			break;
		}
	}

	if(i >= 6) {
		return false; // Attack not from adjacent location
	}

	const unit_map::const_iterator opp = units.find(adj[(i + 3) % 6]);

	// No opposite unit.
	if(opp == units.end()) {
		return false;
	}

	if(opp->incapacitated()) {
		return false;
	}

	// If sides aren't valid teams, then they are enemies.
	if(std::size_t(defender->side() - 1) >= teams.size() || std::size_t(opp->side() - 1) >= teams.size()) {
		return true;
	}

	// Defender and opposite are enemies.
	if(teams[defender->side() - 1].is_enemy(opp->side())) {
		return true;
	}

	// Defender and opposite are friends.
	return false;
}
