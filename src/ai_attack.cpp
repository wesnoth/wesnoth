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
#include "ai_attack.hpp"
#include "events.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "pathfind.hpp"
#include "util.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <set>

const int max_positions = 10000;

//analyze possibility of attacking target on 'loc'
void ai::do_attack_analysis(
	                 const location& loc,
	                 const move_map& srcdst, const move_map& dstsrc,
	                 const move_map& enemy_srcdst, const move_map& enemy_dstsrc,
					 const location* tiles, bool* used_locations,
	                 std::vector<location>& units,
	                 std::vector<attack_analysis>& result,
					 attack_analysis& cur_analysis
	                )
{
	std::cerr << "doing attack analysis...\n";
	if(cur_analysis.movements.size() >= 4)
		return;

	events::pump();

	static double best_results[6];
	if(result.empty()) {
		for(int i = 0; i != 6; ++i) {
			best_results[i] = 0.0;
		}
	}

	const size_t max_positions = 1000;
	if(result.size() > max_positions && !cur_analysis.movements.empty()) {
		std::cerr << "cut analysis short with number of positions\n";
		return;
	}

	const double cur_rating = cur_analysis.movements.empty() ? 0 :
	                          cur_analysis.rating(0.0);

	double rating_to_beat = cur_rating;

	if(!cur_analysis.movements.empty()) {
		assert(cur_analysis.movements.size() < 6);
		double& best_res = best_results[cur_analysis.movements.size()-1];
		rating_to_beat = best_res = maximum(best_res,cur_rating);
	}

	for(size_t i = 0; i != units.size(); ++i) {
		const location current_unit = units[i];
		units.erase(units.begin() + i);

		for(int j = 0; j != 6; ++j) {
			if(used_locations[j])
				continue;

			typedef std::multimap<location,location>::const_iterator Itor;
			std::pair<Itor,Itor> its = dstsrc.equal_range(tiles[j]);
			while(its.first != its.second) {
				if(its.first->second == current_unit)
					break;
				++its.first;
			}

			if(its.first == its.second)
				continue;

			cur_analysis.movements.push_back(std::pair<location,location>(current_unit,tiles[j]));

			//find out how vulnerable we are to attack from enemy units in this hex
			const double vulnerability = power_projection(tiles[j],enemy_srcdst,enemy_dstsrc);
			cur_analysis.vulnerability += vulnerability;

			//calculate how much support we have on this hex from allies. Support does not
			//take into account terrain, because we don't want to move into a hex that is
			//surrounded by good defensive terrain
			const double support = power_projection(tiles[j],srcdst,dstsrc,false);
			cur_analysis.support += support;

			cur_analysis.analyze(map_,units_,state_,gameinfo_,50,*this);

			if(cur_analysis.rating(0.0) > rating_to_beat) {

				result.push_back(cur_analysis);
				used_locations[j] = true;
				do_attack_analysis(loc,srcdst,dstsrc,enemy_srcdst,enemy_dstsrc,
				                   tiles,used_locations,
				                   units,result,cur_analysis);
				used_locations[j] = false;
			}

			cur_analysis.vulnerability -= vulnerability;
			cur_analysis.support -= support;

			cur_analysis.movements.pop_back();
		}

		units.insert(units.begin() + i, current_unit);
	}
}


struct battle_type {
	battle_type(const gamemap::location& a, const gamemap::location& d,
	            gamemap::TERRAIN t)
	         : attacker(a),defender(d),terrain(t),weapon(-1)
	{}

	const gamemap::location attacker;
	const gamemap::location defender;
	const gamemap::TERRAIN terrain;
	int weapon;
	battle_stats stats;
};

bool operator<(const battle_type& a, const battle_type& b)
{
	return a.attacker < b.attacker ||
	       a.attacker == b.attacker && a.defender < b.defender ||
		   a.attacker == b.attacker && a.defender == b.defender &&
		   a.terrain < b.terrain;
}

bool operator==(const battle_type& a, const battle_type& b)
{
	return a.attacker == b.attacker && a.defender == b.defender &&
	       a.terrain == b.terrain;
}

std::set<battle_type> weapon_choice_cache;

int ai::choose_weapon(const location& att, const location& def,
					  battle_stats& cur_stats, gamemap::TERRAIN terrain)
{
	const std::map<location,unit>::const_iterator itor = units_.find(att);
	if(itor == units_.end())
		return -1;

	static int cache_hits = 0;
	static int cache_misses = 0;

	battle_type battle(att,def,terrain);
	const std::set<battle_type>::const_iterator cache_itor = weapon_choice_cache.find(battle);

	if(cache_itor != weapon_choice_cache.end()) {
		assert(*cache_itor == battle);

		++cache_hits;
		cur_stats = cache_itor->stats;

		if(!(size_t(cache_itor->weapon) < itor->second.attacks().size())) {
			std::cerr << "cached illegal weapon: " << cache_itor->weapon
			          << "/" << itor->second.attacks().size() << "\n";
		}

		assert(size_t(cache_itor->weapon) < itor->second.attacks().size());
		return cache_itor->weapon;
	}

	++cache_misses;

	if((cache_misses%100) == 0) {
		std::cerr << "cache_stats: " << cache_hits << ":" << cache_misses << " " << weapon_choice_cache.size() << "\n";
	}

	int current_choice = -1;
	double current_rating = 0.0;
	const std::vector<attack_type>& attacks = itor->second.attacks();
	assert(!attacks.empty());

	const unit_map::const_iterator d_itor = units_.find(def);
	int d_hitpoints = d_itor->second.hitpoints();
	int a_hitpoints = itor->second.hitpoints();
	
	for(size_t a = 0; a != attacks.size(); ++a) {
		const battle_stats stats = evaluate_battle_stats(map_,att,def,a,units_,
		                                                 state_,gameinfo_,terrain,false);

		//TODO: improve this rating formula!
		const double rating =
		   (double(stats.chance_to_hit_defender)/100.0)*
		               minimum<int>(stats.damage_defender_takes,d_hitpoints)*stats.nattacks -
		   (double(stats.chance_to_hit_attacker)/100.0)*
		               minimum<int>(stats.damage_attacker_takes,a_hitpoints)*stats.ndefends;
		if(rating > current_rating || current_choice == -1) {
			current_choice = a;
			current_rating = rating;
			cur_stats = stats;
		}
	}

	assert(size_t(current_choice) < attacks.size());

	battle.stats = cur_stats;
	battle.weapon = current_choice;
	weapon_choice_cache.insert(battle);

	return current_choice;
}

void ai::attack_analysis::analyze(const gamemap& map,
                                  unit_map& units,
						 	      const gamestatus& status,
							      const game_data& info, int num_sims, ai& ai_obj)
{
	const unit_map::const_iterator defend_it = units.find(target);
	assert(defend_it != units.end());

	//see if the target is a threat to our leader or an ally's leader
	gamemap::location adj[6];
	get_adjacent_tiles(target,adj);
	size_t tile;
	for(tile = 0; tile != 6; ++tile) {
		const unit_map::const_iterator leader = units.find(adj[tile]);
		if(leader != units.end() && leader->second.can_recruit() && ai_obj.current_team().is_enemy(leader->second.side()) == false) {
			break;
		}
	}

	leader_threat = (tile != 6);

	target_value = defend_it->second.type().cost();
	target_value += (double(defend_it->second.experience())/
	                 double(defend_it->second.max_experience()))*target_value;
	target_starting_damage = defend_it->second.max_hitpoints() -
	                         defend_it->second.hitpoints();
	chance_to_kill = 0.0;
	avg_damage_inflicted = 0.0;
	avg_damage_taken = 0.0;
	resources_used = 0.0;
	terrain_quality = 0.0;
	counter_strength_ratio = 0.0;
	avg_losses = 0.0;

	const int target_max_hp = defend_it->second.max_hitpoints();
	const int target_hp = defend_it->second.hitpoints();
	static std::vector<int> hitpoints;
	static std::vector<battle_stats> stats;

	hitpoints.clear();
	stats.clear();
	weapons.clear();

	std::vector<std::pair<location,location> >::const_iterator m;
	for(m = movements.begin(); m != movements.end(); ++m) {
		battle_stats bat_stats;
		const int weapon = ai_obj.choose_weapon(m->first,target, bat_stats, map[m->second.x][m->second.y]);

		assert(weapon != -1);
		weapons.push_back(weapon);

		stats.push_back(bat_stats);
		hitpoints.push_back(units.find(m->first)->second.hitpoints());
	}

	for(int j = 0; j != num_sims; ++j) {

		int defenderxp = 0;

		int defhp = target_hp;
		for(size_t i = 0; i != movements.size() && defhp; ++i) {
			const battle_stats& stat = stats[i];
			int atthp = hitpoints[i];

			int attacks = stat.nattacks;
			int defends = stat.ndefends;

			unit_map::const_iterator att = units.find(movements[i].first);
			double cost = att->second.type().cost();

			const bool on_village = map.underlying_terrain(map[movements[i].second.x][movements[i].second.y]) == gamemap::TOWER;

			//up to double the value of a unit based on experience
			cost += (double(att->second.experience())/
			         double(att->second.max_experience()))*cost;

			terrain_quality += (double(stat.chance_to_hit_attacker)/100.0)*cost * (on_village ? 0.5 : 1.0);
			resources_used += cost;

			while(attacks || defends) {
				if(attacks) {
					const int roll = rand()%100;
					if(roll < stat.chance_to_hit_defender) {
						defhp -= stat.damage_defender_takes;
						if(defhp <= 0) {

							//the reward for advancing a unit is to
							//get a 'negative' loss of that unit
							const int xp = defend_it->second.type().level()*10;
							if(xp >= att->second.max_experience() -
							         att->second.experience()) {
								avg_losses -= att->second.type().cost();
							}

							//the reward for killing with a unit that
							//plagues is to get a 'negative' loss of that unit
							if(stat.attacker_plague) {
								avg_losses -= att->second.type().cost();
							}

							defhp = 0;
							break;
						}

						atthp += stat.amount_attacker_drains;
						if(atthp > hitpoints[i])
							atthp = hitpoints[i];
					}

					--attacks;
				}

				if(defends) {
					const int roll = rand()%100;
					if(roll < stat.chance_to_hit_attacker) {
						atthp -= stat.damage_attacker_takes;
						if(atthp <= 0) {
							atthp = 0;

							//penalty for allowing plague is a 'negative' kill
							if(stat.defender_plague) {
								chance_to_kill -= 1.0;
							}
							break;
						}

						defhp += stat.amount_defender_drains;
						if(defhp > target_max_hp)
							defhp = target_max_hp;
					}

					--defends;
				}
			}

			if(defhp <= 0) {
				break;
			} else if(atthp == 0) {
				avg_losses += cost;
			}
			
			//if the attacker moved onto a village, reward it for doing so
			else if(on_village) {
				atthp += game_config::heal_amount*2; //double reward to emphasize getting onto villages
			}

			defenderxp += (atthp == 0 ? 8:1)*att->second.type().level();

			avg_damage_taken += hitpoints[i] - atthp;
		}

		//penalty for allowing advancement is a 'negative' kill, and
		//defender's hitpoints get restored to maximum
		if(defend_it->second.experience() < defend_it->second.max_experience()&&
		   defend_it->second.experience() + defenderxp >=
		   defend_it->second.max_experience()) {
			chance_to_kill -= 1.0;
			defhp = defend_it->second.hitpoints();
		} else if(defhp == 0) {
			chance_to_kill += 1.0;
		} else if(map.underlying_terrain(map[defend_it->first.x][defend_it->first.y]) == gamemap::TOWER) {
			defhp += game_config::heal_amount;
			if(defhp > target_hp)
				defhp = target_hp;
		}

		avg_damage_inflicted += target_hp - defhp;
	}

	chance_to_kill /= num_sims;
	avg_damage_inflicted /= num_sims;
	avg_damage_taken /= num_sims;
	terrain_quality /= resources_used;
	resources_used /= num_sims;
	avg_losses /= num_sims;
}

double ai::attack_analysis::rating(double aggression) const
{
	if(leader_threat) {
		aggression = 1.0;
	}

	double value = chance_to_kill*target_value - avg_losses;

	//prefer to attack already damaged targets
	value += ((target_starting_damage/3 + avg_damage_inflicted)*
					                     (target_value/resources_used) -
	   (1.0-aggression)*avg_damage_taken*(resources_used/target_value))/10.0;

	value += support - vulnerability*terrain_quality;

	value /= ((resources_used/2) + (resources_used/2)*terrain_quality);

	if(leader_threat) {
		value *= 5.0;
	}

	return value;
}

std::vector<ai::attack_analysis> ai::analyze_targets(
	             const move_map& srcdst, const move_map& dstsrc,
	             const move_map& enemy_srcdst, const move_map& enemy_dstsrc
                )
{
	log_scope("analyzing targets...");

	weapon_choice_cache.clear();

	std::vector<attack_analysis> res;

	std::vector<location> unit_locs;
	for(unit_map::const_iterator i = units_.begin(); i != units_.end(); ++i) {
		if(i->second.side() == team_num_) {
			unit_locs.push_back(i->first);
		}
	}

	bool used_locations[6];
	std::fill(used_locations,used_locations+6,false);

	for(unit_map::const_iterator j = units_.begin(); j != units_.end(); ++j) {

		//attack anyone who is on the enemy side, and who is not invisible or turned to stone
		if(current_team().is_enemy(j->second.side()) && j->second.stone() == false &&
		   j->second.invisible(map_.underlying_terrain(map_[j->first.x][j->first.y]), 
				state_.get_time_of_day().lawful_bonus,j->first,
				units_,teams_) == false) {
			std::cerr << "analyzing attack on " << j->first.x+1 << "," << j->first.y+1 << "\n";
			location adjacent[6];
			get_adjacent_tiles(j->first,adjacent);
			attack_analysis analysis;
			analysis.target = j->first;
			analysis.vulnerability = 0.0;
			analysis.support = 0.0;

			const int ticks = SDL_GetTicks();

			do_attack_analysis(j->first,srcdst,dstsrc,enemy_srcdst,enemy_dstsrc,
			            adjacent,used_locations,unit_locs,res,analysis);

			const int time_taken = SDL_GetTicks() - ticks;
			static int max_time = 0;
			if(time_taken > max_time)
				max_time = time_taken;

			std::cerr << "do_analysis took " << time_taken << " (" << max_time << ")\n";
		}
	}

	return res;
}

double ai::power_projection(const gamemap::location& loc, const move_map& srcdst, const move_map& dstsrc, bool use_terrain) const
{
	static gamemap::location used_locs[6];
	static double ratings[6];
	int num_used_locs = 0;

	static gamemap::location locs[6];
	get_adjacent_tiles(loc,locs);

	double res = 0.0;

	for(int i = 0; i != 6; ++i) {
		if(map_.on_board(locs[i]) == false) {
			continue;
		}

		const gamemap::TERRAIN terrain = map_[locs[i].x][locs[i].y];

		typedef move_map::const_iterator Itor;
		typedef std::pair<Itor,Itor> Range;
		Range its = dstsrc.equal_range(locs[i]);
		
		gamemap::location* const beg_used = used_locs;
		gamemap::location* end_used = used_locs + num_used_locs;

		double best_rating = 0.0;
		gamemap::location best_unit;

		for(int n = 0; n != 2; ++n) {
			for(Itor it = its.first; it != its.second; ++it) {
				if(std::find(beg_used,end_used,it->second) != end_used) {
					continue;
				}

				const unit_map::const_iterator u = units_.find(it->second);

				//unit might have been killed, and no longer exist
				if(u == units_.end()) {
					continue;
				}

				const unit& un = u->second;
	
				const double hp = double(un.hitpoints())/
				                  double(un.max_hitpoints());
				int most_damage = 0;
				for(std::vector<attack_type>::const_iterator att =
				    un.attacks().begin(); att != un.attacks().end(); ++att) {
					const int damage = att->damage()*att->num_attacks();
					if(damage > most_damage)
						most_damage = damage;
				}

				const bool village = map_.underlying_terrain(terrain) == gamemap::TOWER;
				const double village_bonus = (use_terrain && village) ? 2.0 : 1.0;

				const double defense = use_terrain ? double(100 - un.defense_modifier(map_,terrain))/100.0 : 0.5;
				const double rating = village_bonus*hp*defense*double(most_damage);
				if(rating > best_rating) {
					best_rating = rating;
					best_unit = it->second;
				}
			}

			//if this is the second time through, then we are looking at a unit
			//that has already been used, but for whom we may have found
			//a better position to attack from
			if(n == 1 && best_unit.valid()) {
				end_used = beg_used + num_used_locs;
				gamemap::location* const pos = std::find(beg_used,end_used,best_unit);
				const int index = pos - beg_used;
				if(best_rating >= ratings[index]) {
					res -= ratings[index];
					res += best_rating;
					used_locs[index] = best_unit;
					ratings[index] = best_rating;
				}

				best_unit = gamemap::location();
				break;
			}

			if(best_unit.valid()) {
				break;
			}

			end_used = beg_used;
		}

		if(best_unit.valid()) {
			used_locs[num_used_locs] = best_unit;
			ratings[num_used_locs] = best_rating;
			++num_used_locs;
			res += best_rating;
		}
	}

	return res;
}
