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

#include "global.hpp"

#include "actions.hpp"
#include "ai.hpp"
#include "game_config.hpp"
#include "gamestatus.hpp"
#include "log.hpp"
#include "pathfind.hpp"
#include "util.hpp"
#include "wassert.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <set>

#define LOG_AI LOG_STREAM(info, ai)

const int max_positions = 10000;

//analyze possibility of attacking target on 'loc'
void ai::do_attack_analysis(
	                 const location& loc,
	                 const move_map& srcdst, const move_map& dstsrc,
					 const move_map& fullmove_srcdst, const move_map& fullmove_dstsrc,
	                 const move_map& enemy_srcdst, const move_map& enemy_dstsrc,
					 const location* tiles, bool* used_locations,
	                 std::vector<location>& units,
	                 std::vector<attack_analysis>& result,
					 attack_analysis& cur_analysis
	                )
{
	//this function is called fairly frequently, so interact with the user here.
	user_interact();

	if(cur_analysis.movements.size() >= size_t(attack_depth()))
		return;

	static double best_results[6];
	if(result.empty()) {
		for(int i = 0; i != 6; ++i) {
			best_results[i] = 0.0;
		}
	}

	const size_t max_positions = 1000;
	if(result.size() > max_positions && !cur_analysis.movements.empty()) {
		LOG_AI << "cut analysis short with number of positions\n";
		return;
	}

	const double cur_rating = cur_analysis.movements.empty() ? -1.0 :
	                          cur_analysis.rating(current_team().aggression(),*this);

	double rating_to_beat = cur_rating;

	if(!cur_analysis.movements.empty()) {
		wassert(cur_analysis.movements.size() < 6);
		double& best_res = best_results[cur_analysis.movements.size()-1];
		rating_to_beat = best_res = maximum(best_res,cur_rating);
	}

	for(size_t i = 0; i != units.size(); ++i) {
		const location current_unit = units[i];

		const unit_map::const_iterator unit_itor = units_.find(current_unit);
		wassert(unit_itor != units_.end());

		//see if the unit has the backstab ability -- units with backstab
		//will want to try to have a friendly unit opposite the position they move to
		//see if the unit has the slow ability -- units with slow only attack first
		bool backstab = false, slow = false;
		const std::vector<attack_type>& attacks = unit_itor->second.attacks();
		for(std::vector<attack_type>::const_iterator a = attacks.begin(); a != attacks.end(); ++a) {
			if(a->backstab()) {
				backstab = true;
			}

			if(a->slow()) {
				slow = true;
			}
		}

		if(slow && cur_analysis.movements.empty() == false) {
			continue;
		}

               //check if the friendly unit is surrounded, a unit is surrounded if
               //it is flanked by enemy units and at least one other enemy unit is
               //nearby or if the unit is totaly surrounded by enemies with max. one
               //tile to escape.
               bool is_surrounded = false;
               bool is_flanked = false;
               int enemy_units_around = 0;
               int accessible_tiles = 0;
               gamemap::location adj[6];
               get_adjacent_tiles(current_unit, adj);

               size_t tile;
               for(tile = 0; tile != 3; ++tile) {

                       const unit_map::const_iterator tmp_unit = units_.find(adj[tile]);
                       bool possible_flanked = false;

                       if(map_.on_board(adj[tile]))
                       {
                               accessible_tiles++;
                               if(tmp_unit != units_.end() && team_num_ != tmp_unit->second.side())
                               {
                                       enemy_units_around++;
                                       possible_flanked = true;
                               }
                       }

                       const unit_map::const_iterator tmp_opposite_unit = units_.find(adj[tile + 3]);
                        if(map_.on_board(adj[tile + 3]))
                       {
                               accessible_tiles++;
                               if(tmp_opposite_unit != units_.end() && team_num_ != tmp_opposite_unit->second.side())
                               {
                                       enemy_units_around++;
                                       if(possible_flanked)
                                       {
                                               is_flanked = true;
                                       }
                               }
                       }
               }

               if(is_flanked && enemy_units_around > 2 || enemy_units_around >= accessible_tiles - 1)
                       is_surrounded = true;



		double best_vulnerability = 0.0, best_support = 0.0;
		int best_rating = 0;
		int cur_position = -1;

		//iterate over positions adjacent to the unit, finding the best rated one
		for(int j = 0; j != 6; ++j) {

			//if in this planned attack, a unit is already in this location
			if(used_locations[j]) {
				continue;
			}

			//see if the current unit can reach that position
			typedef std::multimap<location,location>::const_iterator Itor;
			std::pair<Itor,Itor> its = dstsrc.equal_range(tiles[j]);
			while(its.first != its.second) {
				if(its.first->second == current_unit)
					break;
				++its.first;
			}

			//if the unit can't move to this location
			if(its.first == its.second) {
				continue;
			}

			//check to see whether this move would be a backstab
			int backstab_bonus = 1;
			double surround_bonus = 1.0;

			if(tiles[(j+3)%6] != current_unit) {
				const unit_map::const_iterator itor = units_.find(tiles[(j+3)%6]);

				//note that we *could* also check if a unit plans to move there before we're
				//at this stage, but we don't because, since the attack calculations don't
				//actually take backstab into account (too complicated), this could actually
				//make our analysis look *worse* instead of better.
				//so we only check for 'concrete' backstab opportunities.
				//That would also break backstab_check, since it
				//assumes the defender is in place.
				if(itor != units_.end() && 
					backstab_check(tiles[j], loc, units_, teams_)) {
					if(backstab) {
						backstab_bonus = 2;
					}

					surround_bonus = 1.2;
				}


			}

			//see if this position is the best rated we've seen so far
			const int rating = rate_terrain(unit_itor->second,tiles[j]) * backstab_bonus;
			if(cur_position >= 0 && rating < best_rating) {
				continue;
			}

			//find out how vulnerable we are to attack from enemy units in this hex
			const double vulnerability = power_projection(tiles[j],enemy_srcdst,enemy_dstsrc);

			//calculate how much support we have on this hex from allies. Support does not
			//take into account terrain, because we don't want to move into a hex that is
			//surrounded by good defensive terrain
			const double support = power_projection(tiles[j],fullmove_srcdst,fullmove_dstsrc,false);

			//if this is a position with equal defense to another position, but more vulnerability
			//then we don't want to use it
			if(cur_position >= 0 && rating == best_rating && vulnerability/surround_bonus - support*surround_bonus >= best_vulnerability - best_support) {
				continue;
			}

			cur_position = j;
			best_rating = rating;
			best_vulnerability = vulnerability/surround_bonus;
			best_support = support*surround_bonus;
		}

		if(cur_position != -1) {
			units.erase(units.begin() + i);

			cur_analysis.movements.push_back(std::pair<location,location>(current_unit,tiles[cur_position]));

			cur_analysis.vulnerability += best_vulnerability;

			cur_analysis.support += best_support;

			cur_analysis.is_surrounded = is_surrounded;

			cur_analysis.analyze(map_, units_, 50, *this, dstsrc, srcdst, enemy_dstsrc, enemy_srcdst);

			if(cur_analysis.rating(current_team().aggression(),*this) > rating_to_beat) {

				result.push_back(cur_analysis);
				used_locations[cur_position] = true;
				do_attack_analysis(loc,srcdst,dstsrc,fullmove_srcdst,fullmove_dstsrc,enemy_srcdst,enemy_dstsrc,
				                   tiles,used_locations,
				                   units,result,cur_analysis);
				used_locations[cur_position] = false;
			}

			cur_analysis.vulnerability -= best_vulnerability;
			cur_analysis.support -= best_support;

			cur_analysis.movements.pop_back();

			units.insert(units.begin() + i, current_unit);
		}
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
					  battle_stats& cur_stats, gamemap::TERRAIN terrain, bool use_cache)
{
	const std::map<location,unit>::const_iterator itor = units_.find(att);
	if(itor == units_.end())
		return -1;

	static int cache_hits = 0;
	static int cache_misses = 0;

	if(use_cache == false) {
		weapon_choice_cache.clear();
	}

	battle_type battle(att,def,terrain);
	const std::set<battle_type>::const_iterator cache_itor = weapon_choice_cache.find(battle);

	if(cache_itor != weapon_choice_cache.end()) {
		wassert(*cache_itor == battle);

		++cache_hits;
		cur_stats = cache_itor->stats;

		if(!(size_t(cache_itor->weapon) < itor->second.attacks().size())) {
			LOG_STREAM(err, ai) << "cached illegal weapon: " << cache_itor->weapon
			          << "/" << itor->second.attacks().size() << "\n";
		}

		wassert(size_t(cache_itor->weapon) < itor->second.attacks().size());
		return cache_itor->weapon;
	}

	++cache_misses;

	if((cache_misses%100) == 0) {
		LOG_AI << "cache_stats: " << cache_hits << ":" << cache_misses << " " << weapon_choice_cache.size() << "\n";
	}

	int current_choice = -1;
	double current_rating = 0.0;
	const std::vector<attack_type>& attacks = itor->second.attacks();
	wassert(!attacks.empty());

	const unit_map::const_iterator d_itor = units_.find(def);
	int d_hitpoints = d_itor->second.hitpoints();
	int a_hitpoints = itor->second.hitpoints();

	for(size_t a = 0; a != attacks.size(); ++a) {
		const battle_stats stats = evaluate_battle_stats(map_, teams_, att, def, a, units_,
		                                                 state_, terrain);

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

	wassert(size_t(current_choice) < attacks.size());

	battle.stats = cur_stats;
	battle.weapon = current_choice;
	weapon_choice_cache.insert(battle);

	return current_choice;
}

void ai::attack_analysis::analyze(const gamemap& map, unit_map& units, int num_sims, ai& ai_obj,
                                  const ai::move_map& dstsrc, const ai::move_map& srcdst,
                                  const ai::move_map& enemy_dstsrc, const ai::move_map& enemy_srcdst)
{
	const unit_map::const_iterator defend_it = units.find(target);
	wassert(defend_it != units.end());

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
	uses_leader = false;

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
		const int weapon = ai_obj.choose_weapon(m->first,target, bat_stats, map[m->second.x][m->second.y],true);

		wassert(weapon != -1);
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

			if(att->second.can_recruit()) {
				uses_leader = true;
			}

			const bool on_village = map.is_village(movements[i].second);

			//up to double the value of a unit based on experience
			cost += (double(att->second.experience())/
			         double(att->second.max_experience()))*cost;

			terrain_quality += (double(stat.chance_to_hit_attacker)/100.0)*cost * (on_village ? 0.5 : 1.0);
			resources_used += cost;

			bool defender_strikes_first = stat.defender_strikes_first;

			while(attacks || defends) {
				if(attacks && !defender_strikes_first) {
					const int roll = rand()%100;
					if(roll < stat.chance_to_hit_defender) {
						defhp -= stat.damage_defender_takes;
						if(defhp <= 0) {
							defhp = 0;
							break;
						}

						atthp += stat.amount_attacker_drains;
						if(atthp > hitpoints[i]) {
							atthp = hitpoints[i];
						}

					}

					--attacks;
				}

				defender_strikes_first = false;

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
						if(defhp > target_max_hp) {
							defhp = target_max_hp;
						}
					}

					--defends;
				}
			}

			const int xp = defend_it->second.type().level() * (defhp <= 0 ? game_config::kill_experience : 1);

			const int xp_for_advance = att->second.max_experience() - att->second.experience();

			//the reward for advancing a unit is to get a 'negative' loss of that unit
			if(att->second.type().advances_to().empty() == false) {
				if(xp >= xp_for_advance) {
					avg_losses -= att->second.type().cost();

					//ignore any damage done to this unit
					atthp = hitpoints[i];
				} else {
					//the reward for getting a unit closer to advancement is to get
					//the proportion of remaining experience needed, and multiply
					//it by a quarter of the unit cost. This will cause the AI
					//to heavily favor getting xp for close-to-advance units.
					avg_losses -= (att->second.type().cost()*xp)/(xp_for_advance*4);
				}
			}

			if(defhp <= 0) {
				//the reward for killing with a unit that
				//plagues is to get a 'negative' loss of that unit
				if(stat.attacker_plague) {
					avg_losses -= att->second.type().cost();
				}

				break;
			} else if(atthp == 0) {
				avg_losses += cost;
			}

			//if the attacker moved onto a village, reward it for doing so
			else if(on_village) {
				atthp += game_config::cure_amount*2; //double reward to emphasize getting onto villages
			}

			defenderxp += (atthp == 0 ? game_config::kill_experience:1)*att->second.type().level();

			avg_damage_taken += hitpoints[i] - atthp;
		}

		//penalty for allowing advancement is a 'negative' kill, and
		//defender's hitpoints get restored to maximum
		if(defend_it->second.type().advances_to().empty() == false &&
		   defend_it->second.experience() < defend_it->second.max_experience() &&
		   defend_it->second.experience() + defenderxp >=
		   defend_it->second.max_experience()) {
			chance_to_kill -= 1.0;
			defhp = defend_it->second.hitpoints();
		} else if(defhp == 0) {
			chance_to_kill += 1.0;
		} else if(map.is_village(defend_it->first)) {
			defhp += game_config::cure_amount;
			if(defhp > target_hp)
				defhp = target_hp;
		}

		avg_damage_inflicted += target_hp - defhp;
	}

	//calculate the 'alternative_terrain_quality' -- the best possible defensive values
	//the attacking units could hope to achieve if they didn't attack and moved somewhere.
	//this is could for comparative purposes to see just how vulnerable the AI is
	//making itself

	alternative_terrain_quality = 0.0;
	double cost_sum = 0.0;
	for(size_t i = 0; i != movements.size(); ++i) {
		const unit_map::const_iterator att = units.find(movements[i].first);
		const double cost = att->second.type().cost();
		cost_sum += cost;
		alternative_terrain_quality += cost*ai_obj.best_defensive_position(att->first,dstsrc,srcdst,enemy_dstsrc,enemy_srcdst).chance_to_hit;
	}

	alternative_terrain_quality /= cost_sum*100;

	chance_to_kill /= num_sims;
	avg_damage_inflicted /= num_sims;
	avg_damage_taken /= num_sims;
	terrain_quality /= resources_used;
	resources_used /= num_sims;
	avg_losses /= num_sims;

	if(uses_leader) {
		leader_threat = false;
	}
}

double ai::attack_analysis::rating(double aggression, ai& ai_obj) const
{
	if(leader_threat) {
		aggression = 1.0;
	}

	//only use the leader if we do a serious amount of damage
	//compared to how much they do to us.
	if(uses_leader && aggression > -4.0) {
		LOG_AI << "uses leader..\n";
		aggression = -4.0;
	}

	double value = chance_to_kill*target_value - avg_losses*(1.0-aggression);

	if(terrain_quality > alternative_terrain_quality) {
		//this situation looks like it might be a bad move: we are moving our attackers out
		//of their optimal terrain into sub-optimal terrain.
		//calculate the 'exposure' of our units to risk

		const double exposure_mod = uses_leader ? 2.0 : ai_obj.current_team().caution();
		const double exposure = exposure_mod*resources_used*(terrain_quality - alternative_terrain_quality)*vulnerability/maximum<double>(0.01,support);
		LOG_AI << "attack option has base value " << value << " with exposure " << exposure << ": "
			<< vulnerability << "/" << support << " = " << (vulnerability/maximum<double>(support,0.1)) << "\n";
		if(uses_leader) {
			ai_obj.log_message("attack option has value " + str_cast(value) + " with exposure " + str_cast(exposure) + ": " + str_cast(vulnerability) + "/" + str_cast(support));
		}

		value -= exposure*(1.0-aggression);
	}

	//if this attack uses our leader, and the leader can reach the keep, and has gold to spend, reduce
	//the value to reflect the leader's lost recruitment opportunity in the case of an attack
	if(uses_leader && ai_obj.leader_can_reach_keep() && ai_obj.current_team().gold() > 20) {
		value -= double(ai_obj.current_team().gold())*0.5;
	}

	//prefer to attack already damaged targets
	value += ((target_starting_damage/3 + avg_damage_inflicted) - (1.0-aggression)*avg_damage_taken)/10.0;

       //if the unit is surrounded and there is no support or if the unit is surrounded and the average damage
       //is 0, the unit skips its sanity check and tries to break free as good as possible
       if(!is_surrounded || (support != 0 && avg_damage_taken != 0))
       {
               //sanity check: if we're putting ourselves at major risk, and have no chance to kill, and we're not
               //aiding our allies who are also attacking, then don't do it
               if(vulnerability > 50.0 && vulnerability > support*2.0 && chance_to_kill < 0.02 && aggression < 0.75 && !ai_obj.attack_close(target)) {
                       return -1.0;
               }

               if(!leader_threat && vulnerability*terrain_quality > 0.0) {
                       value *= support/(vulnerability*terrain_quality);
               }
        }

	if(!leader_threat && vulnerability*terrain_quality > 0.0) {
		value *= support/(vulnerability*terrain_quality);
	}

	value /= ((resources_used/2) + (resources_used/2)*terrain_quality);

	if(leader_threat) {
		value *= 5.0;
	}

	LOG_AI << "attack on " << target << ": attackers: " << movements.size()
		<< " value: " << value << " chance to kill: " << chance_to_kill << " damage inflicted: "
		<< avg_damage_inflicted << " damage taken: " << avg_damage_taken << " vulnerability: "
		<< vulnerability << " support: " << support << " quality: " << terrain_quality
		<< " alternative quality: " << alternative_terrain_quality << "\n";

	return value;
}

std::vector<ai::attack_analysis> ai::analyze_targets(
	             const move_map& srcdst, const move_map& dstsrc,
	             const move_map& enemy_srcdst, const move_map& enemy_dstsrc
                )
{
	log_scope2(ai, "analyzing targets...");

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

	std::map<location,paths> dummy_moves;
	move_map fullmove_srcdst, fullmove_dstsrc;
	calculate_possible_moves(dummy_moves,fullmove_srcdst,fullmove_dstsrc,false,true);

	for(unit_map::const_iterator j = units_.begin(); j != units_.end(); ++j) {

		//attack anyone who is on the enemy side, and who is not invisible or turned to stone
		if(current_team().is_enemy(j->second.side()) && j->second.stone() == false &&
		   j->second.invisible(map_.underlying_terrain(map_[j->first.x][j->first.y]),
				state_.get_time_of_day().lawful_bonus,j->first,
				units_,teams_) == false) {
			location adjacent[6];
			get_adjacent_tiles(j->first,adjacent);
			attack_analysis analysis;
			analysis.target = j->first;
			analysis.vulnerability = 0.0;
			analysis.support = 0.0;

			const int ticks = SDL_GetTicks();

			do_attack_analysis(j->first,srcdst,dstsrc,fullmove_srcdst,fullmove_dstsrc,enemy_srcdst,enemy_dstsrc,
			            adjacent,used_locations,unit_locs,res,analysis);

			const int time_taken = SDL_GetTicks() - ticks;
			static int max_time = 0;
			if(time_taken > max_time)
				max_time = time_taken;
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

	const int lawful_bonus = state_.get_time_of_day().lawful_bonus;

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

				int tod_modifier = 0;
				if(un.type().alignment() == unit_type::LAWFUL) {
					tod_modifier = lawful_bonus;
				} else if(un.type().alignment() == unit_type::CHAOTIC) {
					tod_modifier = -lawful_bonus;
				}

				const double hp = double(un.hitpoints())/
				                  double(un.max_hitpoints());
				int most_damage = 0;
				for(std::vector<attack_type>::const_iterator att =
				    un.attacks().begin(); att != un.attacks().end(); ++att) {
					const int damage = (att->damage()*att->num_attacks()*(100+tod_modifier))/100;
					if(damage > most_damage) {
						most_damage = damage;
					}
				}

				const bool village = map_.is_village(terrain);
				const double village_bonus = (use_terrain && village) ? 1.5 : 1.0;

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
