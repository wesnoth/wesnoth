/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file statistics.cpp
//! Manage statistics: recruitments, recalls, kills, losses, etc.

#include "global.hpp"
#include "config.hpp"
#include "statistics.hpp"
#include "util.hpp"
#include "log.hpp"
#include "serialization/binary_or_text.hpp"
#include "unit.hpp"

#define ERR_NG lg::err(lg::engine)

namespace {

// This variable is true whenever the statistics are mid-scenario.
// This means a new scenario shouldn't be added to the master stats record.
bool mid_scenario = false;

typedef statistics::stats stats;

int stats_disabled = 0;

struct scenario_stats
{
	explicit scenario_stats(const std::string& name) : scenario_name(name)
	{}

	explicit scenario_stats(const config& cfg);

	config write() const;
	void write(config_writer &out) const;

	std::vector<stats> team_stats;
	std::string scenario_name;
};

scenario_stats::scenario_stats(const config& cfg)
{
	scenario_name = cfg["scenario"];
	const config::child_list& teams = cfg.get_children("team");
	for(config::child_list::const_iterator i = teams.begin(); i != teams.end(); ++i) {
		team_stats.push_back(stats(**i));
	}
}

config scenario_stats::write() const
{
	config res;
	res["scenario"] = scenario_name;
	for(std::vector<stats>::const_iterator i = team_stats.begin(); i != team_stats.end(); ++i) {
		res.add_child("team",i->write());
	}

	return res;
}

void scenario_stats::write(config_writer &out) const
{
	out.write_key_val("scenario", scenario_name);
	for(std::vector<stats>::const_iterator i = team_stats.begin(); i != team_stats.end(); ++i) {
		out.open_child("team");
		i->write(out);
		out.close_child("team");
	}
}

std::vector<scenario_stats> master_stats;

} // end anon namespace

static stats& get_stats(int team)
{
	if(master_stats.empty()) {
		master_stats.push_back(scenario_stats(""));
	}

	std::vector<stats>& team_stats = master_stats.back().team_stats;
	const size_t index = size_t(team-1);
	if(index >= team_stats.size()) {
		team_stats.resize(index+1);
	}

	return team_stats[index];
}

static config write_str_int_map(const stats::str_int_map& m)
{
	config res;
	for(stats::str_int_map::const_iterator i = m.begin(); i != m.end(); ++i) {
		char buf[50];
		snprintf(buf,sizeof(buf),"%d",i->second);
		res[i->first] = buf;
	}

	return res;
}

static void write_str_int_map(config_writer &out, const stats::str_int_map& m)
{
	for(stats::str_int_map::const_iterator i = m.begin(); i != m.end(); ++i) {
		char buf[50];
		snprintf(buf,sizeof(buf),"%d",i->second);
		out.write_key_val(i->first, buf);
	}
}

static stats::str_int_map read_str_int_map(const config& cfg)
{
	stats::str_int_map m;
	for(string_map::const_iterator i = cfg.values.begin(); i != cfg.values.end(); ++i) {
		m[i->first] = atoi(i->second.c_str());
	}

	return m;
}

static config write_battle_result_map(const stats::battle_result_map& m)
{
	config res;
	for(stats::battle_result_map::const_iterator i = m.begin(); i != m.end(); ++i) {
		config& new_cfg = res.add_child("sequence");
		new_cfg = write_str_int_map(i->second);

		char buf[50];
		snprintf(buf,sizeof(buf),"%d",i->first);
		new_cfg["_num"] = buf;
	}

	return res;
}

static void write_battle_result_map(config_writer &out, const stats::battle_result_map& m)
{
	for(stats::battle_result_map::const_iterator i = m.begin(); i != m.end(); ++i) {
		out.open_child("sequence");
		write_str_int_map(out, i->second);

		char buf[50];
		snprintf(buf,sizeof(buf),"%d",i->first);
		out.write_key_val("_num", buf);
		out.close_child("sequence");
	}
}

static stats::battle_result_map read_battle_result_map(const config& cfg)
{
	stats::battle_result_map m;
	const config::child_list c = cfg.get_children("sequence");
	for(config::child_list::const_iterator i = c.begin(); i != c.end(); ++i) {
		config item = **i;
		const int key = atoi(item["_num"].c_str());
		item.values.erase("_num");
		m[key] = read_str_int_map(item);
	}

	return m;
}

static void merge_str_int_map(stats::str_int_map& a, const stats::str_int_map& b)
{
	for(stats::str_int_map::const_iterator i = b.begin(); i != b.end(); ++i) {
		a[i->first] += i->second;
	}
}

static void merge_battle_result_maps(stats::battle_result_map& a, const stats::battle_result_map& b)
{
	for(stats::battle_result_map::const_iterator i = b.begin(); i != b.end(); ++i) {
		merge_str_int_map(a[i->first],i->second);
	}
}

static void merge_stats(stats& a, const stats& b)
{
	merge_str_int_map(a.recruits,b.recruits);
	merge_str_int_map(a.recalls,b.recalls);
	merge_str_int_map(a.advanced_to,b.advanced_to);
	merge_str_int_map(a.deaths,b.deaths);
	merge_str_int_map(a.killed,b.killed);

	merge_battle_result_maps(a.attacks,b.attacks);
	merge_battle_result_maps(a.defends,b.defends);

	a.recruit_cost += b.recruit_cost;
	a.recall_cost += b.recall_cost;

	a.damage_inflicted += b.damage_inflicted;
	a.damage_taken += b.damage_taken;
	a.expected_damage_inflicted += b.expected_damage_inflicted;
	a.expected_damage_taken += b.expected_damage_taken;
	a.turn_damage_inflicted += b.turn_damage_inflicted;
	a.turn_damage_taken += b.turn_damage_taken;
	a.turn_expected_damage_inflicted += b.turn_expected_damage_inflicted;
	a.turn_expected_damage_taken += b.turn_expected_damage_taken;
}

namespace statistics
{

stats::stats() : recruit_cost(0), recall_cost(0),
                 damage_inflicted(0), damage_taken(0),
                 turn_damage_inflicted(0), turn_damage_taken(0),
                 expected_damage_inflicted(0), expected_damage_taken(0),
                 turn_expected_damage_inflicted(0), turn_expected_damage_taken(0)
{}

stats::stats(const config& cfg)
{
	read(cfg);
}

config stats::write() const
{
	config res;
	res.add_child("recruits",write_str_int_map(recruits));
	res.add_child("recalls",write_str_int_map(recalls));
	res.add_child("advances",write_str_int_map(advanced_to));
	res.add_child("deaths",write_str_int_map(deaths));
	res.add_child("killed",write_str_int_map(killed));
	res.add_child("attacks",write_battle_result_map(attacks));
	res.add_child("defends",write_battle_result_map(defends));

	char buf[50];
	snprintf(buf,sizeof(buf),"%d",recruit_cost);
	res["recruit_cost"] = buf;
	snprintf(buf,sizeof(buf),"%d",recall_cost);
	res["recall_cost"] = buf;

	snprintf(buf,sizeof(buf),"%d",damage_inflicted);
	res["damage_inflicted"] = buf;
	snprintf(buf,sizeof(buf),"%d",damage_taken);
	res["damage_taken"] = buf;
	snprintf(buf,sizeof(buf),"%d",expected_damage_inflicted);
	res["expected_damage_inflicted"] = buf;
	snprintf(buf,sizeof(buf),"%d",expected_damage_taken);
	res["expected_damage_taken"] = buf;

	snprintf(buf,sizeof(buf),"%d",turn_damage_inflicted);
	res["turn_damage_inflicted"] = buf;
	snprintf(buf,sizeof(buf),"%d",turn_damage_taken);
	res["turn_damage_taken"] = buf;
	snprintf(buf,sizeof(buf),"%d",turn_expected_damage_inflicted);
	res["turn_expected_damage_inflicted"] = buf;
	snprintf(buf,sizeof(buf),"%d",turn_expected_damage_taken);
	res["turn_expected_damage_taken"] = buf;

	return res;
}

void stats::write(config_writer &out) const
{
	out.open_child("recruits");
	write_str_int_map(out, recruits);
	out.close_child("recruits");
	out.open_child("recalls");
	write_str_int_map(out, recalls);
	out.close_child("recalls");
	out.open_child("advances");
	write_str_int_map(out, advanced_to);
	out.close_child("advances");
	out.open_child("deaths");
	write_str_int_map(out, deaths);
	out.close_child("deaths");
	out.open_child("killed");
	write_str_int_map(out, killed);
	out.close_child("killed");
	out.open_child("attacks");
	write_battle_result_map(out, attacks);
	out.close_child("attacks");
	out.open_child("defends");
	write_battle_result_map(out, defends);
	out.close_child("defends");

	char buf[50];
	snprintf(buf,sizeof(buf),"%d",recruit_cost);
	out.write_key_val("recruit_cost", buf);
	snprintf(buf,sizeof(buf),"%d",recall_cost);
	out.write_key_val("recall_cost", buf);

	snprintf(buf,sizeof(buf),"%d",damage_inflicted);
	out.write_key_val("damage_inflicted", buf);
	snprintf(buf,sizeof(buf),"%d",damage_taken);
	out.write_key_val("damage_taken", buf);
	snprintf(buf,sizeof(buf),"%d",expected_damage_inflicted);
	out.write_key_val("expected_damage_inflicted", buf);
	snprintf(buf,sizeof(buf),"%d",expected_damage_taken);
	out.write_key_val("expected_damage_taken", buf);

	snprintf(buf,sizeof(buf),"%d",turn_damage_inflicted);
	out.write_key_val("turn_damage_inflicted", buf);
	snprintf(buf,sizeof(buf),"%d",turn_damage_taken);
	out.write_key_val("turn_damage_taken", buf);
	snprintf(buf,sizeof(buf),"%d",turn_expected_damage_inflicted);
	out.write_key_val("turn_expected_damage_inflicted", buf);
	snprintf(buf,sizeof(buf),"%d",turn_expected_damage_taken);
	out.write_key_val("turn_expected_damage_taken", buf);
}

void stats::read(const config& cfg)
{
	if(cfg.child("recruits")) {
		recruits = read_str_int_map(*cfg.child("recruits"));
	}
	if(cfg.child("recalls")) {
		recalls = read_str_int_map(*cfg.child("recalls"));
	}
	if(cfg.child("advances")) {
		advanced_to = read_str_int_map(*cfg.child("advances"));
	}
	if(cfg.child("deaths")) {
		deaths = read_str_int_map(*cfg.child("deaths"));
	}
	if(cfg.child("killed")) {
		killed = read_str_int_map(*cfg.child("killed"));
	}
	if(cfg.child("recalls")) {
		recalls = read_str_int_map(*cfg.child("recalls"));
	}
	if(cfg.child("attacks")) {
		attacks = read_battle_result_map(*cfg.child("attacks"));
	}
	if(cfg.child("defends")) {
		defends = read_battle_result_map(*cfg.child("defends"));
	}

	recruit_cost = atoi(cfg["recruit_cost"].c_str());
	recall_cost = atoi(cfg["recall_cost"].c_str());

	damage_inflicted = atoi(cfg["damage_inflicted"].c_str());
	damage_taken = atoi(cfg["damage_taken"].c_str());
	expected_damage_inflicted = atoi(cfg["expected_damage_inflicted"].c_str());
	expected_damage_taken = atoi(cfg["expected_damage_taken"].c_str());

	turn_damage_inflicted = atoi(cfg["turn_damage_inflicted"].c_str());
	turn_damage_taken = atoi(cfg["turn_damage_taken"].c_str());
	turn_expected_damage_inflicted = atoi(cfg["turn_expected_damage_inflicted"].c_str());
	turn_expected_damage_taken = atoi(cfg["turn_expected_damage_taken"].c_str());
}

disabler::disabler() { stats_disabled++; }
disabler::~disabler() { stats_disabled--; }

scenario_context::scenario_context(const std::string& name)
{
	if(!mid_scenario || master_stats.empty()) {
		master_stats.push_back(scenario_stats(name));
	}

	mid_scenario = true;
}

scenario_context::~scenario_context()
{
	mid_scenario = false;
}

attack_context::attack_context(const unit& a, const unit& d, int a_cth, int d_cth)
   : attacker_type(a.id()), defender_type(d.id()),
     attacker_side(a.side()), defender_side(d.side()),
     chance_to_hit_defender(a_cth), chance_to_hit_attacker(d_cth)
{
}

attack_context::~attack_context()
{
	if(stats_disabled > 0)
		return;

	std::string attacker_key = "s" + attacker_res;
	std::string defender_key = "s" + defender_res;

	attacker_stats().attacks[chance_to_hit_defender][attacker_key]++;
	defender_stats().defends[chance_to_hit_attacker][defender_key]++;
}

stats& attack_context::attacker_stats()
{
	return get_stats(attacker_side);
}

stats& attack_context::defender_stats()
{
	return get_stats(defender_side);
}

void attack_context::attack_result(attack_context::ATTACK_RESULT res, int damage)
{
	if(stats_disabled > 0)
		return;

	push_back(attacker_res,(res == MISSES ? '0' : '1'));

	if(res != MISSES) {
		attacker_stats().damage_inflicted += damage;
		defender_stats().damage_taken += damage;
		attacker_stats().turn_damage_inflicted += damage;
		defender_stats().turn_damage_taken += damage;
	}
	const int exp_damage = damage * chance_to_hit_defender;
	attacker_stats().expected_damage_inflicted += exp_damage;
	defender_stats().expected_damage_taken += exp_damage;
	attacker_stats().turn_expected_damage_inflicted += exp_damage;
	defender_stats().turn_expected_damage_taken += exp_damage;

	if(res == KILLS) {
		attacker_stats().killed[defender_type]++;
		defender_stats().deaths[defender_type]++;
	}
}

void attack_context::defend_result(attack_context::ATTACK_RESULT res, int damage)
{
	if(stats_disabled > 0)
		return;

	push_back(defender_res,(res == MISSES ? '0' : '1'));

	if(res != MISSES) {
		attacker_stats().damage_taken += damage;
		defender_stats().damage_inflicted += damage;
		attacker_stats().turn_damage_taken += damage;
		defender_stats().turn_damage_inflicted += damage;
	}
	const int exp_damage = damage * chance_to_hit_attacker;
	attacker_stats().expected_damage_taken += exp_damage;
	defender_stats().expected_damage_inflicted += exp_damage;
	attacker_stats().turn_expected_damage_taken += exp_damage;
	defender_stats().turn_expected_damage_inflicted += exp_damage;

	if(res == KILLS) {
		attacker_stats().deaths[attacker_type]++;
		defender_stats().killed[attacker_type]++;
	}
}

void recruit_unit(const unit& u)
{
	if(stats_disabled > 0)
		return;

	stats& s = get_stats(u.side());
	s.recruits[u.id()]++;
	s.recruit_cost += u.cost();
}

void recall_unit(const unit& u)
{
	if(stats_disabled > 0)
		return;

	stats& s = get_stats(u.side());
	s.recalls[u.id()]++;
	s.recall_cost += u.cost();
}

void un_recall_unit(const unit& u)
{
	if(stats_disabled > 0)
		return;

	stats& s = get_stats(u.side());
	s.recalls[u.id()]--;
	s.recall_cost -= u.cost();
}

void un_recruit_unit(const unit& u)
{
	if(stats_disabled > 0)
		return;

	stats& s = get_stats(u.side());
	s.recruits[u.id()]--;
	s.recruit_cost -= u.cost();
}


void advance_unit(const unit& u)
{
	if(stats_disabled > 0)
		return;

	stats& s = get_stats(u.side());
	s.advanced_to[u.id()]++;
}

void reset_turn_stats(int side)
{
	stats &s = get_stats(side);
	s.turn_damage_inflicted = 0;
	s.turn_damage_taken = 0;
	s.turn_expected_damage_inflicted = 0;
	s.turn_expected_damage_taken = 0;
}

stats calculate_stats(int category, int side)
{
	if(category == 0) {
		stats res;
		for(int i = 1; i <= int(master_stats.size()); ++i) {
			merge_stats(res,calculate_stats(i,side));
		}

		return res;
	} else {
		const size_t index = master_stats.size() - size_t(category);
		const size_t side_index = size_t(side) - 1;
		if(index < master_stats.size() && side_index < master_stats[index].team_stats.size()) {
			return master_stats[index].team_stats[side_index];
		} else {
			return stats();
		}
	}
}

config write_stats()
{
	config res;
	res["mid_scenario"] = (mid_scenario ? "true" : "false");

	for(std::vector<scenario_stats>::const_iterator i = master_stats.begin(); i != master_stats.end(); ++i) {
		res.add_child("scenario",i->write());
	}

	return res;
}

void write_stats(config_writer &out)
{
	out.write_key_val("mid_scenario", mid_scenario ? "true" : "false");

	for(std::vector<scenario_stats>::const_iterator i = master_stats.begin(); i != master_stats.end(); ++i) {
		out.open_child("scenario");
		i->write(out);
		out.close_child("scenario");
	}
}

void read_stats(const config& cfg)
{
	fresh_stats();
	mid_scenario = (cfg["mid_scenario"] == "true");

	const config::child_list& scenarios = cfg.get_children("scenario");
	for(config::child_list::const_iterator i = scenarios.begin(); i != scenarios.end(); ++i) {
		master_stats.push_back(scenario_stats(**i));
	}
}

void fresh_stats()
{
	master_stats.clear();
	mid_scenario = false;
}

void clear_current_scenario()
{
	if(master_stats.empty() == false) {
		master_stats.pop_back();
		mid_scenario = false;
	}
}

int sum_str_int_map(const stats::str_int_map& m)
{
	int res = 0;
	for(stats::str_int_map::const_iterator i = m.begin(); i != m.end(); ++i) {
		res += i->second;
	}

	return res;
}

} // end namespace statistics

