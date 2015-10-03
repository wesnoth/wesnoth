/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * AI Move decision support
 * @file ai/default/move.cpp
 * */


#include "../../global.hpp"

#include "ai.hpp"
#include "../composite/goal.hpp"

#include "../../foreach.hpp"
#include "../../gettext.hpp"
#include "../../log.hpp"
#include "../../map.hpp"
#include "../../terrain_filter.hpp"
#include "../../wml_exception.hpp"


static lg::log_domain log_ai("ai/move");
#define LOG_AI LOG_STREAM(info, log_ai)
#define DBG_AI LOG_STREAM(debug, log_ai)
#define ERR_AI LOG_STREAM(err, log_ai)

namespace ai {

struct move_cost_calculator : pathfind::cost_calculator
{
	move_cost_calculator(const unit& u, const gamemap& map,
			const unit_map& units, const move_map& enemy_dstsrc)
	  : unit_(u), map_(map), units_(units),
	    enemy_dstsrc_(enemy_dstsrc),
		max_moves_(u.total_movement()),
		avoid_enemies_(u.usage() == "scout")
	{}

	double cost(const map_location& loc, const double) const
	{
		const t_translation::t_terrain terrain = map_[loc];

		const double move_cost = unit_.movement_cost(terrain);

		if(move_cost > max_moves_) // impassable
			return getNoPathValue();

		double res = move_cost;
		if(avoid_enemies_){
			res *= 1.0 + enemy_dstsrc_.count(loc);
		}

		//if there is a unit (even a friendly one) on this tile, we increase the cost to
		//try discourage going through units, to thwart the 'single file effect'
		if (units_.count(loc))
			res *= 4.0;

		return res;
	}

private:
	const unit& unit_;
	const gamemap& map_;
	const unit_map& units_;
	const move_map& enemy_dstsrc_;
	const int max_moves_;
	const bool avoid_enemies_;
};


map_location ai_default::form_group(const std::vector<location>& route, const move_map& dstsrc, std::set<location>& res)
{
	if(route.empty()) {
		return location();
	}

	std::vector<location>::const_iterator i;
	for(i = route.begin(); i != route.end(); ++i) {
		if(units_.count(*i) > 0) {
			continue;
		}

		size_t n = 0, nunits = res.size();

		const std::pair<move_map::const_iterator,move_map::const_iterator> itors = dstsrc.equal_range(*i);
		for(move_map::const_iterator j = itors.first; j != itors.second; ++j) {
			if(res.count(j->second) != 0) {
				++n;
			} else {
				const unit_map::const_iterator un = units_.find(j->second);
				if(un == units_.end() || un->second.can_recruit() || un->second.movement_left() < un->second.total_movement()) {
					continue;
				}

				res.insert(j->second);
			}
		}

		//if not all our units can reach this position.
		if(n < nunits) {
			break;
		}
	}

	if(i != route.begin()) {
		--i;
	}

	return *i;
}

void ai_default::enemies_along_path(const std::vector<location>& route, const move_map& dstsrc, std::set<location>& res)
{
	for(std::vector<location>::const_iterator i = route.begin(); i != route.end(); ++i) {
		map_location adj[6];
		get_adjacent_tiles(*i,adj);
		for(size_t n = 0; n != 6; ++n) {
			const std::pair<move_map::const_iterator,move_map::const_iterator> itors = dstsrc.equal_range(adj[n]);
			for(move_map::const_iterator j = itors.first; j != itors.second; ++j) {
				res.insert(j->second);
			}
		}
	}
}

bool ai_default::move_group(const location& dst, const std::vector<location>& route, const std::set<location>& units)
{
	const std::vector<location>::const_iterator itor = std::find(route.begin(),route.end(),dst);
	if(itor == route.end()) {
		return false;
	}

	LOG_AI << "group has " << units.size() << " members\n";

	location next;

	size_t direction = 0;

	//find the direction the group is moving in
	if(itor+1 != route.end()) {
		next = *(itor+1);
	} else if(itor != route.begin()) {
		next = *(itor-1);
	}

	if(next.valid()) {
		location adj[6];
		get_adjacent_tiles(dst,adj);

		direction = std::find(adj,adj+6,next) - adj;
	}

	std::deque<location> preferred_moves;
	preferred_moves.push_back(dst);

	std::map<location,pathfind::paths> possible_moves;
	move_map srcdst, dstsrc;
	calculate_possible_moves(possible_moves,srcdst,dstsrc,false);

	bool gamestate_changed = false;

	for(std::set<location>::const_iterator i = units.begin(); i != units.end(); ++i) {
		const unit_map::const_iterator un = units_.find(*i);
		if(un == units_.end()) {
			continue;
		}

		location best_loc;
		int best_defense = -1;
		for(std::deque<location>::const_iterator j = preferred_moves.begin(); j != preferred_moves.end(); ++j) {
			if(units_.count(*j)) {
				continue;
			}

			const std::pair<move_map::const_iterator,move_map::const_iterator> itors = dstsrc.equal_range(*j);
			move_map::const_iterator m;
			for(m = itors.first; m != itors.second; ++m) {
				if(m->second == *i) {
					break;
				}
			}

			if(m == itors.second) {
				continue;
			}

			const int defense = un->second.defense_modifier(map_.get_terrain(*j));
			if(best_loc.valid() == false || defense < best_defense) {
				best_loc = *j;
				best_defense = defense;
			}
		}

		if(best_loc.valid()) {
		       	const map_location res = move_unit(*i,best_loc,gamestate_changed);

			//if we were ambushed, abort the group's movement.
			if (res != best_loc) {
				return gamestate_changed;
			}

			// FIXME: suokko's r29531 included the following line.  Correct?
			// units_.find(best_loc)->second.set_movement(0);

			preferred_moves.erase(std::find(preferred_moves.begin(),preferred_moves.end(),best_loc));

			//find locations that are 'perpendicular' to the direction of movement for further units to move to.
			location adj[6];
			get_adjacent_tiles(best_loc,adj);
			for(size_t n = 0; n != 6; ++n) {
				if(n != direction && ((n+3)%6) != direction && map_.on_board(adj[n]) &&
				   units_.count(adj[n]) == 0 && std::count(preferred_moves.begin(),preferred_moves.end(),adj[n]) == 0) {
					preferred_moves.push_front(adj[n]);
					LOG_AI << "added moves: " << adj[n].x + 1 << "," << adj[n].y + 1 << "\n";
				}
			}
		} else {
			LOG_AI << "Could not move group member to any of " << preferred_moves.size() << " locations\n";
		}
	}

	return gamestate_changed;
}

double ai_default::rate_group(const std::set<location>& group, const std::vector<location>& battlefield) const
{
	double strength = 0.0;
	for(std::set<location>::const_iterator i = group.begin(); i != group.end(); ++i) {
		const unit_map::const_iterator u = units_.find(*i);
		if(u == units_.end()) {
			continue;
		}

		const unit& un = u->second;

		int defense = 0;
		for(std::vector<location>::const_iterator j = battlefield.begin(); j != battlefield.end(); ++j) {
			defense += un.defense_modifier(map_.get_terrain(*j));
		}

		defense /= battlefield.size();

		int best_attack = 0;
		const std::vector<attack_type>& attacks = un.attacks();
		for(std::vector<attack_type>::const_iterator a = attacks.begin(); a != attacks.end(); ++a) {
			const int strength = a->num_attacks()*a->damage();
			best_attack = std::max<int>(strength,best_attack);
		}

		const int rating = (defense*best_attack*un.hitpoints())/(100*un.max_hitpoints());
		strength += double(rating);
	}

	return strength;
}

double ai_default::compare_groups(const std::set<location>& our_group, const std::set<location>& their_group, const std::vector<location>& battlefield) const
{
	const double a = rate_group(our_group,battlefield);
	const double b = std::max<double>(rate_group(their_group,battlefield),0.01);
	return a/b;
}

// structure storing the maximal possible rating of a target
struct rated_target{
	rated_target(const std::vector<target>::iterator& t, double r) : tg(t), max_rating(r) {};
	std::vector<target>::iterator tg;
	double max_rating;
};

// compare maximal possible rating of targets
// we can be smarter about the equal case, but keep old behavior for the moment
struct rated_target_comparer {
	bool operator()(const rated_target& a, const rated_target& b) const {
		return a.max_rating > b.max_rating;
	}
};

double ai_default::rate_target(const target& tg, const unit_map::iterator& u,
			const move_map& dstsrc, const move_map& enemy_dstsrc,
			const pathfind::plain_route& rt)
{
	double move_cost = rt.move_cost;

	if(move_cost > 0) {
		// if this unit can move to that location this turn, it has a very very low cost
		typedef std::multimap<map_location,map_location>::const_iterator multimapItor;
		std::pair<multimapItor,multimapItor> locRange = dstsrc.equal_range(u->first);
		while (locRange.first != locRange.second) {
			if (locRange.first->second == u->first) {
				move_cost = 0;
				break;
			}
			++locRange.first;
		}
	}

	double rating = tg.value;

	if(rating == 0)
		return rating; // all following operations are only multiplications of 0

	// far target have a lower rating
	if(move_cost > 0) {
		rating /= move_cost;
	}

	//for 'support' targets, they are rated much higher if we can get there within two turns,
	//otherwise they are worthless to go for at all.
	if(tg.type == target::SUPPORT) {
		if(move_cost <= u->second.movement_left()*2) {
			rating *= 10.0;
		} else {
			rating = 0.0;
			return rating;
		}
	}

	//scouts do not like encountering enemies on their paths
	if(u->second.usage() == "scout") {
		//scouts get a bonus for going after villages
		if(tg.type == target::VILLAGE) {
				rating *= get_scout_village_targeting();
		}

		std::set<location> enemies_guarding;
		enemies_along_path(rt.steps,enemy_dstsrc,enemies_guarding);
		// note that an empty route means no guardian and thus optimal rating

		if(enemies_guarding.size() > 1) {
			rating /= enemies_guarding.size();
		} else {
			//scouts who can travel on their route without coming in range of many enemies
			//get a massive bonus, so that they can be processed first, and avoid getting
			//bogged down in lots of grouping
			rating *= 100;
		}
	}

	return rating;
}


std::pair<map_location,map_location> ai_default::choose_move(std::vector<target>& targets, const move_map& srcdst, const move_map& dstsrc, const move_map& enemy_dstsrc)
{
	log_scope2(log_ai, "choosing move");

	raise_user_interact();

	unit_map::iterator u;

	//find the first eligible unit
	for(u = units_.begin(); u != units_.end(); ++u) {
		if(!(u->second.side() != get_side() || u->second.can_recruit() || u->second.movement_left() <= 0 || u->second.incapacitated())) {
			break;
		}
	}

	if(u == units_.end()) {
		LOG_AI  << "no eligible units found\n";
		return std::pair<location,location>();
	}

	//guardian units stay put
	if (u->second.get_state("guardian")) {
		LOG_AI << u->second.type_id() << " is guardian, staying still\n";
		return std::pair<location,location>(u->first,u->first);
	}

	const pathfind::plain_route dummy_route;
	assert(dummy_route.steps.empty() && dummy_route.move_cost == 0);

	// We will sort all targets by a quick maximal possible rating,
	// so we will be able to start real work by the most promising ones
	// and if its real value is better than other maximal values
	// then we can skip them.

	std::vector<rated_target> rated_targets;
	for(std::vector<target>::iterator tg = targets.begin(); tg != targets.end(); ++tg) {
		// passing a dummy route to have the maximal rating
		double max_rating = rate_target(*tg, u, dstsrc, enemy_dstsrc, dummy_route);
		rated_targets.push_back( rated_target(tg, max_rating) );
	}

	//use stable_sort for the moment to preserve old AI behavior
	std::stable_sort(rated_targets.begin(), rated_targets.end(), rated_target_comparer());

	const move_cost_calculator cost_calc(u->second, map_, units_, enemy_dstsrc);

	pathfind::plain_route best_route;
	unit_map::iterator best = units_.end();
	double best_rating = -1.0;

	std::vector<rated_target>::iterator best_rated_target = rated_targets.end();

	std::vector<rated_target>::iterator rated_tg = rated_targets.begin();

	for(; rated_tg != rated_targets.end(); ++rated_tg) {
		const target& tg = *(rated_tg->tg);

		LOG_AI << "Considering target at: " << tg.loc <<"\n";
		assert(map_.on_board(tg.loc));

		raise_user_interact();

		// locStopValue controls how quickly we give up on the A* search, due
		// to it seeming futile. Be very cautious about changing this value,
		// as it can cause the AI to give up on searches and just do nothing.
		const double locStopValue = 500.0;
		pathfind::plain_route real_route = pathfind::a_star_search(u->first, tg.loc, locStopValue, &cost_calc, map_.w(), map_.h());

		if(real_route.steps.empty()) {
			LOG_AI << "Can't reach target: " << locStopValue << " = " << tg.value << "/" << best_rating << "\n";
			continue;
		}

		double real_rating = rate_target(tg, u, dstsrc, enemy_dstsrc, real_route);

		LOG_AI << tg.value << "/" << real_route.move_cost << " = " << real_rating << "\n";

		if(real_rating > best_rating){
			best_rating = real_rating;
			best_rated_target = rated_tg;
			best_route = real_route;
			best = u;
			//prevent divivion by zero
			//FIXME: stupid, should fix it at the division
			if(best_rating == 0)
				best_rating = 0.000000001;

			// if already higher than the maximal values of the next ratings
			// (which are sorted, so only need to check the next one)
			// then we have found the best target.
			if(rated_tg+1 != rated_targets.end() && best_rating >= (rated_tg+1)->max_rating)
				break;
		}
	}

	LOG_AI << "choose target...\n";

	if(best_rated_target == rated_targets.end()) {
		LOG_AI << "no eligible targets found\n";
		return std::pair<location,location>();
	}

	assert(best_rating >= 0);
	std::vector<target>::iterator best_target = best_rated_target->tg;

	//if we have the 'simple_targeting' flag set, then we don't
	//see if any other units can put a better bid forward for this
	//target
	bool simple_targeting = get_simple_targeting();

	if(simple_targeting == false) {
		LOG_AI << "complex targeting...\n";
		//now see if any other unit can put a better bid forward
		for(++u; u != units_.end(); ++u) {
			if (u->second.side() != get_side() || u->second.can_recruit() ||
			    u->second.movement_left() <= 0 || u->second.get_state("guardian") ||
			    u->second.incapacitated())
			{
				continue;
			}

			raise_user_interact();

			const move_cost_calculator calc(u->second, map_, units_, enemy_dstsrc);
			const double locStopValue = std::min(best_target->value / best_rating, (double) 100.0);
			pathfind::plain_route cur_route = a_star_search(u->first, best_target->loc, locStopValue, &calc, map_.w(), map_.h());

			if(cur_route.steps.empty()) {
				continue;
			}

			double rating = rate_target(*best_target, u, dstsrc, enemy_dstsrc, cur_route);

			if(best == units_.end() || rating > best_rating) {
				best_rating = rating;
				best = u;
				best_route = cur_route;
			}
		}

		LOG_AI << "done complex targeting...\n";
	} else {
		u = units_.end();
	}

	LOG_AI << "best unit: " << best->first << '\n';

	assert(best_target != targets.end());

	//if our target is a position to support, then we
	//see if we can move to a position in support of this target
	if(best_target->type == target::SUPPORT) {
		LOG_AI << "support...\n";

		std::vector<location> locs;
		access_points(srcdst,best->first,best_target->loc,locs);

		if(locs.empty() == false) {
			LOG_AI << "supporting unit at " << best_target->loc.x + 1 << "," << best_target->loc.y + 1 << "\n";
			location best_loc;
			int best_defense = 0;
			double best_vulnerability = 0.0;

			for(std::vector<location>::const_iterator i = locs.begin(); i != locs.end(); ++i) {
				const int defense = best->second.defense_modifier(map_.get_terrain(*i));
				//FIXME: suokko multiplied by 10 * get_caution(). ?
				const double vulnerability = power_projection(*i,enemy_dstsrc);

				if(best_loc.valid() == false || defense < best_defense || (defense == best_defense && vulnerability < best_vulnerability)) {
					best_loc = *i;
					best_defense = defense;
					best_vulnerability = vulnerability;
				}
			}

			LOG_AI << "returning support...\n";
			return std::pair<location,location>(best->first,best_loc);
		}
	}

	std::map<map_location,pathfind::paths> dummy_possible_moves;
	move_map fullmove_srcdst;
	move_map fullmove_dstsrc;
	calculate_possible_moves(dummy_possible_moves,fullmove_srcdst,fullmove_dstsrc,false,true);

	bool dangerous = false;

	if(get_grouping() != "no") {
		LOG_AI << "grouping...\n";
		const unit_map::const_iterator unit_at_target = units_.find(best_target->loc);
		int movement = best->second.movement_left();

		const bool defensive_grouping = get_grouping() == "defensive";

		//we stop and consider whether the route to this
		//target is dangerous, and whether we need to group some units to move in unison toward the target
		//if any point along the path is too dangerous for our single unit, then we hold back
		for(std::vector<location>::const_iterator i = best_route.steps.begin(); i != best_route.steps.end() && movement > 0; ++i) {

			//FIXME: suokko multiplied by 10 * get_caution(). ?
			const double threat = power_projection(*i,enemy_dstsrc);
			//FIXME: sukko doubled the power-projection them in the second test.  ?
			if((threat >= double(best->second.hitpoints()) && threat > power_projection(*i,fullmove_dstsrc)) ||
			   (i+1 >= best_route.steps.end()-1 && unit_at_target != units_.end() && current_team().is_enemy(unit_at_target->second.side()))) {
				dangerous = true;
				break;
			}

			if(!defensive_grouping) {
				movement -= best->second.movement_cost(map_.get_terrain(*i));
			}
		}

		LOG_AI << "done grouping...\n";
	}

	if(dangerous) {
		LOG_AI << "dangerous path\n";
		std::set<location> group, enemies;
		const location dst = form_group(best_route.steps,dstsrc,group);
		enemies_along_path(best_route.steps,enemy_dstsrc,enemies);

		const double our_strength = compare_groups(group,enemies,best_route.steps);

		if(our_strength > 0.5 + get_caution()) {
			LOG_AI << "moving group\n";
			const bool res = move_group(dst,best_route.steps,group);
			if(res) {
				return std::pair<location,location>(location(1,1),location());
			} else {
				LOG_AI << "group didn't move " << group.size() << "\n";

				//the group didn't move, so end the first unit in the group's turn, to prevent an infinite loop
				return std::pair<location,location>(best->first,best->first);

			}
		} else {
			LOG_AI << "massing to attack " << best_target->loc.x + 1 << "," << best_target->loc.y + 1
				<< " " << our_strength << "\n";

			const double value = best_target->value;
			const location target_loc = best_target->loc;
			const location loc = best->first;
			const unit& un = best->second;

			targets.erase(best_target);

			//find the best location to mass units at for an attack on the enemies
			location best_loc;
			double best_threat = 0.0;
			int best_distance = 0;

			const double max_acceptable_threat = un.hitpoints()/4;

			std::set<location> mass_locations;

			const std::pair<move_map::const_iterator,move_map::const_iterator> itors = srcdst.equal_range(loc);
			for(move_map::const_iterator i = itors.first; i != itors.second; ++i) {
				const int distance = distance_between(target_loc,i->second);
				const int defense = un.defense_modifier(map_.get_terrain(i->second));
				//FIXME: suokko multiplied by 10 * get_caution(). ?
				const double threat = (power_projection(i->second,enemy_dstsrc)*defense)/100;

				if(best_loc.valid() == false || (threat < std::max<double>(best_threat,max_acceptable_threat) && distance < best_distance)) {
					best_loc = i->second;
					best_threat = threat;
					best_distance = distance;
				}

				if(threat < max_acceptable_threat) {
					mass_locations.insert(i->second);
				}
			}

			for(std::set<location>::const_iterator j = mass_locations.begin(); j != mass_locations.end(); ++j) {
				if(*j != best_loc && distance_between(*j,best_loc) < 3) {
					LOG_AI << "found mass-to-attack target... " << *j << " with value: " << value*4.0 << "\n";
					targets.push_back(target(*j,value*4.0,target::MASS));
					best_target = targets.end() - 1;
				}
			}

			return std::pair<location,location>(loc,best_loc);
		}
	}

	for(std::vector<location>::reverse_iterator ri =
	    best_route.steps.rbegin(); ri != best_route.steps.rend(); ++ri) {

		if(game_config::debug) {
			//game_display::debug_highlight(*ri,static_cast<size_t>(0.2));
		}

		//this is set to 'true' if we are hesitant to proceed because of enemy units,
		//to rally troops around us.
		bool is_dangerous = false;

		typedef std::multimap<location,location>::const_iterator Itor;
		std::pair<Itor,Itor> its = dstsrc.equal_range(*ri);
		while(its.first != its.second) {
			if(its.first->second == best->first) {
				if(!should_retreat(its.first->first,best,fullmove_srcdst,fullmove_dstsrc,enemy_dstsrc,
								   get_caution())) {
					const double value = best_target->value - best->second.cost()/20.0;

					if(value > 0.0 && best_target->type != target::MASS) {
						//there are enemies ahead. Rally troops around us to
						//try to take the target
						if(is_dangerous) {
							LOG_AI << "found reinforcement target... " << its.first->first << " with value: " << value*2.0 << "\n";
							targets.push_back(target(its.first->first,value*2.0,target::BATTLE_AID));
						}

						best_target->value = value;
					} else {
						targets.erase(best_target);
					}

					LOG_AI << "Moving to " << its.first->first.x + 1 << "," << its.first->first.y + 1 << "\n";

					return std::pair<location,location>(its.first->second,its.first->first);
				} else {
					LOG_AI << "dangerous!\n";
					is_dangerous = true;
				}
			}

			++its.first;
		}
	}

	if(best != units_.end()) {
		LOG_AI << "Could not make good move, staying still\n";

		//this sounds like the road ahead might be dangerous, and that's why we don't advance.
		//create this as a target, attempting to rally units around
		targets.push_back(target(best->first,best_target->value));
		best_target = targets.end() - 1;
		return std::pair<location,location>(best->first,best->first);
	}

	LOG_AI << "Could not find anywhere to move!\n";
	return std::pair<location,location>();
}

void ai_default::access_points(const move_map& srcdst, const location& u, const location& dst, std::vector<location>& out)
{
	const unit_map::const_iterator u_it = units_.find(u);
	if(u_it == units_.end()) {
		return;
	}

	// unit_map single_unit(u_it->first, u_it->second);

	const std::pair<move_map::const_iterator,move_map::const_iterator> locs = srcdst.equal_range(u);
	for(move_map::const_iterator i = locs.first; i != locs.second; ++i) {
		const location& loc = i->second;
		if (int(distance_between(loc,dst)) <= u_it->second.total_movement()) {
			pathfind::shortest_path_calculator calc(u_it->second, current_team(), units_, teams_, map_);
			pathfind::plain_route rt = pathfind::a_star_search(loc, dst, u_it->second.total_movement(), &calc, map_.w(), map_.h());
			if(rt.steps.empty() == false) {
				out.push_back(loc);
			}
		}
	}
}


void ai_default::move_leader_to_keep()
{
	const unit_map::iterator leader = units_.find_leader(get_side());
	if(leader == units_.end() || leader->second.incapacitated()) {
		return;
	}

	// Find where the leader can move
	const pathfind::paths leader_paths(map_, units_, leader->first,
	       teams_, false, false, current_team());
	const map_location& keep = suitable_keep(leader->first,leader_paths);

	std::map<map_location,pathfind::paths> possible_moves;
	possible_moves.insert(std::pair<map_location,pathfind::paths>(leader->first,leader_paths));

	// If the leader is not on keep, move him there.
	if(leader->first != keep) {
		if (leader_paths.destinations.contains(keep) && units_.count(keep) == 0) {
			bool gamestate_changed = false;
			move_unit(leader->first,keep,gamestate_changed);
			if (!gamestate_changed) {
				ERR_AI << "move_leader_to_keep failed!" << std::endl;
			}
		} else {
			// Make a map of the possible locations the leader can move to,
			// ordered by the distance from the keep.
			std::multimap<int,map_location> moves_toward_keep;

			// The leader can't move to his keep, try to move to the closest location
			// to the keep where there are no enemies in range.
			const int current_distance = distance_between(leader->first,keep);
			BOOST_FOREACH (const pathfind::paths::step &dest, leader_paths.destinations)
			{
				if (!units_.find(dest.curr).valid()){
					const int new_distance = distance_between(dest.curr,keep);
					if(new_distance < current_distance) {
						moves_toward_keep.insert(std::make_pair(new_distance, dest.curr));
					}
				}
	 		}

			// Find the first location which we can move to,
			// without the threat of enemies.
			for(std::multimap<int,map_location>::const_iterator j = moves_toward_keep.begin();
		    	j != moves_toward_keep.end(); ++j) {

				if(get_enemy_dstsrc().count(j->second) == 0) {
					bool gamestate_changed = false;
					move_unit(leader->first,j->second,gamestate_changed);
					if (!gamestate_changed) {
						ERR_AI << "move_leader_to_keep failed!" << std::endl;
					}
					break;
				}
			}
		}
	}
}

} //end of namespace ai

