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
#include "util.hpp"

#include <algorithm>
#include <cstdlib>
#include <sstream>

namespace {
	std::vector<team>* teams = NULL;
}

teams_manager::teams_manager(std::vector<team>& teams_list)
{
	teams = &teams_list;
}

teams_manager::~teams_manager()
{
	teams = NULL;
}

team::target::target(const config& cfg)
              : criteria(cfg), value(atof(cfg["value"].c_str()))
{
}

void team::target::write(config& cfg) const
{
	cfg = criteria;
}

team::team_info::team_info(const config& cfg)
{
	gold = cfg["gold"];
	income = cfg["income"];
	name = cfg["name"];
	team_name = cfg["team_name"];
	if(team_name.empty())
		team_name = cfg["side"];

	description = cfg["description"];

	const std::string& village_income = cfg["village_gold"];
	if(village_income.empty())
		income_per_village = game_config::tower_income;
	else
		income_per_village = atoi(village_income.c_str());

	const std::string& aggression_val = cfg["aggression"];
	if(aggression_val.empty())
		aggression = 0.5;
	else
		aggression = atof(aggression_val.c_str());

	const std::string& caution_val = cfg["caution"];
	if(caution_val.empty())
		caution = 0.25;
	else
		caution = atof(caution_val.c_str());

	const std::string& enemies_list = cfg["enemy"];
	if(!enemies_list.empty()) {
		std::vector<std::string> venemies = config::split(enemies_list);
		for(std::vector<std::string>::const_iterator i = venemies.begin();
		    i != venemies.end(); ++i) {
			enemies.push_back(atoi(i->c_str()));
		}
	}

	if(cfg["controller"] == "human")
		controller = HUMAN;
	else if(cfg["controller"] == "network")
		controller = NETWORK;
	else
		controller = AI;

	ai_algorithm = cfg["ai_algorithm"];

	const config* const ai_parameters = cfg.child("ai");
	if(ai_parameters != NULL) {
		ai_params = *ai_parameters;
	}

	const std::string& scouts_val = cfg["villages_per_scout"];
	if(scouts_val.empty()) {
		villages_per_scout = 4;
	} else {
		villages_per_scout = atoi(scouts_val.c_str());
	}

	const std::string& leader_val = cfg["leader_value"];
	if(leader_val.empty()) {
		leader_value = 3.0;
	} else {
		leader_value = atof(leader_val.c_str());
	}

	const std::string& village_val = cfg["village_value"];
	if(village_val.empty()) {
		village_value = 1.0;
	} else {
		village_value = atof(village_val.c_str());
	}

	std::vector<std::string> recruits = config::split(cfg["recruit"]);
	for(std::vector<std::string>::const_iterator i = recruits.begin();
	    i != recruits.end(); ++i) {
		can_recruit.insert(*i);
	}

	recruitment_pattern = config::split(cfg["recruitment_pattern"]);

	//default recruitment pattern is to buy 2 fighters for every 1 archer
	if(recruitment_pattern.empty()) {
		recruitment_pattern.push_back("fighter");
		recruitment_pattern.push_back("fighter");
		recruitment_pattern.push_back("archer");
	}

	//additional targets
	for(config::const_child_itors tgts = cfg.child_range("target");
	    tgts.first != tgts.second; ++tgts.first) {
		targets.push_back(target(**tgts.first));
	}

	use_shroud = (cfg["shroud"] == "yes");
	use_fog = (cfg["fog"] == "yes");
	share_maps = (cfg["share_maps"] != "no");

	music = cfg["music"];
}

void team::team_info::write(config& cfg) const
{
	cfg["gold"] = gold;
	cfg["income"] = income;
	cfg["name"] = name;
	cfg["team_name"] = team_name;
	cfg["description"] = description;

	char buf[50];
	sprintf(buf,"%d",income_per_village);
	cfg["village_gold"] = buf;

	sprintf(buf,"%f",aggression);
	cfg["aggression"] = buf;

	std::stringstream enemies_str;
	for(std::vector<int>::const_iterator en = enemies.begin(); en != enemies.end(); ++en) {
		enemies_str << *en;
		if(en+1 != enemies.end())
			enemies_str << ",";
	}

	cfg["enemy"] = enemies_str.str();

	switch(controller) {
	case AI: cfg["controller"] = "ai"; break;
	case HUMAN: cfg["controller"] = "human"; break;
	case NETWORK: cfg["controller"] = "network"; break;
	default: assert(false);
	}

	sprintf(buf,"%d",villages_per_scout);
	cfg["villages_per_scout"] = buf;

	sprintf(buf,"%f",leader_value);
	cfg["leader_value"] = buf;

	sprintf(buf,"%f",village_value);
	cfg["village_value"] = buf;

	for(std::vector<target>::const_iterator tg = targets.begin(); tg != targets.end(); ++tg) {
		tg->write(cfg.add_child("target"));
	}

	std::stringstream can_recruit_str;
	for(std::set<std::string>::const_iterator cr = can_recruit.begin(); cr != can_recruit.end(); ++cr) {
		if(cr != can_recruit.begin())
			can_recruit_str << ",";

		can_recruit_str << *cr;
	}

	cfg["recruit"] = can_recruit_str.str();

	std::stringstream recruit_pattern_str;
	for(std::vector<std::string>::const_iterator p = recruitment_pattern.begin(); p != recruitment_pattern.end(); ++p) {
		if(p != recruitment_pattern.begin())
			recruit_pattern_str << ",";

		recruit_pattern_str << *p;
	}

	cfg["recruitment_pattern"] = recruit_pattern_str.str();

	cfg["shroud"] = use_shroud ? "yes" : "no";
	cfg["fog"] = use_fog ? "yes" : "no";
	cfg["share_maps"] = share_maps ? "yes" : "no";

	if(music.empty() == false)
		cfg["music"] = music;
}

team::team(const config& cfg, int gold) : gold_(gold), info_(cfg)
{
	//gold is the maximum of 'gold' and what is given in the config file
	if(info_.gold.empty() == false)
		gold_ = maximum(gold,::atoi(info_.gold.c_str()));

	//load in the villages the side controls at the start
	const config::child_list& villages = cfg.get_children("village");
	for(config::child_list::const_iterator v = villages.begin(); v != villages.end(); ++v) {
		towers_.insert(gamemap::location(**v));
	}

	const std::string& shroud_data = cfg["shroud_data"];
	for(std::string::const_iterator sh = shroud_data.begin(); sh != shroud_data.end(); ++sh) {
		if(*sh == '|')
			shroud_.resize(shroud_.size()+1);

		if(shroud_.empty() == false) {
			if(*sh == '1')
				shroud_.back().push_back(true);
			else if(*sh == '0')
				shroud_.back().push_back(false);
		}
	}
}

void team::write(config& cfg) const
{
	info_.write(cfg);

	char buf[50];
	sprintf(buf,"%d",gold_);
	cfg["gold"] = buf;

	//write village locations
	for(std::set<gamemap::location>::const_iterator t = towers_.begin(); t != towers_.end(); ++t) {
		t->write(cfg.add_child("village"));
	}

	std::stringstream shroud_str;
	for(std::vector<std::vector<bool> >::const_iterator sh = shroud_.begin(); sh != shroud_.end(); ++sh) {
		shroud_str << '|';

		for(std::vector<bool>::const_iterator i = sh->begin(); i != sh->end(); ++i) {
			shroud_str << (*i ? '1' : '0');
		}

		shroud_str << '\n';
	}

	cfg["shroud_data"] = shroud_str.str();
}

void team::get_tower(const gamemap::location& loc)
{
	towers_.insert(loc);
}

void team::lose_tower(const gamemap::location& loc)
{
	if(owns_tower(loc)) {
		towers_.erase(towers_.find(loc));
	}
}

void team::clear_towers()
{
	towers_.clear();
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
	return atoi(info_.income.c_str()) +
	       towers_.size()*info_.income_per_village+game_config::base_income;
}

void team::new_turn()
{
	gold_ += income();
}

void team::get_shared_maps()
{
	if(teams == NULL || info_.team_name == "" || info_.share_maps == false) {
		return;
	}

	for(std::vector<team>::const_iterator t = teams->begin(); t != teams->end(); ++t) {
		if(t->info_.team_name != info_.team_name) {
			continue;
		}

		const shroud_map& v = t->shroud_;
		for(size_t x = 0; x != v.size(); ++x) {
			for(size_t y = 0; y != v[x].size(); ++y) {
				if(v[x][y]) {
					clear_shroud(x,y);
				}
			}
		}
	}
}

void team::spend_gold(int amount)
{
	gold_ -= amount;
}

const std::set<std::string>& team::recruits() const
{
	return info_.can_recruit;
}

std::set<std::string>& team::recruits()
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
	//if we have a team name, we are friends with anyone who has the same team name
	if(info_.team_name.empty() == false) {
		return teams != NULL && size_t(n-1) < teams->size() &&
	           (*teams)[n-1].info_.team_name != info_.team_name;
	}

	//if enemies aren't listed, then everyone is an enemy
	if(info_.enemies.empty())
		return true;

	return std::find(info_.enemies.begin(),info_.enemies.end(),n) != info_.enemies.end();
}

double team::aggression() const
{
	return info_.aggression;
}

double team::caution() const
{
	return info_.caution;
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

void team::make_human()
{
	info_.controller = team_info::HUMAN;
}

void team::make_ai()
{
	info_.controller = team_info::AI;
}

const std::string& team::team_name() const
{
	return info_.team_name;
}

const std::string& team::ai_algorithm() const
{
	return info_.ai_algorithm;
}

const config& team::ai_parameters() const
{
	return info_.ai_params;
}

void team::make_network()
{
	info_.controller = team_info::NETWORK;
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

bool team::clear_shroud(size_t x, size_t y)
{
	if(info_.use_shroud == false)
		return false;

	if(x >= shroud_.size())
		shroud_.resize(x+1);

	if(y >= shroud_[x].size())
		shroud_[x].resize(y+1);

	if(shroud_[x][y] == false) {
		shroud_[x][y] = true;
		return true;
	} else {
		return false;
	}
}

bool team::uses_fog() const
{
	return info_.use_fog;
}

bool team::fogged(size_t x, size_t y) const
{
	if(info_.use_fog == false)
		return shrouded(x,y);

	if(x >= fog_.size())
		return true;

	if(y >= fog_[x].size())
		return true;

	if(fog_[x][y])
		return shrouded(x,y);
	else
		return true;
}

bool team::clear_fog(size_t x, size_t y)
{
	if(info_.use_fog == false)
		return false;

	if(x >= fog_.size())
		fog_.resize(x+1);

	if(y >= fog_[x].size())
		fog_[x].resize(y+1);

	if(fog_[x][y] == false) {
		fog_[x][y] = true;
		return true;
	} else {
		return false;
	}
}

void team::refog()
{
	for(std::vector<std::vector<bool> >::iterator i = fog_.begin();
	    i != fog_.end(); ++i) {
		std::fill(i->begin(),i->end(),false);
	}
}

const std::string& team::music() const
{
	return info_.music;
}

int team::nteams()
{
	if(teams == NULL) {
		return 0;
	} else {
		return teams->size();
	}
}

const std::set<gamemap::location> vacant_towers(const std::set<gamemap::location>& towers, const unit_map& units)
{
	std::set<gamemap::location> res;
	
	for(std::set<gamemap::location>::const_iterator i = towers.begin(); i != towers.end(); ++i) {
		if(units.count(*i) == 0) {
			res.insert(*i);
		}
	}

	return res;
}

bool is_observer()
{
	if(teams == NULL) {
		return true;
	}

	for(std::vector<team>::const_iterator i = teams->begin(); i != teams->end(); ++i) {
		if(i->is_human()) {
			return false;
		}
	}

	return true;
}
