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

//! @file actions.cpp
//! Recruiting, Fighting.

#include "actions.hpp"
#include "attack_prediction.hpp"
#include "game_config.hpp"
#include "game_errors.hpp"
#include "game_preferences.hpp"
#include "game_events.hpp"
#include "gettext.hpp"
#include "halo.hpp"
#include "hotkeys.hpp"
#include "menu_events.hpp"
#include "mouse_events.hpp"
#include "pathfind.hpp"
#include "random.hpp"
#include "replay.hpp"
#include "sound.hpp"
#include "statistics.hpp"
#include "unit_abilities.hpp"
#include "unit_display.hpp"
#include "wassert.hpp"
#include "wml_separators.hpp"
#include "serialization/binary_wml.hpp"
#include "serialization/parser.hpp"

#define LOG_NG LOG_STREAM(info, engine)
#define ERR_NW LOG_STREAM(err, network)

struct castle_cost_calculator : cost_calculator
{
	castle_cost_calculator(const gamemap& map) : map_(map)
	{}

	virtual double cost(const gamemap::location&, const gamemap::location& loc, const double, const bool) const
	{
		if(!map_.is_castle(loc))
			return 10000;

		return 1;
	}

private:
	const gamemap& map_;
};

// Conditions placed on victory must be accessible from the global function
// check_victory, but shouldn't be passed to that function as parameters,
// since it is called from a variety of places.
namespace victory_conditions
{
	static bool when_enemies_defeated = true;

	void set_victory_when_enemies_defeated(bool on)
	{
		when_enemies_defeated = on;
	}

	static bool victory_when_enemies_defeated()
	{
		return when_enemies_defeated;
	}
}

bool can_recruit_on(const gamemap& map, const gamemap::location& leader, const gamemap::location loc)
{
	if(!map.on_board(loc))
		return false;

	if(!map.is_castle(loc))
		return false;

	castle_cost_calculator calc(map);
	const paths::route& rt = a_star_search(leader, loc, 100.0, &calc, map.w(), map.h());

	if(rt.steps.empty())
		return false;

	return true;
}

std::string recruit_unit(const gamemap& map, int side,
       unit_map& units, unit new_unit,
       gamemap::location& recruit_location, bool show, bool need_castle, bool full_movement)
{
	const events::command_disabler disable_commands;

	LOG_NG << "recruiting unit for side " << side << "\n";

	// Find the unit that can recruit
	unit_map::const_iterator u = units.begin();

	for(; u != units.end(); ++u) {
		if(u->second.can_recruit() &&
				static_cast<int>(u->second.side()) == side) {

			break;
		}
	}

	if(u == units.end() && (need_castle || !map.on_board(recruit_location))) {
		return _("You don't have a leader to recruit with.");
	}

	wassert(u != units.end() || !need_castle);

	if(need_castle && map.is_keep(u->first) == false) {
		LOG_NG << "Leader not on start: leader is on " << u->first << '\n';
		return _("You must have your leader on a keep to recruit or recall units.");
	}

	if(need_castle) {
		if (units.find(recruit_location) != units.end() ||
			!can_recruit_on(map, u->first, recruit_location)) {

			recruit_location = gamemap::location();
		}
	}

	if(!map.on_board(recruit_location)) {
		recruit_location = find_vacant_tile(map,units,u->first,
		                                    need_castle ? VACANT_CASTLE : VACANT_ANY);
	} else if(units.count(recruit_location) == 1) {
		recruit_location = find_vacant_tile(map,units,recruit_location,VACANT_ANY);
	}

	if(!map.on_board(recruit_location)) {
		return _("There are no vacant castle tiles in which to recruit a unit.");
	}

	if(full_movement) {
		new_unit.set_movement(new_unit.total_movement());
	} else {
		new_unit.set_movement(0);
		new_unit.set_attacks(0);
	}
	new_unit.heal_all();

	units.add(new std::pair<gamemap::location,unit>(recruit_location,new_unit));

	LOG_NG << "firing prerecruit event\n";
	game_events::fire("prerecruit",recruit_location);
	if(show)unit_display::unit_recruited(recruit_location);
	LOG_NG << "firing recruit event\n";
	game_events::fire("recruit",recruit_location);

	const std::string checksum = get_checksum(new_unit, true);

	const config* ran_results = get_random_results();
	if(ran_results != NULL) {
		const std::string rc = (*ran_results)["checksum"];
		if(rc != checksum) {
			ERR_NW << "SYNC: In recruit " << new_unit.id() <<
				": has checksum " << checksum <<
				" while datasource has checksum " <<
				rc << "\n";

			config cfg_unit1;
			new_unit.write(cfg_unit1);
			::write(std::cerr, cfg_unit1);
			//! @todo FIXME enabling this exception will trigger an assertion failure
			// in display.cpp:1010 when loading a map (the cause of that OOS
			// will be fixed soon). (Only tested in SP for bug #9728)
//			if (!game_config::ignore_replay_errors) {
//				throw replay::error("OOS while recruiting.");
//			}
		}

	} else {
		config cfg;
		cfg["checksum"] = checksum;
		set_random_results(cfg);
	}

	// If an OOS happens, this option allows to write
	// the debug info about the recruited unit.
	if(!lg::info.dont_log(lg::engine)) {
		config cfg_unit;
		new_unit.write(cfg_unit);

		LOG_NG << "recruit unit\nChecksum = "
			<< checksum << "\n-----[start data]-----\n";

		::write(lg::info(lg::engine), cfg_unit);
		LOG_NG << "\n----[end data]-----\n";
	}

	return std::string();
}

gamemap::location under_leadership(const unit_map& units,
                                   const gamemap::location& loc, int* bonus)
{

	const unit_map::const_iterator un = units.find(loc);
	if(un == units.end()) {
		return gamemap::location::null_location;
	}
	unit_ability_list abil = un->second.get_abilities("leadership",loc);
	int best_bonus = abil.highest("value").first;
	if(bonus) {
		*bonus = best_bonus;
	}
	return abil.highest("value").second;
}

battle_context::battle_context(const gamemap& map, const std::vector<team>& teams, const unit_map& units,
							   const gamestatus& status, const game_data& gamedata,
							   const gamemap::location& attacker_loc, const gamemap::location& defender_loc,
							   int attacker_weapon, int defender_weapon, double aggression, const combatant *prev_def)
	: attacker_stats_(NULL), defender_stats_(NULL), attacker_combatant_(NULL), defender_combatant_(NULL)
{
	const unit& attacker = units.find(attacker_loc)->second;
	const unit& defender = units.find(defender_loc)->second;
	const double harm_weight = 1.0 - aggression;

	// A Python AI can send an invalid weapon and crash Wesnoth.
	// Haven't found a way for the Python API to prevent this problem.
	// So instead of segfaulting it sends an assertion failure.
	wassert(attacker_weapon < static_cast<int>(attacker.attacks().size()));

	if (attacker_weapon == -1 && attacker.attacks().size() == 1 && attacker.attacks()[0].attack_weight() > 0 )
		attacker_weapon = 0;

	if (attacker_weapon == -1) {
		wassert(defender_weapon == -1);
		attacker_weapon = choose_attacker_weapon(attacker, defender, map, teams, units,
												 status, gamedata, attacker_loc, defender_loc,
												 harm_weight, &defender_weapon, prev_def);
	} else if (defender_weapon == -1) {
		defender_weapon = choose_defender_weapon(attacker, defender, attacker_weapon, map, teams,
												 units, status, gamedata, attacker_loc, defender_loc, prev_def);
	}

	// If those didn't have to generate statistics, do so now.
	if (!attacker_stats_) {
		const attack_type *adef = NULL;
		const attack_type *ddef = NULL;
		if (attacker_weapon >= 0) {
			wassert(attacker_weapon < static_cast<int>(attacker.attacks().size()));
			adef = &attacker.attacks()[attacker_weapon];
		}
		if (defender_weapon >= 0) {
			wassert(defender_weapon < static_cast<int>(defender.attacks().size()));
			ddef = &defender.attacks()[defender_weapon];
		}
		wassert(!defender_stats_ && !attacker_combatant_ && !defender_combatant_);
		attacker_stats_ = new unit_stats(attacker, attacker_loc, attacker_weapon,
										 true, defender, defender_loc, ddef,
										units, teams, status, map, gamedata);
		defender_stats_ = new unit_stats(defender, defender_loc, defender_weapon, false,
										 attacker, attacker_loc, adef,
										 units, teams, status, map, gamedata);
	}
}

battle_context::battle_context(const battle_context &other)
	: attacker_stats_(NULL), defender_stats_(NULL), attacker_combatant_(NULL), defender_combatant_(NULL)
{
	*this = other;
}

battle_context::battle_context(const unit_stats &att, const unit_stats &def)
{
	attacker_stats_ = new unit_stats(att);
	defender_stats_ = new unit_stats(def);
	attacker_combatant_ = NULL;
	defender_combatant_ = NULL;
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
		attacker_stats_ = new unit_stats(*other.attacker_stats_);
		defender_stats_ = new unit_stats(*other.defender_stats_);
		attacker_combatant_ = other.attacker_combatant_ ? new combatant(*other.attacker_combatant_, *attacker_stats_) : NULL;
		defender_combatant_ = other.defender_combatant_ ? new combatant(*other.defender_combatant_, *defender_stats_) : NULL;
	}
	return *this;
}

//! @todo FIXME: Hand previous defender unit in here.
int battle_context::choose_defender_weapon(const unit &attacker, const unit &defender, unsigned attacker_weapon,
										   const gamemap& map, const std::vector<team>& teams, const unit_map& units,
										   const gamestatus& status, const game_data& gamedata,
										   const gamemap::location& attacker_loc, const gamemap::location& defender_loc,
										   const combatant *prev_def)
{
	const attack_type &att = attacker.attacks()[attacker_weapon];
	std::vector<unsigned int> choices;

	// What options does defender have?
	unsigned int i;
	for (i = 0; i < defender.attacks().size(); i++) {
		const attack_type &def = defender.attacks()[i];
		if (def.range() == att.range() && def.defense_weight() > 0) {
			choices.push_back(i);
		}
	}
	if (choices.size() == 0)
		return -1;
	if (choices.size() == 1)
		return choices[0];

	// Multiple options:
	// First pass : get the best weight and the minimum simple rating for this weight.
	// simple rating = number of blows * damage per blows (resistance taken in account) * cth * weight
	// Elligible attacks for defense should have a simple rating greater or equal to this weight.

	double max_weight = 0.0;
	int min_rating = 0;

	for (i = 0; i < choices.size(); i++) {
		const attack_type &def = defender.attacks()[choices[i]];
		if (def.defense_weight() > max_weight) {
			max_weight = def.defense_weight();
			unit_stats *def_stats = new unit_stats(defender, defender_loc, choices[i], false,
								attacker, attacker_loc, &att,
								units, teams, status, map, gamedata);
			min_rating = static_cast<int>(def_stats->num_blows * def_stats->damage *
				def_stats->chance_to_hit * def.defense_weight());

			delete def_stats;
		}
		else if (def.defense_weight() == max_weight) {
			unit_stats *def_stats = new unit_stats(defender, defender_loc, choices[i], false,
								attacker, attacker_loc, &att,
								units, teams, status, map, gamedata);
			int simple_rating = static_cast<int>(def_stats->num_blows * def_stats->damage *
				def_stats->chance_to_hit * def.defense_weight());

			if (simple_rating < min_rating )
				min_rating = simple_rating;
			delete def_stats;
		}
	}

	// Multiple options: simulate them, save best.
	for (i = 0; i < choices.size(); i++) {
		const attack_type &def = defender.attacks()[choices[i]];
		unit_stats *att_stats = new unit_stats(attacker, attacker_loc, attacker_weapon,
											   true, defender, defender_loc, &def,
											   units, teams, status, map, gamedata);
		unit_stats *def_stats = new unit_stats(defender, defender_loc, choices[i], false,
											   attacker, attacker_loc, &att,
											   units, teams, status, map, gamedata);

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

	return defender_stats_->attack_num;
}

int battle_context::choose_attacker_weapon(const unit &attacker, const unit &defender,
												const gamemap& map, const std::vector<team>& teams, const unit_map& units,
												const gamestatus& status, const game_data& gamedata,
												const gamemap::location& attacker_loc, const gamemap::location& defender_loc,
												double harm_weight, int *defender_weapon, const combatant *prev_def)
{
	std::vector<unsigned int> choices;

	// What options does attacker have?
	unsigned int i;
	for (i = 0; i < attacker.attacks().size(); i++) {
		const attack_type &att = attacker.attacks()[i];
		if (att.attack_weight() > 0) {
			choices.push_back(i);
		}
	}
	if (choices.size() == 0)
		return -1;
	if (choices.size() == 1) {
		*defender_weapon = choose_defender_weapon(attacker, defender, choices[0], map, teams, units,
			status, gamedata, attacker_loc, defender_loc, prev_def);
		return choices[0];
	}

	// Multiple options: simulate them, save best.
	unit_stats *best_att_stats = NULL, *best_def_stats = NULL;
	combatant *best_att_comb = NULL, *best_def_comb = NULL;

	for (i = 0; i < choices.size(); i++) {
		const attack_type &att = attacker.attacks()[choices[i]];
		int def_weapon = choose_defender_weapon(attacker, defender, choices[i], map, teams, units,
												status, gamedata, attacker_loc, defender_loc, prev_def);
		// If that didn't simulate, do so now.
		if (!attacker_combatant_) {
			const attack_type *def = NULL;
			if (def_weapon >= 0) {
				def = &defender.attacks()[def_weapon];
			}
			attacker_stats_ = new unit_stats(attacker, attacker_loc, choices[i],
											 true, defender, defender_loc, def,
											 units, teams, status, map, gamedata);
			defender_stats_ = new unit_stats(defender, defender_loc, def_weapon, false,
											 attacker, attacker_loc, &att,
											 units, teams, status, map, gamedata);
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

	*defender_weapon = defender_stats_->attack_num;
	return attacker_stats_->attack_num;
}

// Does combat A give us a better result than combat B?
bool battle_context::better_combat(const combatant &us_a, const combatant &them_a,
								   const combatant &us_b, const combatant &them_b,
								   double harm_weight)
{
	double a, b;

	// Compare: P(we kill them) - P(they kill us).
	a = them_a.hp_dist[0] - us_a.hp_dist[0] * harm_weight;
	b = them_b.hp_dist[0] - us_b.hp_dist[0] * harm_weight;
	if (a - b < -0.01)
		return false;
	if (a - b > 0.01)
		return true;

	// Compare: damage to them - damage to us (average_hp replaces -damage)
	a = us_a.average_hp()*harm_weight - them_a.average_hp();
	b = us_b.average_hp()*harm_weight - them_b.average_hp();
	if (a - b < -0.01)
		return false;
	if (a - b > 0.01)
		return true;

	// All else equal: go for most damage.
	return them_a.average_hp() < them_b.average_hp();
}

// Get the simulation results.
//! @todo FIXME: better to initialize combatant initially (move into unit_stats?), just do fight() when required.
const combatant &battle_context::get_attacker_combatant(const combatant *prev_def)
{
	// We calculate this lazily, since AI doesn't always need it.
	if (!attacker_combatant_) {
		wassert(!defender_combatant_);
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
		wassert(!attacker_combatant_);
		attacker_combatant_ = new combatant(*attacker_stats_);
		defender_combatant_ = new combatant(*defender_stats_, prev_def);
		attacker_combatant_->fight(*defender_combatant_);
	}
	return *defender_combatant_;
}

battle_context::unit_stats::unit_stats(const unit &u, const gamemap::location& u_loc,
									   int u_attack_num, bool attacking,
									   const unit &opp, const gamemap::location& opp_loc,
									   const attack_type *opp_weapon,
									   const unit_map& units,
									   const std::vector<team>& teams,
									   const gamestatus& status,
									   const gamemap& map,
									   const game_data& gamedata)
{
	// Get the current state of the unit.
	attack_num = u_attack_num;
	if (attack_num >= 0) {
		weapon = &u.attacks()[attack_num];
	} else {
		weapon = NULL;
	}
	is_attacker = attacking;
	is_poisoned = utils::string_bool(u.get_state("poisoned"));
	is_slowed = utils::string_bool(u.get_state("slowed"));
	if(u.hitpoints() < 0) {
//! @todo FIXME enable after 1.3.2 and find out why this happens -- Mordante
//		LOG_STREAM(err, config) << "Unit with " << u.hitpoints() << " hitpoints found, set to 0 for damage calculations\n";
		hp = 0;
	} else {
		hp = u.hitpoints();
	}
	max_hp = u.max_hitpoints();

	// Get the weapon characteristics, if any.
	if (weapon) {
		weapon->set_specials_context(u_loc, opp_loc, &gamedata, &units, &map, &status, &teams, attacking, opp_weapon);
		if (opp_weapon)
			opp_weapon->set_specials_context(u_loc, opp_loc, &gamedata, &units, &map, &status, &teams, !attacking, weapon);
		slows = weapon->get_special_bool("slow");
		drains = weapon->get_special_bool("drains") && !utils::string_bool(opp.get_state("not_living"));
		stones = weapon->get_special_bool("stones");
		poisons = weapon->get_special_bool("poison") && opp.get_state("not_living") != "yes" && opp.get_state("poisoned") != "yes";
		backstab_pos = is_attacker && backstab_check(u_loc, opp_loc, units, teams);
		rounds = weapon->get_specials("berserk").highest("value", 1).first;
		firststrike = weapon->get_special_bool("firststrike");

		// Handle plague.
		unit_ability_list plague_specials = weapon->get_specials("plague");
		plagues = !plague_specials.empty() && opp.get_state("not_living") != "yes" &&
				  strcmp(opp.undead_variation().c_str(), "null") && !map.is_village(opp_loc);

		if (!plague_specials.empty()) {
			if((*plague_specials.cfgs.front().first)["type"] == "")
				plague_type = u.id();
			else
				plague_type = (*plague_specials.cfgs.front().first)["type"];
		}

		// Compute chance to hit.
		chance_to_hit = opp.defense_modifier(map.get_terrain(opp_loc));
		unit_ability_list cth_specials = weapon->get_specials("chance_to_hit");
		unit_abilities::effect cth_effects(cth_specials, chance_to_hit, backstab_pos);
		chance_to_hit = cth_effects.get_composite_value();

		// Compute base damage done with the weapon.
		int base_damage = weapon->damage();
		unit_ability_list dmg_specials = weapon->get_specials("damage");
		unit_abilities::effect dmg_effect(dmg_specials, base_damage, backstab_pos);
		base_damage = dmg_effect.get_composite_value();

		// Get the damage multiplier applied to the base damage of the weapon.
		int damage_multiplier = 100;

		// Time of day bonus.
		damage_multiplier += combat_modifier(status, units, u_loc, u.alignment(), u.is_fearless(), map);

		// Leadership bonus.
		int leader_bonus = 0;
		if (under_leadership(units, u_loc, &leader_bonus).valid())
			damage_multiplier += leader_bonus;

		// Resistance modifier.
		damage_multiplier *= opp.damage_from(*weapon, !attacking, opp_loc);

		// Compute both the normal and slowed damage. For the record,
		// drain = normal damage / 2 and slow_drain = slow_damage / 2.
		damage = round_damage(base_damage, damage_multiplier, 10000);
		slow_damage = round_damage(base_damage, damage_multiplier, 20000);
		if (is_slowed)
			damage = slow_damage;

		// Compute the number of blows and handle swarm.
		unit_ability_list swarm_specials = weapon->get_specials("attacks");

		if (!swarm_specials.empty()) {
			swarm = true;
			swarm_min = swarm_specials.highest("attacks_min").first;
			swarm_max = swarm_specials.highest("attacks_max", weapon->num_attacks()).first;
			num_blows = swarm_min + (swarm_max - swarm_min) * hp / max_hp;
		} else {
			swarm = false;
			num_blows = weapon->num_attacks();
			swarm_min = num_blows;
			swarm_max = num_blows;
		}
	} else {
		slows = false;
		drains = false;
		stones = false;
		plagues = false;
		poisons = false;
		backstab_pos = false;
		swarm = false;
		rounds = 1;
		firststrike = false;

		chance_to_hit = 0;
		damage = 0;
		slow_damage = 0;
		num_blows = 0;
		swarm_min = 0;
		swarm_max = 0;
	}
}

battle_context::unit_stats::~unit_stats()
{
}

void battle_context::unit_stats::dump() const
{
	printf("==================================\n");
	printf("is_attacker:	%d\n", static_cast<int>(is_attacker));
	printf("is_poisoned:	%d\n", static_cast<int>(is_poisoned));
	printf("is_slowed:	%d\n", static_cast<int>(is_slowed));
	printf("slows:		%d\n", static_cast<int>(slows));
	printf("drains:		%d\n", static_cast<int>(drains));
	printf("stones:		%d\n", static_cast<int>(stones));
	printf("poisons:	%d\n", static_cast<int>(poisons));
	printf("backstab_pos:	%d\n", static_cast<int>(backstab_pos));
	printf("swarm:		%d\n", static_cast<int>(swarm));
	printf("rounds:	%d\n", static_cast<int>(rounds));
	printf("firststrike:	%d\n", static_cast<int>(firststrike));
	printf("\n");
	printf("hp:		%d\n", hp);
	printf("max_hp:		%d\n", max_hp);
	printf("chance_to_hit:	%d\n", chance_to_hit);
	printf("damage:		%d\n", damage);
	printf("slow_damage:	%d\n", slow_damage);
	printf("num_blows:	%d\n", num_blows);
	printf("swarm_min:	%d\n", swarm_min);
	printf("swarm_max:	%d\n", swarm_max);
	printf("\n");
}

// Given this harm_weight, are we better than this other context?
bool battle_context::better_attack(class battle_context &that, double harm_weight)
{
	return better_combat(get_attacker_combatant(), get_defender_combatant(),
						 that.get_attacker_combatant(), that.get_defender_combatant(), harm_weight);
}

static std::string unit_dump(std::pair< gamemap::location, unit > const &u)
{
	std::stringstream s;
	s << u.second.id() << " (" << u.first.x + 1 << ',' << u.first.y + 1 << ')';
	return s.str();
}

void attack::fire_event(const std::string& n)
{
	LOG_NG << "firing " << n << " event\n";
	if(n == "attack_end") {
		config dat;
		dat.add_child("first");
		dat.add_child("second");
		if(a_ != units_.end()) {
			(*(dat.child("first")))["weapon"]=a_stats_->weapon->id();
		}
		if(d_ != units_.end()) {
			config *tempcfg = dat.child("second");
			t_string d_weap = "none";
			if(d_stats_->weapon != NULL) {
				if(a_ != units_.end()) {
					d_weap = d_stats_->weapon->id();
				} else {
					// The weapon choice will be invalid since the attacker was removed
					d_weap = "invalid";
				}
			}
			std::pair<std::string,t_string> to_insert("weapon", d_weap);
			tempcfg->values.insert(to_insert);
		}
		game_events::fire(n,attacker_,defender_,dat);
		a_ = units_.find(attacker_);
		d_ = units_.find(defender_);
		return;
	}
	const int defender_side = d_->second.side();
	const int attacker_side = a_->second.side();
	config dat;
	dat.add_child("first");
	dat.add_child("second");
	(*(dat.child("first")))["weapon"]=a_stats_->weapon->id();
	(*(dat.child("second")))["weapon"]=d_stats_->weapon != NULL ? d_stats_->weapon->id() : "none";
	game_events::fire(n,attacker_,defender_,dat);
	// The event could have killed either the attacker or
	// defender, so we have to make sure they still exist
	a_ = units_.find(attacker_);
	d_ = units_.find(defender_);
	//! @todo FIXME: If the event removes this attack, we should stop attacking.
	// The previous code checked if 'attack_with' and 'defend_with'
	// were still within the bounds of the attack arrays,
	// or -1, but it was incorrect.
	// The attack used could be removed and '*_with' variables
	// could still be in bounds but point to a different attack.
	if(a_ == units_.end() || d_ == units_.end()) {
		if (update_display_){
			recalculate_fog(map_,state_,info_,units_,teams_,attacker_side-1);
			recalculate_fog(map_,state_,info_,units_,teams_,defender_side-1);
			gui_.recalculate_minimap();
			gui_.update_display();
		}
		fire_event("attack_end");
		throw attack_end_exception();
	}
}

void attack::refresh_bc()
{
	a_ = units_.find(attacker_);
	d_ = units_.find(defender_);
	if(a_ == units_.end() || d_ == units_.end()) {
		return;
	}
	*bc_ =	battle_context(map_, teams_, units_, state_, info_, attacker_, defender_, attack_with_, defend_with_);
	a_stats_ = &bc_->get_attacker_stats();
	d_stats_ = &bc_->get_defender_stats();
	attacker_cth_ = a_stats_->chance_to_hit;
	defender_cth_ = d_stats_->chance_to_hit;
	attacker_damage_ = a_stats_->damage;
	defender_damage_ = d_stats_->damage;
}

attack::~attack()
{
	delete bc_;
}

attack::attack(game_display& gui, const gamemap& map,
            std::vector<team>& teams,
            gamemap::location attacker,
            gamemap::location defender,
            int attack_with,
			int defend_with,
            unit_map& units,
            const gamestatus& state,
            const game_data& info,
			bool update_display) : gui_(gui),map_(map),teams_(teams),
					attacker_(attacker),defender_(defender),
					attack_with_(attack_with),defend_with_(defend_with),
					units_(units),state_(state),info_(info),
					update_display_(update_display),OOS_error_(false),bc_(NULL)
{
	// Stop the user from issuing any commands while the units are fighting
	const events::command_disabler disable_commands;
	a_ = units_.find(attacker);
	d_ = units_.find(defender);

	if(a_ == units_.end() || d_ == units_.end()) {
		return;
	}

	// no attack weapon => stop here and don't attack
	if (attack_with < 0) {
		a_->second.set_attacks(a_->second.attacks_left()-1);
		a_->second.set_movement(-1);
		return;
	}

	a_->second.set_attacks(a_->second.attacks_left()-1);
	a_->second.set_movement(a_->second.movement_left()-a_->second.attacks()[attack_with].movement_used());
	a_->second.set_state("not_moved","");
	d_->second.set_resting(false);

	// If the attacker was invisible, she isn't anymore!
	a_->second.set_state("hides","");

	bc_ = new battle_context(map_, teams_, units_, state_, info_, attacker_, defender_, attack_with_, defend_with_);
	a_stats_ = &bc_->get_attacker_stats();
	d_stats_ = &bc_->get_defender_stats();

	try {
		fire_event("attack");
	} catch (attack_end_exception) {
		return;
	}
	refresh_bc();

	LOG_NG << "getting attack statistics\n";
	statistics::attack_context attack_stats(a_->second, d_->second, a_stats_->chance_to_hit, d_stats_->chance_to_hit);

	orig_attacks_ = a_stats_->num_blows;
	orig_defends_ = d_stats_->num_blows;
	n_attacks_ = orig_attacks_;
	n_defends_ = orig_defends_;
	attackerxp_ = d_->second.level();
	defenderxp_ = a_->second.level();

	bool defender_strikes_first = (d_stats_->firststrike && ! a_stats_->firststrike);
	unsigned int rounds = maximum<unsigned int>(a_stats_->rounds, d_stats_->rounds) - 1;
	int abs_n_attack_ = 0;
	int abs_n_defend_ = 0;

	static const std::string poison_string("poison");

	LOG_NG << "Fight: (" << attacker << ") vs (" << defender << ") ATT: " << a_stats_->weapon->name() << " " << a_stats_->damage << "-" << a_stats_->num_blows << "(" << a_stats_->chance_to_hit << "%) vs DEF: " << (d_stats_->weapon ? d_stats_->weapon->name() : "none") << " " << d_stats_->damage << "-" << d_stats_->num_blows << "(" << d_stats_->chance_to_hit << "%)" << (defender_strikes_first ? " defender first-strike" : "") << "\n";

	while(n_attacks_ > 0 || n_defends_ > 0) {
		LOG_NG << "start of attack loop...\n";
		abs_n_attack_++;

		if(n_attacks_ > 0 && defender_strikes_first == false) {
			const int ran_num = get_random();
			bool hits = (ran_num%100) < attacker_cth_;

			int damage_defender_takes;
			if(hits) {
				damage_defender_takes = attacker_damage_;
			} else {
				damage_defender_takes = 0;
			}
			// Make sure that if we're serializing a game here,
			// we got the same results as the game did originally.
			const config* ran_results = get_random_results();
			if(ran_results != NULL) {
				const int results_chance = atoi((*ran_results)["chance"].c_str());
				const bool results_hits = (*ran_results)["hits"] == "yes";
				const int results_damage = atoi((*ran_results)["damage"].c_str());

				if(results_chance != attacker_cth_) {
					errbuf_ << "SYNC: In attack " << unit_dump(*a_) << " vs " << unit_dump(*d_)
						<< ": chance to hit defender is inconsistent. Data source: "
						<< results_chance << "; Calculation: " << attacker_cth_
						<< " (over-riding game calculations with data source results)\n";
					attacker_cth_ = results_chance;
					OOS_error_ = true;
				}
				if(hits != results_hits) {
					errbuf_ << "SYNC: In attack " << unit_dump(*a_) << " vs " << unit_dump(*d_)
						<< ": the data source says the hit was "
						<< (results_hits ? "successful" : "unsuccessful")
						<< ", while in-game calculations say the hit was "
						<< (hits ? "successful" : "unsuccessful")
						<< " random number: " << ran_num << " = "
						<< (ran_num%100) << "/" << results_chance
						<< " (over-riding game calculations with data source results)\n";
					hits = results_hits;
					OOS_error_ = true;
				}
				if(results_damage != damage_defender_takes) {
					errbuf_ << "SYNC: In attack " << unit_dump(*a_) << " vs " << unit_dump(*d_)
						<< ": the data source says the hit did " << results_damage
						<< " damage, while in-game calculations show the hit doing "
						<< damage_defender_takes
						<< " damage (over-riding game calculations with data source results)\n";
					damage_defender_takes = results_damage;
					OOS_error_ = true;
				}
			}

			if(update_display_) {
				std::string float_text = "";
				if(hits) {
					if (a_stats_->poisons &&
							!utils::string_bool(d_->second.get_state("poisoned"))) {
						float_text = float_text + _("poisoned") + "\n";
					}

					if(a_stats_->slows && !utils::string_bool(d_->second.get_state("slowed"))) {
						float_text = float_text + _("slowed") + "\n";
					}

					// If the defender is turned to stone, the fight stops immediately
					static const std::string stone_string("stone");
					if (a_stats_->stones) {
						float_text = float_text + _("stone") + "\n";
					}
				}

				unit_display::unit_attack(attacker_,defender_,
						damage_defender_takes,
						*a_stats_->weapon,d_stats_->weapon,
						abs_n_attack_,float_text);
			}
			bool dies = d_->second.take_hit(damage_defender_takes);
			LOG_NG << "defender took " << damage_defender_takes << (dies ? " and died" : "") << "\n";
			if(dies) {
				unit_display::unit_die(defender_,d_->second,a_stats_->weapon,d_stats_->weapon, &(a_->second));
			}
			attack_stats.attack_result(hits ? (dies ? statistics::attack_context::KILLS : statistics::attack_context::HITS)
			                           : statistics::attack_context::MISSES, attacker_damage_);

			if(ran_results == NULL) {
				log_scope2(engine, "setting random results");
				config cfg;
				cfg["hits"] = (hits ? "yes" : "no");
				cfg["dies"] = (dies ? "yes" : "no");

				cfg["damage"] = lexical_cast<std::string>(damage_defender_takes);
				cfg["chance"] = lexical_cast<std::string>(attacker_cth_);

				set_random_results(cfg);
			} else {
				const bool results_dies = (*ran_results)["dies"] == "yes";
				if(results_dies != dies) {
					errbuf_ << "SYNC: In attack " << unit_dump(*a_) << " vs " << unit_dump(*d_)
						<< ": the data source the unit "
						<< (results_dies ? "perished" : "survived")
						<< " while in-game calculations show the unit "
						<< (dies ? "perished" : "survived")
						<< " (over-riding game calculations with data source results)\n";
					dies = results_dies;
					OOS_error_ = true;
				}
			}
			if(hits) {
				try {
					fire_event("attacker_hits");
				} catch (attack_end_exception) {
					break;
				}
				refresh_bc();
			} else {
				try {
					fire_event("attacker_misses");
				} catch (attack_end_exception) {
					break;
				}
				refresh_bc();
			}

			LOG_NG << "done attacking\n";
			if(dies || hits) {
				int amount_drained = a_stats_->drains ? attacker_damage_ / 2 : 0;

				if(amount_drained > 0) {
					char buf[50];
					snprintf(buf,sizeof(buf),"%d",amount_drained);
					if (update_display_){
						gui_.float_label(a_->first,buf,0,255,0);
					}
					a_->second.heal(amount_drained);
				}
			}

			if(dies) { // attacker kills defender
				attackerxp_ = game_config::kill_experience*d_->second.level();
				if(d_->second.level() == 0)
					attackerxp_ = game_config::kill_experience/2;
				a_->second.get_experience(attackerxp_);
				attackerxp_ = defenderxp_ = 0;
				gui_.invalidate(a_->first);

				game_events::entity_location death_loc(d_);
				game_events::entity_location attacker_loc(a_);
				std::string undead_variation = d_->second.undead_variation();
				const int defender_side = d_->second.side();
				fire_event("attack_end");
				game_events::fire("die",death_loc,attacker_loc);

				d_ = units_.find(death_loc);
				a_ = units_.find(attacker_loc);
				if(d_ == units_.end() || !death_loc.matches_unit(d_->second)) {
					// WML has invalidated the dying unit, abort
					break;
				} else if(d_->second.hitpoints() <= 0) {
					units_.erase(d_);
					d_ = units_.end();
				}

				if(a_ == units_.end() || !attacker_loc.matches_unit(a_->second)) {
					// WML has invalidated the killing unit, abort
					break;
				}
				refresh_bc();

				if(a_stats_->plagues) {
					// plague units make new units on the target hex
					game_data::unit_type_map::const_iterator reanimitor;
					LOG_NG<<"trying to reanimate "<<a_stats_->plague_type<<std::endl;
					reanimitor = info_.unit_types.find(a_stats_->plague_type);
					LOG_NG<<"found unit type:"<<reanimitor->second.id()<<std::endl;
					if(reanimitor != info_.unit_types.end()) {
						unit newunit=unit(&info_,&units_,&map_,&state_,&teams_,&reanimitor->second,a_->second.side(),true,true);
						newunit.set_attacks(0);
						// Apply variation
						if(strcmp(undead_variation.c_str(),"null")) {
							config mod;
							config& variation=mod.add_child("effect");
							variation["apply_to"]="variation";
							variation["name"]=undead_variation;
							newunit.add_modification("variation",mod);
							newunit.heal_all();
						}
						units_.add(new std::pair<gamemap::location,unit>(death_loc,newunit));
						preferences::encountered_units().insert(newunit.id());
						if (update_display_){
							gui_.invalidate(death_loc);
						}
					}
				} else {
					   LOG_NG<<"unit not reanimated"<<std::endl;
				}
				if (update_display_){
					recalculate_fog(map_,state_,info_,units_,teams_,defender_side-1);
					gui_.invalidate_all();
					gui_.recalculate_minimap();
					gui_.draw();
				}
				break;
			} else if(hits) {
				if (a_stats_->poisons &&
				   !utils::string_bool(d_->second.get_state("poisoned"))) {
					d_->second.set_state("poisoned","yes");
					LOG_NG << "defender poisoned\n";
				}

				if(a_stats_->slows && !utils::string_bool(d_->second.get_state("slowed"))) {
					d_->second.set_state("slowed","yes");
					defender_damage_ = d_stats_->slow_damage;
					LOG_NG << "defender slowed\n";
				}

				// If the defender is turned to stone, the fight stops immediately
				static const std::string stone_string("stone");
				if (a_stats_->stones) {
					d_->second.set_state("stoned","yes");
					n_defends_ = 0;
					n_attacks_ = 0;
					game_events::fire(stone_string,d_->first,a_->first);
				}
			}

			--n_attacks_;
		}

		// If the defender got to strike first, they use it up here.
		defender_strikes_first = false;
		abs_n_defend_++;

		if(n_defends_ > 0) {
			LOG_NG << "doing defender attack...\n";

			const int ran_num = get_random();
			bool hits = (ran_num%100) < defender_cth_;

			int damage_attacker_takes;
			if(hits) {
				damage_attacker_takes = defender_damage_;
			} else {
				damage_attacker_takes = 0;
			}
			// Make sure that if we're serializing a game here,
			// we got the same results as the game did originally.
			const config* ran_results = get_random_results();
			if(ran_results != NULL) {
				const int results_chance = atoi((*ran_results)["chance"].c_str());
				const bool results_hits = (*ran_results)["hits"] == "yes";
				const int results_damage = atoi((*ran_results)["damage"].c_str());

				if(results_chance != defender_cth_) {
					errbuf_ << "SYNC: In defend " << unit_dump(*a_) << " vs " << unit_dump(*d_)
						<< ": chance to hit attacker is inconsistent. Data source: "
						<< results_chance << "; Calculation: " << defender_cth_
						<< " (over-riding game calculations with data source results)\n";
					defender_cth_ = results_chance;
					OOS_error_ = true;
				}
				if(hits != results_hits) {
					errbuf_ << "SYNC: In defend " << unit_dump(*a_) << " vs " << unit_dump(*d_)
						<< ": the data source says the hit was "
						<< (results_hits ? "successful" : "unsuccessful")
						<< ", while in-game calculations say the hit was "
						<< (hits ? "successful" : "unsuccessful")
						<< " random number: " << ran_num << " = " << (ran_num%100) << "/"
						<< results_chance
						<< " (over-riding game calculations with data source results)\n";
					hits = results_hits;
					OOS_error_ = true;
				}
				if(results_damage != damage_attacker_takes) {
					errbuf_ << "SYNC: In defend " << unit_dump(*a_) << " vs " << unit_dump(*d_)
						<< ": the data source says the hit did " << results_damage
						<< " damage, while in-game calculations show the hit doing "
						<< damage_attacker_takes
						<< " damage (over-riding game calculations with data source results)\n";
					damage_attacker_takes = results_damage;
					OOS_error_ = true;
				}
			}

			if(update_display_) {
				std::string float_text = "";
				if(hits) {
					if (d_stats_->poisons &&
							!utils::string_bool(a_->second.get_state("poisoned"))) {
						float_text = float_text + _("poisoned") + "\n";
					}

					if(d_stats_->slows && !utils::string_bool(a_->second.get_state("slowed"))) {
						float_text = float_text + _("slowed") + "\n";
					}

					// If the defender is turned to stone, the fight stops immediately
					static const std::string stone_string("stone");
					if (d_stats_->stones) {
						float_text = float_text + _("stone") + "\n";
					}
				}
				unit_display::unit_attack(defender_,attacker_,
						damage_attacker_takes,
						*d_stats_->weapon,a_stats_->weapon,
						abs_n_defend_,float_text);
			}
			bool dies = a_->second.take_hit(damage_attacker_takes);
			LOG_NG << "attacker took " << damage_attacker_takes << (dies ? " and died" : "") << "\n";
			if(dies) {
				unit_display::unit_die(attacker_,a_->second,a_stats_->weapon,d_stats_->weapon, &(d_->second));
			}
			if(ran_results == NULL) {
				config cfg;
				cfg["hits"] = (hits ? "yes" : "no");
				cfg["dies"] = (dies ? "yes" : "no");
				cfg["damage"] = lexical_cast<std::string>(damage_attacker_takes);
				cfg["chance"] = lexical_cast<std::string>(defender_cth_);

				set_random_results(cfg);
			} else {
				const bool results_dies = (*ran_results)["dies"] == "yes";
				if(results_dies != dies) {
					errbuf_ << "SYNC: In defend " << unit_dump(*a_) << " vs " << unit_dump(*d_)
						<< ": the data source the unit "
						<< (results_dies ? "perished" : "survived")
						<< " while in-game calculations show the unit "
						<< (dies ? "perished" : "survived")
						<< " (over-riding game calculations with data source results)\n";
					dies = results_dies;
					OOS_error_ = true;
				}
			}
			if(hits) {
				try {
					fire_event("defender_hits");
				} catch (attack_end_exception) {
					break;
				}
				refresh_bc();
			} else {
				try {
					fire_event("defender_misses");
				} catch (attack_end_exception) {
					break;
				}
				refresh_bc();
			}
			attack_stats.defend_result(hits ? (dies ? statistics::attack_context::KILLS : statistics::attack_context::HITS)
			                           : statistics::attack_context::MISSES, defender_damage_);
			if(hits || dies){
				int amount_drained = d_stats_->drains ? defender_damage_ / 2 : 0;

				if(amount_drained > 0) {
					char buf[50];
					snprintf(buf,sizeof(buf),"%d",amount_drained);
					if (update_display_){
						gui_.float_label(d_->first,buf,0,255,0);
					}
					d_->second.heal(amount_drained);
				}
			}

			if(dies) { // defender kills attacker
				defenderxp_ = game_config::kill_experience*a_->second.level();
				if(a_->second.level() == 0)
					defenderxp_ = game_config::kill_experience/2;
				d_->second.get_experience(defenderxp_);
				attackerxp_ = defenderxp_ = 0;
				gui_.invalidate(d_->first);
				std::string undead_variation = a_->second.undead_variation();

				game_events::entity_location death_loc(a_);
				game_events::entity_location defender_loc(d_);
				const int attacker_side = a_->second.side();
				fire_event("attack_end");
				game_events::fire("die",death_loc,defender_loc);

				refresh_bc();

				if(a_ == units_.end() || !death_loc.matches_unit(a_->second)) {
					// WML has invalidated the dying unit, abort
					break;
				} else if(a_->second.hitpoints() <= 0) {
					units_.erase(a_);
					a_ = units_.end();
				}

				if(d_ == units_.end() || !defender_loc.matches_unit(d_->second)) {
					// WML has invalidated the killing unit, abort
					break;
				} else if(d_stats_->plagues) {
					// plague units make new units on the target hex.
					game_data::unit_type_map::const_iterator reanimitor;
					LOG_NG<<"trying to reanimate "<<d_stats_->plague_type<<std::endl;
					reanimitor = info_.unit_types.find(d_stats_->plague_type);
					LOG_NG<<"found unit type:"<<reanimitor->second.id()<<std::endl;
					if(reanimitor != info_.unit_types.end()) {
						unit newunit=unit(&info_,&units_,&map_,&state_,&teams_,&reanimitor->second,d_->second.side(),true,true);
						// Apply variation
						if(strcmp(undead_variation.c_str(),"null")){
							config mod;
							config& variation=mod.add_child("effect");
							variation["apply_to"]="variation";
							variation["name"]=undead_variation;
							newunit.add_modification("variation",mod);
						}
						units_.add(new std::pair<gamemap::location,unit>(death_loc,newunit));
						if (update_display_){
							gui_.invalidate(death_loc);
						}
					}
				} else {
					LOG_NG<<"unit not reanimated"<<std::endl;
				}
				if (update_display_){
					recalculate_fog(map_,state_,info_,units_,teams_,attacker_side-1);
					gui_.invalidate_all();
					gui_.recalculate_minimap();
					gui_.draw();
				}
				break;
			} else if(hits) {
				if (d_stats_->poisons &&
				   !utils::string_bool(a_->second.get_state("poisoned"))) {
					a_->second.set_state("poisoned","yes");
					LOG_NG << "attacker poisoned\n";
				}

				if(d_stats_->slows && !utils::string_bool(a_->second.get_state("slowed"))) {
					a_->second.set_state("slowed","yes");
					attacker_damage_ = a_stats_->slow_damage;
					LOG_NG << "attacker slowed\n";
				}


				// If the attacker is turned to stone, the fight stops immediately
				static const std::string stone_string("stone");
				if (d_stats_->stones) {
					a_->second.set_state("stoned","yes");
					n_defends_ = 0;
					n_attacks_ = 0;
					game_events::fire(stone_string,a_->first,d_->first);
				}
			}

			--n_defends_;
		}

		// Continue the fight to death; if one of the units got stoned,
		// either n_attacks or n_defends is -1
		if(rounds > 0 && n_defends_ == 0 && n_attacks_ == 0) {
			n_attacks_ = orig_attacks_;
			n_defends_ = orig_defends_;
			--rounds;
			defender_strikes_first = (d_stats_->firststrike && ! a_stats_->firststrike);
		}
		if(n_attacks_ <= 0 && n_defends_ <= 0) {
			fire_event("attack_end");
		}
	}

	if(a_ != units.end()) {
		a_->second.set_standing(gui_,a_->first);
		if(attackerxp_)
			a_->second.get_experience(attackerxp_);
	}

	if(d_ != units.end()) {
		d_->second.set_standing(gui_,d_->first);
		if(defenderxp_)
			d_->second.get_experience(defenderxp_);
	}

	if (update_display_){
		gui_.invalidate_unit();
		gui_.invalidate(attacker);
		gui_.invalidate(defender);
		gui_.draw(true,true);
	}

	if(OOS_error_) {
		replay::throw_error(errbuf_.str());
	}

}

int village_owner(const gamemap::location& loc, const std::vector<team>& teams)
{
	for(size_t i = 0; i != teams.size(); ++i) {
		if(teams[i].owns_village(loc))
			return i;
	}

	return -1;
}

bool get_village(const gamemap::location& loc, game_display& disp,
                 std::vector<team>& teams, size_t team_num,
                 const unit_map& units, int *action_timebonus)
{
	if(team_num < teams.size() && teams[team_num].owns_village(loc)) {
		return false;
	}

	const bool has_leader = find_leader(units,int(team_num+1)) != units.end();
	bool grants_timebonus = false;

	// We strip the village off all other sides, unless it is held by an ally
	// and we don't have a leader (and thus can't occupy it)
	for(std::vector<team>::iterator i = teams.begin(); i != teams.end(); ++i) {
		const unsigned int side = i - teams.begin() + 1;
		if(team_num >= teams.size() || has_leader || teams[team_num].is_enemy(side)) {
			i->lose_village(loc);
			if(team_num + 1 != side && action_timebonus) {
				grants_timebonus = true;
			}
		}
	}

	if(grants_timebonus) {
		teams[team_num].set_action_bonus_count(1 + teams[team_num].action_bonus_count());
		*action_timebonus = 1;
	}

	if(team_num >= teams.size()) {
		return false;
	}

	if(has_leader) {
		disp.invalidate(loc);
		return teams[team_num].get_village(loc);
	}

	return false;
}

unit_map::iterator find_leader(unit_map& units, int side)
{
	for(unit_map::iterator i = units.begin(); i != units.end(); ++i) {
		if(static_cast<int>(i->second.side()) == side && i->second.can_recruit())
			return i;
	}

	return units.end();
}

unit_map::const_iterator find_leader(const unit_map& units, int side)
{
	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if(static_cast<int>(i->second.side()) == side && i->second.can_recruit())
			return i;
	}

	return units.end();
}

// Simple algorithm: no maximum number of patients per healer.
void reset_resting(unit_map& units, unsigned int side)
{
	for (unit_map::iterator i = units.begin(); i != units.end(); ++i) {
		if (i->second.side() == side)
			i->second.set_resting(true);
	}
}

void calculate_healing(game_display& disp, const gamemap& map,
                       unit_map& units, unsigned int side,
					   const std::vector<team>& teams, bool update_display)
{
	// We look for all allied units, then we see if our healer is near them.
	for (unit_map::iterator i = units.begin(); i != units.end(); ++i) {

		if (!utils::string_bool(i->second.get_state("healable"),true))
			continue;

		unit_map::iterator curer = units.end();
		std::vector<unit_map::iterator> healers;

		int healing = 0;
		std::string curing;

		unit_ability_list heal = i->second.get_abilities("heals",i->first);

		const bool is_poisoned = utils::string_bool(i->second.get_state("poisoned"));
		if(is_poisoned) {
			// Remove the enemies' healers to determine if poison is slowed or cured
			for(std::vector<std::pair<config*,gamemap::location> >::iterator h_it = heal.cfgs.begin(); h_it != heal.cfgs.end();) {
				unit_map::iterator potential_healer = units.find(h_it->second);
				wassert(potential_healer != units.end());
				if(teams[potential_healer->second.side()-1].is_enemy(side)) {
					h_it = heal.cfgs.erase(h_it);
				} else {
					++h_it;
				}
			}
			for(std::vector<std::pair<config*,gamemap::location> >::const_iterator heal_it = heal.cfgs.begin(); heal_it != heal.cfgs.end(); ++heal_it) {
				if((*heal_it->first)["poison"] == "cured") {
					curer = units.find(heal_it->second);
					// Full curing only occurs on the healer turn (may be changed)
					if(curer->second.side() == side) {
						curing = "cured";
					} else if(curing != "cured") {
						curing = "slowed";
					}
				} else if(curing != "cured" && (*heal_it->first)["poison"] == "slowed") {
					curer = units.find(heal_it->second);
					curing = "slowed";
				}
			}
		}

		// For heal amounts, only consider healers on side which is starting now.
		// Remove all healers not on this side.
		for(std::vector<std::pair<config*,gamemap::location> >::iterator h_it = heal.cfgs.begin(); h_it != heal.cfgs.end();) {
			unit_map::iterator potential_healer = units.find(h_it->second);
			wassert(potential_healer != units.end());
			if(potential_healer->second.side() != side) {
				h_it = heal.cfgs.erase(h_it);
			} else {
				++h_it;
			}
		}

		unit_abilities::effect heal_effect(heal,0,false);
		healing = heal_effect.get_composite_value();

		for(std::vector<unit_abilities::individual_effect>::const_iterator heal_loc = heal_effect.begin(); heal_loc != heal_effect.end(); ++heal_loc) {
			healers.push_back(units.find(heal_loc->loc));
		}

		if(i->second.side() == side) {
			unit_ability_list regen = i->second.get_abilities("regenerate",i->first);
			unit_abilities::effect regen_effect(regen,0,false);
			if(regen_effect.get_composite_value() > healing) {
				healing = regen_effect.get_composite_value();
				healers.clear();
			}
			if(regen.cfgs.size()) {
				for(std::vector<std::pair<config*,gamemap::location> >::const_iterator regen_it = regen.cfgs.begin(); regen_it != regen.cfgs.end(); ++regen_it) {
					if((*regen_it->first)["poison"] == "cured") {
						curer = units.end();
						curing = "cured";
					} else if(curing != "cured" && (*regen_it->first)["poison"] == "slowed") {
						curer = units.end();
						curing = "slowed";
					}
				}
			}
			if (map.gives_healing(i->first)) {
				if(map.gives_healing(i->first) > healing) {
					healing = map.gives_healing(i->first);
					healers.clear();
				}
				//! @todo FIXME
				curing = "cured";
				curer = units.end();
			}
			if(i->second.resting()) {
				healing += game_config::rest_heal_amount;
			}
		}
		if(is_poisoned) {
			if(curing == "cured") {
				i->second.set_state("poisoned","");
				healing = 0;
				healers.clear();
				healers.push_back(curer);
			} else if(curing == "slowed") {
				healing = 0;
				healers.clear();
				healers.push_back(curer);
			} else {
				healers.clear();
				healing = 0;
				if(i->second.side() == side) {
					healing = -game_config::poison_amount;
				}
			}
		}

		if (curing == "" && healing==0) {
			continue;
		}

		int pos_max = i->second.max_hitpoints() - i->second.hitpoints();
		int neg_max = -(i->second.hitpoints() - 1);
		if(healing > 0 && pos_max <= 0) {
			// Do not try to "heal" if HP >= max HP
			continue;
		}
		if(healing > pos_max) {
			healing = pos_max;
		} else if(healing < neg_max) {
			healing = neg_max;
		}

		if ( !recorder.is_skipping()
			&& update_display
			&& !(i->second.invisible(i->first,units,teams) &&
				teams[disp.viewing_team()].is_enemy(side))) {
			unit_display::unit_healing(i->second,i->first,healers,healing);
		}
		if (healing > 0)
			i->second.heal(healing);
		else if (healing < 0)
			i->second.take_hit(-healing);
	}
}


unit get_advanced_unit(const game_data& info,
                  unit_map& units,
                  const gamemap::location& loc, const std::string& advance_to)
{
	const std::map<std::string,unit_type>::const_iterator new_type = info.unit_types.find(advance_to);
	const unit_map::iterator un = units.find(loc);
	if(new_type != info.unit_types.end() && un != units.end()) {
		unit new_unit(un->second);
		new_unit.get_experience(-new_unit.max_experience());
		new_unit.advance_to(&(new_type->second));
		return new_unit;
	} else {
		throw game::game_error("Could not find the unit being advanced"
		                             " to: " + advance_to);
	}
}

void advance_unit(const game_data& info,
                  unit_map& units,
                  gamemap::location loc, const std::string& advance_to)
{
	if(units.count(loc) == 0) {
		return;
	}
	const unit& new_unit = get_advanced_unit(info,units,loc,advance_to);
	LOG_NG << "firing advance event\n";
	game_events::fire("advance",loc);
	statistics::advance_unit(new_unit);

	preferences::encountered_units().insert(new_unit.id());
	LOG_STREAM(info, config) << "Added '" << new_unit.id() << "' to encountered units\n";

	units.replace(new std::pair<gamemap::location,unit>(loc,new_unit));
	LOG_NG << "firing post_advance event\n";
	game_events::fire("post_advance",loc);
}

void check_victory(unit_map& units, std::vector<team>& teams)
{
	std::vector<int> seen_leaders;
	for(unit_map::const_iterator i = units.begin();
	    i != units.end(); ++i) {
		if(i->second.can_recruit()) {
			LOG_NG << "seen leader for side " << i->second.side() << "\n";
			seen_leaders.push_back(i->second.side());
		}
	}

	// Clear villages for teams that have no leader
	for(std::vector<team>::iterator tm = teams.begin(); tm != teams.end(); ++tm) {
		if(std::find(seen_leaders.begin(),seen_leaders.end(),tm-teams.begin() + 1) == seen_leaders.end()) {
			tm->clear_villages();
		}
	}

	bool found_enemies = false;
	bool found_player = false;

	for(size_t n = 0; n != seen_leaders.size(); ++n) {
		const size_t side = seen_leaders[n]-1;

		wassert(side < teams.size());

		for(size_t m = n+1; m != seen_leaders.size(); ++m) {
			if(side < teams.size() && teams[side].is_enemy(seen_leaders[m])) {
				found_enemies = true;
			}
		}

		if (teams[side].is_human() || teams[side].is_persistent()) {
			found_player = true;
		}
	}

	if(found_enemies == false) {
		if (found_player) {
			game_events::fire("enemies defeated");
		}

		if (victory_conditions::victory_when_enemies_defeated() == false
			&& (found_player || is_observer())){
			// This level has asked not to be ended by this condition.
			return;
		}

		if(non_interactive()) {
			std::cout << "winner: ";
			for(std::vector<int>::const_iterator i = seen_leaders.begin(); i != seen_leaders.end(); ++i) {
				std::string ai = teams[*i - 1].ai_algorithm();
				if (ai == "") ai = "default ai";
				std::cout << *i << " (using " << ai << ") ";
			}
			std::cout << "\n";
		}

		LOG_NG << "throwing end level exception...\n";
		throw end_level_exception(found_player ? VICTORY : DEFEAT);
	}
}

time_of_day timeofday_at(const gamestatus& status,const unit_map& units,const gamemap::location& loc, const gamemap& map)
{
	int lighten = maximum<int>(map.get_terrain_info(map.get_terrain(loc)).light_modification() , 0);
	int darken = minimum<int>(map.get_terrain_info(map.get_terrain(loc)).light_modification() , 0);

	time_of_day tod = status.get_time_of_day(lighten + darken,loc);

	if(loc.valid()) {
		gamemap::location locs[7];
		locs[0] = loc;
		get_adjacent_tiles(loc,locs+1);

		for(int i = 0; i != 7; ++i) {
			const unit_map::const_iterator itor = units.find(locs[i]);
			if(itor != units.end() &&
			   itor->second.get_ability_bool("illuminates",itor->first) && !itor->second.incapacitated()) {
				unit_ability_list illum = itor->second.get_abilities("illuminates",itor->first);
				unit_abilities::effect illum_effect(illum,lighten,false);
				int mod = illum_effect.get_composite_value();
				if(mod + tod.lawful_bonus > illum.highest("max_value").first) {
					mod = illum.highest("max_value").first - tod.lawful_bonus;
				}
				lighten = maximum<int>(mod, lighten);
				darken = minimum<int>(mod, darken);
			}
		}
	}
	tod = status.get_time_of_day(lighten + darken,loc);

	return tod;
}

int combat_modifier(const gamestatus& status,
			const unit_map& units,
			const gamemap::location& loc,
			 unit_type::ALIGNMENT alignment,
			 bool is_fearless,
			const gamemap& map)
{
	const time_of_day& tod = timeofday_at(status,units,loc,map);

	int bonus = tod.lawful_bonus;

	if(alignment == unit_type::NEUTRAL)
		bonus = 0;
	else if(alignment == unit_type::CHAOTIC)
		bonus = -bonus;
	if(is_fearless)
		bonus = maximum<int>(bonus, 0);

	return bonus;
}

namespace {

bool clear_shroud_loc(const gamemap& map, team& tm,
                      const gamemap::location& loc,
                      std::vector<gamemap::location>* cleared)
{
	bool result = false;
	gamemap::location adj[7];
	get_adjacent_tiles(loc,adj);
	adj[6] = loc;
	for(int i = 0; i != 7; ++i) {

		// We clear one past the edge of the board, so that the half-hexes
		// at the edge can also be cleared of fog/shroud.
		if(map.on_board(adj[i]) || map.on_board(loc)) {
			const bool res = tm.clear_shroud(adj[i].x,adj[i].y) ||
								tm.clear_fog(adj[i].x,adj[i].y);

			if(res && cleared != NULL) {
				cleared->push_back(adj[i]);
			}

			result |= res;
		}
	}

	return result;
}

//! Returns true iff some shroud is cleared.
//! seen_units will return new units that have been seen by this unit.
//! If known_units is NULL, seen_units can be NULL and will not be changed.
bool clear_shroud_unit(const gamemap& map,
		const gamestatus& status,
		const game_data& gamedata,
		const unit_map& units, const gamemap::location& loc,
		std::vector<team>& teams, int team,
		const std::set<gamemap::location>* known_units,
		std::set<gamemap::location>* seen_units)
{
	std::vector<gamemap::location> cleared_locations;

	const unit_map::const_iterator u = units.find(loc);
	if(u == units.end()) {
		return false;
	}

	unit_map temp_units(u->first, u->second);

	paths p(map,status,gamedata,temp_units,loc,teams,true,false,teams[team]);
	for(paths::routes_map::const_iterator i = p.routes.begin();
	    i != p.routes.end(); ++i) {
		clear_shroud_loc(map,teams[team],i->first,&cleared_locations);
	}

	// Clear the location the unit is at
	clear_shroud_loc(map,teams[team],loc,&cleared_locations);

	// Remove all redundant location.
	// If a unit is on this location, the sighed event is called twice.
	std::unique(cleared_locations.begin(),cleared_locations.end());

	for(std::vector<gamemap::location>::const_iterator it =
	    cleared_locations.begin(); it != cleared_locations.end(); ++it) {

		const unit_map::const_iterator sighted = units.find(*it);
		if(sighted != units.end() &&
		  (sighted->second.invisible(*it,units,teams) == false
		  || teams[team].is_enemy(sighted->second.side()) == false)) {
			//check if we know this unit, but we always know oursefl
			//just in case we managed to move on a fogged hex (teleport)
			if(!(seen_units == NULL || known_units == NULL)
					&& known_units->count(*it) == 0 && *it != loc) {
				if (!utils::string_bool(sighted->second.get_state("stoned")))
				{
					seen_units->insert(*it);
				}
				if ( teams[team].uses_shroud() || teams[team].uses_fog())
				{
					static const std::string sighted_str("sighted");
					game_events::fire(sighted_str,*it,loc);
				}
			}
		}
	}

	return cleared_locations.empty() == false;
}

}

void recalculate_fog(const gamemap& map, const gamestatus& status,
		const game_data& gamedata, unit_map& units,
		std::vector<team>& teams, int team) {

	teams[team].refog();

	for(unit_map::iterator i = units.begin(); i != units.end(); ++i) {
		if(static_cast<int>(i->second.side()) == team + 1) {
			const unit_movement_resetter move_resetter(i->second);

			clear_shroud_unit(map,status,gamedata,units,i->first,teams,team,NULL,NULL);
		}
	}
	game_events::pump();
}

bool clear_shroud(game_display& disp, const gamestatus& status,
		const gamemap& map, const game_data& gamedata,
                  unit_map& units, std::vector<team>& teams, int team)
{
	if(teams[team].uses_shroud() == false && teams[team].uses_fog() == false)
		return false;

	bool result = false;

	unit_map::iterator i;
	for(i = units.begin(); i != units.end(); ++i) {
		if(static_cast<int>(i->second.side()) == team + 1) {
			const unit_movement_resetter move_resetter(i->second);

			result |= clear_shroud_unit(map,status,gamedata,units,i->first,teams,team,NULL,NULL);
		}
	}
	game_events::pump();

	if (teams[team].uses_fog()) {
		recalculate_fog(map,status,gamedata,units,teams,team);
	}

	disp.labels().recalculate_shroud();

	return result;
}

size_t move_unit(game_display* disp, const game_data& gamedata,
                 const gamestatus& status, const gamemap& map,
                 unit_map& units, std::vector<team>& teams,
                 std::vector<gamemap::location> route,
                 replay* move_recorder, undo_list* undo_stack,
                 gamemap::location *next_unit, bool continue_move, bool should_clear_shroud)
{
	wassert(route.empty() == false);

	// Stop the user from issuing any commands while the unit is moving
	const events::command_disabler disable_commands;

	unit_map::iterator ui = units.find(route.front());

	wassert(ui != units.end());

	ui->second.set_goto(gamemap::location());

	const size_t team_num = ui->second.side()-1;


	team& team = teams[team_num];
	const bool check_shroud = should_clear_shroud && team.auto_shroud_updates() &&
		(team.uses_shroud() || team.uses_fog());

	std::set<gamemap::location> known_units;
	if(check_shroud) {
		for(unit_map::const_iterator u = units.begin(); u != units.end(); ++u) {
			if(team.fogged(u->first.x,u->first.y) == false) {
				known_units.insert(u->first);
				team.see(u->second.side()-1);
			}
		}
	}

	// See how far along the given path we can move.
	const int starting_moves = ui->second.movement_left();
	int moves_left = starting_moves;
	std::set<gamemap::location> seen_units;
	bool discovered_unit = false;
	bool should_clear_stack = false;
	std::vector<gamemap::location>::const_iterator step;
	std::string ambushed_string;
	for(step = route.begin()+1; step != route.end(); ++step) {
		const t_translation::t_letter terrain = map[*step];

		const unit_map::const_iterator enemy_unit = units.find(*step);
		const bool skirmisher = ui->second.get_ability_bool("skirmisher",*step);

		const int mv = ui->second.movement_cost(terrain);
		if(discovered_unit || continue_move == false && seen_units.empty() == false ||
		   mv > moves_left || enemy_unit != units.end() && team.is_enemy(enemy_unit->second.side())) {
			break;
		} else {
			moves_left -= mv;
		}

		if(!skirmisher && enemy_zoc(map,units,teams,*step,team,ui->second.side())) {
			moves_left = 0;
		}

		// If we use fog or shroud, see if we have sighted an enemy unit,
		// in which case we should stop immediately.
		// Cannot use check shroud, because also need to check if delay shroud is on.
		if(should_clear_shroud && (team.uses_shroud() || team.uses_fog())) {
			//we don't want to interrupt our move when we are on an other unit
			//or a uncaptured village (except if it was our plan to end there)
			if( units.count(*step) == 0 &&
				(!map.is_village(*step) || team.owns_village(*step) || step+1==route.end()) ) {
				LOG_NG << "checking for units from " << (step->x+1) << "," << (step->y+1) << "\n";

				// Temporarily reset the unit's moves to full
				const unit_movement_resetter move_resetter(ui->second);

				// We have to swap out any unit that is already in the hex,
				// so we can put our unit there, then we'll swap back at the end.
				const temporary_unit_placer unit_placer(units,*step,ui->second);
				if( team.auto_shroud_updates()) {
					should_clear_stack |= clear_shroud_unit(map,status,gamedata,units,*step,teams,
					    ui->second.side()-1,&known_units,&seen_units);
				} else {
					clear_shroud_unit(map,status,gamedata,units,*step,teams,
							ui->second.side()-1,&known_units,&seen_units);
				}
				if(should_clear_stack) {
					disp->invalidate_all();
				}
			}
		}
		// Check to see if the unit was deleted
		// during a sighted event in clear_shroud_unit()
		ui = units.find(route.front());
		if(ui == units.end()) {
			//! @todo FIXME: the correct behavior for sighted event would be
			// to halt movement, then fire "sighted" after firing "moveto" (see below).
			// However, since we have fired "sighted" during movement calculations
			// this is a workaround to prevent a crash.
			if(move_recorder != NULL) {
				move_recorder->add_movement(route.front(),*step);
			}
			return (step - route.begin());
		}

		// Check if we have discovered an invisible enemy unit
		gamemap::location adjacent[6];
		get_adjacent_tiles(*step,adjacent);

		for(int i = 0; i != 6; ++i) {
			// Check if we are checking ourselves
			if(adjacent[i] == ui->first)
				continue;

			const unit_map::const_iterator it = units.find(adjacent[i]);
			if(it != units.end() && teams[ui->second.side()-1].is_enemy(it->second.side()) &&
			   it->second.invisible(it->first,units,teams)) {
				discovered_unit = true;
				unit_ability_list hides = it->second.get_abilities("hides",it->first);
				for(std::vector<std::pair<config*,gamemap::location> >::const_iterator hide_it = hides.cfgs.begin();
				    hide_it != hides.cfgs.end(); ++hide_it) {
					ambushed_string =(*hide_it->first)["alert"];
				}
				should_clear_stack = true;
				moves_left = 0;
				break;
			}
		}
	}

	// Make sure we don't tread on another unit.
	std::vector<gamemap::location>::const_iterator begin = route.begin();

	std::vector<gamemap::location> steps(begin,step);
	while (!steps.empty()) {
		gamemap::location const &loc = steps.back();
		if (units.count(loc) == 0)
			break;
		moves_left += ui->second.movement_cost(map[loc]);
		steps.pop_back();
	}

	wassert(steps.size() <= route.size());

	// If we can't get all the way there and have to set a go-to.
	if(steps.size() != route.size() && discovered_unit == false) {
		if(seen_units.empty() == false) {
			ui->second.set_interrupted_move(route.back());
		} else {
			ui->second.set_goto(route.back());
		}
	} else {
		ui->second.set_interrupted_move(gamemap::location());
	}

	if(steps.size() < 2) {
		return 0;
	}

	if (next_unit != NULL)
		*next_unit = steps.back();

	// Move the unit on the screen. Hide the unit in its current location,
	// but don't actually remove it until the move is done,
	// so that while the unit is moving status etc.
	// will still display the correct number of units.
	unit_display::move_unit(map,steps,ui->second,teams);

	ui->second.set_movement(moves_left);

	std::pair<gamemap::location,unit> *p = units.extract(ui->first);
	p->first = steps.back();
	units.add(p);
	ui = units.find(p->first);

	if(move_recorder != NULL) {
		move_recorder->add_movement(steps.front(),steps.back());
	}

	bool event_mutated = false;

	int orig_village_owner = -1;
	int action_time_bonus = 0;

	if(map.is_village(steps.back())) {
		orig_village_owner = village_owner(steps.back(),teams);

		if(size_t(orig_village_owner) != team_num) {
			ui->second.set_movement(0);
			event_mutated = get_village(steps.back(),*disp,teams,team_num,units,&action_time_bonus);
		}
	}

	if(disp != NULL) {
		// Clear display helpers before firing events
		disp->unhighlight_reach();
		disp->set_route(NULL);
		disp->draw();
	}
	if(game_events::fire("moveto",steps.back())) {
		event_mutated = true;
	}

	ui = units.find(steps.back());
	if(undo_stack != NULL) {
		if(event_mutated || should_clear_stack || ui == units.end()) {
			apply_shroud_changes(*undo_stack,disp,status,map,gamedata,units,teams,team_num);
			undo_stack->clear();
		} else {
			// MP_COUNTDOWN: added param
			undo_stack->push_back(undo_action(ui->second,steps,starting_moves,action_time_bonus,orig_village_owner));
		}
	}

	if(disp != NULL) {

		// Show messages on the screen here
		if(discovered_unit) {
			if (ambushed_string.empty())
				ambushed_string = _("Ambushed!");
			// We've been ambushed, display an appropriate message
			disp->announce(ambushed_string, font::BAD_COLOUR);
		}

		if(continue_move == false && seen_units.empty() == false) {
			// The message depends on how many units have been sighted,
			// and whether they are allies or enemies, so calculate that out here
			int nfriends = 0, nenemies = 0;
			for(std::set<gamemap::location>::const_iterator i = seen_units.begin(); i != seen_units.end(); ++i) {
				LOG_NG << "processing unit at " << (i->x+1) << "," << (i->y+1) << "\n";
				const unit_map::const_iterator u = units.find(*i);

				// Unit may have been removed by an event.
				if(u == units.end()) {
					LOG_NG << "was removed\n";
					continue;
				}

				if(team.is_enemy(u->second.side())) {
					++nenemies;
				} else {
					++nfriends;
				}

				LOG_NG << "processed...\n";
				teams[team_num].see(u->second.side()-1);
			}

			// The message we display is different depending on
			// whether the units sighted were enemies or friends,
			// and their respective number.
			utils::string_map symbols;
			symbols["friends"] = lexical_cast<std::string>(nfriends);
			symbols["enemies"] = lexical_cast<std::string>(nenemies);
			std::string message;
			SDL_Color msg_colour;
			if(nfriends == 0 || nenemies == 0) {
				if(nfriends > 0) {
					message = vngettext("Friendly unit sighted", "$friends friendly units sighted", nfriends, symbols);
					msg_colour = font::GOOD_COLOUR;
				} else if(nenemies > 0) {
					message = vngettext("Enemy unit sighted!", "$enemies enemy units sighted!", nenemies, symbols);
					msg_colour = font::BAD_COLOUR;
				}
			}
			else {
				symbols["friendphrase"] = vngettext("Part of 'Units sighted! (...)' sentence^1 friendly", "$friends friendly", nfriends, symbols);
				symbols["enemyphrase"] = vngettext("Part of 'Units sighted! (...)' sentence^1 enemy", "$enemies enemy", nenemies, symbols);
				message = vgettext("Units sighted! ($friendphrase, $enemyphrase)", symbols);
				msg_colour = font::NORMAL_COLOUR;
			}

			if(steps.size() < route.size()) {
				// See if the "Continue Move" action has an associated hotkey
				const hotkey::hotkey_item& hk = hotkey::get_hotkey(hotkey::HOTKEY_CONTINUE_MOVE);
				if(!hk.null()) {
					symbols["hotkey"] = hk.get_name();
					message += "\n" + vgettext("(press $hotkey to keep moving)", symbols);
				}
			}

			disp->announce(message, msg_colour);
		}

		disp->draw();
		disp->recalculate_minimap();
	}

	wassert(steps.size() <= route.size());

	return steps.size();
}

bool unit_can_move(const gamemap::location& loc, const unit_map& units,
                   const gamemap& map, const std::vector<team>& teams)
{
	const unit_map::const_iterator u_it = units.find(loc);
	wassert(u_it != units.end());

	const unit& u = u_it->second;
	const team& current_team = teams[u.side()-1];

	if(!u.attacks_left() && u.movement_left()==0)
		return false;

	// Units with goto commands that have already done their gotos this turn
	// (i.e. don't have full movement left) should have red globes.
	if(u.has_moved() && u.has_goto()) {
		return false;
	}

	gamemap::location locs[6];
	get_adjacent_tiles(loc,locs);
	for(int n = 0; n != 6; ++n) {
		if(map.on_board(locs[n])) {
			const unit_map::const_iterator i = units.find(locs[n]);
			if(i != units.end()) {
				if(!i->second.incapacitated() && current_team.is_enemy(i->second.side())) {
					return true;
				}
			}

			if(u.movement_cost(map[locs[n]]) <= u.movement_left()) {
				return true;
			}
		}
	}

	return false;
}

void apply_shroud_changes(undo_list& undos, game_display* disp, const gamestatus& status, const gamemap& map,
	const game_data& gamedata, unit_map& units, std::vector<team>& teams, int team)
{
	// No need to do this if the team isn't using fog or shroud.
	if(!teams[team].uses_shroud() && !teams[team].uses_fog())
		return;

	/*
		This function works thusly:
		1. run through the list of undo_actions
		2. for each one, play back the unit's move
		3. for each location along the route, clear any "shrouded" squares that the unit can see
		4. clear the undo_list
		5. call clear_shroud to update the fog of war for each unit.
		6. fix up associated display stuff (done in a similar way to turn_info::undo())
	*/
	for(undo_list::iterator un = undos.begin(); un != undos.end(); ++un) {
		std::cout << "Turning an undo...\n";
		if(un->is_recall() || un->is_recruit()) continue;
		// We're not really going to mutate the unit, just temporarily
		// set its moves to maximum, but then switch them back.
		const unit_movement_resetter move_resetter(un->affected_unit);

		std::vector<gamemap::location>::const_iterator step;
		for(step = un->route.begin(); step != un->route.end(); ++step) {
			// We have to swap out any unit that is already in the hex,
			// so we can put our unit there, then we'll swap back at the end.
			const temporary_unit_placer unit_placer(units,*step,un->affected_unit);
			clear_shroud_unit(map,status,gamedata,units,*step,teams,team,NULL,NULL);

			//! @todo FIXME
			// There is potential for a bug, here. If the "sighted"
			// events, raised by the clear_shroud_unit function,
			// loops on all units, changing them all, the unit which
			// was swapped by the temporary unit placer will not be
			// affected. However, if we place the pump() out of the
			// temporary_unit_placer scope, the "sighted" event will
			// be raised with an invalid source unit, which is even
			// worse.
			game_events::pump();
		}
	}
	if(disp != NULL) {
		disp->invalidate_unit();
		disp->invalidate_game_status();
		clear_shroud(*disp,status,map,gamedata,units,teams,team);
		disp->recalculate_minimap();
		disp->unhighlight_reach();
		disp->set_route(NULL);
		disp->invalidate_all();
	} else {
		recalculate_fog(map,status,gamedata,units,teams,team);
	}
}

bool backstab_check(const gamemap::location& attacker_loc,
	const gamemap::location& defender_loc,
	const unit_map& units, const std::vector<team>& teams)
{
	const unit_map::const_iterator defender =
		units.find(defender_loc);
	if(defender == units.end()) return false; // No defender

	gamemap::location adj[6];
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
	if(opp->second.incapacitated()) return false;
	if(size_t(defender->second.side()-1) >= teams.size() ||
		size_t(opp->second.side()-1) >= teams.size())
		return true; // If sides aren't valid teams, then they are enemies
        if(teams[defender->second.side()-1].is_enemy(opp->second.side()))
                return true; // Defender and opposite are enemies
	return false; // Defender and opposite are friends
}
