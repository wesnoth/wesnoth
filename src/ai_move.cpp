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
#include "ai_move.hpp"
#include "display.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "map.hpp"
#include "util.hpp"

#include <iostream>

struct move_cost_calculator
{
	move_cost_calculator(const unit& u, const gamemap& map,
	                     const game_data& data,
					 	 const unit_map& units,
	                     const gamemap::location& loc,
						 const std::multimap<gamemap::location,gamemap::location>& dstsrc)
	  : unit_(u), map_(map), data_(data), units_(units),
	    move_type_(u.type().movement_type()), loc_(loc), dstsrc_(dstsrc)
	{}

	double cost(const gamemap::location& loc, double so_far) const
	{
		if(!map_.on_board(loc))
			return 1000.0;

		//if this unit can move to that location this turn, it has a very
		//very low cost
		typedef std::multimap<gamemap::location,gamemap::location>::const_iterator Itor;
		std::pair<Itor,Itor> range = dstsrc_.equal_range(loc);
		while(range.first != range.second) {
			if(range.first->second == loc_) {
				return 0.01;
			}

			++range.first;
		}

		const gamemap::TERRAIN terrain = map_[loc.x][loc.y];

		const double modifier = 1.0;//move_type_.defense_modifier(map_,terrain);
		const double move_cost = move_type_.movement_cost(map_,terrain);

		double enemies = 0;
	/*	//is this stuff responsible for making it take a long time?
		location adj[7];
		adj[0] = loc;
		get_adjacent_tiles(loc,adj+1);
		for(int i = 0; i != 7; ++i) {
			const std::map<location,unit>::const_iterator en=units_.find(adj[i]);
			//at the moment, don't allow any units to be in the path
			if(i == 0 && en != units_.end()) {
				return 1000.0;
			}

			if(en != units_.end() && en->second.side() == enemy_) {
				enemies += 1.0;
			}
		}
*/
		double res = modifier*move_cost + enemies*2.0;

		//if there is a unit (even a friendly one) on this tile, we increase the cost to
		//try discourage going through units, to thwart the 'single file effect'
		if(units_.count(loc))
			res *= 4.0;

		assert(res > 0);
		return res;
	}

private:
	const unit& unit_;
	const gamemap& map_;
	const game_data& data_;
	const unit_map& units_;
	const unit_movement_type& move_type_;
	const gamemap::location loc_;
	const ai::move_map dstsrc_;

};

std::vector<ai::target> ai::find_targets(unit_map::const_iterator leader, const move_map& enemy_srcdst, const move_map& enemy_dstsrc)
{
	log_scope("finding targets...");

	const bool has_leader = leader != units_.end();

	std::vector<target> targets;

	//if enemy units are in range of the leader, then we target the enemies who are in range.
	if(has_leader) {
		const double threat = power_projection(leader->first,enemy_srcdst,enemy_dstsrc);
		if(threat > 0.0) {
			//find the location of enemy threats
			std::set<gamemap::location> threats;

			gamemap::location adj[6];
			get_adjacent_tiles(leader->first,adj);
			for(size_t n = 0; n != 6; ++n) {
				std::pair<move_map::const_iterator,move_map::const_iterator> itors = enemy_dstsrc.equal_range(adj[n]);
				while(itors.first != itors.second) {
					if(units_.count(itors.first->second)) {
						threats.insert(itors.first->second);
					}

					++itors.first;
				}
			}

			assert(threats.empty() == false);

			const double value = threat/double(threats.size());
			for(std::set<gamemap::location>::const_iterator i = threats.begin(); i != threats.end(); ++i) {
				targets.push_back(target(*i,value));
			}
		}
	}

	if(has_leader && current_team().village_value() > 0.0) {
		const std::vector<location>& villages = map_.villages();
		for(std::vector<location>::const_iterator t = villages.begin(); t != villages.end(); ++t) {
			assert(map_.on_board(*t));
			bool get_village = true;
			for(size_t i = 0; i != teams_.size(); ++i) {
				if(!current_team().is_enemy(i+1) && teams_[i].owns_village(*t)) {
					get_village = false;
					break;
				}
			}

			if(get_village) {
				targets.push_back(target(*t,current_team().village_value()));
			}
		}
	}

	std::vector<team::target>& team_targets = current_team().targets();

	//find the enemy leaders and explicit targets
	unit_map::const_iterator u;
	for(u = units_.begin(); u != units_.end(); ++u) {

		//is an enemy leader
		if(u->second.can_recruit() && current_team().is_enemy(u->second.side())) {
			assert(map_.on_board(u->first));
			targets.push_back(target(u->first,current_team().leader_value()));
		}

		//explicit targets for this team
		for(std::vector<team::target>::iterator j = team_targets.begin();
		    j != team_targets.end(); ++j) {
			if(u->second.matches_filter(j->criteria)) {
				std::cerr << "found explicit target..." << j->value << "\n";
				targets.push_back(target(u->first,j->value));
			}
		}
	}

	std::vector<double> new_values;

	for(std::vector<target>::iterator i = targets.begin();
	    i != targets.end(); ++i) {

		new_values.push_back(i->value);

		for(std::vector<target>::const_iterator j = targets.begin(); j != targets.end(); ++j) {
			if(i->loc == j->loc) {
				continue;
			}

			const double distance = abs(j->loc.x - i->loc.x) +
			                        abs(j->loc.y - i->loc.y);
			new_values.back() += j->value/(distance*distance);
		}
	}

	assert(new_values.size() == targets.size());
	for(size_t n = 0; n != new_values.size(); ++n) {
		std::cerr << "target value: " << targets[n].value << " -> " << new_values[n] << "\n";
		targets[n].value = new_values[n];
	}

	return targets;
}

std::pair<gamemap::location,gamemap::location> ai::choose_move(std::vector<target>& targets,const move_map& dstsrc, const move_map& enemy_srcdst, const move_map& enemy_dstsrc)
{
	log_scope("choosing move");

	user_interact();

	std::vector<target>::const_iterator ittg;
	for(ittg = targets.begin(); ittg != targets.end(); ++ittg) {
		assert(map_.on_board(ittg->loc));
	}

	paths::route best_route;
	unit_map::iterator best = units_.end();
	double best_rating = 0.1;
	std::vector<target>::iterator best_target = targets.end();

	unit_map::iterator u;

	//find the first eligible unit
	for(u = units_.begin(); u != units_.end(); ++u) {
		if(!(u->second.side() != team_num_ || u->second.can_recruit() || u->second.movement_left() <= 0)) {
			break;
		}
	}

	if(u == units_.end()) {
		std::cerr  << "no eligible units found\n";
		return std::pair<location,location>();
	}

	//guardian units stay put
	if(u->second.is_guardian()) {
		std::cerr << u->second.type().name() << " is guardian, staying still\n";
		return std::pair<location,location>(u->first,u->first);
	}

	const move_cost_calculator cost_calc(u->second,map_,gameinfo_,units_,u->first,dstsrc);

	//choose the best target for that unit
	for(std::vector<target>::iterator tg = targets.begin(); tg != targets.end(); ++tg) {
		user_interact();

		assert(map_.on_board(tg->loc));
		const paths::route cur_route = a_star_search(u->first,tg->loc,
		                       minimum(tg->value/best_rating,500.0),cost_calc);
		const double rating = tg->value/cur_route.move_left;
		std::cerr << tg->value << "/" << cur_route.move_left << " = " << rating << "\n";
		if(best_target == targets.end() || rating > best_rating) {
			best_rating = rating;
			best_target = tg;
			best = u;
			best_route = cur_route;
		}
	}


	if(best_target == targets.end()) {
		std::cerr  << "no eligible targets found\n";
		return std::pair<location,location>();
	}

	//now see if any other unit can put a better bid forward
	for(++u; u != units_.end(); ++u) {
		if(u->second.side() != team_num_ || u->second.can_recruit() ||
		   u->second.movement_left() <= 0 || u->second.is_guardian()) {
			continue;
		}

		user_interact();

		const move_cost_calculator calc(u->second,map_,gameinfo_,units_,u->first,dstsrc);
		const paths::route cur_route = a_star_search(u->first,best_target->loc,
			               minimum(best_target->value/best_rating,100.0),calc);
		const double rating = best_target->value/cur_route.move_left;
		if(best == units_.end() || rating > best_rating) {
			best_rating = rating;
			best = u;
			best_route = cur_route;
		}
	}

	assert(best_target >= targets.begin() && best_target < targets.end());

	for(ittg = targets.begin(); ittg != targets.end(); ++ittg) {
		assert(map_.on_board(ittg->loc));
	}

	std::map<gamemap::location,paths> dummy_possible_moves;
	move_map fullmove_srcdst;
	move_map fullmove_dstsrc;
	calculate_possible_moves(dummy_possible_moves,fullmove_srcdst,fullmove_dstsrc,false,true);

	for(std::vector<location>::reverse_iterator ri =
	    best_route.steps.rbegin(); ri != best_route.steps.rend(); ++ri) {

		if(game_config::debug) {
			display::debug_highlight(*ri,0.2);
		}

		//this is set to 'true' if we are hesitant to proceed because of enemy units,
		//to rally troops around us.
		bool is_dangerous = false;

		typedef std::multimap<location,location>::const_iterator Itor;
		std::pair<Itor,Itor> its = dstsrc.equal_range(*ri);
		while(its.first != its.second) {
			if(its.first->second == best->first) {
				if(!should_retreat(its.first->first,fullmove_srcdst,fullmove_dstsrc,enemy_srcdst,enemy_dstsrc)) {
					const double value = best_target->value - best->second.type().cost()/20.0;

					if(value > 0.0) {
						//there are enemies ahead. Rally troops around us to
						//try to take the target
						if(is_dangerous) {
							targets.push_back(target(its.first->first,value*2.0));
						}

						best_target->value = value;
					} else {
						targets.erase(best_target);
					}
				
					return std::pair<location,location>(its.first->second,its.first->first);
				} else {
					is_dangerous = true;
				}
			}

			++its.first;
		}
	}

	if(best != units_.end()) {
		std::cerr  << "Could not make good move, staying still\n";

		//this sounds like the road ahead might be dangerous, and that's why we don't advance.
		//create this as a target, attempting to rally units around
		targets.push_back(target(best->first,best_target->value));
		return std::pair<location,location>(best->first,best->first);
	}

	std::cerr  << "Could not find anywhere to move!\n";
	return std::pair<location,location>();
}
