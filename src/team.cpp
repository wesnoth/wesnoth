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
#include "game_config.hpp"
#include "replay.hpp"
#include "team.hpp"

#include <algorithm>
#include <cstdlib>

team::target::target(config& cfg)
              : criteria(cfg), value(atof(cfg.values["value"].c_str()))
{
}

team::team_info::team_info(config& cfg)
{
	gold = cfg.values["gold"];
	name = cfg.values["name"];

	const std::string& aggression_val = cfg.values["aggression"];
	if(aggression_val.empty())
		aggression = 0.5;
	else
		aggression = atof(aggression_val.c_str());

	const std::string& enemies_list = cfg.values["enemy"];
	if(!enemies_list.empty()) {
		std::vector<std::string> venemies = config::split(enemies_list);
		for(std::vector<std::string>::const_iterator i = venemies.begin();
		    i != venemies.end(); ++i) {
			enemies.push_back(atoi(i->c_str()));
		}
	}

	if(cfg.values["controller"] == "human")
		controller = HUMAN;
	else if(cfg.values["controller"] == "network")
		controller = NETWORK;
	else
		controller = AI;

	const std::string& scouts_val = cfg.values["villages_per_scout"];
	if(scouts_val.empty()) {
		villages_per_scout = 8;
	} else {
		villages_per_scout = atoi(scouts_val.c_str());
	}

	const std::string& leader_val = cfg.values["leader_value"];
	if(leader_val.empty()) {
		leader_value = 3.0;
	} else {
		leader_value = atof(leader_val.c_str());
	}

	const std::string& village_val = cfg.values["village_value"];
	if(village_val.empty()) {
		village_value = 1.0;
	} else {
		village_value = atof(village_val.c_str());
	}

	std::vector<std::string> recruits = config::split(cfg.values["recruit"]);
	for(std::vector<std::string>::const_iterator i = recruits.begin();
	    i != recruits.end(); ++i) {
		can_recruit.insert(*i);
	}

	recruitment_pattern = config::split(cfg.values["recruitment_pattern"]);

	//default recruitment pattern is to buy 2 fighters for every 1 archer
	if(recruitment_pattern.empty()) {
		recruitment_pattern.push_back("fighter");
		recruitment_pattern.push_back("fighter");
		recruitment_pattern.push_back("archer");
	}

	//additional targets
	std::vector<config*>& tgts = cfg.children["target"];
	for(std::vector<config*>::iterator tgt = tgts.begin();
	    tgt != tgts.end(); ++tgt) {
		targets.push_back(target(**tgt));
	}

	use_shroud = (cfg.values["shroud"] == "yes");
}

team::team(config& cfg, int gold) : gold_(gold), info_(cfg)
{
	if(info_.gold.empty() == false)
		gold_ = ::atoi(info_.gold.c_str());
}

void team::get_tower(const gamemap::location& loc)
{
	towers_.insert(loc);
}

void team::lose_tower(const gamemap::location& loc)
{
	towers_.erase(towers_.find(loc));
}

const std::set<gamemap::location>& team::towers() const
{
	return towers_;
}

bool team::owns_tower(const gamemap::location& loc) const
{
	return towers_.count(loc) > 0;
}

int team::gold() const
{
	return gold_;
}

int team::income() const
{
	return towers_.size()*game_config::tower_income+game_config::base_income;
}

void team::new_turn()
{
	gold_ += income();
}

void team::spend_gold(int amount)
{
	gold_ -= amount;
}

const std::set<std::string>& team::recruits() const
{
	return info_.can_recruit;
}

const std::vector<std::string>& team::recruitment_pattern() const
{
	return info_.recruitment_pattern;
}

const std::string& team::name() const
{
	return info_.name;
}

bool team::is_enemy(int n) const
{
	//if enemies aren't listed, then everyone is an enemy
	if(info_.enemies.empty())
		return true;

	return std::find(info_.enemies.begin(),info_.enemies.end(),n) !=
	                                       info_.enemies.end();
}

double team::aggression() const
{
	return info_.aggression;
}

bool team::is_human() const
{
	return info_.controller == team_info::HUMAN;
}

bool team::is_ai() const
{
	return info_.controller == team_info::AI;
}

bool team::is_network() const
{
	return info_.controller == team_info::NETWORK;
}

double team::leader_value() const
{
	return info_.leader_value;
}

double team::village_value() const
{
	return info_.village_value;
}

int team::villages_per_scout() const
{
	return info_.villages_per_scout;
}

std::vector<team::target>& team::targets()
{
	return info_.targets;
}

bool team::uses_shroud() const
{
	return info_.use_shroud;
}

bool team::shrouded(size_t x, size_t y) const
{
	if(info_.use_shroud == false)
		return false;

	if(x >= shroud_.size())
		return true;

	if(y >= shroud_[x].size())
		return true;

	return !shroud_[x][y];
}

void team::clear_shroud(size_t x, size_t y)
{
	if(info_.use_shroud == false)
		return;

	if(x >= shroud_.size())
		shroud_.resize(x+1);

	if(y >= shroud_[x].size())
		shroud_[x].resize(y+1);

	shroud_[x][y] = true;
}
