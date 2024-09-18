/*
	Copyright (C) 2009 - 2024
	by Yurii Chernyi <terraninfo@terraninfo.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 * Helper functions for the object which operates in the context of AI for specific side
 * This is part of AI interface
 * @file
 */

#include "ai/default/contexts.hpp"

#include "game_board.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "ai/composite/goal.hpp"
#include "pathfind/pathfind.hpp"

static lg::log_domain log_ai("ai/general");
#define DBG_AI LOG_STREAM(debug, log_ai)
#define LOG_AI LOG_STREAM(info, log_ai)
#define WRN_AI LOG_STREAM(warn, log_ai)
#define ERR_AI LOG_STREAM(err, log_ai)

// =======================================================================
namespace ai {

// default ai context
default_ai_context::default_ai_context()
{
}

default_ai_context::~default_ai_context()
{
}

// default ai context proxy

default_ai_context_proxy::~default_ai_context_proxy()
{
}

void default_ai_context_proxy::init_default_ai_context_proxy(default_ai_context &target)
{
	init_readwrite_context_proxy(target);
	target_= &target.get_default_ai_context();
}

default_ai_context_impl::~default_ai_context_impl()
{
}

int default_ai_context_impl::count_free_hexes_in_castle(const map_location &loc, std::set<map_location> &checked_hexes)
{
	int ret = 0;
	unit_map &units_ = resources::gameboard->units();
	for(const map_location& adj : get_adjacent_tiles(loc)) {
		if (checked_hexes.find(adj) != checked_hexes.end())
			continue;
		checked_hexes.insert(adj);
		if (resources::gameboard->map().is_castle(adj)) {
			const unit_map::const_iterator u = units_.find(adj);
			ret += count_free_hexes_in_castle(adj, checked_hexes);
			if (u == units_.end()
				|| (current_team().is_enemy(u->side())
					&& u->invisible(adj))
				|| ((&resources::gameboard->get_team(u->side()) == &current_team())
					&& u->movement_left() > 0)) {
				ret += 1;
			}
		}
	}
	return ret;
}

default_ai_context& default_ai_context_impl::get_default_ai_context(){
	return *this;
}

int default_ai_context_impl::rate_terrain(const unit& u, const map_location& loc) const
{
	const gamemap &map_ = resources::gameboard->map();
	const t_translation::terrain_code terrain = map_.get_terrain(loc);
	const int defense = u.defense_modifier(terrain);
	int rating = 100 - defense;

	const int healing_value = 10;
	const int friendly_village_value = 5;
	const int neutral_village_value = 10;
	const int enemy_village_value = 15;

	if(map_.gives_healing(terrain) && u.get_ability_bool("regenerate", loc) == false) {
		rating += healing_value;
	}

	if(map_.is_village(terrain)) {
		int owner = resources::gameboard->village_owner(loc);

		if(owner == get_side()) {
			rating += friendly_village_value;
		} else if(owner == 0) {
			rating += neutral_village_value;
		} else {
			rating += enemy_village_value;
		}
	}

	return rating;
}

std::vector<target> default_ai_context_impl::find_targets(const move_map& enemy_dstsrc)
{

	log_scope2(log_ai, "finding targets...");
	unit_map &units_ = resources::gameboard->units();
	unit_map::iterator leader = units_.find_leader(get_side());
	const gamemap &map_ = resources::gameboard->map();
	const bool has_leader = leader != units_.end();

	std::vector<target> targets;

	//=== start getting targets

	//if enemy units are in range of the leader, then we target the enemies who are in range.
	if(has_leader) {
		double threat = power_projection(leader->get_location(), enemy_dstsrc);
		if(threat > 0.0) {
			//find the location of enemy threats
			std::set<map_location> threats;

			for(const map_location& adj : get_adjacent_tiles(leader->get_location())) {
				std::pair<move_map::const_iterator,move_map::const_iterator> itors = enemy_dstsrc.equal_range(adj);
				while(itors.first != itors.second) {
					if(units_.count(itors.first->second)) {
						threats.insert(itors.first->second);
					}

					++itors.first;
				}
			}

			assert(threats.empty() == false);

			const double value = threat/static_cast<double>(threats.size());
			for(std::set<map_location>::const_iterator i = threats.begin(); i != threats.end(); ++i) {
				LOG_AI << "found threat target... " << *i << " with value: " << value;
				targets.emplace_back(*i,value,ai_target::type::threat);
			}
		}
	}

	double corner_distance = distance_between(map_location::ZERO(), map_location(map_.w(),map_.h()));
	double village_value = get_village_value();
	if(has_leader && village_value > 0.0) {
		std::map<map_location,pathfind::paths> friends_possible_moves;
		move_map friends_srcdst, friends_dstsrc;
		calculate_possible_moves(friends_possible_moves, friends_srcdst, friends_dstsrc, false, true);

		for(const map_location& village_loc : map_.villages()) {
			assert(map_.on_board(village_loc));

			bool ally_village = false;
			for(const team& t : resources::gameboard->teams()) {
				if(!current_team().is_enemy(t.side()) && t.owns_village(village_loc)) {
					ally_village = true;
					break;
				}
			}

			if (ally_village)
			{
				//Support seems to cause the AI to just 'sit around' a lot, so
				//only turn it on if it's explicitly enabled.
				if(get_support_villages()) {
					double enemy = power_projection(village_loc, enemy_dstsrc);
					if (enemy > 0)
					{
						enemy *= 1.7;
						double our = power_projection(village_loc, friends_dstsrc);
						double value = village_value * our / enemy;
						add_target(target(village_loc, value, ai_target::type::support));
					}
				}
			}
			else
			{
				double leader_distance = distance_between(village_loc, leader->get_location());
				double value = village_value * (1.0 - leader_distance / corner_distance);
				LOG_AI << "found village target... " << village_loc
					<< " with value: " << value
					<< " distance: " << leader_distance;
				targets.emplace_back(village_loc,value,ai_target::type::village);
			}
		}
	}

	std::vector<goal_ptr>& goals = get_goals();

	//find the enemy leaders and explicit targets
	unit_map::const_iterator u;
	if (get_leader_value()>0.0) {
		for(u = units_.begin(); u != units_.end(); ++u) {
			//is a visible enemy leader
			if (u->can_recruit() && current_team().is_enemy(u->side())
			    && !u->invisible(u->get_location())) {
				assert(map_.on_board(u->get_location()));
				LOG_AI << "found enemy leader (side: " << u->side() << ") target... " << u->get_location() << " with value: " << get_leader_value();
				targets.emplace_back(u->get_location(), get_leader_value(), ai_target::type::leader);
			}
		}

	}

	//explicit targets for this team
	for(std::vector<goal_ptr>::iterator j = goals.begin();
	    j != goals.end(); ++j) {

		if (!(*j)->active()) {
			continue;
		}
		(*j)->add_targets(std::back_inserter(targets));

	}

	//=== end getting targets

	std::vector<double> new_values;

	for(std::vector<target>::iterator i = targets.begin();
	    i != targets.end(); ++i) {

		new_values.push_back(i->value);

		for(std::vector<target>::const_iterator j = targets.begin(); j != targets.end(); ++j) {
			if(i->loc == j->loc) {
				continue;
			}

			const double distance = std::abs(j->loc.x - i->loc.x) +
						std::abs(j->loc.y - i->loc.y);
			new_values.back() += j->value/(distance*distance);
		}
	}

	assert(new_values.size() == targets.size());
	for(std::size_t n = 0; n != new_values.size(); ++n) {
		LOG_AI << "target value: " << targets[n].value << " -> " << new_values[n];
		targets[n].value = new_values[n];
	}

	return targets;
}

const std::vector<target>& default_ai_context_impl::additional_targets() const
{
	return additional_targets_;
}

void default_ai_context_impl::add_target(const target& t) const
{
	additional_targets_.push_back(t);
}

void default_ai_context_impl::clear_additional_targets() const
{
	additional_targets_.clear();
}

config default_ai_context_impl::to_default_ai_context_config() const
{
	return config();
}

} //of namespace ai
