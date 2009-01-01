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
 *  @file statistics.cpp
 *  Manage statistics: recruitments, recalls, kills, losses, etc.
 */

#include "global.hpp"
#include "statistics.hpp"
#include "log.hpp"
#include "serialization/binary_or_text.hpp"
#include "unit.hpp"

#define ERR_NG lg::err(lg::engine)
#define DBG_NG LOG_STREAM(debug, engine)

namespace {

// This variable is true whenever the statistics are mid-scenario.
// This means a new scenario shouldn't be added to the master stats record.
bool mid_scenario = false;

typedef statistics::stats stats;

int stats_disabled = 0;

struct scenario_stats
{
	explicit scenario_stats(const std::string& name) :
		team_stats(),
		scenario_name(name)
	{}

	explicit scenario_stats(const config& cfg);

	config write() const;
	void write(config_writer &out) const;

	std::map<std::string,stats> team_stats;
	std::string scenario_name;
};

scenario_stats::scenario_stats(const config& cfg) :
	team_stats(),
	scenario_name(cfg["scenario"])
{
	const config::child_list& teams = cfg.get_children("team");
	for(config::child_list::const_iterator i = teams.begin(); i != teams.end(); ++i) {
		team_stats[(**i)["save_id"]] = stats(**i);
	}
}

config scenario_stats::write() const
{
	config res;
	res["scenario"] = scenario_name;
	for(std::map<std::string,stats>::const_iterator i = team_stats.begin(); i != team_stats.end(); ++i) {
		res.add_child("team",i->second.write());
	}

	return res;
}

void scenario_stats::write(config_writer &out) const
{
	out.write_key_val("scenario", scenario_name);
	for(std::map<std::string,stats>::const_iterator i = team_stats.begin(); i != team_stats.end(); ++i) {
		out.open_child("team");
		i->second.write(out);
		out.close_child("team");
	}
}

std::vector<scenario_stats> master_stats;

} // end anon namespace

static stats& get_stats(std::string save_id)
{
	if(master_stats.empty()) {
		master_stats.push_back(scenario_stats(""));
	}

	std::map<std::string,stats>& team_stats = master_stats.back().team_stats;
	return team_stats[save_id];
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
	DBG_NG << "Merging statistics\n";
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
	// Only take the last value for this turn
	a.turn_damage_inflicted = b.turn_damage_inflicted;
	a.turn_damage_taken = b.turn_damage_taken;
	a.turn_expected_damage_inflicted = b.turn_expected_damage_inflicted;
	a.turn_expected_damage_taken = b.turn_expected_damage_taken;

	a.new_expected_damage_inflicted += b.new_expected_damage_inflicted;
	a.new_expected_damage_taken += b.new_expected_damage_taken;
	// Only take the last value for this turn
	a.new_turn_expected_damage_inflicted = b.new_turn_expected_damage_inflicted;
	a.new_turn_expected_damage_taken = b.new_turn_expected_damage_taken;
}

namespace statistics
{

stats::stats() :
	recruits(),
	recalls(),
	advanced_to(),
	deaths(),
	killed(),
	recruit_cost(0),
	recall_cost(0),
	attacks(),
	defends(),
	damage_inflicted(0),
	damage_taken(0),
	turn_damage_inflicted(0),
	turn_damage_taken(0),
	expected_damage_inflicted(0),
	expected_damage_taken(0),
	turn_expected_damage_inflicted(0),
	turn_expected_damage_taken(0),
	new_expected_damage_inflicted(0),
	new_expected_damage_taken(0),
	new_turn_expected_damage_inflicted(0),
	new_turn_expected_damage_taken(0),
	save_id()
{}

stats::stats(const config& cfg) :
	recruits(),
	recalls(),
	advanced_to(),
	deaths(),
	killed(),
	recruit_cost(0),
	recall_cost(0),
	attacks(),
	defends(),
	damage_inflicted(0),
	damage_taken(0),
	turn_damage_inflicted(0),
	turn_damage_taken(0),
	expected_damage_inflicted(0),
	expected_damage_taken(0),
	turn_expected_damage_inflicted(0),
	turn_expected_damage_taken(0),
	new_expected_damage_inflicted(0),
	new_expected_damage_taken(0),
	new_turn_expected_damage_inflicted(0),
	new_turn_expected_damage_taken(0),
	save_id(std::string())
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

	std::stringstream ss;
	ss << recruit_cost;
	res["recruit_cost"] = ss.str();
	ss.str(std::string());
	ss << recall_cost;
	res["recall_cost"] = ss.str();

	ss.str(std::string());
	ss << damage_inflicted;
	res["damage_inflicted"] = ss.str();
	ss.str(std::string());
	ss << damage_taken;
	res["damage_taken"] = ss.str();
	ss.str(std::string());
	ss << expected_damage_inflicted;
	res["expected_damage_inflicted"] = ss.str();
	ss.str(std::string());
	ss << expected_damage_taken;
	res["expected_damage_taken"] = ss.str();
	ss.str(std::string());
	ss << turn_damage_inflicted;

	res["turn_damage_inflicted"] = ss.str();
	ss.str(std::string());
	ss << turn_damage_taken;
	res["turn_damage_taken"] = ss.str();
	ss.str(std::string());
	ss << turn_expected_damage_inflicted;
	res["turn_expected_damage_inflicted"] = ss.str();
	ss.str(std::string());
	ss << turn_expected_damage_taken;
	res["turn_expected_damage_taken"] = ss.str();

	ss.str(std::string());
	ss << new_expected_damage_inflicted;
	res["new_expected_damage_inflicted"] = ss.str();
	ss.str(std::string());
	ss << new_expected_damage_taken;
	res["new_expected_damage_taken"] = ss.str();
	ss.str(std::string());
	ss << new_turn_expected_damage_inflicted;
	res["new_turn_expected_damage_inflicted"] = ss.str();
	ss.str(std::string());
	ss << new_turn_expected_damage_taken;
	res["new_turn_expected_damage_taken"] = ss.str();

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

	std::stringstream ss;
	ss << recruit_cost;
	out.write_key_val("recruit_cost", ss.str());
	ss.str(std::string());
	ss << recall_cost;
	out.write_key_val("recall_cost", ss.str());

	ss.str(std::string());
	ss << damage_inflicted;
	out.write_key_val("damage_inflicted", ss.str());
	ss.str(std::string());
	ss << damage_taken;
	out.write_key_val("damage_taken", ss.str());
	ss.str(std::string());
	ss << expected_damage_inflicted;
	out.write_key_val("expected_damage_inflicted", ss.str());
	ss.str(std::string());
	ss << expected_damage_taken;
	out.write_key_val("expected_damage_taken", ss.str());
	ss.str(std::string());
	ss << turn_damage_inflicted;

	out.write_key_val("turn_damage_inflicted", ss.str());
	ss.str(std::string());
	ss << turn_damage_taken;
	out.write_key_val("turn_damage_taken", ss.str());
	ss.str(std::string());
	ss << turn_expected_damage_inflicted;
	out.write_key_val("turn_expected_damage_inflicted", ss.str());
	ss.str(std::string());
	ss << turn_expected_damage_taken;
	out.write_key_val("turn_expected_damage_taken", ss.str());

	ss.str(std::string());
	ss << new_expected_damage_inflicted;
	out.write_key_val("new_expected_damage_inflicted", ss.str());
	ss.str(std::string());
	ss << new_expected_damage_taken;
	out.write_key_val("new_expected_damage_taken", ss.str());
	ss.str(std::string());
	ss << new_turn_expected_damage_inflicted;
	out.write_key_val("new_turn_expected_damage_inflicted", ss.str());
	ss.str(std::string());
	ss << new_turn_expected_damage_taken;
	out.write_key_val("new_turn_expected_damage_taken", ss.str());

	out.write_key_val("save_id", save_id);

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

	recruit_cost = lexical_cast<long long>(cfg["recruit_cost"]);
	recall_cost = lexical_cast<long long>(cfg["recall_cost"]);

	damage_inflicted = lexical_cast<long long>(cfg["damage_inflicted"]);
	damage_taken = lexical_cast<long long>(cfg["damage_taken"]);
	expected_damage_inflicted = lexical_cast<long long>(cfg["expected_damage_inflicted"]);
	expected_damage_taken = lexical_cast<long long>(cfg["expected_damage_taken"]);

	turn_damage_inflicted = lexical_cast<long long>(cfg["turn_damage_inflicted"]);
	turn_damage_taken = lexical_cast<long long>(cfg["turn_damage_taken"]);
	turn_expected_damage_inflicted = lexical_cast<long long>(cfg["turn_expected_damage_inflicted"]);
	turn_expected_damage_taken = lexical_cast<long long>(cfg["turn_expected_damage_taken"]);

	new_expected_damage_inflicted = lexical_cast_default<long long>(cfg["new_expected_damage_inflicted"],expected_damage_inflicted);
	new_expected_damage_taken = lexical_cast_default<long long>(cfg["new_expected_damage_taken"],expected_damage_taken);
	new_turn_expected_damage_inflicted = lexical_cast_default<long long>(cfg["new_turn_expected_damage_inflicted"],turn_expected_damage_inflicted);
	new_turn_expected_damage_taken = lexical_cast_default<long long>(cfg["new_turn_expected_damage_taken"],turn_expected_damage_taken);
	save_id = cfg["save_id"];
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

attack_context::attack_context(const unit& a,
		const unit& d, int a_cth, int d_cth) :
	attacker_type(a.type_id()),
	defender_type(d.type_id()),
	attacker_side(a.side_id()),
	defender_side(d.side_id()),
	chance_to_hit_defender(a_cth),
	chance_to_hit_attacker(d_cth),
	attacker_res(),
	defender_res()
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

void attack_context::attack_excepted_damage(double attacker_inflict_, double defender_inflict_)
{
	const long long attacker_inflict = static_cast<long long>(attacker_inflict_ * stats::desimal_shift);
	const long long defender_inflict = static_cast<long long>(defender_inflict_ * stats::desimal_shift);
	attacker_stats().new_expected_damage_inflicted	+= attacker_inflict;
	attacker_stats().new_expected_damage_taken	+= defender_inflict;
	defender_stats().new_expected_damage_inflicted	+= defender_inflict;
	defender_stats().new_expected_damage_taken	+= attacker_inflict;
	attacker_stats().new_turn_expected_damage_inflicted += attacker_inflict;
	attacker_stats().new_turn_expected_damage_taken	    += defender_inflict;
	defender_stats().new_turn_expected_damage_inflicted += defender_inflict;
	defender_stats().new_turn_expected_damage_taken	    += attacker_inflict;
}


void attack_context::attack_result(attack_context::ATTACK_RESULT res, long long damage, long long drain)
{
	if(stats_disabled > 0)
		return;

	attacker_res.push_back(res == MISSES ? '0' : '1');

	if(res != MISSES) {
		// handle drain
		attacker_stats().damage_taken -= drain;
		defender_stats().damage_inflicted -= drain;
		attacker_stats().turn_damage_taken -= drain;
		defender_stats().turn_damage_inflicted -= drain;

		attacker_stats().damage_inflicted += damage;
		defender_stats().damage_taken += damage;
		attacker_stats().turn_damage_inflicted += damage;
		defender_stats().turn_damage_taken += damage;
	}
	const int exp_damage = damage * chance_to_hit_defender * 10;
	const int exp_drain  = drain  * chance_to_hit_defender * 10;

	attacker_stats().expected_damage_taken -= exp_drain;
	defender_stats().expected_damage_inflicted -= exp_drain;
	attacker_stats().turn_expected_damage_taken -= exp_drain;
	defender_stats().turn_expected_damage_inflicted -= exp_drain;

	// handle drain
	attacker_stats().expected_damage_inflicted += exp_damage;
	defender_stats().expected_damage_taken += exp_damage;
	attacker_stats().turn_expected_damage_inflicted += exp_damage;
	defender_stats().turn_expected_damage_taken += exp_damage;

	if(res == KILLS) {
		attacker_stats().killed[defender_type]++;
		defender_stats().deaths[defender_type]++;
	}
}

void attack_context::defend_result(attack_context::ATTACK_RESULT res, long long damage, long long drain)
{
	if(stats_disabled > 0)
		return;

	defender_res.push_back(res == MISSES ? '0' : '1');


	if(res != MISSES) {
		//handle drain
		defender_stats().damage_taken -= drain;
		attacker_stats().damage_inflicted -= drain;
		defender_stats().turn_damage_taken -= drain;
		attacker_stats().turn_damage_inflicted -= drain;

		attacker_stats().damage_taken += damage;
		defender_stats().damage_inflicted += damage;
		attacker_stats().turn_damage_taken += damage;
		defender_stats().turn_damage_inflicted += damage;
	}
	const long long exp_damage = damage * chance_to_hit_attacker * 10;
	const long long exp_drain = drain * chance_to_hit_attacker * 10;
	//handle drain
	defender_stats().expected_damage_taken -= exp_drain;
	attacker_stats().expected_damage_inflicted -= exp_drain;
	defender_stats().turn_expected_damage_taken -= exp_drain;
	attacker_stats().turn_expected_damage_inflicted -= exp_drain;

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

	stats& s = get_stats(u.side_id());
	s.recruits[u.type_id()]++;
	s.recruit_cost += u.cost();
}

void recall_unit(const unit& u)
{
	if(stats_disabled > 0)
		return;

	stats& s = get_stats(u.side_id());
	s.recalls[u.type_id()]++;
	s.recall_cost += u.cost();
}

void un_recall_unit(const unit& u)
{
	if(stats_disabled > 0)
		return;

	stats& s = get_stats(u.side_id());
	s.recalls[u.type_id()]--;
	s.recall_cost -= u.cost();
}

void un_recruit_unit(const unit& u)
{
	if(stats_disabled > 0)
		return;

	stats& s = get_stats(u.side_id());
	s.recruits[u.type_id()]--;
	s.recruit_cost -= u.cost();
}


void advance_unit(const unit& u)
{
	if(stats_disabled > 0)
		return;

	stats& s = get_stats(u.side_id());
	s.advanced_to[u.type_id()]++;
}

void reset_turn_stats(std::string save_id)
{
	stats &s = get_stats(save_id);
	s.turn_damage_inflicted = 0;
	s.turn_damage_taken = 0;
	s.turn_expected_damage_inflicted = 0;
	s.turn_expected_damage_taken = 0;
	s.new_turn_expected_damage_inflicted = 0;
	s.new_turn_expected_damage_taken = 0;
	s.save_id = save_id;
}

stats calculate_stats(int category, std::string save_id)
{
	DBG_NG << "calculate_stats, category: " << category << " side: " << save_id << " master_stats.size: " << master_stats.size() << "\n";
	if(category == 0) {
		stats res;
		// We are going from last to first to include correct turn stats in result
		for(int i = int(master_stats.size()); i > 0 ; --i) {
			merge_stats(res,calculate_stats(i,save_id));
		}

		return res;
	} else {
		const size_t index = master_stats.size() - size_t(category);
		if(index < master_stats.size() && master_stats[index].team_stats.find(save_id) != master_stats[index].team_stats.end()) {
			return master_stats[index].team_stats[save_id];
		} else {
			return stats();
		}
	}
}

config write_stats()
{
	config res;
	res["mid_scenario"] = (mid_scenario ? "yes" : "no");

	for(std::vector<scenario_stats>::const_iterator i = master_stats.begin(); i != master_stats.end(); ++i) {
		res.add_child("scenario",i->write());
	}

	return res;
}

void write_stats(config_writer &out)
{
	out.write_key_val("mid_scenario", mid_scenario ? "yes" : "no");

	for(std::vector<scenario_stats>::const_iterator i = master_stats.begin(); i != master_stats.end(); ++i) {
		out.open_child("scenario");
		i->write(out);
		out.close_child("scenario");
	}
}

void read_stats(const config& cfg)
{
	fresh_stats();
	mid_scenario = (utils::string_bool(cfg["mid_scenario"]));

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

