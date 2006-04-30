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

#include "actions.hpp"
#include "checksum.hpp"
#include "game_config.hpp"
#include "game_errors.hpp"
#include "game_events.hpp"
#include "gettext.hpp"
#include "halo.hpp"
#include "hotkeys.hpp"
#include "menu_events.hpp"
#include "mouse_events.hpp"
#include "pathfind.hpp"
#include "preferences.hpp"
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
	bool when_enemies_defeated = true;

	void set_victory_when_enemies_defeated(bool on)
	{
		when_enemies_defeated = on;
	}

	bool victory_when_enemies_defeated()
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
	const paths::route& rt = a_star_search(leader, loc, 100.0, &calc, map.x(), map.y());

	if(rt.steps.empty())
		return false;

	return true;
}

std::string recruit_unit(const gamemap& map, int side,
       unit_map& units, unit new_unit,
       gamemap::location& recruit_location, display* disp, bool need_castle, bool full_movement)
{
	const events::command_disabler disable_commands;

	LOG_NG << "recruiting unit for side " << side << "\n";
	typedef std::map<gamemap::location,unit> units_map;

	//find the unit that can recruit
	units_map::const_iterator u = units.begin();

	for(; u != units.end(); ++u) {
		if(u->second.can_recruit() && (int)u->second.side() == side) {
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

	const bool show = disp != NULL && !disp->turbo() &&
	                  !disp->fogged(recruit_location.x,recruit_location.y);
	if(show) {
		disp->draw(true,true);
	}

	units.insert(std::pair<gamemap::location,unit>( recruit_location,new_unit));

	if(show) {
	    unit_map::iterator un = disp->get_units().find(recruit_location);
	    if( un !=disp->get_units().end()) {
			un->second.set_hidden(true);
			disp->scroll_to_tile(recruit_location.x,recruit_location.y,display::ONSCREEN);
			un->second.set_hidden(false);
			un->second.set_recruited(*disp);
			while(!un->second.get_animation()->animation_finished()) {
				disp->invalidate(recruit_location);
				disp->draw();
				events::pump();
				if(!disp->turbo()) SDL_Delay(10);

			}
			un->second.set_standing(*disp);
		}
	}
	LOG_NG << "firing recruit event\n";
	game_events::fire("recruit",recruit_location);

	checksumstream cs;
	config cfg_unit;
	new_unit.write(cfg_unit);
	::write_compressed(cs,cfg_unit);

	const config* ran_results = get_random_results();
	if(ran_results != NULL) {
		unsigned long rc = lexical_cast_default<unsigned long>
			((*ran_results)["checksum"], 0);
		if((*ran_results)["checksum"].empty() || rc != cs.checksum()) {
			ERR_NW << "SYNC: In recruit " << new_unit.id() <<
				": has checksum " << cs.checksum() <<
				" while datasource has checksum " <<
				rc << "\n";

			::write(std::cerr, cfg_unit);
			// FIXME: this was not playtested, so I will disable it
			// for release.
			//if (!game_config::ignore_replay_errors) throw replay::error();
		}

	} else {
		config cfg;
		cfg["checksum"] = lexical_cast<std::string>(cs.checksum());
		set_random_results(cfg);
	}

	return std::string();
}

void validate_recruit_unit()
{

}

gamemap::location under_leadership(const units_map& units,
                                   const gamemap::location& loc, int* bonus)
{
	
	const unit_map::const_iterator un = units.find(loc);
	if(un == units.end()) {
		return gamemap::location::null_location;
	}
	int best_bonus = un->second.get_abilities("leadership",loc).highest("value").first;
	if(bonus) {
		*bonus = best_bonus;
	}
	return un->second.get_abilities("leadership",loc).highest("value").second;
}

double pr_atleast(int m, double p, int n, int d)
{
	// calculate Pr[A does damage in [m,...)], where
	// p probability to hit, n swings, d damage/swing
	double P = 0.0;
	// 0 damage can happen when unit has no attack of this type
	if(d == 0)
		return (m <= 0) ? 1.0 : P;
	for(int k = (m + d - 1)/d; k <= n; ++k) {
		double r = 1.0;
		const int k2 = (k > n - k) ? (n - k) : k;
		for(int i = 0; i < k2; ++i) { r *= (n-i); r /= (k2-i); }
		P += r * pow(p, k) * pow(1-p, n-k);
	}
	return P;
}

double pr_between(int mn, int mx, double p, int n, int d)
{
	// calculate Pr[A does damage in [mn,mx)], where
	// p probability to hit, n swings, d damage/swing
	return pr_atleast(mn, p, n, d) - pr_atleast(mx, p, n, d);
}

int reduce(int i, int u, int v)
{
	// i-th swingpair, but reduce since u < v
	if(i == 0)
	    return i;
	else
	    return ((i-1) / v)*u + minimum<int>(u, ((i-1) % v) + 1);
}

double pr_kills_during(const int hpa, const int dmga, const double pa,
	const int swa, const int hpb, const int dmgb, const double pb,
	const int swb, const int n, const bool second)
{
	if ((swa < swb) && (swa < (n-1) % swb + 1)) // not our move
		return 0.0;
	// A kills B during swing n, and is it second?
	// take into account where one unit doesn't get all n swings
	const double t1 = pr_between(hpb - dmga, hpb, pa,
		(swa<swb) ? reduce(n-1, swa, swb) : (n-1), dmga);
	const int n2 = second ? n : (n - 1);
	const double t2 = 1.0 - pr_atleast(hpa, pb,
		(swa>swb) ? reduce(n2, swb, swa) : n2, dmgb);
	return t1 * pa * t2;
}

battle_stats evaluate_battle_stats(const gamemap& map,
                                   std::vector<team>& teams,
                                   const gamemap::location& attacker,
                                   const gamemap::location& defender,
                                   int attack_with,
                                   units_map& units,
                                   const gamestatus& state,
                                   const game_data& gamedata,
                                   gamemap::TERRAIN attacker_terrain_override,
                                   battle_stats_strings *strings)
{
	battle_stats res;
	LOG_NG << "Evaluating battle stats...\n";
	res.attack_with = attack_with;

	if (strings)
		strings->defend_name = _("none");

	const units_map::const_iterator a = units.find(attacker);
	const units_map::const_iterator d = units.find(defender);

	wassert(a != units.end());
	wassert(d != units.end());

	const gamemap::TERRAIN attacker_terrain = attacker_terrain_override ?
	                 attacker_terrain_override : map[attacker.x][attacker.y];
	const gamemap::TERRAIN defender_terrain = map[defender.x][defender.y];

	res.attacker_hp = a->second.hitpoints();
	res.defender_hp = d->second.hitpoints();

	res.chance_to_hit_attacker = a->second.defense_modifier(attacker_terrain);
	res.chance_to_hit_defender = d->second.defense_modifier(defender_terrain);
	
	const std::vector<attack_type>& attacker_attacks = a->second.attacks();
	const std::vector<attack_type>& defender_attacks = d->second.attacks();

	wassert((unsigned)attack_with < attacker_attacks.size());
	const attack_type& attack = attacker_attacks[attack_with];
	
	double best_defend_rating = 0.0;
	int defend_with = -1;
	res.ndefends = 0;
	LOG_NG << "Finding defender weapon...\n";
	for(int defend_option = 0; defend_option != int(defender_attacks.size()); ++defend_option) {
		if(defender_attacks[defend_option].range() == attack.range()) {
			if (defender_attacks[defend_option].defense_weight() > 0) {
				const attack_type& defend = defender_attacks[defend_option];
				attack.set_specials_context(attacker,defender,&gamedata,&units,&map,&state,&teams,true,&defend);
				defend.set_specials_context(attacker,defender,&gamedata,&units,&map,&state,&teams,false,&attack);
				int d_nattacks = defend.num_attacks();
				
				unit_ability_list swarm = defend.get_specials("attacks");
				if(!swarm.empty()) {
					int swarm_min_attacks = swarm.highest("attacks_min").first;
					int swarm_max_attacks = swarm.highest("attacks_max",d_nattacks).first;
					int hitp = d->second.hitpoints();
					int mhitp = d->second.max_hitpoints();
					
					d_nattacks = swarm_min_attacks + (swarm_max_attacks - swarm_min_attacks) * hitp / mhitp;
				}
				
				// calculate damage
				int bonus = 100;
				int divisor = 100;
				
				int base_damage = defend.damage();
				int resistance_modifier = a->second.damage_from(defend,true,a->first);
				bool backstab = backstab_check(d->first,a->first,units,teams);
				{ // modify damage
					unit_ability_list dmg_specials = defend.get_specials("damage");
					unit_abilities::effect dmg_effect(dmg_specials,base_damage,backstab);
					base_damage = dmg_effect.get_composite_value();
				}
				
				const int tod_modifier = combat_modifier(state,units,d->first,d->second.alignment(),map);
				bonus += tod_modifier;
				
				int leader_bonus = 0;
				if (under_leadership(units, defender, &leader_bonus).valid()) {
					bonus += leader_bonus;
		
				}
				if (utils::string_bool(d->second.get_state("slowed"))) {
					divisor *= 2;
				}
		
				bonus *= resistance_modifier;
				divisor *= 100;
				const int final_damage = round_damage(base_damage, bonus, divisor);
				
				const double rating = a->second.damage_from(defender_attacks[defend_option],true,a->first)
					*final_damage
					*d_nattacks
					*defender_attacks[defend_option].defense_weight();
				if(defend_with == -1 || rating > best_defend_rating) {
					best_defend_rating = rating;
					defend_with = defend_option;
				}
			}
		}
	}
	
	res.defend_with = defend_with;
	config tmp_config;
	static attack_type no_weapon(tmp_config,"fake_attack","");
	const attack_type& defend = defend_with == -1 ? no_weapon : defender_attacks[defend_with];
	attack.set_specials_context(attacker,defender,&gamedata,&units,&map,&state,&teams,true,&defend);
	defend.set_specials_context(attacker,defender,&gamedata,&units,&map,&state,&teams,false,&attack);
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	LOG_NG << "getting weapon specials...\n";

	static const std::string to_the_death_string("berserk");
	res.rounds = attack.get_specials(to_the_death_string).highest("value").first;
	res.defender_strikes_first = false;

	unit_ability_list plague = attack.get_specials("plague");
	static const std::string plague_string("plague");
	res.attacker_plague = !utils::string_bool(d->second.get_state("not_living")) &&
	  (!plague.empty()) &&
	  strcmp(d->second.undead_variation().c_str(),"null") &&
	  !map.is_village(defender);
	
	if(!plague.empty()) {
		if((*plague.cfgs.front().first)["type"] == "") {
		  res.attacker_plague_type = a->second.id();
		} else {
		  res.attacker_plague_type = (*plague.cfgs.front().first)["type"];
		}
	}
	res.defender_plague = false;

	static const std::string slow_string("slow");
	res.attacker_slows = attack.get_special_bool(slow_string);
	static const std::string poison_string("poison");
	res.attacker_poisons = attack.get_special_bool(poison_string);
	static const std::string stones_string("stones");
	res.attacker_stones = attack.get_special_bool(stones_string);
	
	{ // modify chance to hit
		bool backstab = backstab_check(a->first,d->first,units,teams);
		unit_ability_list cth_specials = attack.get_specials("chance_to_hit");
		unit_abilities::effect cth_effects(cth_specials,res.chance_to_hit_defender,backstab);
		res.chance_to_hit_defender = cth_effects.get_composite_value();
	}
	
	// compute swarm attacks;
	unit_ability_list swarm = attack.get_specials("attacks");
	if(!swarm.empty()) {
		int swarm_min_attacks = swarm.highest("attacks_min").first;
		int swarm_max_attacks = swarm.highest("attacks_max",attack.num_attacks()).first;
		int hitp = a->second.hitpoints();
		int mhitp = a->second.max_hitpoints();
		
		res.nattacks = swarm_min_attacks + (swarm_max_attacks - swarm_min_attacks) * hitp / mhitp;
		
	} else {
		res.nattacks = attack.num_attacks();
	}

	if (strings) {
		strings->attack_name = attack.name();
		strings->attack_type = egettext(attack.type().c_str());
		strings->attack_special = attack.weapon_specials();
		strings->attack_icon = attack.icon();

		strings->range = gettext(N_(attack.range().c_str()));
	}

	const bool counterattack = defend_with != -1;

	static const std::string EMPTY_COLUMN = std::string(1, COLUMN_SEPARATOR) + ' ' + COLUMN_SEPARATOR;

	res.damage_attacker_takes = 0;
	res.amount_attacker_drains = 0;
	res.amount_defender_drains = 0;
	if (counterattack) {
		res.rounds = maximum<int>(res.rounds,defend.get_specials(to_the_death_string).highest("value").first);

		bool backstab = backstab_check(d->first,a->first,units,teams);
		{ // modify chance to hit
			unit_ability_list cth_specials = defend.get_specials("chance_to_hit");
			unit_abilities::effect cth_effects(cth_specials,res.chance_to_hit_attacker,backstab);
			res.chance_to_hit_attacker = cth_effects.get_composite_value();
		}

		int bonus = 100;
		int divisor = 100;

		int base_damage = defend.damage();
		int resistance_modifier = a->second.damage_from(defend,true,a->first);
		
		{ // modify damage
			unit_ability_list dmg_specials = defend.get_specials("damage");
			unit_abilities::effect dmg_effects(dmg_specials,base_damage,backstab);
			base_damage = dmg_effects.get_composite_value();
		}
		
		if (strings) {
			std::stringstream str_base;
			str_base << _("base damage") << COLUMN_SEPARATOR << base_damage;
			strings->defend_calculations.push_back(str_base.str());
		}

		const int tod_modifier = combat_modifier(state,units,d->first,d->second.alignment(),map);
		bonus += tod_modifier;

		if (strings && tod_modifier != 0) {
			std::stringstream str_mod;
			const time_of_day& tod = timeofday_at(state,units,d->first,map);
			str_mod << tod.name << EMPTY_COLUMN << (tod_modifier > 0 ? "+" : "") << tod_modifier << '%';
			strings->defend_calculations.push_back(str_mod.str());
		}

		int leader_bonus = 0;
		if (under_leadership(units, defender, &leader_bonus).valid()) {
			bonus += leader_bonus;

			if (strings) {
				std::stringstream str;
				str << _("leadership") << EMPTY_COLUMN << '+' << leader_bonus << '%';
				strings->defend_calculations.push_back(str.str());
			}
		}
/*
		if (charge) {
			bonus *= 2;

			if (strings) {
				std::stringstream str;
				str << _("charge") << EMPTY_COLUMN << _("Doubled");
				strings->defend_calculations.push_back(str.str());
			}
		}
*/
		if (utils::string_bool(d->second.get_state("slowed"))) {
			divisor *= 2;
			if (strings) {
				std::stringstream str;
				str << _("slowed") << EMPTY_COLUMN << _("Halved");
				strings->defend_calculations.push_back(str.str());
			}
		}

		if (strings && resistance_modifier != 100) {
			const int resist = resistance_modifier - 100;
			std::stringstream str_resist;
			str_resist << gettext(resist < 0 ? N_("attacker resistance vs") : N_("attacker vulnerability vs"))
			           << ' ' << gettext(defend.type().c_str()) << EMPTY_COLUMN
			           << (resist > 0 ? "+" : "") << resist << '%';
			strings->defend_calculations.push_back(str_resist.str());
		}

		bonus *= resistance_modifier;
		divisor *= 100;
		const int final_damage = round_damage(base_damage, bonus, divisor);
		res.damage_attacker_takes = final_damage;

		if (strings) {
			const int difference = final_damage - base_damage;
			std::stringstream str;
			str << _("total damage") << COLUMN_SEPARATOR << res.damage_attacker_takes
			    << COLUMN_SEPARATOR << (difference >= 0 ? "+" : "")
			    << difference;
			strings->defend_calculations.push_back(str.str());
		}

		// compute swarm attacks;
		unit_ability_list swarm = defend.get_specials("attacks");
		if(!swarm.empty()) {
			int swarm_min_attacks = swarm.highest("attacks_min").first;
			int swarm_max_attacks = swarm.highest("attacks_max",defend.num_attacks()).first;
			int hitp = d->second.hitpoints();
			int mhitp = d->second.max_hitpoints();
			
			res.ndefends = swarm_min_attacks + (swarm_max_attacks - swarm_min_attacks) * hitp / mhitp;
			
		} else {
			res.ndefends = defend.num_attacks();
		}

		if (strings) {
			strings->defend_name = defend.name();
			strings->defend_type = egettext(defend.type().c_str());
			strings->defend_special = defend.weapon_specials();
			strings->defend_icon = defend.icon();
		}

		//if the defender drains, and the attacker is a living creature, then
		//the defender will drain for half the damage it does
		if (defend.get_special_bool("drains") && !utils::string_bool(a->second.get_state("not_living"))) {
			res.amount_defender_drains = res.damage_attacker_takes/2;
		}

		unit_ability_list defend_plague = attack.get_specials("plague");
		res.defender_plague = !utils::string_bool(a->second.get_state("not_living")) &&
		  (!defend_plague.empty()) &&
		  strcmp(a->second.undead_variation().c_str(),"null") &&
		  !map.is_village(attacker);
		if(!plague.empty()) {
			if((*plague.cfgs.front().first)["type"] == "") {
			  res.defender_plague_type = d->second.id();
			} else {
			  res.defender_plague_type = (*plague.cfgs.front().first)["type"];
			}
		}

		res.defender_slows = (defend.get_special_bool(slow_string));
		res.defender_poisons = (defend.get_special_bool(poison_string));
		res.defender_stones = (defend.get_special_bool(stones_string));

		static const std::string first_strike = "firststrike";
		res.defender_strikes_first = defend.get_special_bool(first_strike) && !attack.get_special_bool(first_strike);
	}

	int bonus = 100;
	int divisor = 100;

	int base_damage = attack.damage();
	int resistance_modifier = d->second.damage_from(attack,false,d->first);

	{ // modify damage
		bool backstab = backstab_check(a->first,d->first,units,teams);
		{ // modify damage
			unit_ability_list dmg_specials = attack.get_specials("damage");
			unit_abilities::effect dmg_effect(dmg_specials,base_damage,backstab);
			base_damage = dmg_effect.get_composite_value();
		}
	}
	
	if (strings) {
		std::stringstream str_base;
		str_base << _("base damage") << COLUMN_SEPARATOR << base_damage << COLUMN_SEPARATOR;
		strings->attack_calculations.push_back(str_base.str());
	}

	const int tod_modifier = combat_modifier(state,units,a->first,a->second.alignment(),map);

	bonus += tod_modifier;

	if (strings && tod_modifier != 0) {
		std::stringstream str_mod;
		const time_of_day& tod = timeofday_at(state,units,a->first,map);
		str_mod << tod.name << EMPTY_COLUMN << (tod_modifier > 0 ? "+" : "") << tod_modifier << '%';
		strings->attack_calculations.push_back(str_mod.str());
	}

	int leader_bonus = 0;
	if (under_leadership(units,attacker,&leader_bonus).valid()) {
		bonus += leader_bonus;

		if (strings) {
			std::stringstream str;
			str << _("leadership") << EMPTY_COLUMN << '+' << leader_bonus << '%';
			strings->attack_calculations.push_back(str.str());
		}
	}
	if (strings && resistance_modifier != 100) {
		const int resist = resistance_modifier - 100;
		std::stringstream str_resist;
		str_resist << gettext(resist < 0 ? N_("defender resistance vs") : N_("defender vulnerability vs"))
		           << ' ' << gettext(attack.type().c_str());

		str_resist << EMPTY_COLUMN
		           << (resist > 0 ? "+" : "") << resist << '%';
		strings->attack_calculations.push_back(str_resist.str());

	}

	if (utils::string_bool(a->second.get_state("slowed"))) {
		divisor *= 2;

		if (strings) {
			std::stringstream str;
			str << _("slowed") << EMPTY_COLUMN << _("Halved");
			strings->attack_calculations.push_back(str.str());
		}
	}

	bonus *= resistance_modifier;
	divisor *= 100;
	const int final_damage = round_damage(base_damage, bonus, divisor);
	res.damage_defender_takes = final_damage;

	if (strings) {
		const int difference = final_damage - base_damage;
		std::stringstream str;
		str << _("total damage") << COLUMN_SEPARATOR << res.damage_defender_takes
		    << COLUMN_SEPARATOR << (difference >= 0 ? "+" : "")
		    << difference;
		strings->attack_calculations.push_back(str.str());
	}

	//if the attacker drains, and the defender is a living creature, then
	//the attacker will drain for half the damage it does
	if(attack.get_special_bool("drains") && utils::string_bool(d->second.get_state("not_living"))) {
		res.amount_attacker_drains = res.damage_defender_takes/2;
	}

	// FIXME: doesn't take into account berserk+slow or drain
	if (strings && res.amount_attacker_drains == 0 &&
		res.amount_defender_drains == 0 &&
		!(res.rounds &&
			(res.attacker_slows || res.defender_slows)))
	{
		const int maxrounds = (res.rounds ? res.rounds : 1);
		const int hpa = res.attacker_hp;
		const int hpb = res.defender_hp;
		const int dmga = res.damage_defender_takes;
		const int dmgb = res.damage_attacker_takes;
		const double pa = res.chance_to_hit_defender/100.0;
		const double pb = res.chance_to_hit_attacker/100.0;
		const int swa = res.nattacks;
		const int swb = res.ndefends;
		double P1 = 0;

		for(int n = 1; n <= maxrounds*maximum<int>(swa,swb); ++n)
			P1 += pr_kills_during(hpa, dmga, pa, swa,
			hpb, dmgb, pb, swb, n, res.defender_strikes_first);
		const double P3 = (1.0 - pr_atleast(hpb,pa,maxrounds*swa,dmga))
			* (1.0 - pr_atleast(hpa,pb,maxrounds*swb,dmgb));
		std::stringstream str;
		if (P3 > 0.99) {
			str << _("(both should survive)") << EMPTY_COLUMN;
		} else {
			str << _("% Pr[kills/killed by/both survive]")
			    << EMPTY_COLUMN << (int)(P1*100+0.5)
			    << '/' << (int)((1-P1-P3)*100+0.5)
			    << '/' << (int)(P3*100+0.5);
		}
		strings->attack_calculations.push_back(str.str());
	}
	LOG_NG << "done...\n";

	return res;
}


battle_context::battle_context(const gamemap& map, const std::vector<team>& teams, const std::map<gamemap::location,unit>& units,
							   const gamestatus& status, const game_data& gamedata,
							   const gamemap::location& attacker_loc, const gamemap::location& defender_loc,
							   const attack_type& attacker_weapon): attacker_stats_(NULL), defender_stats_(NULL)
{
	const unit& attacker = units.find(attacker_loc)->second;
	const unit& defender = units.find(defender_loc)->second;

	// To choose the best defender weapon, we have to compare different
	// weapons with an heuristic function. The heuristic function requires
	// the statistics of the battle to be able to do a good job. Therefore,
	// for each defender weapon, we compute the statistics and call the
	// heuristic function to obtain its rating. We keep the weapon that
	// has the best rating.
	int best_rating = -1;

	std::vector<attack_type>::const_iterator i;
	for (i = defender.attacks().begin(); i != defender.attacks().end(); i++) {
		// Skip weapons that do not match the attacker weapon range.
		if (i->range() != attacker_weapon.range())
			continue;
		
		// Skip weapons that have a null defense weight.
		if (i->defense_weight() <= 0)
			continue;

		unit_stats *current_attacker = new unit_stats(attacker, attacker_loc, &attacker_weapon, true,
													  defender, defender_loc, &(*i),
													  units, teams, status, map, gamedata);
		unit_stats *current_defender = new unit_stats(defender, defender_loc, &(*i), false,
													  attacker, attacker_loc, &attacker_weapon,
													  units, teams, status, map, gamedata);
		int current_rating = rate_defender_weapon(*current_attacker, *current_defender);
		assert(current_rating >= 0);

		if (current_rating > best_rating) {
			delete attacker_stats_;
			delete defender_stats_;
			attacker_stats_ = current_attacker;
			defender_stats_ = current_defender;
			best_rating = current_rating;
		} else {
			delete current_attacker;
			delete current_defender;
		}
	}

	// If there is no defender weapon, compute the stats without a defender weapon.
	if (attacker_stats_ == NULL) {
		wassert(defender_stats_ == NULL);
		attacker_stats_ = new unit_stats(attacker, attacker_loc, &attacker_weapon, true, 
										 defender, defender_loc, NULL,
										 units, teams, status, map, gamedata);
		defender_stats_ = new unit_stats(defender, defender_loc, NULL, false, 
										 attacker, attacker_loc, &attacker_weapon,
										 units, teams, status, map, gamedata);
	}
}

battle_context::battle_context(const battle_context &other)
	: attacker_stats_(NULL), defender_stats_(NULL)
{
	*this = other;
}

battle_context& battle_context::operator=(const battle_context &other)
{
	if (&other != this) {
		delete attacker_stats_;
		delete defender_stats_;
		attacker_stats_ = new unit_stats(*other.attacker_stats_);
		defender_stats_ = new unit_stats(*other.defender_stats_);
	}
	return *this;
}

battle_context::unit_stats::unit_stats(const unit &u, const gamemap::location& u_loc,
									   const attack_type *u_weapon, bool attacking,
									   const unit &opp, const gamemap::location& opp_loc,
									   const attack_type *opp_weapon,
									   const std::map<gamemap::location,unit>& units,
									   const std::vector<team>& teams,
									   const gamestatus& status,
									   const gamemap& map,
									   const game_data& gamedata)
{
	// Get the current state of the unit.
	weapon = u_weapon;
	is_attacker = attacking;
	is_poisoned = utils::string_bool(u.get_state("poisoned"));
	is_slowed = utils::string_bool(u.get_state("slowed"));
	hp = u.hitpoints();
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
		damage_multiplier += combat_modifier(status, units, u_loc, u.alignment(), map);
	
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

void battle_context::unit_stats::dump() const
{
	printf("==================================\n");
	printf("is_attacker:	%d\n", (int) is_attacker);
	printf("is_poisoned:	%d\n", (int) is_poisoned);
	printf("is_slowed:	%d\n", (int) is_slowed);
	printf("slows:		%d\n", (int) slows);
	printf("drains:		%d\n", (int) drains);
	printf("stones:		%d\n", (int) stones);
	printf("poisons:	%d\n", (int) poisons);
	printf("backstab_pos:	%d\n", (int) backstab_pos);
	printf("swarm:		%d\n", (int) swarm);
	printf("rounds:	%d\n", (int) rounds);
	printf("firststrike:	%d\n", (int) firststrike);
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

// How good is this weapon attacking?  Higher is better.  WML controls weighting factor.
unsigned int battle_context::rate_attacker_weapon(double attack_weight) const
{
	// Weapon can kill the enemy in one blow: good
	if (attacker_stats_->damage >= (int)defender_stats_->hp)
		attack_weight *= 2;

	// Opponent drains: bad.
	if (defender_stats_->drains)
		attack_weight /= 2;

	attack_weight *= attacker_stats_->num_blows * attacker_stats_->damage;
	if (defender_stats_->num_blows * defender_stats_->damage)
		attack_weight /= defender_stats_->num_blows * defender_stats_->damage;

	return (unsigned int)(attack_weight * 100);
}

int battle_context::rate_defender_weapon(const unit_stats&, const unit_stats& d_stats)
{
	// This is the old defend heuristic needed to maintain backward
	// compatibility with evaluate_battle_stats(). Improve it eventually
	// when battle_context is used by attack() and friends.
	return (int) (d_stats.damage * d_stats.num_blows * d_stats.weapon->defense_weight());
}


static std::string unit_dump(std::pair< gamemap::location, unit > const &u)
{
	std::stringstream s;
	s << u.second.id() << " (" << u.first.x + 1 << ',' << u.first.y + 1 << ')';
	return s.str();
}

void attack(display& gui, const gamemap& map,
            std::vector<team>& teams,
            gamemap::location attacker,
            gamemap::location defender,
            int attack_with,
            units_map& units,
            const gamestatus& state,
            const game_data& info,
			bool update_display)
{
	//stop the user from issuing any commands while the units are fighting
	const events::command_disabler disable_commands;
	
	units_map::iterator a = units.find(attacker);
	units_map::iterator d = units.find(defender);

	if(a == units.end() || d == units.end()) {
		return;
	}
	bool OOS_error = false;

	int attackerxp = d->second.level();
	int defenderxp = a->second.level();
	
	a->second.set_attacks(a->second.attacks_left()-1);
	a->second.set_movement(a->second.movement_left()-a->second.attacks()[attack_with].movement_used());
	d->second.set_resting(false);
	
	//if the attacker was invisible, she isn't anymore!
	static const std::string hides("hides");
	a->second.set_state(hides,"");

	battle_context bc(map, teams, units, state, info, attacker, defender, a->second.attacks()[attack_with]);
	const battle_context::unit_stats& a_stats = bc.get_attacker_stats();
	const battle_context::unit_stats& d_stats = bc.get_defender_stats();

	LOG_NG << "getting attack statistics\n";

	statistics::attack_context attack_stats(a->second, d->second, a_stats.chance_to_hit, d_stats.chance_to_hit);
	
	LOG_NG << "firing attack event\n";
	config dat;
	dat.add_child("first");
	dat.add_child("second");
	(*(dat.child("first")))["weapon"]=a_stats.weapon->name();
	(*(dat.child("second")))["weapon"]=d_stats.weapon != NULL ? d_stats.weapon->name() : "none";
	game_events::fire("attack",attacker,defender,dat);
	//the event could have killed either the attacker or
	//defender, so we have to make sure they still exist
	a = units.find(attacker);
	d = units.find(defender);
	if(a == units.end() || d == units.end()) {
		return;
	}
	
	int orig_attacks = a_stats.num_blows;
	int orig_defends = d_stats.num_blows;
	int n_attacks = orig_attacks;
	int n_defends = orig_defends;
	int attacker_cth = a_stats.chance_to_hit;
	int defender_cth = d_stats.chance_to_hit;
	int attacker_damage = a_stats.damage;
	int defender_damage = d_stats.damage;
	bool defender_strikes_first = (d_stats.firststrike && ! a_stats.firststrike);
	unsigned int rounds = maximum<unsigned int>(a_stats.rounds, d_stats.rounds) - 1;

	static const std::string poison_string("poison");

	while(n_attacks > 0 || n_defends > 0) {
		LOG_NG << "start of attack loop...\n";

		if(n_attacks > 0 && defender_strikes_first == false) {
			const int ran_num = get_random();
			bool hits = (ran_num%100) < attacker_cth;

			int damage_defender_takes;
			if(hits) {
				damage_defender_takes = attacker_damage;
			} else {
				damage_defender_takes = 0;
			}
			//make sure that if we're serializing a game here,
			//we got the same results as the game did originally
			const config* ran_results = get_random_results();
			if(ran_results != NULL) {
				const int results_chance = atoi((*ran_results)["chance"].c_str());
				const bool results_hits = (*ran_results)["hits"] == "yes";
				const int results_damage = atoi((*ran_results)["damage"].c_str());

				if(results_chance != attacker_cth) {
					ERR_NW << "SYNC: In attack " << unit_dump(*a) << " vs " << unit_dump(*d)
						<< ": chance to hit defender is inconsistent. Data source: "
						<< results_chance << "; Calculation: " << attacker_cth
						<< " (over-riding game calculations with data source results)\n";
					attacker_cth = results_chance;
					OOS_error = true;
				}
				if(hits != results_hits) {
					ERR_NW << "SYNC: In attack " << unit_dump(*a) << " vs " << unit_dump(*d)
						<< ": the data source says the hit was "
						<< (results_hits ? "successful" : "unsuccessful")
						<< ", while in-game calculations say the hit was "
						<< (hits ? "successful" : "unsuccessful")
						<< " random number: " << ran_num << " = "
						<< (ran_num%100) << "/" << results_chance
						<< " (over-riding game calculations with data source results)\n";
					hits = results_hits;
					OOS_error = true;
				}
				if(results_damage != damage_defender_takes) {
					ERR_NW << "SYNC: In attack " << unit_dump(*a) << " vs " << unit_dump(*d)
						<< ": the data source says the hit did " << results_damage
						<< " damage, while in-game calculations show the hit doing "
						<< damage_defender_takes
						<< " damage (over-riding game calculations with data source results)\n";
					damage_defender_takes = results_damage;
					OOS_error = true;
				}
			}
			
			bool dies = unit_display::unit_attack(gui,units,map,attacker,defender,
				            damage_defender_takes,
							*a_stats.weapon, 
							update_display);
			if(hits) {
				const int defender_side = d->second.side();
				const int attacker_side = a->second.side();
				LOG_NG << "firing attacker_hits event\n";
				config dat;
				dat.add_child("first");
				dat.add_child("second");
				(*(dat.child("first")))["weapon"]=a_stats.weapon->name();
				(*(dat.child("second")))["weapon"]=d_stats.weapon != NULL ? d_stats.weapon->name() : "none";
				game_events::fire("attacker_hits",attacker,defender,dat);
				a = units.find(attacker);
				d = units.find(defender);
				if(a == units.end() || d == units.end()) {
					if (update_display){
						recalculate_fog(map,state,info,units,teams,attacker_side-1);
						recalculate_fog(map,state,info,units,teams,defender_side-1);
						gui.recalculate_minimap();
						gui.update_display();
					}
					LOG_NG << "firing attack_end event\n";
					game_events::fire("attack_end",attacker,defender,dat);
					a = units.find(attacker);
					d = units.find(defender);
					break;
				}
			} else {
				const int defender_side = d->second.side();
				const int attacker_side = a->second.side();
				LOG_NG << "firing attacker_misses event\n";
				config dat;
				dat.add_child("first");
				dat.add_child("second");
				(*(dat.child("first")))["weapon"]=a_stats.weapon->name();
				(*(dat.child("second")))["weapon"]=d_stats.weapon != NULL ? d_stats.weapon->name() : "none";
				game_events::fire("attacker_misses",attacker,defender,dat);
				a = units.find(attacker);
				d = units.find(defender);
				if(a == units.end() || d == units.end()) {
					if (update_display){
						recalculate_fog(map,state,info,units,teams,attacker_side-1);
						recalculate_fog(map,state,info,units,teams,defender_side-1);
						gui.recalculate_minimap();
						gui.update_display();
					}
					LOG_NG << "firing attack_end event\n";
					game_events::fire("attack_end",attacker,defender,dat);
					a = units.find(attacker);
					d = units.find(defender);
					break;
				}
			}

			LOG_NG << "done attacking\n";

			attack_stats.attack_result(hits ? (dies ? statistics::attack_context::KILLS : statistics::attack_context::HITS)
			                           : statistics::attack_context::MISSES, attacker_damage);

			if(ran_results == NULL) {
				log_scope2(engine, "setting random results");
				config cfg;
				cfg["hits"] = (hits ? "yes" : "no");
				cfg["dies"] = (dies ? "yes" : "no");

				cfg["damage"] = lexical_cast<std::string>(damage_defender_takes);
				cfg["chance"] = lexical_cast<std::string>(attacker_cth);

				set_random_results(cfg);
			} else {
				const bool results_dies = (*ran_results)["dies"] == "yes";
				if(results_dies != dies) {
					ERR_NW << "SYNC: In attack " << unit_dump(*a) << " vs " << unit_dump(*d)
						<< ": the data source the unit "
						<< (results_dies ? "perished" : "survived")
						<< " while in-game calculations show the unit "
						<< (dies ? "perished" : "survived")
						<< " (over-riding game calculations with data source results)\n";
					dies = results_dies;
					OOS_error = true;
				}
			}

			if(dies || hits) {
				int amount_drained = a_stats.drains ? attacker_damage / 2 : 0;
			
				if(amount_drained > 0) {
					char buf[50];
					snprintf(buf,sizeof(buf),"%d",amount_drained);
					if (update_display){
						gui.float_label(a->first,buf,0,255,0);
					}
					a->second.heal(amount_drained);
				}
			}

			if(dies) {//attacker kills defender
				attackerxp = game_config::kill_experience*d->second.level();
				if(d->second.level() == 0)
					attackerxp = game_config::kill_experience/2;

				a->second.get_experience(attackerxp);
				gui.invalidate(a->first);
				attackerxp = 0;
				defenderxp = 0;

				gamemap::location loc = d->first;
				gamemap::location attacker_loc = a->first;
				std::string undead_variation = d->second.undead_variation();
				const int defender_side = d->second.side();
				LOG_NG << "firing attack_end event\n";
				game_events::fire("attack_end",attacker,defender,dat);
				LOG_NG << "firing die event\n";
				game_events::fire("die",loc,a->first);
				d = units.find(loc);
				a = units.end();

				if(d != units.end() && d->second.hitpoints() <= 0) {
					units.erase(d);
					d = units.end();
				}

				//plague units make new units on the target hex
				if(a_stats.plagues) {
				        a = units.find(attacker_loc);
				        game_data::unit_type_map::const_iterator reanimitor;
				        LOG_NG<<"trying to reanimate "<<a_stats.plague_type<<std::endl;
				        reanimitor = info.unit_types.find(a_stats.plague_type);
				        LOG_NG<<"found unit type:"<<reanimitor->second.id()<<std::endl;

					if(reanimitor != info.unit_types.end()) {
					       unit newunit=unit(&info,&units,&map,&state,&teams,&reanimitor->second,a->second.side(),true,true);
					       newunit.set_attacks(0);

					       //apply variation
					       if(strcmp(undead_variation.c_str(),"null")){
						 config mod;
						 config& variation=mod.add_child("effect");
						 variation["apply_to"]="variation";
						 variation["name"]=undead_variation;
						 newunit.add_modification("variation",mod);
						 newunit.heal_all();
					       }

					       units.insert(std::pair<gamemap::location,unit>(loc,newunit));
						   if (update_display){
						       gui.invalidate(loc);
						   }
					}else{
					       LOG_NG<<"unit not reanimated"<<std::endl;
					}
				}
				if (update_display){
					recalculate_fog(map,state,info,units,teams,defender_side-1);
					gui.recalculate_minimap();
					gui.draw();
				}
				break;
			} else if(hits) {
				if (a_stats.poisons &&
				   !utils::string_bool(d->second.get_state("poisoned"))) {
					if (update_display){
						gui.float_label(d->first,_("poisoned"),255,0,0);
					}
					d->second.set_state("poisoned","yes");
				}

				if(a_stats.slows && !utils::string_bool(d->second.get_state("slowed"))) {
					if (update_display){
						gui.float_label(d->first,_("slowed"),255,0,0);
					}
					d->second.set_state("slowed","yes");
					defender_damage = d_stats.slow_damage;
				}

				//if the defender is turned to stone, the fight stops immediately
				static const std::string stone_string("stone");
				if (a_stats.stones) {
					if (update_display){
						gui.float_label(d->first,_("stone"),255,0,0);
					}
					d->second.set_state("stoned","yes");
					n_defends = 0;
					n_attacks = 0;
					game_events::fire(stone_string,d->first,a->first);
				}
			}

			--n_attacks;
		}

		//if the defender got to strike first, they use it up here.
		defender_strikes_first = false;

		if(n_defends > 0) {
			LOG_NG << "doing defender attack...\n";

			const int ran_num = get_random();
			bool hits = (ran_num%100) < defender_cth;

			int damage_attacker_takes;
			if(hits) {
				damage_attacker_takes = defender_damage;
			} else {
				damage_attacker_takes = 0;
			}
			//make sure that if we're serializing a game here,
			//we got the same results as the game did originally
			const config* ran_results = get_random_results();
			if(ran_results != NULL) {
				const int results_chance = atoi((*ran_results)["chance"].c_str());
				const bool results_hits = (*ran_results)["hits"] == "yes";
				const int results_damage = atoi((*ran_results)["damage"].c_str());

				if(results_chance != defender_cth) {
					ERR_NW << "SYNC: In defend " << unit_dump(*a) << " vs " << unit_dump(*d)
						<< ": chance to hit attacker is inconsistent. Data source: "
						<< results_chance << "; Calculation: " << defender_cth
						<< " (over-riding game calculations with data source results)\n";
					defender_cth = results_chance;
					OOS_error = true;
				}
				if(hits != results_hits) {
					ERR_NW << "SYNC: In defend " << unit_dump(*a) << " vs " << unit_dump(*d)
						<< ": the data source says the hit was "
						<< (results_hits ? "successful" : "unsuccessful")
						<< ", while in-game calculations say the hit was "
						<< (hits ? "successful" : "unsuccessful")
						<< " random number: " << ran_num << " = " << (ran_num%100) << "/"
						<< results_chance
						<< " (over-riding game calculations with data source results)\n";
					hits = results_hits;
					OOS_error = true;
				}
				if(results_damage != damage_attacker_takes) {
					ERR_NW << "SYNC: In defend " << unit_dump(*a) << " vs " << unit_dump(*d)
						<< ": the data source says the hit did " << results_damage
						<< " damage, while in-game calculations show the hit doing "
						<< damage_attacker_takes
						<< " damage (over-riding game calculations with data source results)\n";
					damage_attacker_takes = results_damage;
					OOS_error = true;
				}
			}

			bool dies = unit_display::unit_attack(gui,units,map,defender,attacker,
			               damage_attacker_takes,
						   *d_stats.weapon, 
						   update_display);
			if(hits) {
				const int defender_side = d->second.side();
				const int attacker_side = a->second.side();
				LOG_NG << "firing defender_hits event\n";
				config dat;
				dat.add_child("first");
				dat.add_child("second");
				(*(dat.child("first")))["weapon"]=a_stats.weapon->name();
				(*(dat.child("second")))["weapon"]=d_stats.weapon != NULL ? d_stats.weapon->name() : "none";
				game_events::fire("defender_hits",attacker,defender,dat);
				a = units.find(attacker);
				d = units.find(defender);
				
				// FIXME: If the event removes this attack, we should stop attacking.
				// The previous code checked if 'attack_with' and 'defend_with' were still within the bounds of
				// the attack arrays, or -1, but it was incorrect. The attack used could be removed and '*_with'
				// variables could still be in bounds but point to a different attack.
				if(a == units.end() || d == units.end()) {
					if (update_display){
						recalculate_fog(map,state,info,units,teams,attacker_side-1);
						recalculate_fog(map,state,info,units,teams,defender_side-1);
						gui.recalculate_minimap();
						gui.update_display();
					}
					LOG_NG << "firing attack_end event\n";
					game_events::fire("attack_end",attacker,defender,dat);
					break;
				}
			} else {
				const int defender_side = d->second.side();
				const int attacker_side = a->second.side();
				LOG_NG << "firing defender_misses event\n";
				config dat;
				dat.add_child("first");
				dat.add_child("second");
				(*(dat.child("first")))["weapon"]=a_stats.weapon->name();
				(*(dat.child("second")))["weapon"]=d_stats.weapon != NULL ? d_stats.weapon->name() : "none";
				game_events::fire("defender_misses",attacker,defender,dat);
				a = units.find(attacker);
				d = units.find(defender);
				if(a == units.end() || d == units.end()) {
					if (update_display){
						recalculate_fog(map,state,info,units,teams,attacker_side-1);
						recalculate_fog(map,state,info,units,teams,defender_side-1);
						gui.recalculate_minimap();
						gui.update_display();
					}
					LOG_NG << "firing attack_end event\n";
					game_events::fire("attack_end",attacker,defender,dat);
					break;
				}
			}

			attack_stats.defend_result(hits ? (dies ? statistics::attack_context::KILLS : statistics::attack_context::HITS)
			                           : statistics::attack_context::MISSES, defender_damage);

			if(ran_results == NULL) {
				config cfg;
				cfg["hits"] = (hits ? "yes" : "no");
				cfg["dies"] = (dies ? "yes" : "no");
				cfg["damage"] = lexical_cast<std::string>(damage_attacker_takes);
				cfg["chance"] = lexical_cast<std::string>(defender_cth);

				set_random_results(cfg);
			} else {
				const bool results_dies = (*ran_results)["dies"] == "yes";
				if(results_dies != dies) {
					ERR_NW << "SYNC: In defend " << unit_dump(*a) << " vs " << unit_dump(*d)
						<< ": the data source the unit "
						<< (results_dies ? "perished" : "survived")
						<< " while in-game calculations show the unit "
						<< (dies ? "perished" : "survived")
						<< " (over-riding game calculations with data source results)\n";
					dies = results_dies;
					OOS_error = true;
				}
			}

			if(hits || dies){
				int amount_drained = d_stats.drains ? defender_damage / 2 : 0;
			
				if(amount_drained > 0) {
					char buf[50];
					snprintf(buf,sizeof(buf),"%d",amount_drained);
					if (update_display){
						gui.float_label(d->first,buf,0,255,0);
					}
					d->second.heal(amount_drained);
				}
			}

			if(dies) {//defender kills attacker
				defenderxp = game_config::kill_experience*a->second.level();
				if(a->second.level() == 0)
					defenderxp = game_config::kill_experience/2;

				d->second.get_experience(defenderxp);
				gui.invalidate(d->first);
				defenderxp = 0;
				attackerxp = 0;

				std::string undead_variation = a->second.undead_variation();
				gamemap::location loc = a->first;
				gamemap::location defender_loc = d->first;
				const int attacker_side = a->second.side();
				LOG_NG << "firing attack_end event\n";
				game_events::fire("attack_end",attacker,defender,dat);
				LOG_NG << "firing die event\n";
				game_events::fire("die",loc,d->first);
				a = units.find(loc);
				d = units.end();

				if(a != units.end() && a->second.hitpoints() <= 0) {
					units.erase(a);
					a = units.end();
				}

				//plague units make new units on the target hex.
				if(d_stats.plagues) {
				        d = units.find(defender_loc);
				        game_data::unit_type_map::const_iterator reanimitor;
				        LOG_NG<<"trying to reanimate "<<d_stats.plague_type<<std::endl;
				        reanimitor = info.unit_types.find(d_stats.plague_type);
				        LOG_NG<<"found unit type:"<<reanimitor->second.id()<<std::endl;

					if(reanimitor != info.unit_types.end()) {
					       unit newunit=unit(&info,&units,&map,&state,&teams,&reanimitor->second,d->second.side(),true,true);
					       //apply variation
					       if(strcmp(undead_variation.c_str(),"null")){
						 config mod;
						 config& variation=mod.add_child("effect");
						 variation["apply_to"]="variation";
						 variation["name"]=undead_variation;
						 newunit.add_modification("variation",mod);
					       }

					       units.insert(std::pair<gamemap::location,unit>(loc,newunit));
						   if (update_display){
						       gui.invalidate(loc);
						   }
					}else{
					       LOG_NG<<"unit not reanimated"<<std::endl;
					}
				}
				if (update_display){
					gui.recalculate_minimap();
					gui.draw();
					recalculate_fog(map,state,info,units,teams,attacker_side-1);
				}
				break;
			} else if(hits) {
				if (d_stats.poisons &&
				   !utils::string_bool(a->second.get_state("poisoned"))) {
					if (update_display){
						gui.float_label(a->first,_("poisoned"),255,0,0);
					}
					a->second.set_state("poisoned","yes");
				}

				if(d_stats.slows && !utils::string_bool(a->second.get_state("slowed"))) {
					if (update_display){
						gui.float_label(a->first,_("slowed"),255,0,0);
					}
					a->second.set_state("slowed","yes");
					attacker_damage = a_stats.slow_damage;
				}


				//if the attacker is turned to stone, the fight stops immediately
				static const std::string stone_string("stone");
				if (d_stats.stones) {
					if (update_display){
						gui.float_label(a->first,_("stone"),255,0,0);
					}
					a->second.set_state("stoned","yes");
					n_defends = 0;
					n_attacks = 0;
					game_events::fire(stone_string,a->first,d->first);
				}
			}

			--n_defends;
		}

		// continue the fight to death; if one of the units got stoned,
		// either n_attacks or n_defends is -1
		if(rounds > 0 && n_defends == 0 && n_attacks == 0) {
			n_attacks = orig_attacks;
			n_defends = orig_defends;
			--rounds;
			defender_strikes_first = (d_stats.firststrike && ! a_stats.firststrike);
		}
		if(n_attacks <= 0 && n_defends <= 0) {
			LOG_NG << "firing attack_end event\n";
			game_events::fire("attack_end",attacker,defender,dat);
			a = units.find(attacker);
			d = units.find(defender);
		}
	}

	if(attackerxp && a != units.end()) {
		a->second.get_experience(attackerxp);
	}

	if(defenderxp && d != units.end()) {
		d->second.get_experience(defenderxp);
	}

	if (update_display){
		gui.invalidate_unit();
		gui.invalidate(attacker);
		gui.invalidate(defender);
		gui.draw(true,true);
	}
	
	if(OOS_error) {
		if (!game_config::ignore_replay_errors) {
			throw replay::error();
		}
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

bool get_village(const gamemap::location& loc, std::vector<team>& teams,
                 size_t team_num, const unit_map& units)
{
	return get_village(loc,teams,team_num,units,NULL);
}

bool get_village(const gamemap::location& loc, std::vector<team>& teams,
                 size_t team_num, const unit_map& units, int *action_timebonus)
{
	if(team_num < teams.size() && teams[team_num].owns_village(loc)) {
		return false;
	}

	const bool has_leader = find_leader(units,int(team_num+1)) != units.end();
	bool grants_timebonus = false;

	//we strip the village off all other sides, unless it is held by an ally
	//and we don't have a leader (and thus can't occupy it)
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
		return teams[team_num].get_village(loc);
	}

	return false;
}

unit_map::iterator find_leader(unit_map& units, int side)
{
	for(unit_map::iterator i = units.begin(); i != units.end(); ++i) {
		if((int)i->second.side() == side && i->second.can_recruit())
			return i;
	}

	return units.end();
}

unit_map::const_iterator find_leader(const unit_map& units, int side)
{
	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if((int)i->second.side() == side && i->second.can_recruit())
			return i;
	}

	return units.end();
}

// Simple algorithm: no maximum number of patients per healer.
void reset_resting(std::map<gamemap::location,unit>& units, unsigned int side)
{
	for (unit_map::iterator i = units.begin(); i != units.end(); ++i) {
		if (i->second.side() == side)
			i->second.set_resting(true);
	}
}

void calculate_healing(display& disp, const gamestatus& status, const gamemap& map,
                       units_map& units, unsigned int side,
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
		// Only consider healers on side which is starting now
		// remove all healers not on this side
		for(std::vector<std::pair<config*,gamemap::location> >::iterator h_it = heal.cfgs.begin(); h_it != heal.cfgs.end();) {
			unit_map::iterator potential_healer = units.find(h_it->second);
			wassert(potential_healer != units.end());
			if(potential_healer->second.side()!=side) {
				heal.cfgs.erase(h_it);
			} else {
				++h_it;
			}
		}
		
		unit_abilities::effect heal_effect(heal,0,false);
		healing = heal_effect.get_composite_value();
		
		for(std::vector<std::pair<config*,gamemap::location> >::const_iterator heal_it = heal.cfgs.begin(); heal_it != heal.cfgs.end(); ++heal_it) {
			if((*heal_it->first)["poison"] == "cured") {
				curer = units.find(heal_it->second);
				curing = "cured";
			} else if(curing != "cured" && (*heal_it->first)["poison"] == "slowed") {
				curer = units.find(heal_it->second);
				curing = "slowed";
			}
		}
		
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
				// FIXME
				curing = "cured";
				curer = units.end();
			}
			if(i->second.resting()) {
				healing += game_config::rest_heal_amount;
			}
		}
		if(utils::string_bool(i->second.get_state("poisoned"))) {
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
		if(healing > pos_max) {
			healing = pos_max;
		} else if(healing < neg_max) {
			healing = neg_max;
		}
		if(healing == 0) {
			continue;
		}

		if (disp.turbo() || recorder.is_skipping()
			|| disp.fogged(i->first.x, i->first.y)
			|| !update_display 
			|| (i->second.invisible(map.underlying_union_terrain(map[i->first.x][i->first.y]),
							  status.get_time_of_day().lawful_bonus,i->first,units,teams) &&
				teams[disp.viewing_team()].is_enemy(side))) {
			// Simple path.
			if (healing > 0)
				i->second.heal(healing);
			else if (healing < 0)
				i->second.take_hit(-healing);
			continue;
		}

		// This is all the pretty stuff.
		bool start_time_set = false;
		int start_time = 0;
		disp.scroll_to_tile(i->first.x, i->first.y, display::ONSCREEN);
		disp.select_hex(i->first);
		
		for(std::vector<unit_map::iterator>::iterator heal_anim_it = healers.begin(); heal_anim_it != healers.end(); ++heal_anim_it) {
			(*heal_anim_it)->second.set_healing(disp);
			if(start_time_set) {
				start_time = minimum<int>(start_time,(*heal_anim_it)->second.get_animation()->get_first_frame_time());
			} else {
				start_time = (*heal_anim_it)->second.get_animation()->get_first_frame_time();
			}
		}
		if (healing < 0) {
			i->second.set_poisoned(disp, -healing);
			start_time = minimum<int>(start_time, i->second.get_animation()->get_first_frame_time());
			// FIXME
			sound::play_sound("groan.wav");
			disp.float_label(i->first, lexical_cast<std::string>(-healing), 255,0,0);
		} else {
			i->second.set_healed(disp, healing);
			start_time = minimum<int>(start_time, i->second.get_animation()->get_first_frame_time());
			sound::play_sound("heal.wav");
			disp.float_label(i->first, lexical_cast<std::string>(healing), 0,255,0);
		}
		// restart all anims in a synchronized way
		i->second.restart_animation(disp, start_time);
		for(std::vector<unit_map::iterator>::iterator heal_reanim_it = healers.begin(); heal_reanim_it != healers.end(); ++heal_reanim_it) {
			(*heal_reanim_it)->second.restart_animation(disp, start_time);
		}

		bool finished;
		do {
			finished = (i->second.get_animation()->animation_finished());
			disp.invalidate(i->first);
			for(std::vector<unit_map::iterator>::iterator heal_fanim_it = healers.begin(); heal_fanim_it != healers.end(); ++heal_fanim_it) {
				finished &= (*heal_fanim_it)->second.get_animation()->animation_finished();
				disp.invalidate((*heal_fanim_it)->first);
			}
			if (healing > 0) {
				i->second.heal(1);
				--healing;
			} else if (healing < 0) {
				i->second.take_hit(1);
				++healing;
			}
			finished &= (!healing);
			disp.draw();
			events::pump();
			SDL_Delay(10);
		} while (!finished);

		i->second.set_standing(disp);
		for(std::vector<unit_map::iterator>::iterator heal_sanim_it = healers.begin(); heal_sanim_it != healers.end(); ++heal_sanim_it) {
			(*heal_sanim_it)->second.set_standing(disp);
		}
		disp.update_display();
		events::pump();
	}
}


unit get_advanced_unit(const game_data& info,
                  units_map& units,
                  const gamemap::location& loc, const std::string& advance_to)
{
	const std::map<std::string,unit_type>::const_iterator new_type = info.unit_types.find(advance_to);
	const units_map::iterator un = units.find(loc);
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
                  units_map& units,
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

	units.erase(loc);
	units.insert(std::pair<gamemap::location,unit>(loc,new_unit));
	LOG_NG << "firing post_advance event\n";
	game_events::fire("post_advance",loc);
}

void check_victory(units_map& units,
                   std::vector<team>& teams)
{
	std::vector<int> seen_leaders;
	for(units_map::const_iterator i = units.begin();
	    i != units.end(); ++i) {
		if(i->second.can_recruit()) {
			LOG_NG << "seen leader for side " << i->second.side() << "\n";
			seen_leaders.push_back(i->second.side());
		}
	}

	//clear villages for teams that have no leader
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
			if (victory_conditions::victory_when_enemies_defeated() == false) {
				// this level has asked not to be ended by this condition
				return;
			}
		}


		if(non_interactive()) {
			std::cout << "winner: ";
			for(std::vector<int>::const_iterator i = seen_leaders.begin(); i != seen_leaders.end(); ++i) {
				std::cout << *i << " ";
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
			const units_map& units,
			const gamemap::location& loc,
			 unit_type::ALIGNMENT alignment,
			const gamemap& map)
{
	const time_of_day& tod = timeofday_at(status,units,loc,map);

	int bonus = tod.lawful_bonus;

	if(alignment == unit_type::NEUTRAL)
		bonus = 0;
	else if(alignment == unit_type::CHAOTIC)
		bonus = -bonus;

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

		//we clear one past the edge of the board, so that the half-hexes
		//at the edge can also be cleared of fog/shroud
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

//returns true iff some shroud is cleared
//seen_units will return new units that have been seen by this unit
//if known_units is NULL, seen_units can be NULL and will not be changed
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

	unit_map temp_units;
	temp_units.insert(*u);

	paths p(map,status,gamedata,temp_units,loc,teams,true,false,teams[team]);
	for(paths::routes_map::const_iterator i = p.routes.begin();
	    i != p.routes.end(); ++i) {
		clear_shroud_loc(map,teams[team],i->first,&cleared_locations);
	}

	//clear the location the unit is at
	clear_shroud_loc(map,teams[team],loc,&cleared_locations);

	for(std::vector<gamemap::location>::const_iterator it =
	    cleared_locations.begin(); it != cleared_locations.end(); ++it) {

		const unit_map::const_iterator sighted = units.find(*it);
		if(sighted != units.end() &&
		  (sighted->second.invisible(map.underlying_union_terrain(map[it->x][it->y]),status.get_time_of_day().lawful_bonus,*it,units,teams) == false
		  || teams[team].is_enemy(sighted->second.side()) == false)) {
			if(seen_units == NULL || known_units == NULL) {
				static const std::string sighted("sighted");
				game_events::raise(sighted,*it,loc);
			} else if(known_units->count(*it) == 0) {
				seen_units->insert(*it);
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
		if((int)i->second.side() == team+1) {
			const unit_movement_resetter move_resetter(i->second);

			clear_shroud_unit(map,status,gamedata,units,i->first,teams,team,NULL,NULL);
		}
	}
	game_events::pump();
}

bool clear_shroud(display& disp, const gamestatus& status,
		const gamemap& map, const game_data& gamedata,
                  unit_map& units, std::vector<team>& teams, int team)
{
	if(teams[team].uses_shroud() == false && teams[team].uses_fog() == false)
		return false;

	bool result = false;

	unit_map::iterator i;
	for(i = units.begin(); i != units.end(); ++i) {
		if((int)i->second.side() == team+1) {
			const unit_movement_resetter move_resetter(i->second);

			result |= clear_shroud_unit(map,status,gamedata,units,i->first,teams,team,NULL,NULL);
		}
	}
	game_events::pump();

	recalculate_fog(map,status,gamedata,units,teams,team);

	disp.labels().recalculate_shroud();

	return result;
}

size_t move_unit(display* disp, const game_data& gamedata,
                 const gamestatus& status, const gamemap& map,
                 unit_map& units, std::vector<team>& teams,
                 std::vector<gamemap::location> route,
                 replay* move_recorder, undo_list* undo_stack,
                 gamemap::location *next_unit, bool continue_move, bool should_clear_shroud)
{
	wassert(route.empty() == false);

	//stop the user from issuing any commands while the unit is moving
	const events::command_disabler disable_commands;

	unit_map::iterator ui = units.find(route.front());

	wassert(ui != units.end());

	ui->second.set_goto(gamemap::location());

	unit u = ui->second;

	const size_t team_num = u.side()-1;

	const bool skirmisher = u.get_ability_bool("skirmisher",ui->first);

	team& team = teams[team_num];
	const bool check_shroud = should_clear_shroud && team.auto_shroud_updates() &&
		(team.uses_shroud() || team.uses_fog());

	//if we use shroud/fog of war, count out the units we can currently see
	std::set<gamemap::location> known_units;
	if(check_shroud) {
		for(unit_map::const_iterator u = units.begin(); u != units.end(); ++u) {
			if(team.fogged(u->first.x,u->first.y) == false) {
				known_units.insert(u->first);
				team.see(u->second.side()-1);
			}
		}
	}

	//see how far along the given path we can move
	const int starting_moves = u.movement_left();
	int moves_left = starting_moves;
	std::set<gamemap::location> seen_units;
	bool discovered_unit = false;
	bool should_clear_stack = false;
	std::vector<gamemap::location>::const_iterator step;
	for(step = route.begin()+1; step != route.end(); ++step) {
		const gamemap::TERRAIN terrain = map[step->x][step->y];

		const unit_map::const_iterator enemy_unit = units.find(*step);

		const int mv = u.movement_cost(terrain);
		if(discovered_unit || continue_move == false && seen_units.empty() == false ||
		   mv > moves_left || enemy_unit != units.end() && team.is_enemy(enemy_unit->second.side())) {
			break;
		} else {
			moves_left -= mv;
		}

		if(!skirmisher && enemy_zoc(map,status,units,teams,*step,team,u.side())) {
			moves_left = 0;
		}

		//if we use fog or shroud, see if we have sighted an enemy unit, in
		//which case we should stop immediately.
		if(check_shroud) {
			if(units.count(*step) == 0 && !map.is_village(*step)) {
				LOG_NG << "checking for units from " << (step->x+1) << "," << (step->y+1) << "\n";

				//temporarily reset the unit's moves to full
				const unit_movement_resetter move_resetter(ui->second);

				//we have to swap out any unit that is already in the hex, so we can put our
				//unit there, then we'll swap back at the end.
				const temporary_unit_placer unit_placer(units,*step,ui->second);

				should_clear_stack |= clear_shroud_unit(map,status,gamedata,units,*step,teams,
				                                        ui->second.side()-1,&known_units,&seen_units);

				if(should_clear_stack) {
					disp->invalidate_all();
				}
			}
		}

		//check if we have discovered an invisible enemy unit
		gamemap::location adjacent[6];
		get_adjacent_tiles(*step,adjacent);

		for(int i = 0; i != 6; ++i) {
			//check if we are checking ourselves
			if(adjacent[i] == ui->first)
				continue;

			const units_map::const_iterator it = units.find(adjacent[i]);
			if(it != units.end() && teams[u.side()-1].is_enemy(it->second.side()) &&
			   it->second.invisible(map.underlying_union_terrain(map[it->first.x][it->first.y]),
			   status.get_time_of_day().lawful_bonus,it->first,units,teams)) {
				discovered_unit = true;
				should_clear_stack = true;
				moves_left = 0;
				break;
			}
		}
	}

	//make sure we don't tread on another unit
	std::vector<gamemap::location>::const_iterator begin = route.begin();

	std::vector<gamemap::location> steps(begin,step);
	while (!steps.empty()) {
		gamemap::location const &loc = steps.back();
		if (units.count(loc) == 0)
			break;
		moves_left += u.movement_cost(map[loc.x][loc.y]);
		steps.pop_back();
	}

	wassert(steps.size() <= route.size());

	if (next_unit != NULL)
		*next_unit = steps.back();

	//if we can't get all the way there and have to set a go-to.
	if(steps.size() != route.size() && discovered_unit == false) {
		if(seen_units.empty() == false) {
			u.set_interrupted_move(route.back());
		} else {
			u.set_goto(route.back());
		}
	} else {
		u.set_interrupted_move(gamemap::location());
	}

	if(steps.size() < 2) {
		return 0;
	}

	//move the unit on the screen. Hide the unit in its current location, but don't actually
	//remove it until the move is done, so that while the unit is moving status etc will
	//still display the correct number of units.
	if(disp != NULL) {
		ui->second.set_hidden(true);
		disp->invalidate(ui->first);
		unit_display::move_unit(*disp,map,steps,u,status.get_time_of_day(),units,teams);
		ui->second.set_hidden(false);
	}

	u.set_movement(moves_left);


	units.erase(ui);
	ui = units.insert(std::pair<gamemap::location,unit>(steps.back(),u)).first;
	if(disp != NULL) {
		disp->invalidate_unit();
		disp->invalidate(steps.back());
		disp->draw();
	}

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
			event_mutated = get_village(steps.back(),teams,team_num,units,&action_time_bonus);
		}
	}

	if(game_events::fire("moveto",steps.back())) {
		event_mutated = true;
	}

	if(undo_stack != NULL) {
		if(event_mutated || should_clear_stack) {
			apply_shroud_changes(*undo_stack,disp,status,map,gamedata,units,teams,team_num);
			undo_stack->clear();
		} else {
			//MP_COUNTDOWN: added param
			undo_stack->push_back(undo_action(u,steps,starting_moves,action_time_bonus,orig_village_owner));
		}
	}

	if(disp != NULL) {
		disp->set_route(NULL);

		//show messages on the screen here
		if(discovered_unit) {
			//we've been ambushed, so display an appropriate message
			font::add_floating_label(_("Ambushed!"),font::SIZE_XLARGE,font::BAD_COLOUR,
			                         disp->map_area().w/2,disp->map_area().h/3,
									 0.0,0.0,100,disp->map_area(),font::CENTER_ALIGN);
		}

		if(continue_move == false && seen_units.empty() == false) {
			//the message depends on how many units have been sighted, and whether
			//they are allies or enemies, so calculate that out here
			int nfriends = 0, nenemies = 0;
			for(std::set<gamemap::location>::const_iterator i = seen_units.begin(); i != seen_units.end(); ++i) {
				LOG_NG << "processing unit at " << (i->x+1) << "," << (i->y+1) << "\n";
				const unit_map::const_iterator u = units.find(*i);

				//unit may have been removed by an event.
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

			char* msg_id;

			//the message we display is different depending on whether units sighted
			//were enemies or friends, and whether there is one or more
			if(seen_units.size() == 1) {
				if(nfriends == 1) {
					msg_id = N_("Friendly unit sighted");
				} else {
					msg_id = N_("Enemy unit sighted!");
				}
			}
			else if(nfriends == 0 || nenemies == 0) {
				if(nfriends > 0) {
					msg_id = N_("$friends Friendly units sighted");
				} else {
					msg_id = N_("$enemies Enemy units sighted!");
				}
			}
			else {
				msg_id = N_("Units sighted! ($friends friendly, $enemies enemy)");
			}

			utils::string_map symbols;
			symbols["friends"] = lexical_cast<std::string>(nfriends);
			symbols["enemies"] = lexical_cast<std::string>(nenemies);

			std::stringstream msg;
			msg << gettext(msg_id);

			if(steps.size() < route.size()) {
				//see if the "Continue Move" action has an associated hotkey
				const hotkey::hotkey_item& hk = hotkey::get_hotkey(hotkey::HOTKEY_CONTINUE_MOVE);
				if(!hk.null()) {
					symbols["hotkey"] = hk.get_name();
					msg << '\n' << _("(press $hotkey to continue)");
				}
			}
			const std::string message = utils::interpolate_variables_into_string(msg.str(), &symbols);

			font::add_floating_label(message,font::SIZE_XLARGE,font::BAD_COLOUR,
			                         disp->map_area().w/2,disp->map_area().h/3,
									 0.0,0.0,100,disp->map_area(),font::CENTER_ALIGN);
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

	//units with goto commands that have already done their gotos this turn
	//(i.e. don't have full movement left) should be red
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

			if(u.movement_cost(map[locs[n].x][locs[n].y]) <= u.movement_left()) {
				return true;
			}
		}
	}

	return false;
}

void apply_shroud_changes(undo_list& undos, display* disp, const gamestatus& status, const gamemap& map,
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
		if(un->is_recall()) continue;
		//we're not really going to mutate the unit, just temporarily
		//set its moves to maximum, but then switch them back
		const unit_movement_resetter move_resetter(un->affected_unit);

		std::vector<gamemap::location>::const_iterator step;
		for(step = un->route.begin(); step != un->route.end(); ++step) {
			//we have to swap out any unit that is already in the hex, so we can put our
			//unit there, then we'll swap back at the end.
			const temporary_unit_placer unit_placer(units,*step,un->affected_unit);
			clear_shroud_unit(map,status,gamedata,units,*step,teams,team,NULL,NULL);

			//FIXME
			//there is potential for a bug, here. If the "sighted"
			//events, raised by the clear_shroud_unit function,
			//loops on all units, changing them all, the unit which
			//was swapped by the temporary unit placer will not be
			//affected. However, if we place the pump() out of the
			//temporary_unit_placer scope, the "sighted" event will
			//be raised with an invalid source unit, which is even
			//worse.
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
	} else {
		recalculate_fog(map,status,gamedata,units,teams,team);
	}
}

bool backstab_check(const gamemap::location& attacker_loc,
	const gamemap::location& defender_loc,
	const units_map& units, const std::vector<team>& teams)
{
	const units_map::const_iterator defender =
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

	const units_map::const_iterator opp =
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
