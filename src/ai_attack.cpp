/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version
   or at your option any later version2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file ai_attack.cpp
//! Calculate & analyse attacks.

#include "global.hpp"

#include "ai.hpp"
#include "attack_prediction.hpp"
#include "game_config.hpp"
#include "gamestatus.hpp"
#include "log.hpp"

#include <cassert>

#define LOG_AI LOG_STREAM(info, ai)

const int max_positions = 10000;

//! Analyze possibility of attacking target on 'loc'.
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
	// This function is called fairly frequently, so interact with the user here.
	raise_user_interact();

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
		assert(cur_analysis.movements.size() < 6);
		double& best_res = best_results[cur_analysis.movements.size()-1];
		rating_to_beat = best_res = maximum(best_res,cur_rating);
	}

	for(size_t i = 0; i != units.size(); ++i) {
		const location current_unit = units[i];

		unit_map::iterator unit_itor = units_.find(current_unit);
		assert(unit_itor != units_.end());

		// See if the unit has the backstab ability.
		// Units with backstab will want to try to have a
		// friendly unit opposite the position they move to.
		//
		// See if the unit has the slow ability -- units with slow only attack first.
		bool backstab = false, slow = false;
		std::vector<attack_type>& attacks = unit_itor->second.attacks();
		for(std::vector<attack_type>::iterator a = attacks.begin(); a != attacks.end(); ++a) {
			a->set_specials_context(gamemap::location(),gamemap::location(),
															&gameinfo_,&units_,&map_,&state_,&teams_,true,NULL);
			if(a->get_special_bool("backstab")) {
				backstab = true;
			}

			if(a->get_special_bool("slow")) {
				slow = true;
			}
		}

		if(slow && cur_analysis.movements.empty() == false) {
			continue;
		}

               // Check if the friendly unit is surrounded,
			   // A unit is surrounded if it is flanked by enemy units
			   // and at least one other enemy unit is nearby
			   // or if the unit is totaly surrounded by enemies
			   // with max. one tile to escape.
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

		// Iterate over positions adjacent to the unit, finding the best rated one.
		for(int j = 0; j != 6; ++j) {

			// If in this planned attack, a unit is already in this location.
			if(used_locations[j]) {
				continue;
			}

			// See if the current unit can reach that position.
			if (tiles[j] != current_unit) {
				typedef std::multimap<location,location>::const_iterator Itor;
				std::pair<Itor,Itor> its = dstsrc.equal_range(tiles[j]);
				while(its.first != its.second) {
					if(its.first->second == current_unit)
						break;
					++its.first;
				}

				// If the unit can't move to this location.
				if(its.first == its.second || units_.find(tiles[j]) != units_.end()) {
					continue;
				}
			}

			// Check to see whether this move would be a backstab.
			int backstab_bonus = 1;
			double surround_bonus = 1.0;

			if(tiles[(j+3)%6] != current_unit) {
				const unit_map::const_iterator itor = units_.find(tiles[(j+3)%6]);

				// Note that we *could* also check if a unit plans to move there
				// before we're at this stage, but we don't because, since the
				// attack calculations don't actually take backstab into account (too complicated),
				// this could actually make our analysis look *worse* instead of better.
				// So we only check for 'concrete' backstab opportunities.
				// That would also break backstab_check, since it assumes
				// the defender is in place.
				if(itor != units_.end() &&
					backstab_check(tiles[j], loc, units_, teams_)) {
					if(backstab) {
						backstab_bonus = 2;
					}

					surround_bonus = 1.2;
				}


			}

			// See if this position is the best rated we've seen so far.
			const int rating = rate_terrain(unit_itor->second,tiles[j]) * backstab_bonus;
			if(cur_position >= 0 && rating < best_rating) {
				continue;
			}

			// Find out how vulnerable we are to attack from enemy units in this hex.
			const double vulnerability = power_projection(tiles[j],enemy_dstsrc);

			// Calculate how much support we have on this hex from allies.
			// Support does not take into account terrain, because we don't want
			// to move into a hex that is surrounded by good defensive terrain.
			const double support = power_projection(tiles[j],fullmove_dstsrc,false);

			// If this is a position with equal defense to another position,
			// but more vulnerability then we don't want to use it.
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

			cur_analysis.analyze(map_, units_, teams_, state_, gameinfo_, *this, dstsrc, srcdst, enemy_dstsrc, current_team().aggression());

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


void ai::attack_analysis::analyze(const gamemap& map, unit_map& units,
								  const std::vector<team>& teams,
								  const gamestatus& status, const game_data& gamedata,
								  class ai& ai_obj,
                                  const move_map& dstsrc, const move_map& srcdst,
                                  const move_map& enemy_dstsrc, double aggression)
{
	const unit_map::const_iterator defend_it = units.find(target);
	assert(defend_it != units.end());

	// See if the target is a threat to our leader or an ally's leader.
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

	target_value = defend_it->second.cost();
	target_value += (double(defend_it->second.experience())/
	                 double(defend_it->second.max_experience()))*target_value;
	target_starting_damage = defend_it->second.max_hitpoints() -
	                         defend_it->second.hitpoints();

	// Calculate the 'alternative_terrain_quality' -- the best possible defensive values
	// the attacking units could hope to achieve if they didn't attack and moved somewhere.
	// This is used for comparative purposes, to see just how vulnerable the AI is
	// making itself.
	alternative_terrain_quality = 0.0;
	double cost_sum = 0.0;
	for(size_t i = 0; i != movements.size(); ++i) {
		const unit_map::const_iterator att = units.find(movements[i].first);
		const double cost = att->second.cost();
		cost_sum += cost;
		alternative_terrain_quality += cost*ai_obj.best_defensive_position(movements[i].first,dstsrc,srcdst,enemy_dstsrc).chance_to_hit;
	}
	alternative_terrain_quality /= cost_sum*100;

	avg_damage_inflicted = 0.0;
	avg_damage_taken = 0.0;
	resources_used = 0.0;
	terrain_quality = 0.0;
	avg_losses = 0.0;
	chance_to_kill = 0.0;

	double def_avg_experience = 0.0;
	double first_chance_kill = 0.0;

	double prob_dead_already = 0.0;
	assert(!movements.empty());
	std::vector<std::pair<location,location> >::const_iterator m;

	battle_context *prev_bc = NULL;
	const combatant *prev_def = NULL;

	for (m = movements.begin(); m != movements.end(); ++m) {
		// We fix up units map to reflect what this would look like.
		std::pair<gamemap::location,unit> *up = units.extract(m->first);
		up->first = m->second;
		units.add(up);

		if (up->second.can_recruit()) {
			uses_leader = true;
			leader_threat = false;
		}

		int att_weapon = -1, def_weapon = -1;
		bool from_cache = false;
		battle_context *bc;

		// This cache is only about 99% correct, but speeds up evaluation by about 1000 times.
		// We recalculate when we actually attack.
		std::map<std::pair<location, const unit_type *>,std::pair<battle_context::unit_stats,battle_context::unit_stats> >::iterator usc;
		if(up->second.type()) {
			usc = ai_obj.unit_stats_cache_.find(std::pair<location, const unit_type *>(target, up->second.type()));
		} else {
			usc = ai_obj.unit_stats_cache_.end();
		}
		// Just check this attack is valid for this attacking unit (may be modified)
		if (usc != ai_obj.unit_stats_cache_.end() &&
				usc->second.first.attack_num <
				static_cast<int>(up->second.attacks().size())) {

			from_cache = true;
			bc = new battle_context(usc->second.first, usc->second.second);
		} else {
			bc = new battle_context(map, teams, units, status, gamedata, m->second, target, att_weapon, def_weapon, aggression, prev_def);
		}
		const combatant &att = bc->get_attacker_combatant(prev_def);
		const combatant &def = bc->get_defender_combatant(prev_def);

		delete prev_bc;
		prev_bc = bc;
		prev_def = &bc->get_defender_combatant(prev_def);

		if (!from_cache && up->second.type()) {
			ai_obj.unit_stats_cache_.insert(std::pair<std::pair<location, const unit_type *>,std::pair<battle_context::unit_stats,battle_context::unit_stats> >
											(std::pair<location, const unit_type *>(target, up->second.type()),
											 std::pair<battle_context::unit_stats,battle_context::unit_stats>(bc->get_attacker_stats(),
																											  bc->get_defender_stats())));
		}

		// Note we didn't fight at all if defender is already dead.
		double prob_fought = (1.0 - prob_dead_already);

		//! @todo FIXME: add combatant.prob_killed
		double prob_killed = def.hp_dist[0] - prob_dead_already;
		prob_dead_already = def.hp_dist[0];

		double prob_died = att.hp_dist[0];
		double prob_survived = (1.0 - prob_died) * prob_fought;

		double cost = up->second.cost();
		const bool on_village = map.is_village(m->second);
		// Up to double the value of a unit based on experience
		cost += (double(up->second.experience())/double(up->second.max_experience()))*cost;
		resources_used += cost;
		avg_losses += cost * prob_died;

		// Double reward to emphasize getting onto villages if they survive.
		if (on_village) {
			avg_damage_taken -= game_config::poison_amount*2 * prob_survived;
		}

		terrain_quality += (double(bc->get_defender_stats().chance_to_hit)/100.0)*cost * (on_village ? 0.5 : 1.0);

		double advance_prob = 0.0;
		// The reward for advancing a unit is to get a 'negative' loss of that unit
		if (!up->second.advances_to().empty()) {
			int xp_for_advance = up->second.max_experience() - up->second.experience();
			int kill_xp, fight_xp;

			// See bug #6272... in some cases, unit already has got enough xp to advance,
			// but hasn't (bug elsewhere?).  Can cause divide by zero.
			if (xp_for_advance <= 0)
				xp_for_advance = 1;

			fight_xp = defend_it->second.level();
			kill_xp = fight_xp * game_config::kill_experience;

			if (fight_xp >= xp_for_advance)
				advance_prob = prob_fought;
			else if (kill_xp >= xp_for_advance)
				advance_prob = prob_killed;
			avg_losses -= up->second.cost() * advance_prob;

			// The reward for getting a unit closer to advancement
			// (if it didn't advance) is to get the proportion of
			// remaining experience needed, and multiply it by
			// a quarter of the unit cost.
			// This will cause the AI to heavily favor
			// getting xp for close-to-advance units.
			avg_losses -= (up->second.cost()*fight_xp)/(xp_for_advance*4) * (prob_fought - prob_killed);
			avg_losses -= (up->second.cost()*kill_xp)/(xp_for_advance*4) * prob_killed;

			// The reward for killing with a unit that plagues
			// is to get a 'negative' loss of that unit.
			if (bc->get_attacker_stats().plagues) {
				avg_losses -= prob_killed * up->second.cost();
			}
		}

		// If we didn't advance, we took this damage.
		avg_damage_taken += (up->second.hitpoints() - att.average_hp()) * (1.0 - advance_prob);

		//! @todo FIXME: attack_prediction.cpp should understand advancement directly.
		// For each level of attacker def gets 1 xp or kill_experience.
		def_avg_experience += up->second.level() *
			(1.0 - att.hp_dist[0] + game_config::kill_experience * att.hp_dist[0]);
		if (m == movements.begin()) {
			first_chance_kill = def.hp_dist[0];
		}
	}

	if (!defend_it->second.advances_to().empty() &&
		def_avg_experience >= defend_it->second.max_experience() - defend_it->second.experience()) {
		// It's likely to advance: only if we can kill with first blow.
		chance_to_kill = first_chance_kill;
		// Negative average damage (it will advance).
		avg_damage_inflicted = defend_it->second.hitpoints() - defend_it->second.max_hitpoints();
	} else {
		chance_to_kill = prev_def->hp_dist[0];
		avg_damage_inflicted = defend_it->second.hitpoints() - prev_def->average_hp(map.gives_healing(defend_it->first));
	}

	delete prev_bc;
	terrain_quality /= resources_used;

	// Restore the units to their original positions.
	for (m = movements.begin(); m != movements.end(); ++m) {
		std::pair<gamemap::location,unit> *up = units.extract(m->second);
		up->first = m->first;
		units.add(up);
	}
}

double ai::attack_analysis::rating(double aggression, ai& ai_obj) const
{
	if(leader_threat) {
		aggression = 1.0;
	}

	// Only use the leader if we do a serious amount of damage,
	// compared to how much they do to us.
	if(uses_leader && aggression > -4.0) {
		LOG_AI << "uses leader..\n";
		aggression = -4.0;
	}

	double value = chance_to_kill*target_value - avg_losses*(1.0-aggression);

	if(terrain_quality > alternative_terrain_quality) {
		// This situation looks like it might be a bad move:
		// we are moving our attackers out of their optimal terrain
		// into sub-optimal terrain.
		// Calculate the 'exposure' of our units to risk.

		const double exposure_mod = uses_leader ? 2.0 : ai_obj.current_team().caution();
		const double exposure = exposure_mod*resources_used*(terrain_quality - alternative_terrain_quality)*vulnerability/maximum<double>(0.01,support);
		LOG_AI << "attack option has base value " << value << " with exposure " << exposure << ": "
			<< vulnerability << "/" << support << " = " << (vulnerability/maximum<double>(support,0.1)) << "\n";
		if(uses_leader) {
			ai_obj.log_message("attack option has value " + str_cast(value) + " with exposure " + str_cast(exposure) + ": " + str_cast(vulnerability) + "/" + str_cast(support));
		}

		value -= exposure*(1.0-aggression);
	}

	// If this attack uses our leader, and the leader can reach the keep,
	// and has gold to spend, reduce the value to reflect the leader's
	// lost recruitment opportunity in the case of an attack.
	if(uses_leader && ai_obj.leader_can_reach_keep() && ai_obj.current_team().gold() > 20) {
		value -= double(ai_obj.current_team().gold())*0.5;
	}

	// Prefer to attack already damaged targets.
	value += ((target_starting_damage/3 + avg_damage_inflicted) - (1.0-aggression)*avg_damage_taken)/10.0;

       // If the unit is surrounded and there is no support,
	   // or if the unit is surrounded and the average damage is 0,
	   // the unit skips its sanity check and tries to break free as good as possible.
       if(!is_surrounded || (support != 0 && avg_damage_taken != 0))
       {
               // Sanity check: if we're putting ourselves at major risk,
			   // and have no chance to kill, and we're not aiding our allies
			   // who are also attacking, then don't do it.
               if(vulnerability > 50.0 && vulnerability > support*2.0
			   && chance_to_kill < 0.02 && aggression < 0.75
			   && !ai_obj.attack_close(target)) {
                       return -1.0;
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
		<< " value: " << value << " chance to kill: " << chance_to_kill
		<< " damage inflicted: " << avg_damage_inflicted
		<< " damage taken: " << avg_damage_taken
		<< " vulnerability: " << vulnerability
		<< " support: " << support
		<< " quality: " << terrain_quality
		<< " alternative quality: " << alternative_terrain_quality << "\n";

	return value;
}

std::vector<ai::attack_analysis> ai::analyze_targets(
	             const move_map& srcdst, const move_map& dstsrc,
	             const move_map& enemy_srcdst, const move_map& enemy_dstsrc
                )
{
	log_scope2(ai, "analyzing targets...");

	std::vector<attack_analysis> res;

	std::vector<location> unit_locs;
	for(unit_map::const_iterator i = units_.begin(); i != units_.end(); ++i) {
		if(i->second.side() == team_num_ && i->second.attacks_left()) {
			unit_locs.push_back(i->first);
		}
	}

	bool used_locations[6];
	std::fill(used_locations,used_locations+6,false);

	std::map<location,paths> dummy_moves;
	move_map fullmove_srcdst, fullmove_dstsrc;
	calculate_possible_moves(dummy_moves,fullmove_srcdst,fullmove_dstsrc,false,true);

	unit_stats_cache_.clear();

	for(unit_map::const_iterator j = units_.begin(); j != units_.end(); ++j) {

		// Attack anyone who is on the enemy side,
		// and who is not invisible or turned to stone.
		if(current_team().is_enemy(j->second.side()) && !j->second.incapacitated() &&
		   j->second.invisible(j->first,units_,teams_) == false) {
			location adjacent[6];
			get_adjacent_tiles(j->first,adjacent);
			attack_analysis analysis;
			analysis.target = j->first;
			analysis.vulnerability = 0.0;
			analysis.support = 0.0;

//			const int ticks = SDL_GetTicks();

			do_attack_analysis(j->first,srcdst,dstsrc,fullmove_srcdst,fullmove_dstsrc,enemy_srcdst,enemy_dstsrc,
			            adjacent,used_locations,unit_locs,res,analysis);

//			const int time_taken = SDL_GetTicks() - ticks;
//			static int max_time = 0;
//			if(time_taken > max_time)
//				max_time = time_taken;
		}
	}

	return res;
}

double ai::power_projection(const gamemap::location& loc,  const move_map& dstsrc, bool use_terrain) const
{
	gamemap::location used_locs[6];
	int ratings[6];
	int num_used_locs = 0;

	gamemap::location locs[6];
	get_adjacent_tiles(loc,locs);

	const int lawful_bonus = state_.get_time_of_day().lawful_bonus;

	int res = 0;

	bool changed = false;
	for (int i = 0;; ++i) {
		if (i == 6) {
			if (!changed) break;
			// Loop once again, in case a unit found a better spot
			// and freed the place for another unit.
			changed = false;
			i = 0;
		}

		if (map_.on_board(locs[i]) == false) {
			continue;
		}

		const t_translation::t_letter terrain = map_[locs[i]];

		typedef move_map::const_iterator Itor;
		typedef std::pair<Itor,Itor> Range;
		Range its = dstsrc.equal_range(locs[i]);

		gamemap::location* const beg_used = used_locs;
		gamemap::location* end_used = used_locs + num_used_locs;

		int best_rating = 0;
		gamemap::location best_unit;

		for(Itor it = its.first; it != its.second; ++it) {
			const unit_map::const_iterator u = units_.find(it->second);

			// Unit might have been killed, and no longer exist
			if(u == units_.end()) {
				continue;
			}

			const unit& un = u->second;

			int tod_modifier = 0;
			if(un.alignment() == unit_type::LAWFUL) {
				tod_modifier = lawful_bonus;
			} else if(un.alignment() == unit_type::CHAOTIC) {
				tod_modifier = -lawful_bonus;
			}

			int hp = un.hitpoints() * 1000 / un.max_hitpoints();
			int most_damage = 0;
			for(std::vector<attack_type>::const_iterator att =
			    un.attacks().begin(); att != un.attacks().end(); ++att) {
				int damage = att->damage() * att->num_attacks() *
				             (100 + tod_modifier);
				if(damage > most_damage) {
					most_damage = damage;
				}
			}

			int village_bonus = use_terrain && map_.is_village(terrain) ? 3 : 2;
			int defense = use_terrain ? 100 - un.defense_modifier(terrain) : 50;
			int rating = hp * defense * most_damage * village_bonus / 200;
			if(rating > best_rating) {
				gamemap::location *pos = std::find(beg_used, end_used, it->second);
				// Check if the spot is the same or better than an older one.
				if (pos == end_used || rating >= ratings[pos - beg_used]) {
					best_rating = rating;
					best_unit = it->second;
				}
			}
		}

		if (!best_unit.valid()) continue;
		gamemap::location *pos = std::find(beg_used, end_used, best_unit);
		int index = pos - beg_used;
		if (index == num_used_locs)
			++num_used_locs;
		else if (best_rating == ratings[index])
			continue;
		else {
			// The unit was in another spot already, so remove its older rating
			// from the final result, and require a new run to fill its old spot.
			res -= ratings[index];
			changed = true;
		}
		used_locs[index] = best_unit;
		ratings[index] = best_rating;
		res += best_rating;
	}

	return res / 100000.;
}

//! There is no real hope for us: we should try to do some damage to the enemy.
//! We can spend some cycles here, since it's rare.
bool ai::desperate_attack(const gamemap::location &loc)
{
	const unit &u = units_.find(loc)->second;
	std::cerr << "desperate attack by '" << u.id() << "' " << loc << "\n";

	gamemap::location adj[6];
	get_adjacent_tiles(loc, adj);

	double best_kill_prob = 0.0;
	unsigned int best_weapon = 0;
	int best_def_weapon = -1;
	unsigned best_dir = 0;

	{
		for(unsigned int n = 0; n != 6; ++n) {
			const unit_map::iterator enemy = find_visible_unit(units_,adj[n], map_, teams_, current_team());
			if (enemy != units_.end() &&
			   current_team().is_enemy(enemy->second.side()) && !enemy->second.incapacitated()) {
				const std::vector<attack_type>& attacks = u.attacks();
				for (unsigned int i = 0; i != attacks.size(); ++i) {
					// Skip weapons with attack_weight=0
					if (attacks[i].attack_weight() > 0) {
						battle_context bc(map_, teams_, units_, state_, gameinfo_, loc, adj[n], i);
						combatant att(bc.get_attacker_stats());
						combatant def(bc.get_defender_stats());
						att.fight(def);
						if (def.hp_dist[0] > best_kill_prob) {
							best_kill_prob = def.hp_dist[0];
							best_weapon = i;
							best_def_weapon = bc.get_defender_stats().attack_num;
							best_dir = n;
						}
					}
				}
			}
		}
	}

	if (best_kill_prob > 0.0) {
		attack_enemy(loc, adj[best_dir], best_weapon, best_def_weapon);
		return true;
	}

	double least_hp = u.hitpoints() + 1;

	{
		// Who would do most damage to us when they attack?  (approximate: may be different ToD)
		for (unsigned int n = 0; n != 6; ++n) {
			const unit_map::iterator enemy = find_visible_unit(units_,adj[n], map_, teams_, current_team());
			if (enemy != units_.end() &&
			   current_team().is_enemy(enemy->second.side()) && !enemy->second.incapacitated()) {

				const std::vector<attack_type>& attacks = units_.find(adj[n])->second.attacks();
				for (unsigned int i = 0; i != attacks.size(); ++i) {
					// SKip weapons with attack_weight=0
					if (attacks[i].attack_weight() > 0) {
						battle_context bc(map_, teams_, units_, state_, gameinfo_, adj[n], loc, i);
						combatant att(bc.get_attacker_stats());
						combatant def(bc.get_defender_stats());
						att.fight(def);
						if (def.average_hp() < least_hp) {
							least_hp = def.average_hp();
							best_dir = n;
						}
					}
				}
			}
		}
	}

	// It is possible that there were no adjacent units to attack...
	if (least_hp != u.hitpoints() + 1) {
		battle_context bc(map_, teams_, units_, state_, gameinfo_, loc, adj[best_dir], -1, -1, 0.5);
		attack_enemy(loc, adj[best_dir], bc.get_attacker_stats().attack_num,
					 bc.get_defender_stats().attack_num);
		return true;
	}
	return false;
}
