/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file team.cpp
 *  Team-management, allies, setup at start of scenario.
 */

#include "global.hpp"

#include "ai_configuration.hpp"
#include "ai_manager.hpp"
#include "foreach.hpp"
#include "game_events.hpp"
#include "gamestatus.hpp"
#include "log.hpp"
#include "map.hpp"


#define LOG_NG LOG_STREAM(info, engine)
#define WRN_NG LOG_STREAM(warn, engine)


namespace {
	std::vector<team>* teams = NULL;
}

//static member initialization
const int team::default_team_gold = 100;

teams_manager::teams_manager(std::vector<team>& teams_list)
{
	assert(!teams);
	teams = &teams_list;
}

teams_manager::~teams_manager()
{
	teams = NULL;
}

const std::vector<team>& teams_manager::get_teams()
{
	assert(teams);
	return *teams;
}

bool teams_manager::is_observer()
{
	for(std::vector<team>::const_iterator i = teams->begin(); i != teams->end(); ++i) {
		if(i->is_local()) {
			return false;
		}
	}

	return true;
}

team::target::target(const config& cfg)
              : criteria(cfg), value(atof(cfg["value"].c_str()))
{
}

void team::target::write(config& cfg) const
{
	cfg = criteria;
}

team::team_info::team_info(const config& cfg) :
		name(cfg["name"]),
		gold(cfg["gold"]),
		start_gold(),
		income(cfg["income"]),
		income_per_village(),
		average_price(0),
		number_of_possible_recruits_to_force_recruit(),
		can_recruit(),
		global_recruitment_pattern(),
		recruitment_pattern(),
		enemies(),
		team_name(cfg["team_name"]),
		user_team_name(cfg["user_team_name"]),
		save_id(cfg["save_id"]),
		current_player(cfg["current_player"]),
		countdown_time(cfg["countdown_time"]),
		action_bonus_count(lexical_cast_default<int>(cfg["action_bonus_count"])),
		flag(cfg["flag"]),
		flag_icon(cfg["flag_icon"]),
		description(cfg["id"]),
		objectives(cfg["objectives"]),
		objectives_changed(utils::string_bool(cfg["objectives_changed"])),
		controller(),
		villages_per_scout(),
		leader_value(),
		village_value(),
		aggression_(),
		caution_(),
		targets(),
		share_maps(false),
		share_view(false),
		disallow_observers(utils::string_bool(cfg["disallow_observers"])),
		allow_player(utils::string_bool(cfg["allow_player"], true)),
		no_leader(utils::string_bool(cfg["no_leader"])),
		hidden(utils::string_bool(cfg["hidden"])),
		music(cfg["music"]),
		colour(cfg["colour"].size() ? cfg["colour"] : cfg["side"]),
		side(lexical_cast_default<int>(cfg["side"], 1))
{
	// If are starting new scenario overide settings from [ai] tags
	if (!user_team_name.translatable())
		user_team_name = user_team_name.from_serialized(user_team_name);

	config ai_memory;
	config global_ai_parameters;//AI parameters which do not have a filter applied
	std::vector<config> ai_parameters;//AI parameters inside [ai] tags. May contain filters.
	const config& default_ai_parameters = ai_configuration::get_default_ai_parameters();
	std::string ai_algorithm_type;
	config effective_ai_params;//Needed only to set some legacy stuff in team_info

	ai_configuration::parse_side_config(cfg, ai_algorithm_type, global_ai_parameters, ai_parameters, default_ai_parameters, ai_memory, effective_ai_params );
	ai_manager::add_ai_for_side(side,ai_algorithm_type,true);

	ai_manager::set_active_ai_effective_parameters_for_side(side,effective_ai_params);
	ai_manager::set_active_ai_global_parameters_for_side(side,global_ai_parameters);
	ai_manager::set_active_ai_memory_for_side(side,ai_memory);
	ai_manager::set_active_ai_parameters_for_side(side,ai_parameters);


	//legacy parameters
	number_of_possible_recruits_to_force_recruit = lexical_cast<float>(effective_ai_params["number_of_possible_recruits_to_force_recruit"]);
	villages_per_scout = lexical_cast<int>(effective_ai_params["villages_per_scout"]);
	leader_value = lexical_cast<double>(effective_ai_params["leader_value"]);
	village_value = lexical_cast<double>(effective_ai_params["village_value"]);
	aggression_ = lexical_cast<double>(effective_ai_params["aggression"]);
	caution_ = lexical_cast<double>(effective_ai_params["caution"]);

	//START OF MESSY CODE
	//========================================================================
	//this part will be cleaned later

		std::vector<std::string> recruits = utils::split(cfg["recruit"]);
		for(std::vector<std::string>::const_iterator i = recruits.begin(); i != recruits.end(); ++i) {
			can_recruit.insert(*i);
		}

		recruitment_pattern = utils::split(cfg["recruitment_pattern"]);

				if(recruitment_pattern.empty())
				{
					recruitment_pattern =
						utils::split(global_ai_parameters["recruitment_pattern"]);
				}
		// Keep a copy of the initial recruitment_pattern,
		// since it can be changed on a per-time-of-day
		// or per-turn basis inside [ai] sections.
		global_recruitment_pattern = recruitment_pattern;


		// Additional targets
		foreach (const config &tgt, cfg.child_range("target")) {
			targets.push_back(target(tgt));
		}

		foreach (const config &tgt, global_ai_parameters.child_range("target")) {
			targets.push_back(target(tgt));
		}

	//all past this point should be non-ai code
	//========================================================

	// at the start of a scenario "start_gold" is not set, we need to take the
	// value from the gold setting (or fall back to the gold default)
	if (!cfg["start_gold"].empty())
		start_gold = cfg["start_gold"];
	else if (!this->gold.empty())
		start_gold = this->gold;
	else
		start_gold = str_cast(default_team_gold);

	if(team_name.empty()) {
		team_name = cfg["side"];
	}

	if(save_id.empty()) {
		save_id = description;
	}
	if (current_player.empty()) {
		current_player = save_id;
	}

	const std::string temp_rgb_str = cfg["team_rgb"];
	std::map<std::string, color_range>::iterator global_rgb = game_config::team_rgb_range.find(cfg["side"]);

	if(!temp_rgb_str.empty()){
		std::vector<Uint32> temp_rgb = string2rgb(temp_rgb_str);
		team_color_range_[side] = color_range(temp_rgb);
	}else if(global_rgb != game_config::team_rgb_range.end()){
		team_color_range_[side] = global_rgb->second;
	}

	const std::string& village_income = cfg["village_gold"];
	if(village_income.empty())
		income_per_village = game_config::village_income;
	else
		income_per_village = lexical_cast_default<int>(village_income, game_config::village_income);

	const std::string& enemies_list = cfg["enemy"];
	if(!enemies_list.empty()) {
		std::vector<std::string> venemies = utils::split(enemies_list);
		for(std::vector<std::string>::const_iterator i = venemies.begin(); i != venemies.end(); ++i) {
			enemies.push_back(atoi(i->c_str()));
		}
	}

	std::string control = cfg["controller"];
	if (control == "human")
		controller = HUMAN;
	else if (control == "human_ai")
		controller = HUMAN_AI;
	else if (control == "network")
		controller = NETWORK;
	else if (control == "network_ai")
		controller = NETWORK_AI;
	else if (control == "null")
	{
		disallow_observers = utils::string_bool(cfg["disallow_observers"],true);
		controller = EMPTY;
	}
	else
		controller = AI;

	//========================================================
	//END OF MESSY CODE

	// Share_view and share_maps can't both be enabled,
	// so share_view overrides share_maps.
	share_view = utils::string_bool(cfg["share_view"]);
	share_maps = !share_view && utils::string_bool(cfg["share_maps"],true);

	LOG_NG << "team_info::team_info(...): team_name: " << team_name
	       << ", share_maps: " << share_maps << ", share_view: " << share_view << ".\n";
}

void team::team_info::write(config& cfg) const
{
	const std::vector<config> &ai_params = ai_manager::get_active_ai_parameters_for_side(side);;
	for(std::vector<config>::const_iterator ai = ai_params.begin(); ai != ai_params.end(); ++ai) {
		cfg.add_child("ai",*ai);
	}
	const config &ai_memory_ = ai_manager::get_active_ai_memory_for_side(side);
	if(!ai_memory_.empty()) cfg.add_child("ai_memory", ai_memory_ );
	cfg["ai_algorithm"] = ai_manager::get_active_ai_algorithm_type_for_side(side);

	cfg["gold"] = gold;
	cfg["start_gold"] = start_gold;
	cfg["income"] = income;
	cfg["name"] = name;
	cfg["team_name"] = team_name;
	cfg["user_team_name"] = user_team_name;
	cfg["save_id"] = save_id;
	cfg["current_player"] = current_player;
	cfg["flag"] = flag;
	cfg["flag_icon"] = flag_icon;
	cfg["id"] = description;
	cfg["objectives"] = objectives;
	cfg["objectives_changed"] = objectives_changed ? "yes" : "no";
	cfg["countdown_time"]= countdown_time;
	cfg["action_bonus_count"]= str_cast(action_bonus_count);
	cfg["village_gold"] = str_cast(income_per_village);
	cfg["disallow_observers"] = disallow_observers ? "yes" : "no";
	cfg["allow_player"] = allow_player ? "yes" : "no";
	cfg["no_leader"] = no_leader ? "yes" : "no";
	cfg["hidden"] = hidden ? "yes" : "no";
	cfg["number_of_possible_recruits_to_force_recruit"] = lexical_cast<std::string>(number_of_possible_recruits_to_force_recruit);

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
	case HUMAN_AI: cfg["controller"] = "human_ai"; break;
	case NETWORK: cfg["controller"] = "network"; break;
	case NETWORK_AI: cfg["controller"] = "network_ai"; break;
	case EMPTY: cfg["controller"] = "null"; break;
	default: assert(false); return;
	}

	cfg["villages_per_scout"] = str_cast(villages_per_scout);
	cfg["leader_value"] = str_cast(leader_value);
	cfg["village_value"] = str_cast(village_value);
	cfg["aggression"] = str_cast(aggression_);
	cfg["caution"] = str_cast(caution_);

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

	std::stringstream global_recruit_pattern_str;
	for(std::vector<std::string>::const_iterator p = global_recruitment_pattern.begin();
		   	p != global_recruitment_pattern.end(); ++p) {
		if(p != global_recruitment_pattern.begin())
			global_recruit_pattern_str << ",";

		global_recruit_pattern_str << *p;
	}

	cfg["global_recruitment_pattern"] = global_recruit_pattern_str.str();

	std::stringstream recruit_pattern_str;
	for(std::vector<std::string>::const_iterator p = recruitment_pattern.begin();
		   	p != recruitment_pattern.end(); ++p) {
		if(p != recruitment_pattern.begin())
			recruit_pattern_str << ",";

		recruit_pattern_str << *p;
	}

	cfg["recruitment_pattern"] = recruit_pattern_str.str();

	cfg["share_maps"] = share_maps ? "yes" : "no";
	cfg["share_view"] = share_view ? "yes" : "no";

	if(music.empty() == false)
		cfg["music"] = music;

	cfg["colour"] = lexical_cast_default<std::string>(colour);
}

void team::merge_shroud_map_data(const std::string& shroud_data)
{
	shroud_.merge(shroud_data);
}

team::team(const config& cfg, const gamemap& map, int gold) :
		gold_(gold),
		villages_(),
		shroud_(),
		fog_(),
		auto_shroud_updates_(true),
		info_(cfg),
		countdown_time_(0),
		action_bonus_count_(0),
		enemies_(),
		seen_(),
		ally_shroud_(),
		ally_fog_()
{
	fog_.set_enabled( utils::string_bool(cfg["fog"]) );
	shroud_.set_enabled( utils::string_bool(cfg["shroud"]) );
	shroud_.read(cfg["shroud_data"]);

	LOG_NG << "team::team(...): team_name: " << info_.team_name
	       << ", shroud: " << uses_shroud() << ", fog: " << uses_fog() << ".\n";

	// To ensure some mimimum starting gold,
	// gold is the maximum of 'gold' and what is given in the config file
	if(info_.gold.empty() == false)
	{
		gold_ = std::max(gold,::atoi(info_.gold.c_str()));
		if (gold_ != ::atoi(info_.gold.c_str()))
			info_.start_gold = str_cast(gold) + " (" + info_.start_gold + ")";
	}

	// Load in the villages the side controls at the start
	foreach (const config &v, cfg.child_range("village"))
	{
		map_location loc(v, game_events::get_state_of_game());
		if (map.is_village(loc)) {
			villages_.insert(loc);
		} else {
			WRN_NG << "[side] " << name() << " [village] points to a non-village location " << loc << "\n";
		}
	}

	countdown_time_=lexical_cast_default<int>(cfg["countdown_time"],0);
	action_bonus_count_=lexical_cast_default<int>(cfg["action_bonus_count"],0);
}

void team::write(config& cfg) const
{
	info_.write(cfg);
	cfg["shroud"] = uses_shroud() ? "yes" : "no";
	cfg["fog"] = uses_fog() ? "yes" : "no";
	cfg["gold"] = str_cast(gold_);

	// Write village locations
	for(std::set<map_location>::const_iterator t = villages_.begin(); t != villages_.end(); ++t) {
		t->write(cfg.add_child("village"));
	}

	cfg["shroud_data"] = shroud_.write();

	cfg["countdown_time"] = str_cast(countdown_time_);
	cfg["action_bonus_count"] = str_cast(action_bonus_count_);
}

bool team::get_village(const map_location& loc)
{
	villages_.insert(loc);
	return game_events::fire("capture",loc);
}

void team::lose_village(const map_location& loc)
{
	if(owns_village(loc)) {
		villages_.erase(villages_.find(loc));
	}
}

void team::remove_recruit(const std::string& recruit)
{
	info_.can_recruit.erase(recruit);
	info_.average_price = 0;
}

void team::set_recruits(const std::set<std::string>& recruits)
{
	info_.can_recruit = recruits;
	info_.average_price = 0;
}

void team::add_recruits(const std::set<std::string>& recruits)
{
	std::copy(recruits.begin(),recruits.end(),
			std::inserter(info_.can_recruit, info_.can_recruit.end()));
	info_.average_price = 0;
}


namespace {
	struct count_average {
		count_average(size_t& a) : a_(a), sum_(0), count_(0)
		{}
		~count_average() {
			// If no recruits disable leader from moving to keep
			if (count_ == 0)
				a_ = 424242;
			else
				a_ = sum_/count_;
		}
		void operator()(const std::string& recruit)
		{
			unit_type_data::unit_type_map::const_iterator i = unit_type_data::types().find_unit_type(recruit);
			if (i == unit_type_data::types().end())
				return;
			++count_;
			sum_ += i->second.cost();
		}
		private:
		size_t& a_;
		size_t sum_;
		size_t count_;
	};
}

size_t team::average_recruit_price()
{
	if (info_.average_price)
		return info_.average_price;

	{
		count_average avg(info_.average_price);
		BOOST_FOREACH(std::string type, info_.can_recruit){
			avg(type);
		}
	}
	return info_.average_price;
}

void team::set_time_of_day(int turn, const time_of_day& tod)
{
	config aiparams_effective;//current effective_parameters_for_team are to be cleared
	const std::vector<config>& ai_params = ai_manager::get_active_ai_parameters_for_side(info_.side);

	for(std::vector<config>::const_iterator i = ai_params.begin(); i != ai_params.end(); ++i) {
		const std::string& time_of_day = (*i)["time_of_day"];
		if(time_of_day.empty() == false) {
			const std::vector<std::string>& times = utils::split(time_of_day);
			if(std::count(times.begin(),times.end(),tod.id) == 0) {
				continue;
			}
		}

		const std::string& turns = (*i)["turns"];
		if(turns.empty() == false) {
			bool matched = false;

			const std::vector<std::string>& turns_list = utils::split(turns);
			for(std::vector<std::string>::const_iterator j = turns_list.begin(); j != turns_list.end(); ++j) {
				const std::pair<int,int> range = utils::parse_range(*j);
				if(turn >= range.first && turn <= range.second) {
					matched = true;
					break;
				}
			}

			if(!matched) {
				continue;
			}
		}

		aiparams_effective.append(*i);
	}

    // Get the recruitment pattern from the matching [ai] section,
    // and fall back to the global recruitment pattern otherwise.
	info_.recruitment_pattern = utils::split(aiparams_effective["recruitment_pattern"]);
	if (info_.recruitment_pattern.empty())
		info_.recruitment_pattern = info_.global_recruitment_pattern;
	info_.aggression_ = lexical_cast_default<double>(aiparams_effective["aggression"],info_.aggression_);
	info_.caution_ = lexical_cast_default<double>(aiparams_effective["caution"],info_.caution_);
	info_.number_of_possible_recruits_to_force_recruit = lexical_cast_default<float>(aiparams_effective["number_of_possible_recruits_to_force_recruit"],info_.number_of_possible_recruits_to_force_recruit);
	ai_manager::set_active_ai_effective_parameters_for_side(info_.side,aiparams_effective);
}

bool team::calculate_enemies(size_t index) const
{
	if(teams == NULL || index >= teams->size()) {
		return false;
	}

	while(enemies_.size() <= index) {
		enemies_.push_back(calculate_is_enemy(enemies_.size()));
	}

	return enemies_.back();
}

bool team::calculate_is_enemy(size_t index) const
{
	// We're not enemies of ourselves
	if(&(*teams)[index] == this) {
		return false;
	}

	// If we have a team name, we are friends
	// with anyone who has the same team name
	if(info_.team_name.empty() == false) {
		return (*teams)[index].info_.team_name != info_.team_name;
	}

	// If enemies aren't listed, then everyone is an enemy
	if(info_.enemies.empty())
		return true;

	return std::find(info_.enemies.begin(),info_.enemies.end(),int(index+1)) != info_.enemies.end();
}

void team::set_share_view( bool share_view ){
	info_.share_view = share_view;
}

void team::change_controller(const std::string& controller)
{
	team::team_info::CONTROLLER cid;
	if (controller == "human")
		cid = team::team_info::HUMAN;
	else if (controller == "human_ai")
		cid = team::team_info::HUMAN_AI;
	else if (controller == "network")
		cid = team::team_info::NETWORK;
	else if (controller == "network_ai")
		cid = team::team_info::NETWORK_AI;
	else if (controller == "null")
		cid = team::team_info::EMPTY;
	else
		cid = team::team_info::AI;

	info_.controller = cid;
}

void team::change_team(const std::string& name, const std::string& user_name)
{
	info_.team_name = name;
	if (!user_name.empty())
	{
		info_.user_team_name = user_name;
	}
	else
	{
		info_.user_team_name = name;
	}

	clear_caches();	
}

void team::clear_caches(){
	// Reset the cache of allies for all teams
	if(teams != NULL) {
		for(std::vector<team>::const_iterator i = teams->begin(); i != teams->end(); ++i) {
			i->enemies_.clear();
			i->ally_shroud_.clear();
			i->ally_fog_.clear();
		}
	}
}

void team::set_objectives(const t_string& new_objectives, bool silently)
{
	info_.objectives = new_objectives;
	if(!silently)
		info_.objectives_changed = true;
}

const std::string& team::ai_algorithm() const
{
	return ai_manager::get_active_ai_algorithm_type_for_side(info_.side);
}

const config& team::ai_parameters() const
{
	return ai_manager::get_active_ai_effective_parameters_for_side(info_.side);
}

const config& team::ai_memory() const
{
	return ai_manager::get_active_ai_memory_for_side(info_.side);
}

void team::set_ai_memory(const config& ai_mem){
	ai_manager::set_active_ai_memory_for_side(info_.side,ai_mem);
}

void team::set_ai_parameters(const config::const_child_itors &ai_parameters)
{
	std::vector<config> ai_params;
	foreach (const config &p, ai_parameters) {
		ai_params.push_back(p);
	}
	ai_manager::set_active_ai_parameters_for_side(info_.side,ai_params);
}

bool team::shrouded(const map_location& loc) const
{
	if(!teams)
		return shroud_.value(loc.x+1,loc.y+1);

	return shroud_.shared_value(ally_shroud(*teams),loc.x+1,loc.y+1);
}

bool team::fogged(const map_location& loc) const
{
	if(shrouded(loc)) return true;

	if(!teams)
		return fog_.value(loc.x+1,loc.y+1);

	return fog_.shared_value(ally_fog(*teams),loc.x+1,loc.y+1);
}

const std::vector<const team::shroud_map*>& team::ally_shroud(const std::vector<team>& teams) const
{
	if(ally_shroud_.empty()) {
		for(size_t i = 0; i < teams.size(); ++i) {
			if(!is_enemy(i + 1) && (&(teams[i]) == this || teams[i].share_view() || teams[i].share_maps())) {
				ally_shroud_.push_back(&(teams[i].shroud_));
			}
		}
	}

	return ally_shroud_;
}

const std::vector<const team::shroud_map*>& team::ally_fog(const std::vector<team>& teams) const
{
	if(ally_fog_.empty()) {
		for(size_t i = 0; i < teams.size(); ++i) {
			if(!is_enemy(i + 1) && (&(teams[i]) == this || teams[i].share_view())) {
				ally_fog_.push_back(&(teams[i].fog_));
			}
		}
	}

	return ally_fog_;
}

bool team::knows_about_team(size_t index, bool is_multiplayer) const
{
	const team& t = (*teams)[index];

	// We know about our own team
	if(this == &t) return true;

	// If we aren't using shroud or fog, then we know about everyone
	if(!uses_shroud() && !uses_fog()) return true;

	// We don't know about enemies
	if(is_enemy(index+1)) return false;

	// We know our allies in multiplayer
	if (is_multiplayer) return true;

	// We know about allies we're sharing maps with
	if(share_maps() && t.uses_shroud()) return true;

	// We know about allies we're sharing view with
	if(share_view() && (t.uses_fog() || t.uses_shroud())) return true;

	return false;
}

bool team::copy_ally_shroud()
{
	if(!teams || !share_maps())
		return false;

	return shroud_.copy_from(ally_shroud(*teams));
}

int team::nteams()
{
	if(teams == NULL) {
		return 0;
	} else {
		return teams->size();
	}
}

bool is_observer()
{
	if(teams == NULL) {
		return true;
	}

	for(std::vector<team>::const_iterator i = teams->begin(); i != teams->end(); ++i) {
		if(i->is_local()) {
			return false;
		}
	}

	return true;
}

void validate_side(int side)
{
	if(teams == NULL) {
		return;
	}

	if(side < 1 || side > int(teams->size())) {
		throw game::game_error("invalid side(" + str_cast(side) + ") found in unit definition");
	}
}

bool team::shroud_map::clear(size_t x, size_t y)
{
	if(enabled_ == false)
		return false;

	if(x >= data_.size())
		data_.resize(x+1);

	if(y >= data_[x].size())
		data_[x].resize(y+1);

	if(data_[x][y] == false) {
		data_[x][y] = true;
		return true;
	} else {
		return false;
	}
}

void team::shroud_map::place(size_t x, size_t y)
{
	if(enabled_ == false)
		return;

	if(x < data_.size() && y < data_[x].size()) {
		data_[x][y] = false;
	}
}

void team::shroud_map::reset()
{
	if(enabled_ == false)
		return;

	for(std::vector<std::vector<bool> >::iterator i = data_.begin(); i != data_.end(); ++i) {
		std::fill(i->begin(),i->end(),false);
	}
}

bool team::shroud_map::value(size_t x, size_t y) const
{
	if(enabled_ == false)
		return false;

	if(x >= data_.size())
		return true;

	if(y >= data_[x].size())
		return true;

	if(data_[x][y])
		return false;
	else
		return true;
}

bool team::shroud_map::shared_value(const std::vector<const shroud_map*>& maps, size_t x, size_t y) const
{
	if(enabled_ == false)
		return false;

	for(std::vector<const shroud_map*>::const_iterator i = maps.begin(); i != maps.end(); ++i) {
		if((*i)->enabled_ == true && (*i)->value(x,y) == false)
			return false;
	}
	return true;
}

std::string team::shroud_map::write() const
{
	std::stringstream shroud_str;
	for(std::vector<std::vector<bool> >::const_iterator sh = data_.begin(); sh != data_.end(); ++sh) {
		shroud_str << '|';

		for(std::vector<bool>::const_iterator i = sh->begin(); i != sh->end(); ++i) {
			shroud_str << (*i ? '1' : '0');
		}

		shroud_str << '\n';
	}

	return shroud_str.str();
}

void team::shroud_map::read(const std::string& str)
{
	data_.clear();
	for(std::string::const_iterator sh = str.begin(); sh != str.end(); ++sh) {
		if(*sh == '|')
			data_.resize(data_.size()+1);

		if(data_.empty() == false) {
			if(*sh == '1')
				data_.back().push_back(true);
			else if(*sh == '0')
				data_.back().push_back(false);
		}
	}
}

void team::shroud_map::merge(const std::string& str)
{
	int x=0, y=0;
	for(std::string::const_iterator sh = str.begin(); sh != str.end(); ++sh) {
		if(*sh == '|' && sh != str.begin()) {
			y=0;
			x++;
		} else if(*sh == '1') {
			clear(x,y);
			y++;
		} else if(*sh == '0') {
			y++;
		}
	}
}

bool team::shroud_map::copy_from(const std::vector<const shroud_map*>& maps)
{
	if(enabled_ == false)
		return false;

	bool cleared = false;
	for(std::vector<const shroud_map*>::const_iterator i = maps.begin(); i != maps.end(); ++i) {
		if((*i)->enabled_ == false)
			continue;

		const std::vector<std::vector<bool> >& v = (*i)->data_;
		for(size_t x = 0; x != v.size(); ++x) {
			for(size_t y = 0; y != v[x].size(); ++y) {
				if(v[x][y]) {
					cleared |= clear(x,y);
				}
			}
		}
	}
	return cleared;
}

std::map<int, color_range> team::team_color_range_;

const color_range team::get_side_color_range(int side){
  std::string index = get_side_colour_index(side);
  std::map<std::string, color_range>::iterator gp=game_config::team_rgb_range.find(index);

  if(gp != game_config::team_rgb_range.end()){
    return(gp->second);
  }

  return(color_range(0x00FF0000,0x00FFFFFF,0x00000000,0x00FF0000));
}

const SDL_Color team::get_minimap_colour(int side)
{
	// Note: use mid() instead of rep() unless
	// high contrast is needed over a map or minimap!
	return int_to_color(get_side_color_range(side).rep());
}

std::string team::get_side_colour_index(int side)
{
	size_t index = size_t(side-1);

	if(teams != NULL && index < teams->size()) {
		const std::string side_map = (*teams)[index].map_colour_to();
		if(side_map.size()) {
			return side_map;
		}
	}
	std::stringstream id;
	id<<side;
	return id.str();
}

std::string team::get_side_highlight(int side)
{
	return rgb2highlight(get_side_color_range(side+1).mid());
}

void team::log_recruitable(){
	LOG_NG << "Adding recruitable units: \n";
	for (std::set<std::string>::const_iterator it = info_.can_recruit.begin();
		 it != info_.can_recruit.end(); it++) {
		LOG_NG << *it << std::endl;
	}
	LOG_NG << "Added all recruitable units\n";
}

namespace player_teams {
int village_owner(const map_location& loc)
{
	if(! teams) {
		return -1;
	}
	for(size_t i = 0; i != teams->size(); ++i) {
		if((*teams)[i].owns_village(loc))
			return i;
	}
	return -1;
}
}
