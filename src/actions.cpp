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

std::string recruit_unit(const gamemap& map, int side,
       std::map<gamemap::location,unit>& units, unit& new_unit,
       gamemap::location recruit_location, display* disp, bool need_castle)
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

	if(!map.is_starting_position(u->first)) {
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

	new_unit.set_movement(0);
	new_unit.set_attacked();
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

	res.chance_to_hit_attacker =
			a->second.defense_modifier(map,attacker_terrain);

	res.chance_to_hit_defender =
			d->second.defense_modifier(map,defender_terrain);

	const std::vector<attack_type>& attacker_attacks =
			a->second.attacks();
	const std::vector<attack_type>& defender_attacks =
			d->second.attacks();

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
	res.attacker_plague = (attack.special() == plague_string);
	res.defender_plague = false;

	res.attack_name    = attack.name();
	res.attack_type    = attack.type();

	if(include_strings) {
		res.attack_special = attack.special();

		//don't show backstabbing unless it's actually happening
		if(res.attack_special == "backstab" && !backstab)
			res.attack_special = "";

		res.range = (attack.range() == attack_type::SHORT_RANGE ?
		             "Melee" : "Ranged");
	}

	res.nattacks = attack.num_attacks();
	int defend;
	res.ndefends = 0;
	for(defend = 0; defend != int(defender_attacks.size()); ++defend) {
		if(defender_attacks[defend].range() == attack.range())
			break;
	}

	res.defend_with = defend != int(defender_attacks.size()) ? defend : -1;

	const bool counterattack = defend != int(defender_attacks.size());

	static const std::string drain_string("drain");
	static const std::string magical_string("magical");

	res.damage_attacker_takes = 0;
	if(counterattack) {
		//magical attacks always have a 70% chance to hit
		if(defender_attacks[defend].special() == magical_string)
			res.chance_to_hit_attacker = 70;

		res.damage_attacker_takes = int(double(
		 a->second.damage_against(defender_attacks[defend]))
		 * combat_modifier(state,units,d->first,d->second.type().alignment()));

		if(charge)
			res.damage_attacker_takes *= 2;

		if(under_leadership(units,defender))
			res.damage_attacker_takes += res.damage_attacker_takes/8 + 1;

		if(res.damage_attacker_takes < 1)
			res.damage_attacker_takes = 1;

		res.ndefends = defender_attacks[defend].num_attacks();

		res.defend_name    = defender_attacks[defend].name();
		res.defend_type    = defender_attacks[defend].type();

		if(include_strings) {
			res.defend_special = defender_attacks[defend].special();
		}

		if(defender_attacks[defend].special() == drain_string) {
			res.amount_defender_drains = res.damage_attacker_takes/2;
		} else {
			res.amount_defender_drains = 0;
		}

		res.defender_plague =
		        (defender_attacks[defend].special() == plague_string);
	}

	if(attack.special() == magical_string)
		res.chance_to_hit_defender = 70;

	static const std::string marksman_string("marksman");

	//offensive marksman attacks always have at least 60% chance to hit
	if(res.chance_to_hit_defender < 60 && attack.special() == marksman_string)
		res.chance_to_hit_defender = 60;

	res.damage_defender_takes = int(util::round(
			double(d->second.damage_against(attack))
		 * combat_modifier(state,units,a->first,a->second.type().alignment())))
			 * (charge ? 2 : 1) * (backstab ? 2 : 1);

	if(under_leadership(units,attacker))
		res.damage_defender_takes += res.damage_defender_takes/8 + 1;

	if(attack.special() == drain_string) {
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

	//if the attacker was invisible, she isn't anymore!
	static const std::string forest_invisible("ambush");
	a->second.remove_flag(forest_invisible);

	battle_stats stats = evaluate_battle_stats(map,attacker,defender,
	                                           attack_with,units,state,info);

	while(stats.nattacks > 0 || stats.ndefends > 0) {
		if(stats.nattacks > 0) {
			const bool hits = (get_random()%100) < stats.chance_to_hit_defender;
			const bool dies = gui.unit_attack(attacker,defender,
			            hits ? stats.damage_defender_takes : 0,
						a->second.attacks()[attack_with]);
			if(dies) {
				attackerxp = 8*d->second.type().level();
				if(d->second.type().level() == 0)
					attackerxp = 4;

				defenderxp = 0;

				gamemap::location loc = d->first;
				gamemap::location attacker_loc = a->first;
				game_events::fire("die",loc,a->first);

				//the handling of the event may have removed the object
				//so we have to find it again
				units.erase(loc);

				//plague units make clones of themselves on the target hex
				//units on villages that die cannot be plagued
				if(stats.attacker_plague && map[loc.x][loc.y]!=gamemap::TOWER) {
					a = units.find(attacker_loc);
					if(a != units.end()) {
						units.insert(std::pair<gamemap::location,unit>(
						                                 loc,a->second));
						gui.draw_tile(loc.x,loc.y);
						gui.update_display();
					}
				}

				break;
			} else if(hits) {
				static const std::string poison_string("poison");
				if(stats.attack_special == poison_string &&
				   d->second.has_flag("poisoned") == false) {
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
			}

			--stats.nattacks;
		}

		if(stats.ndefends > 0) {
			const bool hits = (get_random()%100) < stats.chance_to_hit_attacker;
			const bool dies = gui.unit_attack(defender,attacker,
			               hits ? stats.damage_attacker_takes : 0,
						   d->second.attacks()[stats.defend_with]);

			if(dies) {
				defenderxp = 8*a->second.type().level();
				if(a->second.type().level() == 0)
					defenderxp = 4;

				attackerxp = 0;

				gamemap::location loc = a->first;
				gamemap::location defender_loc = d->first;
				game_events::fire("die",loc,d->first);

				//the handling of the event may have removed the object
				//so we have to find it again
				units.erase(loc);

				//plague units make clones of themselves on the target hex.
				//units on villages that die cannot be plagued
				if(stats.defender_plague && map[loc.x][loc.y]!=gamemap::TOWER) {
					d = units.find(defender_loc);
					if(d != units.end()) {
						units.insert(std::pair<gamemap::location,unit>(
						                                 loc,d->second));
						gui.draw_tile(loc.x,loc.y);
						gui.update_display();
					}
				}
				break;
			} else if(hits) {
				if(stats.defend_special == "poison" &&
				   a->second.has_flag("poisoned") == false) {
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
               int team_num)
{
	for(size_t i = 0; i != teams.size(); ++i) {
		if(int(i) != team_num && teams[i].owns_tower(loc)) {
			teams[i].lose_tower(loc);
		}
	}

	if(size_t(team_num) < teams.size())
		teams[team_num].get_tower(loc);
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

void calculate_healing(display& disp, const gamemap& map,
                       std::map<gamemap::location,unit>& units, int side)
{
	std::map<gamemap::location,int> healed_units, max_healing;

	std::map<gamemap::location,unit>::iterator i;
	for(i = units.begin(); i != units.end(); ++i) {

		//the unit heals if it's on this side, and if it's on a tower or
		//it has regeneration
		if(i->second.side() == side &&
		   (map[i->first.x][i->first.y] == gamemap::TOWER ||
		    i->second.type().regenerates())) {
			healed_units.insert(std::pair<gamemap::location,int>(
			                            i->first, game_config::cure_amount));

		}

		//otherwise find the maximum healing for the unit
		else {
			int max_heal = 0;
			gamemap::location adjacent[6];
			get_adjacent_tiles(i->first,adjacent);
			for(int j = 0; j != 6; ++j) {
				std::map<gamemap::location,unit>::const_iterator healer =
				                                      units.find(adjacent[j]);
				if(healer != units.end() && healer->second.side() == side) {
					max_heal = maximum(max_heal,
					                 healer->second.type().max_unit_healing());
				}
			}

			if(max_heal > 0) {
				max_healing.insert(std::pair<gamemap::location,int>(i->first,
				                                                    max_heal));
			}
		}
	}

	//now see about units that can heal other units
	for(i = units.begin(); i != units.end(); ++i) {

		if(i->second.side() == side && i->second.type().heals()) {
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
				   adj->second.side() == i->second.side() &&
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
				healed_units.insert(std::pair<gamemap::location,int>(
				                               i->first,-damage));
			}
		}
	}

	for(std::map<gamemap::location,int>::iterator h = healed_units.begin();
	    h != healed_units.end(); ++h) {

		const gamemap::location& loc = h->first;

		const bool show_healing = !disp.turbo() && !recorder.skipping() &&
		                          !disp.fogged(loc.x,loc.y);

		assert(units.count(loc) == 1);

		unit& u = units.find(loc)->second;

		if(h->second > 0 && h->second > u.max_hitpoints()-u.hitpoints()) {
			h->second = u.max_hitpoints()-u.hitpoints();
			if(h->second <= 0)
				continue;
		}

		if(show_healing) {
			disp.scroll_to_tile(loc.x,loc.y,display::WARP);
			disp.select_hex(loc);
			disp.update_display();
		}

		const int DelayAmount = 50;

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

		while(h->second > 0) {
			const display::Pixel heal_colour = disp.rgb(0,0,200);
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
			const display::Pixel damage_colour = disp.rgb(200,0,0);
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
	const std::map<std::string,unit_type>::const_iterator new_type =
	     info.unit_types.find(advance_to);
	std::map<gamemap::location,unit>::iterator un = units.find(loc);
	if(new_type != info.unit_types.end() && un != units.end()) {
		return unit(&(new_type->second),un->second);
	} else {
		throw gamestatus::game_error("Could not find the unit being advanced"
		                             " to: " + advance_to);
	}
}

void advance_unit(const game_data& info,
                  std::map<gamemap::location,unit>& units,
                  const gamemap::location& loc, const std::string& advance_to)
{
	const unit& new_unit = get_advanced_unit(info,units,loc,advance_to);
	units.erase(loc);
	units.insert(std::pair<gamemap::location,unit>(loc,new_unit));
}

void check_victory(std::map<gamemap::location,unit>& units,
                   const std::vector<team>& teams)
{
	std::vector<int> seen_leaders;
	for(std::map<gamemap::location,unit>::const_iterator i = units.begin();
	    i != units.end(); ++i) {
		if(i->second.can_recruit())
			seen_leaders.push_back(i->second.side());
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
		throw end_level_exception(found_human ? VICTORY : DEFEAT);
	}

	//remove any units which are leaderless
	std::map<gamemap::location,unit>::iterator j = units.begin();
	while(j != units.end()) {
		if(std::find(seen_leaders.begin(),seen_leaders.end(),j->second.side())
		   == seen_leaders.end()) {
			units.erase(j);
			j = units.begin();
		} else {
			++j;
		}
	}
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

double combat_modifier(const gamestatus& status,
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

	return 1.0 + static_cast<double>(bonus)/100.0;
}

size_t move_unit(display* disp, const gamemap& map,
                 unit_map& units, std::vector<team>& teams,
                 const std::vector<gamemap::location>& route,
                 replay* move_recorder, undo_list* undo_stack)
{
	//stop the user from issuing any commands while the unit is moving
	const command_disabler disable_commands;

	assert(!route.empty());

	const unit_map::iterator ui = units.find(route.front());

	assert(ui != units.end());

	ui->second.set_goto(gamemap::location());

	unit u = ui->second;

	const int team_num = u.side()-1;

	const bool skirmisher = u.type().is_skirmisher();

	//start off by seeing how far along the given path we can move
	const int starting_moves = u.movement_left();
	int moves_left = starting_moves;
	std::vector<gamemap::location>::const_iterator step;
	for(step = route.begin()+1; step != route.end(); ++step) {
		const gamemap::TERRAIN terrain = map[step->x][step->y];

		const unit_map::const_iterator enemy_unit = units.find(*step);
			
		const int mv = u.type().movement_type().movement_cost(map,terrain);
		if(mv > moves_left || enemy_unit != units.end() &&
		   teams[team_num].is_enemy(enemy_unit->second.side())) {
			break;
		} else {
			moves_left -= mv;
		}

		if(!skirmisher && enemy_zoc(map,units,*step,teams[team_num],u.side())) {
			moves_left = 0;
		}
	}

	//make sure we don't tread on another unit
	std::vector<gamemap::location>::const_iterator begin = route.begin();

	std::vector<gamemap::location> steps(begin,step);
	while(!steps.empty() && units.count(steps.back()) != 0) {
		steps.pop_back();
	}

	assert(steps.size() <= route.size());

	//if we can't get all the way there and have to set a go-to
	if(steps.size() != route.size()) {
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

	int orig_tower_owner = -1;
	if(map[steps.back().x][steps.back().y] == gamemap::TOWER) {
		orig_tower_owner = tower_owner(steps.back(),teams);

		if(orig_tower_owner != team_num) {
			get_tower(steps.back(),teams,team_num);
			u.set_movement(0);
		}
	}

	units.insert(std::pair<gamemap::location,unit>(steps.back(),u));
	if(disp != NULL)
		disp->invalidate_unit();

	const bool event_mutated = game_events::fire("moveto",steps.back());

	if(undo_stack != NULL) {
		if(event_mutated) {
			undo_stack->clear();
		} else {
			undo_stack->push_back(undo_action(steps,starting_moves,
			                                  orig_tower_owner));
		}
	}

	if(disp != NULL) {
		disp->set_route(NULL);
		disp->draw();
	}

	assert(steps.size() <= route.size());

	return steps.size();
}

void clear_shroud_loc(const gamemap& map, team& tm,
                      const gamemap::location& loc,
                      std::vector<gamemap::location>* cleared)
{
	static gamemap::location adj[7];
	get_adjacent_tiles(loc,adj);
	adj[6] = loc;
	for(int i = 0; i != 6; ++i) {
		if(map.on_board(adj[i])) {
			if(tm.fogged(adj[i].x,adj[i].y)) {
				tm.clear_shroud(adj[i].x,adj[i].y);
				tm.clear_fog(adj[i].x,adj[i].y);
				if(cleared != NULL) {
					cleared->push_back(adj[i]);
				}
			}
		}
	}
}

void clear_shroud_unit(const gamemap& map, const game_data& gamedata,
                       const unit_map& units, const gamemap::location& loc,
                       std::vector<team>& teams, int team)
{
	std::vector<gamemap::location> cleared_locations;

	paths p(map,gamedata,units,loc,teams,true,false);
	for(paths::routes_map::const_iterator i = p.routes.begin();
	    i != p.routes.end(); ++i) {
		clear_shroud_loc(map,teams[team],i->first,&cleared_locations);
	}

	for(std::vector<gamemap::location>::const_iterator it =
	    cleared_locations.begin(); it != cleared_locations.end(); ++it) {
		static const std::string sighted("sighted");
		game_events::fire(sighted,*it,loc);
	}
}

bool clear_shroud(display& disp, const gamemap& map, const game_data& gamedata,
                  const unit_map& units, std::vector<team>& teams, int team)
{
	if(teams[team].uses_shroud() == false && teams[team].uses_fog() == false)
		return false;

	teams[team].refog();

	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if(i->second.side() == team+1) {

			//we're not really going to mutate the unit, just temporarily
			//set its moves to maximum, but then switch them back
			unit& mutable_unit = const_cast<unit&>(i->second);
			const int old_moves = mutable_unit.movement_left();
			mutable_unit.set_movement(mutable_unit.total_movement());
			clear_shroud_unit(map,gamedata,units,i->first,teams,team);
			mutable_unit.set_movement(old_moves);
		}
	}

	disp.recalculate_minimap();

	return true;
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
			} else if(u.movement_cost(map,map[locs[n].x][locs[n].y]) <=
			          u.movement_left()) {
				return true;
			}
		}
	}

	return false;
}
