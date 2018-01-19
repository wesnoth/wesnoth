/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * Strategic movement routine, taken from default AI
 */

#include "ai/default/ca_move_to_targets.hpp"

#include "ai/composite/ai.hpp"
#include "ai/actions.hpp"
#include "game_board.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "resources.hpp"
#include "units/unit.hpp"
#include "terrain/filter.hpp"
#include "pathfind/pathfind.hpp"
#include "pathfind/teleport.hpp"

#include <deque>

namespace ai {

namespace ai_default_rca {

static lg::log_domain log_ai_testing_ca_move_to_targets("ai/ca/move_to_targets");
#define DBG_AI LOG_STREAM(debug, log_ai_testing_ca_move_to_targets)
#define LOG_AI LOG_STREAM(info, log_ai_testing_ca_move_to_targets)
#define WRN_AI LOG_STREAM(warn, log_ai_testing_ca_move_to_targets)
#define ERR_AI LOG_STREAM(err, log_ai_testing_ca_move_to_targets)



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
		const t_translation::terrain_code terrain = map_[loc];

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


class remove_wrong_targets {
public:
	remove_wrong_targets(const readonly_context &context)
		:avoid_(context.get_avoid()), map_(resources::gameboard->map())
	{
	}

bool operator()(const target &t){
	if (!map_.on_board(t.loc)) {
		DBG_AI << "removing target "<< t.loc << " due to it not on_board" << std::endl;
		return true;
	}

	if (t.value<=0) {
		DBG_AI << "removing target "<< t.loc << " due to value<=0" << std::endl;
		return true;
	}

	if (avoid_.match(t.loc)) {
		DBG_AI << "removing target "<< t.loc << " due to 'avoid' match" << std::endl;
		return true;
	}

	return false;
}
private:
	const terrain_filter &avoid_;
	const gamemap &map_;

};

move_to_targets_phase::move_to_targets_phase( rca_context &context, const config &cfg )
	: candidate_action(context,cfg)
{
}


move_to_targets_phase::~move_to_targets_phase()
{
}


double move_to_targets_phase::evaluate()
{
	return get_score();
}


void move_to_targets_phase::execute()
{
	unit_map::const_iterator leader = resources::gameboard->units().find_leader(get_side());
	LOG_AI << "finding targets...\n";
	std::vector<target> targets;
	for(;;) {
		if(targets.empty()) {
			targets = find_targets(get_enemy_dstsrc());
			targets.insert(targets.end(),additional_targets().begin(),
				       additional_targets().end());
			LOG_AI << "Found " << targets.size() << " targets\n";
			if(targets.empty()) {
				break;
			}
		}

		targets.erase( std::remove_if(targets.begin(),targets.end(),remove_wrong_targets(*this)), targets.end() );

		if(targets.empty()) {
			break;
		}

		LOG_AI << "choosing move with " << targets.size() << " targets\n";
		std::pair<map_location,map_location> move = choose_move(targets);
		LOG_AI << "choose_move ends with " << targets.size() << " targets\n";

		for(std::vector<target>::const_iterator ittg = targets.begin();
				ittg != targets.end(); ++ittg) {
			assert(resources::gameboard->map().on_board(ittg->loc));
		}

		if(move.first.valid() == false || move.second.valid() == false) {
			break;
		}

		assert (resources::gameboard->map().on_board(move.first)
			&& resources::gameboard->map().on_board(move.second));

		LOG_AI << "move: " << move.first << " -> " << move.second << '\n';

		move_result_ptr move_ptr = execute_move_action(move.first,move.second,true);
		if(!move_ptr->is_ok()) {
			WRN_AI << "unexpected outcome of move"<<std::endl;
			break;
		}
	}
}





// structure storing the maximal possible rating of a target
struct rated_target{
	rated_target(const std::vector<target>::iterator& t, double r) : tg(t), max_rating(r) {}
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


double move_to_targets_phase::rate_target(const target& tg, const unit_map::iterator& u,
			const move_map& dstsrc, const move_map& enemy_dstsrc,
			const pathfind::plain_route& rt)
{
	double move_cost = rt.move_cost;

	if(move_cost > 0) {
		// if this unit can move to that location this turn, it has a very very low cost
		typedef std::multimap<map_location,map_location>::const_iterator multimapItor;
		std::pair<multimapItor,multimapItor> locRange = dstsrc.equal_range(tg.loc);
		while (locRange.first != locRange.second) {
			if (locRange.first->second == u->get_location()) {
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
	if(tg.type == target::TYPE::SUPPORT) {
		if (move_cost <= u->movement_left() * 2) {
			rating *= 10.0;
		} else {
			rating = 0.0;
			return rating;
		}
	}

	//scouts do not like encountering enemies on their paths
	if (u->usage() == "scout") {
		//scouts get a bonus for going after villages
		if(tg.type == target::TYPE::VILLAGE) {
				rating *= get_scout_village_targeting();
		}

		std::set<map_location> enemies_guarding;
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



std::pair<map_location,map_location> move_to_targets_phase::choose_move(std::vector<target>& targets)
{
	log_scope2(log_ai_testing_ca_move_to_targets, "choosing move");

	raise_user_interact();
	unit_map &units_ = resources::gameboard->units();
	const gamemap &map_ = resources::gameboard->map();

	unit_map::iterator u;

	//take care of all the guardians first
	for(u = units_.begin(); u != units_.end(); ++u) {
		if (!(u->side() != get_side() || (u->can_recruit() && !get_leader_ignores_keep()) || u->movement_left() <= 0 || u->incapacitated())) {
			if (u->get_state("guardian")) {
				LOG_AI << u->type_id() << " is guardian, staying still\n";
				return std::make_pair(u->get_location(), u->get_location());
			}
		}
	}

	//now find the first eligible remaining unit
	for(u = units_.begin(); u != units_.end(); ++u) {
		if (!(u->side() != get_side() || (u->can_recruit() && !get_leader_ignores_keep()) || u->movement_left() <= 0 || u->incapacitated())) {
			break;
		}
	}

	if(u == units_.end()) {
		LOG_AI  << "no eligible units found\n";
		return std::pair<map_location,map_location>();
	}

	const pathfind::plain_route dummy_route;
	assert(dummy_route.steps.empty() && dummy_route.move_cost == 0);

	// We will sort all targets by a quick maximal possible rating,
	// so we will be able to start real work by the most promising ones
	// and if its real value is better than other maximal values
	// then we can skip them.

	const move_map& dstsrc = get_dstsrc();
	const move_map& enemy_dstsrc = get_enemy_dstsrc();
	std::vector<rated_target> rated_targets;
	for(std::vector<target>::iterator tg = targets.begin(); tg != targets.end(); ++tg) {
		// passing a dummy route to have the maximal rating
		double max_rating = rate_target(*tg, u, dstsrc, enemy_dstsrc, dummy_route);
		rated_targets.emplace_back(tg, max_rating);
	}

	//use stable_sort for the moment to preserve old AI behavior
	std::stable_sort(rated_targets.begin(), rated_targets.end(), rated_target_comparer());

	const move_cost_calculator cost_calc(*u, map_, units_, enemy_dstsrc);

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
		const pathfind::teleport_map allowed_teleports = pathfind::get_teleport_locations(*u, current_team());
		pathfind::plain_route real_route = a_star_search(u->get_location(), tg.loc, locStopValue, cost_calc, map_.w(), map_.h(), &allowed_teleports);

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
		LOG_AI << "no eligible targets found for unit at " << u->get_location() << std::endl;
		return std::make_pair(u->get_location(), u->get_location());
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
			if (u->side() != get_side() || (u->can_recruit() && !get_leader_ignores_keep()) ||
			    u->movement_left() <= 0 || u->get_state("guardian") ||
			    u->incapacitated())
			{
				continue;
			}

			raise_user_interact();

			const move_cost_calculator calc(*u, map_, units_, enemy_dstsrc);

			///@todo 1.9: lower this value for perf,
			// but best_rating is too big for scout and support
			// which give a too small locStopValue
			// so keep costy A* for the moment.
			//const double locStopValue = std::min(best_target->value / best_rating, (double) 100.0);

			const double locStopValue = 500.0;
			const pathfind::teleport_map allowed_teleports = pathfind::get_teleport_locations(*u, current_team());
			pathfind::plain_route cur_route = pathfind::a_star_search(u->get_location(), best_target->loc, locStopValue, calc, map_.w(), map_.h(), &allowed_teleports);

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

	LOG_AI << "best unit: " << best->get_location() << '\n';

	assert(best_target != targets.end());

	//if our target is a position to support, then we
	//see if we can move to a position in support of this target
	const move_map& srcdst = get_srcdst();
	if(best_target->type == target::TYPE::SUPPORT) {
		LOG_AI << "support...\n";

		std::vector<map_location> locs;
		access_points(srcdst, best->get_location(), best_target->loc, locs);

		if(locs.empty() == false) {
			LOG_AI << "supporting unit at " << best_target->loc.wml_x() << "," << best_target->loc.wml_y() << "\n";
			map_location best_loc;
			int best_defense = 0;
			double best_vulnerability = 0.0;

			for(std::vector<map_location>::const_iterator i = locs.begin(); i != locs.end(); ++i) {
				const int defense = best->defense_modifier(map_.get_terrain(*i));
				//FIXME: suokko multiplied by 10 * get_caution(). ?
				const double vulnerability = power_projection(*i,enemy_dstsrc);

				if(best_loc.valid() == false || defense < best_defense || (defense == best_defense && vulnerability < best_vulnerability)) {
					best_loc = *i;
					best_defense = defense;
					best_vulnerability = vulnerability;
				}
			}

			LOG_AI << "returning support...\n";
			return std::make_pair(best->get_location(), best_loc);
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
		int movement = best->movement_left();

		const bool defensive_grouping = get_grouping() == "defensive";

		//we stop and consider whether the route to this
		//target is dangerous, and whether we need to group some units to move in unison toward the target
		//if any point along the path is too dangerous for our single unit, then we hold back
		for(std::vector<map_location>::const_iterator i = best_route.steps.begin(); i != best_route.steps.end() && movement > 0; ++i) {

			//FIXME: suokko multiplied by 10 * get_caution(). ?
			const double threat = power_projection(*i,enemy_dstsrc);
			//FIXME: sukko doubled the power-projection them in the second test.  ?
			if ((threat >= best->hitpoints() && threat > power_projection(*i,fullmove_dstsrc)) ||
			   (i+1 >= best_route.steps.end()-1 && unit_at_target != units_.end() && current_team().is_enemy(unit_at_target->side()))) {
				dangerous = true;
				break;
			}

			if(!defensive_grouping) {
				movement -= best->movement_cost(map_.get_terrain(*i));
			}
		}

		LOG_AI << "done grouping...\n";
	}

	if(dangerous) {
		LOG_AI << "dangerous path\n";
		std::set<map_location> group, enemies;
		const map_location dst = form_group(best_route.steps,dstsrc,group);
		enemies_along_path(best_route.steps,enemy_dstsrc,enemies);

		const double our_strength = compare_groups(group,enemies,best_route.steps);

		if(our_strength > 0.5 + get_caution()) {
			LOG_AI << "moving group\n";
			const bool res = move_group(dst,best_route.steps,group);
			if(res) {
				return std::pair<map_location,map_location>(map_location(1,1),map_location());
			} else {
				LOG_AI << "group didn't move " << group.size() << "\n";

				//the group didn't move, so end the first unit in the group's turn, to prevent an infinite loop
				return std::make_pair(best->get_location(), best->get_location());

			}
		} else {
			LOG_AI << "massing to attack " << best_target->loc.wml_x() << "," << best_target->loc.wml_y()
				<< " " << our_strength << "\n";

			const double value = best_target->value;
			const map_location target_loc = best_target->loc;
			const map_location loc = best->get_location();
			const unit& un = *best;

			targets.erase(best_target);

			//find the best location to mass units at for an attack on the enemies
			map_location best_loc;
			double best_threat = 0.0;
			int best_distance = 0;

			const double max_acceptable_threat = un.hitpoints() / 4.0;

			std::set<map_location> mass_locations;

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

			for(std::set<map_location>::const_iterator j = mass_locations.begin(); j != mass_locations.end(); ++j) {
				if(*j != best_loc && distance_between(*j,best_loc) < 3) {
					LOG_AI << "found mass-to-attack target... " << *j << " with value: " << value*4.0 << "\n";
					targets.emplace_back(*j,value*4.0,target::TYPE::MASS);
					best_target = targets.end() - 1;
				}
			}

			return std::pair<map_location,map_location>(loc,best_loc);
		}
	}

	for(std::vector<map_location>::reverse_iterator ri =
	    best_route.steps.rbegin(); ri != best_route.steps.rend(); ++ri) {

		//this is set to 'true' if we are hesitant to proceed because of enemy units,
		//to rally troops around us.
		bool is_dangerous = false;

		typedef std::multimap<map_location,map_location>::const_iterator Itor;
		std::pair<Itor,Itor> its = dstsrc.equal_range(*ri);
		while(its.first != its.second) {
			if (its.first->second == best->get_location()) {
				if(!should_retreat(its.first->first,best,fullmove_srcdst,fullmove_dstsrc,enemy_dstsrc,
								   get_caution())) {
					double value = best_target->value - best->cost() / 20.0;

					if(value > 0.0 && best_target->type != target::TYPE::MASS) {
						//there are enemies ahead. Rally troops around us to
						//try to take the target
						if(is_dangerous) {
							LOG_AI << "found reinforcement target... " << its.first->first << " with value: " << value*2.0 << "\n";
							targets.emplace_back(its.first->first,value*2.0,target::TYPE::BATTLE_AID);
						}

						best_target->value = value;
					} else {
						targets.erase(best_target);
					}

					LOG_AI << "Moving to " << its.first->first.wml_x() << "," << its.first->first.wml_y() << "\n";

					return std::pair<map_location,map_location>(its.first->second,its.first->first);
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
		targets.emplace_back(best->get_location(), best_target->value);
		best_target = targets.end() - 1;
		return std::make_pair(best->get_location(), best->get_location());
	}

	LOG_AI << "Could not find anywhere to move!\n";
	return std::pair<map_location,map_location>();
}

void move_to_targets_phase::access_points(const move_map& srcdst, const map_location& u, const map_location& dst, std::vector<map_location>& out)
{
	unit_map &units_ = resources::gameboard->units();
	const gamemap &map_ = resources::gameboard->map();
	const unit_map::const_iterator u_it = units_.find(u);
	if(u_it == units_.end()) {
		return;
	}

	// unit_map single_unit(u_it->first, u_it->second);

	const std::pair<move_map::const_iterator,move_map::const_iterator> locs = srcdst.equal_range(u);
	for(move_map::const_iterator i = locs.first; i != locs.second; ++i) {
		const map_location& loc = i->second;
		if (int(distance_between(loc,dst)) <= u_it->total_movement()) {
			pathfind::shortest_path_calculator calc(*u_it, current_team(), resources::gameboard->teams(), map_);
			const pathfind::teleport_map allowed_teleports = pathfind::get_teleport_locations(*u_it, current_team());
			pathfind::plain_route rt = a_star_search(loc, dst, u_it->total_movement(), calc, map_.w(), map_.h(), &allowed_teleports);
			if(rt.steps.empty() == false) {
				out.push_back(loc);
			}
		}
	}
}


double move_to_targets_phase::compare_groups(const std::set<map_location>& our_group, const std::set<map_location>& their_group, const std::vector<map_location>& battlefield) const
{
	const double a = rate_group(our_group,battlefield);
	const double b = std::max<double>(rate_group(their_group,battlefield),0.01);
	return a/b;
}


void move_to_targets_phase::enemies_along_path(const std::vector<map_location>& route, const move_map& dstsrc, std::set<map_location>& res)
{
	for(std::vector<map_location>::const_iterator i = route.begin(); i != route.end(); ++i) {
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


map_location move_to_targets_phase::form_group(const std::vector<map_location>& route, const move_map& dstsrc, std::set<map_location>& res)
{
	unit_map &units_ = resources::gameboard->units();
	if(route.empty()) {
		return map_location();
	}

	std::vector<map_location>::const_iterator i;
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
				if(un == units_.end() || (un->can_recruit() && !get_leader_ignores_keep()) || un->movement_left() < un->total_movement()) {
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


bool move_to_targets_phase::move_group(const map_location& dst, const std::vector<map_location>& route, const std::set<map_location>& units)
{
	unit_map &units_ = resources::gameboard->units();
	const gamemap &map_ = resources::gameboard->map();

	const std::vector<map_location>::const_iterator itor = std::find(route.begin(),route.end(),dst);
	if(itor == route.end()) {
		return false;
	}

	LOG_AI << "group has " << units.size() << " members\n";

	map_location next;

	size_t direction = 0;

	//find the direction the group is moving in
	if(itor+1 != route.end()) {
		next = *(itor+1);
	} else if(itor != route.begin()) {
		next = *(itor-1);
	}

	if(next.valid()) {
		map_location adj[6];
		get_adjacent_tiles(dst,adj);

		direction = std::find(adj,adj+6,next) - adj;
	}

	std::deque<map_location> preferred_moves;
	preferred_moves.push_back(dst);

	std::map<map_location,pathfind::paths> possible_moves;
	move_map srcdst, dstsrc;
	calculate_possible_moves(possible_moves,srcdst,dstsrc,false);

	bool gamestate_changed = false;

	for(std::set<map_location>::const_iterator i = units.begin(); i != units.end(); ++i) {
		const unit_map::const_iterator un = units_.find(*i);
		if(un == units_.end()) {
			continue;
		}

		map_location best_loc;
		int best_defense = -1;
		for(std::deque<map_location>::const_iterator j = preferred_moves.begin(); j != preferred_moves.end(); ++j) {
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

			int defense = un->defense_modifier(map_.get_terrain(*j));
			if(best_loc.valid() == false || defense < best_defense) {
				best_loc = *j;
				best_defense = defense;
			}
		}

		if(best_loc.valid()) {
			move_result_ptr move_res = execute_move_action(*i,best_loc);
			gamestate_changed |= move_res->is_gamestate_changed();


			//if we were ambushed or something went wrong,  abort the group's movement.
			if (!move_res->is_ok()) {
				return gamestate_changed;
			}

			preferred_moves.erase(std::find(preferred_moves.begin(),preferred_moves.end(),best_loc));

			//find locations that are 'perpendicular' to the direction of movement for further units to move to.
			map_location adj[6];
			get_adjacent_tiles(best_loc,adj);
			for(size_t n = 0; n != 6; ++n) {
				if(n != direction && ((n+3)%6) != direction && map_.on_board(adj[n]) &&
				   units_.count(adj[n]) == 0 && std::count(preferred_moves.begin(),preferred_moves.end(),adj[n]) == 0) {
					preferred_moves.push_front(adj[n]);
					LOG_AI << "added moves: " << adj[n].wml_x() << "," << adj[n].wml_y() << "\n";
				}
			}
		} else {
			LOG_AI << "Could not move group member to any of " << preferred_moves.size() << " locations\n";
		}
	}

	return gamestate_changed;
}


double move_to_targets_phase::rate_group(const std::set<map_location>& group, const std::vector<map_location>& battlefield) const
{
	unit_map &units_ = resources::gameboard->units();
	const gamemap &map_ = resources::gameboard->map();

	double strength = 0.0;
	for(std::set<map_location>::const_iterator i = group.begin(); i != group.end(); ++i) {
		const unit_map::const_iterator u = units_.find(*i);
		if(u == units_.end()) {
			continue;
		}

		const unit &un = *u;

		int defense = 0;
		for(std::vector<map_location>::const_iterator j = battlefield.begin(); j != battlefield.end(); ++j) {
			defense += un.defense_modifier(map_.get_terrain(*j));
		}

		defense /= battlefield.size();

		int best_attack = 0;
		for(const attack_type& a : un.attacks()) {
			const int attack_strength = a.num_attacks() * a.damage();
			best_attack = std::max<int>(attack_strength, best_attack);
		}

		const int rating = (defense*best_attack*un.hitpoints())/(100*un.max_hitpoints());
		strength += double(rating);
	}

	return strength;
}



bool move_to_targets_phase::should_retreat(const map_location& loc, const unit_map::const_iterator& un,
		const move_map& srcdst, const move_map& dstsrc, const move_map& enemy_dstsrc,
		double caution)
{
	if(caution <= 0.0) {
		return false;
	}

	double optimal_terrain = best_defensive_position(un->get_location(), dstsrc,
			srcdst, enemy_dstsrc).chance_to_hit/100.0;
	const double proposed_terrain =
		un->defense_modifier(resources::gameboard->map().get_terrain(loc))/100.0;

	// The 'exposure' is the additional % chance to hit
	// this unit receives from being on a sub-optimal defensive terrain.
	const double exposure = proposed_terrain - optimal_terrain;

	const double our_power = power_projection(loc,dstsrc);
	const double their_power = power_projection(loc,enemy_dstsrc);
	return caution*their_power*(1.0+exposure) > our_power;
}

} // end of namespace ai_default_rca

} // end of namespace ai
