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

#include "actions.hpp"
#include "display.hpp"
#include "game.hpp"
#include "game_config.hpp"
#include "game_events.hpp"
#include "key.hpp"
#include "language.hpp"
#include "map.hpp"
#include "pathfind.hpp"
#include "playlevel.hpp"
#include "playturn.hpp"
#include "replay.hpp"
#include "sound.hpp"
#include "statistics.hpp"
#include "util.hpp"

#include <cmath>
#include <set>
#include <string>
#include <sstream>

struct castle_cost_calculator
{
	castle_cost_calculator(const gamemap& map) : map_(map)
	{}

	double cost(const gamemap::location& loc, double cost_so_far) const
	{
		if(!map_.on_board(loc) || map_[loc.x][loc.y] != gamemap::CASTLE)
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

std::string recruit_unit(const gamemap& map, int side,
       std::map<gamemap::location,unit>& units, unit& new_unit,
       gamemap::location recruit_location, display* disp, bool need_castle, bool full_movement)
{
	std::cerr << "recruiting unit for side " << side << "\n";
	typedef std::map<gamemap::location,unit> units_map;

	//find the unit that can recruit
	units_map::const_iterator u;

	for(u = units.begin(); u != units.end(); ++u) {
		if(u->second.can_recruit() && u->second.side() == side) {
			break;
		}
	}

	if(u == units.end())
		return string_table["no_leader_to_recruit"];

	if(map.get_terrain(u->first) != gamemap::KEEP) {
		std::cerr << "Leader not on start: leader is on " << (u->first.x+1) << "," << (u->first.y+1) << "\n";
		return string_table["leader_not_on_start"];
	}

	if(map.on_board(recruit_location)) {
		const paths::route& rt = a_star_search(u->first,recruit_location,
		                                   100.0,castle_cost_calculator(map));
		if(rt.steps.empty() || units.find(recruit_location) != units.end() ||
		   map[recruit_location.x][recruit_location.y] != gamemap::CASTLE)
			recruit_location = gamemap::location();
	}

	if(!map.on_board(recruit_location)) {
		recruit_location = find_vacant_tile(map,units,u->first,
		                                    need_castle ? gamemap::CASTLE : 0);
	}

	if(!map.on_board(recruit_location)) {
		return string_table["no_recruit_location"];
	}

	if(full_movement) {
		new_unit.set_movement(new_unit.total_movement());
	} else {
		new_unit.set_movement(0);
		new_unit.set_attacked();
	}

	units.insert(std::pair<gamemap::location,unit>(
							recruit_location,new_unit));

	if(disp != NULL && !disp->turbo() &&
	   !disp->fogged(recruit_location.x,recruit_location.y)) {
		disp->draw(true,true);

		for(double alpha = 0.0; alpha <= 1.0; alpha += 0.1) {
			disp->draw_tile(recruit_location.x,recruit_location.y,NULL,alpha);
			disp->update_display();
			SDL_Delay(20);
		}
	}

	return std::string();
}

bool under_leadership(const std::map<gamemap::location,unit>& units,
                      const gamemap::location& loc)
{
	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	const std::map<gamemap::location,unit>::const_iterator un =
	     units.find(loc);
	if(un == units.end())
		return false;

	const int side = un->second.side();
	const int level = un->second.type().level();

	for(int i = 0; i != 6; ++i) {
		const std::map<gamemap::location,unit>::const_iterator it =
		     units.find(adjacent[i]);
		if(it != units.end() && it->second.side() == side &&
		   it->second.type().is_leader() && it->second.type().level() > level)
			return true;
	}

	return false;
}

battle_stats evaluate_battle_stats(
                   const gamemap& map,
                   const gamemap::location& attacker,
                   const gamemap::location& defender,
				   int attack_with,
				   std::map<gamemap::location,unit>& units,
				   const gamestatus& state,
				   const game_data& info,
				   gamemap::TERRAIN attacker_terrain_override,
				   bool include_strings)
{
	//if these are both genuine positions, work out the range
	//combat is taking place at
	const int combat_range = attacker_terrain_override == 0 ? distance_between(attacker,defender) : 1;

	battle_stats res;

	res.attack_with = attack_with;

	if(include_strings)
		res.defend_name = string_table["weapon_none"];

	const std::map<gamemap::location,unit>::iterator a = units.find(attacker);
	const std::map<gamemap::location,unit>::iterator d = units.find(defender);

	assert(a != units.end());
	assert(d != units.end());

	const gamemap::TERRAIN attacker_terrain = attacker_terrain_override ?
	                 attacker_terrain_override : map[attacker.x][attacker.y];
	const gamemap::TERRAIN defender_terrain = map[defender.x][defender.y];

	res.chance_to_hit_attacker = a->second.defense_modifier(map,attacker_terrain);
	res.chance_to_hit_defender = d->second.defense_modifier(map,defender_terrain);

	const std::vector<attack_type>& attacker_attacks = a->second.attacks();
	const std::vector<attack_type>& defender_attacks = d->second.attacks();

	assert(attack_with >= 0 && attack_with < int(attacker_attacks.size()));
	const attack_type& attack = attacker_attacks[attack_with];

	static const std::string charge_string("charge");
	const bool charge = (attack.special() == charge_string);

	bool backstab = false;

	static const std::string backstab_string("backstab");
	if(attack.special() == backstab_string) {
		gamemap::location adj[6];
		get_adjacent_tiles(defender,adj);
		int i;
		for(i = 0; i != 6; ++i) {
			if(adj[i] == attacker)
				break;
		}

		if(i != 6) {
			const std::map<gamemap::location,unit>::const_iterator u =
			                    units.find(adj[(i+3)%6]);
			if(u != units.end() && u->second.side() == a->second.side()) {
				backstab = true;
			}
		}
	}

	static const std::string plague_string("plague");
	res.attacker_plague = !d->second.type().not_living() && (attack.special() == plague_string);
	res.defender_plague = false;

	res.attack_name    = attack.name();
	res.attack_type    = attack.type();

	if(include_strings) {
		res.attack_special = attack.special();
		res.attack_icon = attack.icon();

		//don't show backstabbing unless it's actually happening
		if(res.attack_special == "backstab" && !backstab)
			res.attack_special = "";

		res.range = (attack.range() == attack_type::SHORT_RANGE ?
		             "Melee" : "Ranged");
	}

	res.nattacks = attack.num_attacks();
	double best_defend_rating = 0.0;
	int defend = -1;
	res.ndefends = 0;
	for(int defend_option = 0; defend_option != int(defender_attacks.size()); ++defend_option) {
		if(defender_attacks[defend_option].range() == attack.range() &&
			defender_attacks[defend_option].hexes() >= combat_range) {
			const double rating = a->second.damage_against(defender_attacks[defend_option])*defender_attacks[defend_option].num_attacks();
			if(defend == -1 || rating > best_defend_rating) {
				best_defend_rating = rating;
				defend = defend_option;
			}
		}
	}

	res.defend_with = defend;

	const bool counterattack = defend != -1;

	static const std::string drain_string("drain");
	static const std::string magical_string("magical");

	res.damage_attacker_takes = 0;
	if(counterattack) {
		//magical attacks always have a 70% chance to hit
		if(defender_attacks[defend].special() == magical_string)
			res.chance_to_hit_attacker = 70;

		const int base_damage = a->second.damage_against(defender_attacks[defend]);
		const int modifier = combat_modifier(state,units,d->first,d->second.type().alignment());
		res.damage_attacker_takes = (base_damage * (100+modifier))/100;

		if(include_strings) {
			std::stringstream str_base;
			str_base << string_table["base_damage"] << ", ," << defender_attacks[defend].damage();
			res.defend_calculations.push_back(str_base.str());

			std::stringstream str_resist;

			const int resist = a->second.type().movement_type().resistance_against(defender_attacks[defend]) - 100;
			str_resist << string_table["attacker_resistance"] << " " << translate_string(defender_attacks[defend].type())
			           << ",^" << (resist > 0 ? "+" : "") << resist << "%," << base_damage;
			res.defend_calculations.push_back(str_resist.str());

			std::stringstream str_mod;
			const time_of_day& tod = timeofday_at(state,units,d->first);
			str_mod << translate_string_default(tod.id,tod.name) << ",^"
			        << (modifier > 0 ? "+" : "") << modifier << "%," << res.damage_attacker_takes;
			res.defend_calculations.push_back(str_mod.str());
		}

		if(charge) {
			res.damage_attacker_takes *= 2;
			if(include_strings) {
				std::stringstream str;
				str << translate_string("charge") << ",^+100%," << res.damage_attacker_takes;
				res.defend_calculations.push_back(str.str());
			}
		}

		if(under_leadership(units,defender)) {
			res.damage_attacker_takes += res.damage_attacker_takes/8 + 1;
			if(include_strings) {
				std::stringstream str;
				str << translate_string("leadership") << ",^+12.5%," << res.damage_attacker_takes;
				res.defend_calculations.push_back(str.str());
			}
		}

		if(res.damage_attacker_takes < 1) {
			res.damage_attacker_takes = 1;
			if(include_strings) {
				std::stringstream str;
				str << translate_string("minimum_damage") << ", ,1";
				res.defend_calculations.push_back(str.str());
			}
		}

		res.ndefends = defender_attacks[defend].num_attacks();

		res.defend_name    = defender_attacks[defend].name();
		res.defend_type    = defender_attacks[defend].type();

		if(include_strings) {
			res.defend_special = defender_attacks[defend].special();
			res.defend_icon = defender_attacks[defend].icon();
		}

		//if the defender drains, and the attacker is a living creature, then
		//the defender will drain for half the damage it does
		if(defender_attacks[defend].special() == drain_string && !a->second.type().not_living()) {
			res.amount_defender_drains = res.damage_attacker_takes/2;
		} else {
			res.amount_defender_drains = 0;
		}

		res.defender_plague = (defender_attacks[defend].special() == plague_string);
	}

	if(attack.special() == magical_string)
		res.chance_to_hit_defender = 70;

	static const std::string marksman_string("marksman");

	//offensive marksman attacks always have at least 60% chance to hit
	if(res.chance_to_hit_defender < 60 && attack.special() == marksman_string)
		res.chance_to_hit_defender = 60;

	const int base_damage = d->second.damage_against(attack);
	const int modifier = combat_modifier(state,units,a->first,a->second.type().alignment());
	res.damage_defender_takes = ((base_damage * (100 + modifier))/100);

	if(include_strings) {
		std::stringstream str_base;

		str_base << string_table["base_damage"] << ", ," << attack.damage();
		res.attack_calculations.push_back(str_base.str());

		std::stringstream str_resist;

		const int resist = d->second.type().movement_type().resistance_against(attack) - 100;
		str_resist << string_table["defender_resistance"] << " " << translate_string(attack.type())
			       << ",^" << (resist > 0 ? "+" : "") << resist << "%," << base_damage;
		res.attack_calculations.push_back(str_resist.str());

		std::stringstream str_mod;
		const time_of_day& tod = timeofday_at(state,units,a->first);
		str_mod << translate_string_default(tod.id,tod.name) << ",^"
		        << (modifier > 0 ? "+" : "") << modifier << "%," << res.damage_defender_takes;
		res.attack_calculations.push_back(str_mod.str());
	}

	if(charge) {
		res.damage_defender_takes *= 2;
		if(include_strings) {
			std::stringstream str;
			str << translate_string("charge") << ",^+100%," << res.damage_defender_takes;
			res.attack_calculations.push_back(str.str());
		}
	}

	if(backstab) {
		res.damage_defender_takes *= 2;
		if(include_strings) {
			std::stringstream str;
			str << translate_string("backstab") << ",^+100%," << res.damage_defender_takes;
			res.attack_calculations.push_back(str.str());
		}
	}

	if(under_leadership(units,attacker)) {
		res.damage_defender_takes += res.damage_defender_takes/8 + 1;
		if(include_strings) {
			std::stringstream str;
			str << translate_string("leadership") << ",^+12.5%," << res.damage_defender_takes;
			res.attack_calculations.push_back(str.str());
		}
	}

	if(res.damage_defender_takes < 1) {
		res.damage_defender_takes = 1;
		if(include_strings) {
			std::stringstream str;
			str << translate_string("minimum_damage") << ", ,1";
			res.attack_calculations.push_back(str.str());
		}
	}

	//if the attacker drains, and the defender is a living creature, then
	//the attacker will drain for half the damage it does
	if(attack.special() == drain_string && !d->second.type().not_living()) {
		res.amount_attacker_drains = res.damage_defender_takes/2;
	} else {
		res.amount_attacker_drains = 0;
	}

	static const std::string slowed_string("slowed");
	if(a->second.has_flag(slowed_string) && res.nattacks > 1)
		--res.nattacks;

	if(d->second.has_flag(slowed_string) && res.ndefends > 1)
		--res.ndefends;

	return res;
}

void attack(display& gui, const gamemap& map, 
				std::vector<team>& teams,
            const gamemap::location& attacker,
            const gamemap::location& defender,
			int attack_with,
			std::map<gamemap::location,unit>& units,
			const gamestatus& state,
			const game_data& info, bool player_is_attacker)
{
	//stop the user from issuing any commands while the units are fighting
	const command_disabler disable_commands;

	std::map<gamemap::location,unit>::iterator a = units.find(attacker);
	std::map<gamemap::location,unit>::iterator d = units.find(defender);

	assert(a != units.end());
	assert(d != units.end());

	int attackerxp = d->second.type().level();
	int defenderxp = a->second.type().level();

	a->second.set_attacked();
	d->second.set_resting(false);

	//if the attacker was invisible, she isn't anymore!
	static const std::string forest_invisible("ambush");
	a->second.remove_flag(forest_invisible);
	static const std::string night_invisible("nightstalk");
	a->second.remove_flag(night_invisible);

	battle_stats stats = evaluate_battle_stats(map,attacker,defender,
	                                           attack_with,units,state,info);

	statistics::attack_context attack_stats(a->second,d->second,stats);

	static const std::string poison_string("poison");

	while(stats.nattacks > 0 || stats.ndefends > 0) {
		if(stats.nattacks > 0) {
			const int ran_num = get_random();
			bool hits = (ran_num%100) < stats.chance_to_hit_defender;

			//make sure that if we're serializing a game here,
			//we got the same results as the game did originally
			const config* ran_results = get_random_results();
			if(ran_results != NULL) {
				const int results_chance = atoi((*ran_results)["chance"].c_str());
				const bool results_hits = (*ran_results)["hits"] == "yes";
				const int results_damage = atoi((*ran_results)["damage"].c_str());

				if(results_chance != stats.chance_to_hit_defender) {
					std::cerr << "SYNC ERROR: In attack " << a->second.type().name() << " vs "
					          << d->second.type().name() << ": chance to hit defender is inconsistent. Data source: "
							  << results_chance << "; Calculation: " << stats.chance_to_hit_defender
							  << " (over-riding game calculations with data source results)\n";
					hits = results_hits;
				} else if(hits != results_hits) {
					std::cerr << "SYNC ERROR: In attack " << a->second.type().name() << " vs "
					          << d->second.type().name() << ": the data source says the hit was "
							  << (results_hits ? "successful" : "unsuccessful") << ", while in-game calculations say the hit was "
							  << (hits ? "successful" : "unsuccessful")
							  << " random number: " << ran_num << " = " << (ran_num%100) << "/" << results_chance
							  << " (over-riding game calculations with data source results)\n";
					hits = results_hits;
				} else if(results_damage != stats.damage_defender_takes) {
					std::cerr << "SYNC ERROR: In attack " << a->second.type().name() << " vs "
					          << d->second.type().name() << ": the data source says the hit did "
							  << results_damage << " damage, while in-game calculations show the hit doing "
							  << stats.damage_defender_takes << " damage (over-riding game calculations with data source results)\n";
					stats.damage_defender_takes = results_damage;
				}
			}

			bool dies = gui.unit_attack(attacker,defender,
				            hits ? stats.damage_defender_takes : 0,
							a->second.attacks()[attack_with]);

			attack_stats.attack_result(hits ? (dies ? statistics::attack_context::KILLS : statistics::attack_context::HITS)
			                           : statistics::attack_context::MISSES);

			if(ran_results == NULL) {
				config cfg;
				cfg["hits"] = (hits ? "yes" : "no");
				cfg["dies"] = (dies ? "yes" : "no");
				char buf[50];
				sprintf(buf,"%d",stats.damage_defender_takes);
				cfg["damage"] = buf;
				sprintf(buf,"%d",stats.chance_to_hit_defender);
				cfg["chance"] = buf;
				set_random_results(cfg);
			} else {
				const bool results_dies = (*ran_results)["dies"] == "yes";
				if(results_dies != dies) {
					std::cerr << "SYNC ERROR: In attack" << a->second.type().name() << " vs "
					          << d->second.type().name() << ": the data source the unit "
							  << (results_dies ? "perished" : "survived") << " while in-game calculations show the unit "
							  << (dies ? "perished" : "survived") << " (over-riding game calculations with data source results)\n";
					dies = results_dies;
				}
			}

			if(dies) {
				attackerxp = game_config::kill_experience*d->second.type().level();
				if(d->second.type().level() == 0)
					attackerxp = game_config::kill_experience/2;

				a->second.get_experience(attackerxp);
				attackerxp = 0;
				defenderxp = 0;

				gamemap::location loc = d->first;
				gamemap::location attacker_loc = a->first;
				const int defender_side = d->second.side();
				std::cerr << "firing die event\n";
				game_events::fire("die",loc,a->first);
				d = units.end();
				a = units.end();

				//the handling of the event may have removed the object
				//so we have to find it again
				units.erase(loc);

				//plague units make clones of themselves on the target hex
				//units on villages that die cannot be plagued
				if(stats.attacker_plague && map.underlying_terrain(map[loc.x][loc.y]) != gamemap::TOWER) {
					a = units.find(attacker_loc);
					if(a != units.end()) {
						units.insert(std::pair<gamemap::location,unit>(loc,a->second));
						gui.draw_tile(loc.x,loc.y);
					}
				}
				recalculate_fog(map,state,info,units,teams,defender_side-1);
				gui.recalculate_minimap();
				gui.update_display();
				break;
			} else if(hits) {
				if(stats.attack_special == poison_string &&
				   d->second.has_flag("poisoned") == false &&
				   !d->second.type().not_living()) {
					d->second.set_flag("poisoned");
				}

				static const std::string slow_string("slow");
				if(stats.attack_special == slow_string &&
				   d->second.has_flag("slowed") == false) {
					d->second.set_flag("slowed");
					if(stats.ndefends > 1)
						--stats.ndefends;
				}

				if(stats.amount_attacker_drains > 0) {
					a->second.gets_hit(-stats.amount_attacker_drains);
				}

				//if the defender is turned to stone, the fight stops immediately
				static const std::string stone_string("stone");
				if(stats.attack_special == stone_string) {
					d->second.set_flag("stone");
					stats.ndefends = 0;
					stats.nattacks = 0;
					game_events::fire("stone",d->first,a->first);
				}
			}

			--stats.nattacks;
		}

		if(stats.ndefends > 0) {
			const int ran_num = get_random();
			bool hits = (ran_num%100) < stats.chance_to_hit_attacker;

			//make sure that if we're serializing a game here,
			//we got the same results as the game did originally
			const config* ran_results = get_random_results();
			if(ran_results != NULL) {
				const int results_chance = atoi((*ran_results)["chance"].c_str());
				const bool results_hits = (*ran_results)["hits"] == "yes";
				const int results_damage = atoi((*ran_results)["damage"].c_str());

				if(results_chance != stats.chance_to_hit_attacker) {
					std::cerr << "SYNC ERROR: In defend " << a->second.type().name() << " vs "
					          << d->second.type().name() << ": chance to hit attacker is inconsistent. Data source: "
							  << results_chance << "; Calculation: " << stats.chance_to_hit_attacker
							  << " (over-riding game calculations with data source results)\n";
					hits = results_hits;
				} else if(hits != results_hits) {
					std::cerr << "SYNC ERROR: In defend " << a->second.type().name() << " vs "
					          << d->second.type().name() << ": the data source says the hit was "
							  << (results_hits ? "successful" : "unsuccessful") << ", while in-game calculations say the hit was "
							  << (hits ? "successful" : "unsuccessful")
							  << " random number: " << ran_num << " = " << (ran_num%100) << "/" << results_chance
							  << " (over-riding game calculations with data source results)\n";
					hits = results_hits;
				} else if(results_damage != stats.damage_attacker_takes) {
					std::cerr << "SYNC ERROR: In defend " << a->second.type().name() << " vs "
					          << d->second.type().name() << ": the data source says the hit did "
							  << results_damage << " damage, while in-game calculations show the hit doing "
							  << stats.damage_attacker_takes << " damage (over-riding game calculations with data source results)\n";
					stats.damage_attacker_takes = results_damage;
				}
			}

			bool dies = gui.unit_attack(defender,attacker,
			               hits ? stats.damage_attacker_takes : 0,
						   d->second.attacks()[stats.defend_with]);

			attack_stats.defend_result(hits ? (dies ? statistics::attack_context::KILLS : statistics::attack_context::HITS)
			                           : statistics::attack_context::MISSES);

			if(ran_results == NULL) {
				config cfg;
				cfg["hits"] = (hits ? "yes" : "no");
				cfg["dies"] = (dies ? "yes" : "no");
				char buf[50];
				sprintf(buf,"%d",stats.damage_attacker_takes);
				cfg["damage"] = buf;
				sprintf(buf,"%d",stats.chance_to_hit_attacker);
				cfg["chance"] = buf;
				set_random_results(cfg);
			} else {
				const bool results_dies = (*ran_results)["dies"] == "yes";
				if(results_dies != dies) {
					std::cerr << "SYNC ERROR: In defend" << a->second.type().name() << " vs "
					          << d->second.type().name() << ": the data source the unit "
							  << (results_dies ? "perished" : "survived") << " while in-game calculations show the unit "
							  << (dies ? "perished" : "survived") << " (over-riding game calculations with data source results)\n";
					dies = results_dies;
				}
			}

			if(dies) {
				defenderxp = game_config::kill_experience*a->second.type().level();
				if(a->second.type().level() == 0)
					defenderxp = game_config::kill_experience/2;

				d->second.get_experience(defenderxp);
				defenderxp = 0;
				attackerxp = 0;

				gamemap::location loc = a->first;
				gamemap::location defender_loc = d->first;
				const int attacker_side = a->second.side();
				game_events::fire("die",loc,d->first);
				a = units.end();
				d = units.end();

				//the handling of the event may have removed the object
				//so we have to find it again
				units.erase(loc);

				//plague units make clones of themselves on the target hex.
				//units on villages that die cannot be plagued
				if(stats.defender_plague && map.underlying_terrain(map[loc.x][loc.y]) != gamemap::TOWER) {
					d = units.find(defender_loc);
					if(d != units.end()) {
						units.insert(std::pair<gamemap::location,unit>(
						                                 loc,d->second));
						gui.draw_tile(loc.x,loc.y);
					}
				}
				gui.recalculate_minimap();
				gui.update_display();
				recalculate_fog(map,state,info,units,teams,attacker_side-1);
				break;
			} else if(hits) {
				if(stats.defend_special == poison_string &&
				   a->second.has_flag("poisoned") == false &&
				   !a->second.type().not_living()) {
					a->second.set_flag("poisoned");
				}

				static const std::string slow_string("slow");
				if(stats.defend_special == slow_string &&
				   a->second.has_flag("slowed") == false) {
					a->second.set_flag("slowed");
					if(stats.nattacks > 1)
						--stats.nattacks;
				}

				if(stats.amount_defender_drains > 0) {
					d->second.gets_hit(-stats.amount_defender_drains);
				}

				//if the attacker is turned to stone, the fight stops immediately
				static const std::string stone_string("stone");
				if(stats.defend_special == stone_string) {
					a->second.set_flag("stone");
					stats.ndefends = 0;
					stats.nattacks = 0;
					game_events::fire("stone",a->first,d->first);
				}
			}

			--stats.ndefends;
		}
	}

	if(attackerxp) {
		a->second.get_experience(attackerxp);
	}

	if(defenderxp) {
		d->second.get_experience(defenderxp);
	}

	gui.invalidate_unit();
}

int tower_owner(const gamemap::location& loc, std::vector<team>& teams)
{
	for(size_t i = 0; i != teams.size(); ++i) {
		if(teams[i].owns_tower(loc))
			return i;
	}

	return -1;
}


void get_tower(const gamemap::location& loc, std::vector<team>& teams,
               size_t team_num, const unit_map& units)
{
	if(team_num < teams.size() && teams[team_num].owns_tower(loc)) {
		return;
	}

	const bool has_leader = find_leader(units,int(team_num+1)) != units.end();

	//we strip the village off all other sides, unless it is held by an ally
	//and we don't have a leader (and thus can't occupy it)
	for(std::vector<team>::iterator i = teams.begin(); i != teams.end(); ++i) {
		const int side = i - teams.begin() + 1;
		if(team_num >= teams.size() || has_leader || teams[team_num].is_enemy(side)) {
			i->lose_tower(loc);
		}
	}

	if(team_num >= teams.size()) {
		return;
	}

	if(has_leader) {
		teams[team_num].get_tower(loc);
	}
}

std::map<gamemap::location,unit>::iterator
   find_leader(std::map<gamemap::location,unit>& units, int side)
{
	for(std::map<gamemap::location,unit>::iterator i = units.begin();
	    i != units.end(); ++i) {
		if(i->second.side() == side && i->second.can_recruit())
			return i;
	}

	return units.end();
}

std::map<gamemap::location,unit>::const_iterator
   find_leader(const std::map<gamemap::location,unit>& units, int side)
{
	for(std::map<gamemap::location,unit>::const_iterator i = units.begin();
	    i != units.end(); ++i) {
		if(i->second.side() == side && i->second.can_recruit())
			return i;
	}

	return units.end();
}

namespace {

//function which returns true iff the unit at 'loc' will heal a unit from side 'side'
//on this turn.
//
//units heal other units if they are (1) on the same side as them; or (2) are on a
//different but allied side, and there are no 'higher priority' sides also adjacent
//to the healer
bool will_heal(const gamemap::location& loc, int side, const std::vector<team>& teams,
			   const unit_map& units)
{
	const unit_map::const_iterator healer_it = units.find(loc);
	if(healer_it == units.end() || healer_it->second.type().heals() == false)
		return false;

	const unit& healer = healer_it->second;
	if(healer.side() == side)
		return true;

	if(size_t(side-1) >= teams.size() || size_t(healer.side()-1) >= teams.size())
		return false;

	//if the healer is an enemy, it won't heal
	if(teams[healer.side()-1].is_enemy(side))
		return false;

	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int n = 0; n != 6; ++n) {
		const unit_map::const_iterator u = units.find(adjacent[n]);
		if(u != units.end() && u->second.hitpoints() < u->second.max_hitpoints()) {
			const int unit_side = u->second.side();

			//the healer won't heal an ally if there is a wounded unit on the same
			//side next to her
			if(unit_side == healer.side())
				return false;

			//choose an arbitrary order for healing
			if(unit_side > side)
				return false;
		}
	}

	//there's no-one of higher priority nearby, so the ally will heal
	return true;
}

}

void calculate_healing(display& disp, const gamemap& map,
                       std::map<gamemap::location,unit>& units, int side,
					   const std::vector<team>& teams)
{
	std::map<gamemap::location,int> healed_units, max_healing;

	std::map<gamemap::location,unit>::iterator i;
	int amount_healed;
	for(i = units.begin(); i != units.end(); ++i) {
		amount_healed = 0;

		//the unit heals if it's on this side, and it's on a tower or
		//it has regeneration, and it is wounded
		if(i->second.side() == side) {
			if(i->second.hitpoints() < i->second.max_hitpoints()){
				if((map.underlying_terrain(map[i->first.x][i->first.y]) == gamemap::TOWER ||
				 i->second.type().regenerates())) {
					amount_healed = game_config::cure_amount;
				}
			}
			if(amount_healed != 0)
				healed_units.insert(std::pair<gamemap::location,int>(
			                            i->first, amount_healed));
		}

		//otherwise find the maximum healing for the unit
		if(amount_healed == 0) {
			int max_heal = 0;
			gamemap::location adjacent[6];
			get_adjacent_tiles(i->first,adjacent);
			for(int j = 0; j != 6; ++j) {
				if(will_heal(adjacent[j],i->second.side(),teams,units)) {
					const unit_map::const_iterator healer = units.find(adjacent[j]);
					max_heal = maximum(max_heal,healer->second.type().max_unit_healing());
				}
			}

			if(max_heal > 0) {
				max_healing.insert(std::pair<gamemap::location,int>(i->first,max_heal));
			}
		}
	}

	//now see about units that can heal other units
	for(i = units.begin(); i != units.end(); ++i) {

		if(will_heal(i->first,side,teams,units)) {
			gamemap::location adjacent[6];
			bool gets_healed[6];
			get_adjacent_tiles(i->first,adjacent);

			int nhealed = 0;
			int j;
			for(j = 0; j != 6; ++j) {
				const std::map<gamemap::location,unit>::const_iterator adj =
				                                   units.find(adjacent[j]);
				if(adj != units.end() &&
				   adj->second.hitpoints() < adj->second.max_hitpoints() &&
				   adj->second.side() == side &&
				   healed_units[adj->first] < max_healing[adj->first]) {
					++nhealed;
					gets_healed[j] = true;
				} else {
					gets_healed[j] = false;
				}
			}

			if(nhealed == 0)
				continue;

			const int healing_per_unit = i->second.type().heals()/nhealed;

			for(j = 0; j != 6; ++j) {
				if(!gets_healed[j])
					continue;

				assert(units.find(adjacent[j]) != units.end());

				healed_units[adjacent[j]]
				        = minimum(max_healing[adjacent[j]],
				                  healed_units[adjacent[j]]+healing_per_unit);
			}
		}
	}

	//poisoned units will take the same amount of damage per turn, as
	//curing heals until they are reduced to 1 hitpoint. If they are
	//cured on a turn, they recover 0 hitpoints that turn, but they
	//are no longer poisoned
	for(i = units.begin(); i != units.end(); ++i) {

		if(i->second.side() == side && i->second.has_flag("poisoned")) {
			const int damage = minimum<int>(game_config::cure_amount,
			                                i->second.hitpoints()-1);

			if(damage > 0) {
				healed_units.insert(std::pair<gamemap::location,int>(i->first,-damage));
			}
		}
	}

	for(i = units.begin(); i != units.end(); ++i) {
		if(i->second.side() == side) {
			if(i->second.is_resting()) {
				const std::map<gamemap::location,int>::iterator u =
					healed_units.find(i->first);
				if(u != healed_units.end()) {
					healed_units[i->first] += game_config::rest_heal_amount;
				} else {
					healed_units.insert(std::pair<gamemap::location,int>(i->first,game_config::rest_heal_amount));
				}
			}
			i->second.set_resting(true);
		}
	}

	for(std::map<gamemap::location,int>::iterator h = healed_units.begin();
	    h != healed_units.end(); ++h) {

		const gamemap::location& loc = h->first;

		const bool show_healing = !disp.turbo() && !recorder.skipping() &&
		                          !disp.fogged(loc.x,loc.y);

		assert(units.count(loc) == 1);

		unit& u = units.find(loc)->second;

		if(show_healing) {
			disp.scroll_to_tile(loc.x,loc.y,display::WARP);
			disp.select_hex(loc);
			disp.update_display();
		}

		const int DelayAmount = 50;

		std::cerr << "unit is poisoned? " << (u.has_flag("poisoned") ? "yes" : "no") << "," << h->second << "," << max_healing[h->first] << "\n";

		if(u.has_flag("poisoned") && h->second > 0) {

			//poison is purged only if we are on a village or next to a curer
			if(h->second >= game_config::cure_amount ||
			   max_healing[h->first] >= game_config::cure_amount) {
				u.remove_flag("poisoned");

				if(show_healing) {
					sound::play_sound("heal.wav");
					SDL_Delay(DelayAmount);
					disp.invalidate_unit();
					disp.update_display();
				}
			}

			h->second = 0;
		} else if(h->second < 0) {
			if(show_healing)
				sound::play_sound("groan.wav");
		} else if(h->second > 0) {
			if(show_healing)
				sound::play_sound("heal.wav");
		}

		if(h->second > 0 && h->second > u.max_hitpoints()-u.hitpoints()) {
			h->second = u.max_hitpoints()-u.hitpoints();
			if(h->second <= 0)
				continue;
		}

		while(h->second > 0) {
			const Uint16 heal_colour = disp.rgb(0,0,200);
			u.heal(1);

			if(show_healing) {
				if(is_odd(h->second))
					disp.draw_tile(loc.x,loc.y,NULL,0.5,heal_colour);
				else
					disp.draw_tile(loc.x,loc.y);
				SDL_Delay(DelayAmount);
				disp.update_display();
			}

			--h->second;
		}

		while(h->second < 0) {
			const Uint16 damage_colour = disp.rgb(200,0,0);
			u.gets_hit(1);

			if(show_healing) {
				if(is_odd(h->second))
					disp.draw_tile(loc.x,loc.y,NULL,0.5,damage_colour);
				else
					disp.draw_tile(loc.x,loc.y);

				SDL_Delay(DelayAmount);
				disp.update_display();
			}

			++h->second;
		}

		if(show_healing) {
			disp.draw_tile(loc.x,loc.y);
			disp.update_display();
		}
	}
}

unit get_advanced_unit(const game_data& info,
                  std::map<gamemap::location,unit>& units,
                  const gamemap::location& loc, const std::string& advance_to)
{
	const std::map<std::string,unit_type>::const_iterator new_type = info.unit_types.find(advance_to);
	const std::map<gamemap::location,unit>::iterator un = units.find(loc);
	if(new_type != info.unit_types.end() && un != units.end()) {
		return unit(&(new_type->second),un->second);
	} else {
		throw gamestatus::game_error("Could not find the unit being advanced"
		                             " to: " + advance_to);
	}
}

void advance_unit(const game_data& info,
                  std::map<gamemap::location,unit>& units,
                  gamemap::location loc, const std::string& advance_to)
{
	const unit& new_unit = get_advanced_unit(info,units,loc,advance_to);

	statistics::advance_unit(new_unit);
	
	units.erase(loc);
	units.insert(std::pair<gamemap::location,unit>(loc,new_unit));
}

void check_victory(std::map<gamemap::location,unit>& units,
                   std::vector<team>& teams)
{
	std::vector<int> seen_leaders;
	for(std::map<gamemap::location,unit>::const_iterator i = units.begin();
	    i != units.end(); ++i) {
		if(i->second.can_recruit()) {
			std::cerr << "seen leader for side " << i->second.side() << "\n";
			seen_leaders.push_back(i->second.side());
		}
	}

	//clear villages for teams that have no leader
	for(std::vector<team>::iterator tm = teams.begin(); tm != teams.end(); ++tm) {
		if(std::find(seen_leaders.begin(),seen_leaders.end(),tm-teams.begin() + 1) == seen_leaders.end()) {
			tm->clear_towers();
		}
	}

	bool found_enemies = false;
	bool found_human = false;

	for(size_t n = 0; n != seen_leaders.size(); ++n) {
		const size_t side = seen_leaders[n]-1;

		assert(side < teams.size());

		for(size_t m = n+1; m != seen_leaders.size(); ++m) {
			if(side < teams.size() && teams[side].is_enemy(seen_leaders[m])) {
				found_enemies = true;
			}
		}

		if(side < teams.size() && teams[side].is_human()) {
			found_human = true;
		}
	}

	if(found_enemies == false) {
		if(found_human) {
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

		throw end_level_exception(found_human ? VICTORY : DEFEAT);
	}

	//remove any units which are leaderless
	//this code currently removed, to try not removing leaderless enemies
	/*
	std::map<gamemap::location,unit>::iterator j = units.begin();
	while(j != units.end()) {
		if(std::find(seen_leaders.begin(),seen_leaders.end(),j->second.side()) == seen_leaders.end()) {
			units.erase(j);
			j = units.begin();
		} else {
			++j;
		}
	}*/
}

const time_of_day& timeofday_at(const gamestatus& status,
                                const std::map<gamemap::location,unit>& units,
                                const gamemap::location& loc)
{
	bool lighten = false;

	if(loc.valid()) {
		gamemap::location locs[7];
		locs[0] = loc;
		get_adjacent_tiles(loc,locs+1);

		for(int i = 0; i != 7; ++i) {
			const std::map<gamemap::location,unit>::const_iterator itor =
			                                              units.find(locs[i]);
			if(itor != units.end() &&
			   itor->second.type().illuminates()) {
				lighten = true;
			}
		}
	}

	return status.get_time_of_day(lighten);
}

int combat_modifier(const gamestatus& status,
                    const std::map<gamemap::location,unit>& units,
					const gamemap::location& loc,
					unit_type::ALIGNMENT alignment)
{
	const time_of_day& tod = timeofday_at(status,units,loc);

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
		if(map.on_board(adj[i])) {
			if(tm.fogged(adj[i].x,adj[i].y)) {
				const bool res = tm.clear_shroud(adj[i].x,adj[i].y) ||
				                 tm.clear_fog(adj[i].x,adj[i].y);

				if(res && cleared != NULL) {
					cleared->push_back(adj[i]);
				}

				result |= res;
			}
		}
	}

	return result;
}

//returns true iff some shroud is cleared
//returns true/false in seen_unit if new units has/has not been seen
//if known_units is NULL, seen_unit can be NULL and seen_unit is undefined
bool clear_shroud_unit(const gamemap& map, 
		                 const gamestatus& status,
							  const game_data& gamedata,
                       const unit_map& units, const gamemap::location& loc,
                       std::vector<team>& teams, int team,
					   const std::set<gamemap::location>* known_units,
						bool* seen_unit)
{
	bool res;

	std::vector<gamemap::location> cleared_locations;

	paths p(map,status,gamedata,units,loc,teams,true,false);
	for(paths::routes_map::const_iterator i = p.routes.begin();
	    i != p.routes.end(); ++i) {
		clear_shroud_loc(map,teams[team],i->first,&cleared_locations);
	}

	//clear the location the unit is at
	clear_shroud_loc(map,teams[team],loc,&cleared_locations);
	
	res = (cleared_locations.empty() == false);

	for(std::vector<gamemap::location>::const_iterator it =
	    cleared_locations.begin(); it != cleared_locations.end(); ++it) {
		if(units.count(*it)) {
			if(seen_unit == NULL) {
				static const std::string sighted("sighted");
				game_events::fire(sighted,*it,loc);
			} else if(known_units->count(*it) == 0) {
				*seen_unit = true;
				return res;
			}
		}
	}

	if(seen_unit != NULL) {
		*seen_unit = false;
	}
	return res;
}

}

void recalculate_fog(const gamemap& map, const gamestatus& status,
		const game_data& gamedata, const unit_map& units, 
		std::vector<team>& teams, int team) {

	teams[team].refog();

	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if(i->second.side() == team+1) {

			//we're not really going to mutate the unit, just temporarily
			//set its moves to maximum, but then switch them back
			unit& mutable_unit = const_cast<unit&>(i->second);
			const unit_movement_resetter move_resetter(mutable_unit);

			clear_shroud_unit(map,status,gamedata,units,i->first,teams,team,NULL,NULL);
		}
	}
}

bool clear_shroud(display& disp, const gamestatus& status, 
		            const gamemap& map, const game_data& gamedata,
                  const unit_map& units, std::vector<team>& teams, int team)
{
	if(teams[team].uses_shroud() == false && teams[team].uses_fog() == false)
		return false;

	bool result = false;

	unit_map::const_iterator i;
	for(i = units.begin(); i != units.end(); ++i) {
		if(i->second.side() == team+1) {

			//we're not really going to mutate the unit, just temporarily
			//set its moves to maximum, but then switch them back
			unit& mutable_unit = const_cast<unit&>(i->second);
			const unit_movement_resetter move_resetter(mutable_unit);

			result |= clear_shroud_unit(map,status,gamedata,units,i->first,teams,team,NULL,NULL);
		}
	}

	recalculate_fog(map,status,gamedata,units,teams,team);

	return result;
}

size_t move_unit(display* disp, const game_data& gamedata, 
                 const gamestatus& status, const gamemap& map,
                 unit_map& units, std::vector<team>& teams,
                 const std::vector<gamemap::location>& route,
                 replay* move_recorder, undo_list* undo_stack, gamemap::location *next_unit)
{
	//stop the user from issuing any commands while the unit is moving
	const command_disabler disable_commands;

	assert(!route.empty());

	unit_map::iterator ui = units.find(route.front());

	assert(ui != units.end());

	ui->second.set_goto(gamemap::location());

	unit u = ui->second;

	const size_t team_num = u.side()-1;

	const bool skirmisher = u.type().is_skirmisher();

	//if we use shroud/fog of war, count out the units we can currently see
	std::set<gamemap::location> seen_units;
	if(teams[team_num].uses_shroud() || teams[team_num].uses_fog()) {
		for(unit_map::const_iterator u = units.begin(); u != units.end(); ++u) {
			if(teams[team_num].fogged(u->first.x,u->first.y) == false) {
				seen_units.insert(u->first);
			}
		}
	}

	//see how far along the given path we can move
	const int starting_moves = u.movement_left();
	int moves_left = starting_moves;
	bool seen_unit = false;
	bool discovered_unit = false;
	bool should_clear_stack = false;
	std::vector<gamemap::location>::const_iterator step;
	for(step = route.begin()+1; step != route.end(); ++step) {
		const gamemap::TERRAIN terrain = map[step->x][step->y];

		const unit_map::const_iterator enemy_unit = units.find(*step);
			
		const int mv = u.movement_cost(map,terrain);
		if(discovered_unit || seen_unit || mv > moves_left || enemy_unit != units.end() &&
		   teams[team_num].is_enemy(enemy_unit->second.side())) {
			break;
		} else {
			moves_left -= mv;
		}

		if(!skirmisher && enemy_zoc(map,status,units,teams,*step,teams[team_num],
					u.side())) {
			moves_left = 0;
		}

		//if we use fog or shroud, see if we have sighted an enemy unit, in
		//which case we should stop immediately.
		if(teams[team_num].uses_shroud() || teams[team_num].uses_fog()) {
			if(units.count(*step) == 0 && map.underlying_terrain(map.get_terrain(*step)) != gamemap::TOWER) {
				units.insert(std::pair<gamemap::location,unit>(*step,ui->second));
				
				bool res;

				should_clear_stack |= 
					clear_shroud_unit(map,status,gamedata,units,*step,teams,
				   	ui->second.side()-1,&seen_units,&res);
				units.erase(*step);

				//we've seen a new unit. Stop on the next iteration
				if(res) {
					seen_unit = true;
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

			const std::map<gamemap::location,unit>::const_iterator it = units.find(adjacent[i]);
			if(it != units.end() && teams[u.side()-1].is_enemy(it->second.side()) &&
					it->second.invisible(map.underlying_terrain(map[it->first.x][it->first.y]),status.get_time_of_day().lawful_bonus,it->first,units,teams)) {
				discovered_unit = true;
				should_clear_stack = true;
				break;
			}
		}
	}

	//make sure we don't tread on another unit
	std::vector<gamemap::location>::const_iterator begin = route.begin();

	std::vector<gamemap::location> steps(begin,step);
	while(!steps.empty() && units.count(steps.back()) != 0) {
		steps.pop_back();
	}

	assert(steps.size() <= route.size());

	if (next_unit != NULL )
		*next_unit = steps.back();

	//if we can't get all the way there and have to set a go-to,
	//unless we stop early because of sighting a unit
	if(steps.size() != route.size() && !seen_unit) {
		ui->second.set_goto(route.back());
		u.set_goto(route.back());
	}

	if(steps.size() < 2) {
		return 0;
	}

	units.erase(ui);
	if(disp != NULL)
		disp->move_unit(steps,u);

	if(move_recorder != NULL)
		move_recorder->add_movement(steps.front(),steps.back());

	u.set_movement(moves_left);

	ui = units.insert(std::pair<gamemap::location,unit>(steps.back(),u)).first;
	if(disp != NULL) {
		disp->invalidate_unit();
		disp->invalidate(steps.back());
	}

	int orig_tower_owner = -1;
	if(map.underlying_terrain(map[steps.back().x][steps.back().y]) == gamemap::TOWER) {
		orig_tower_owner = tower_owner(steps.back(),teams);

		if(orig_tower_owner != team_num) {
			get_tower(steps.back(),teams,team_num,units);
			ui->second.set_movement(0);
		}
	}

	const bool event_mutated = game_events::fire("moveto",steps.back());

	if(undo_stack != NULL) {
		if(event_mutated || should_clear_stack) {
			undo_stack->clear();
		} else {
			undo_stack->push_back(undo_action(steps,starting_moves,orig_tower_owner));
		}
	}

	if(disp != NULL) {
		disp->set_route(NULL);
		disp->draw();
		disp->recalculate_minimap();
	}

	assert(steps.size() <= route.size());

	return steps.size();
}

bool unit_can_move(const gamemap::location& loc, const unit_map& units,
                   const gamemap& map, const std::vector<team>& teams)
{
	const unit_map::const_iterator u_it = units.find(loc);
	assert(u_it != units.end());
	
	const unit& u = u_it->second;
	const team& current_team = teams[u.side()-1];

	if(!u.can_attack())
		return false;

	//units with goto commands that have already done their gotos this turn
	//(i.e. don't have full movement left) should be red
	if(u.movement_left() < u.total_movement() && u.get_goto().valid()) {
		return false;
	}

	gamemap::location locs[6];
	get_adjacent_tiles(loc,locs);
	for(int n = 0; n != 6; ++n) {
		if(map.on_board(locs[n])) {
			const unit_map::const_iterator i = units.find(locs[n]);
			if(i != units.end()) {
				if(current_team.is_enemy(i->second.side())) {
					return true;
				}
			}
			
			if(u.movement_cost(map,map[locs[n].x][locs[n].y]) <= u.movement_left()) {
				return true;
			}
		}
	}

	return false;
}
