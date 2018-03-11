/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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
 *  @file
 *  Manage statistics: recruitments, recalls, kills, losses, etc.
 */

#include "game_board.hpp"
#include "statistics.hpp"
#include "log.hpp"
#include "resources.hpp" // Needed for teams, to get team save_id for a unit
#include "serialization/binary_or_text.hpp"
#include "team.hpp" // Needed to get team save_id
#include "units/unit.hpp"

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

namespace {

// This variable is true whenever the statistics are mid-scenario.
// This means a new scenario shouldn't be added to the master stats record.
bool mid_scenario = false;

typedef statistics::stats stats;
typedef std::map<std::string,stats> team_stats_t;

std::string get_team_save_id(const unit & u)
{
	assert(resources::gameboard);
	return resources::gameboard->get_team(u.side()).save_id();
}

struct scenario_stats
{
	explicit scenario_stats(const std::string& name) :
		team_stats(),
		scenario_name(name)
	{}

	explicit scenario_stats(const config& cfg);

	config write() const;
	void write(config_writer &out) const;

	team_stats_t team_stats;
	std::string scenario_name;
};

scenario_stats::scenario_stats(const config& cfg) :
	team_stats(),
	scenario_name(cfg["scenario"])
{
	for(const config &team : cfg.child_range("team")) {
		team_stats[team["save_id"]] = stats(team);
	}
}

config scenario_stats::write() const
{
	config res;
	res["scenario"] = scenario_name;
	for(team_stats_t::const_iterator i = team_stats.begin(); i != team_stats.end(); ++i) {
		res.add_child("team",i->second.write());
	}

	return res;
}

void scenario_stats::write(config_writer &out) const
{
	out.write_key_val("scenario", scenario_name);
	for(team_stats_t::const_iterator i = team_stats.begin(); i != team_stats.end(); ++i) {
		out.open_child("team");
		i->second.write(out);
		out.close_child("team");
	}
}

std::vector<scenario_stats> master_stats;

} // end anon namespace

static stats &get_stats(const std::string &save_id)
{
	if(master_stats.empty()) {
		master_stats.emplace_back(std::string());
	}

	team_stats_t& team_stats = master_stats.back().team_stats;
	return team_stats[save_id];
}

static config write_str_int_map(const stats::str_int_map& m)
{
	config res;
	for(stats::str_int_map::const_iterator i = m.begin(); i != m.end(); ++i) {
		std::string n = std::to_string(i->second);
		if(res.has_attribute(n)) {
			res[n] = res[n].str() + "," + i->first;
		} else {
			res[n] = i->first;
		}
	}

	return res;
}

static void write_str_int_map(config_writer &out, const stats::str_int_map& m)
{
	using reverse_map = std::multimap<int, std::string>;
	reverse_map rev;
	std::transform(
		m.begin(), m.end(),
		std::inserter(rev, rev.begin()),
		[](const stats::str_int_map::value_type p) {
			return std::make_pair(p.second, p.first);
		}
	);
	reverse_map::const_iterator i = rev.begin(), j;
	while(i != rev.end()) {
		j = rev.upper_bound(i->first);
		std::vector<std::string> vals;
		std::transform(i, j, std::back_inserter(vals), [](const reverse_map::value_type& p) {
			return p.second;
		});
		out.write_key_val(std::to_string(i->first), utils::join(vals));
		i = j;
	}
}

static stats::str_int_map read_str_int_map(const config& cfg)
{
	stats::str_int_map m;
	for(const config::attribute &i : cfg.attribute_range()) {
		try {
			for(const std::string& val : utils::split(i.second)) {
				m[val] = std::stoi(i.first);
			}
		} catch(std::invalid_argument&) {
			ERR_NG << "Invalid statistics entry; skipping\n";
		}
	}

	return m;
}

static config write_battle_result_map(const stats::battle_result_map& m)
{
	config res;
	for(stats::battle_result_map::const_iterator i = m.begin(); i != m.end(); ++i) {
		config& new_cfg = res.add_child("sequence");
		new_cfg = write_str_int_map(i->second);
		new_cfg["_num"] = i->first;
	}

	return res;
}

static void write_battle_result_map(config_writer &out, const stats::battle_result_map& m)
{
	for(stats::battle_result_map::const_iterator i = m.begin(); i != m.end(); ++i) {
		out.open_child("sequence");
		write_str_int_map(out, i->second);
		out.write_key_val("_num", i->first);
		out.close_child("sequence");
	}
}

static stats::battle_result_map read_battle_result_map(const config& cfg)
{
	stats::battle_result_map m;
	for(const config &i : cfg.child_range("sequence"))
	{
		config item = i;
		int key = item["_num"];
		item.remove_attribute("_num");
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
	save_id()
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

	res["recruit_cost"] = recruit_cost;
	res["recall_cost"] = recall_cost;

	res["damage_inflicted"] = damage_inflicted;
	res["damage_taken"] = damage_taken;
	res["expected_damage_inflicted"] = expected_damage_inflicted;
	res["expected_damage_taken"] = expected_damage_taken;

	res["turn_damage_inflicted"] = turn_damage_inflicted;
	res["turn_damage_taken"] = turn_damage_taken;
	res["turn_expected_damage_inflicted"] = turn_expected_damage_inflicted;
	res["turn_expected_damage_taken"] = turn_expected_damage_taken;

	res["save_id"] = save_id;

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

	out.write_key_val("recruit_cost", recruit_cost);
	out.write_key_val("recall_cost", recall_cost);

	out.write_key_val("damage_inflicted", damage_inflicted);
	out.write_key_val("damage_taken", damage_taken);
	out.write_key_val("expected_damage_inflicted", expected_damage_inflicted);
	out.write_key_val("expected_damage_taken", expected_damage_taken);

	out.write_key_val("turn_damage_inflicted", turn_damage_inflicted);
	out.write_key_val("turn_damage_taken", turn_damage_taken);
	out.write_key_val("turn_expected_damage_inflicted", turn_expected_damage_inflicted);
	out.write_key_val("turn_expected_damage_taken", turn_expected_damage_taken);

	out.write_key_val("save_id", save_id);
}

void stats::read(const config& cfg)
{
	if (const config &c = cfg.child("recruits")) {
		recruits = read_str_int_map(c);
	}
	if (const config &c = cfg.child("recalls")) {
		recalls = read_str_int_map(c);
	}
	if (const config &c = cfg.child("advances")) {
		advanced_to = read_str_int_map(c);
	}
	if (const config &c = cfg.child("deaths")) {
		deaths = read_str_int_map(c);
	}
	if (const config &c = cfg.child("killed")) {
		killed = read_str_int_map(c);
	}
	if (const config &c = cfg.child("recalls")) {
		recalls = read_str_int_map(c);
	}
	if (const config &c = cfg.child("attacks")) {
		attacks = read_battle_result_map(c);
	}
	if (const config &c = cfg.child("defends")) {
		defends = read_battle_result_map(c);
	}

	recruit_cost = cfg["recruit_cost"].to_int();
	recall_cost = cfg["recall_cost"].to_int();

	damage_inflicted = cfg["damage_inflicted"].to_long_long();
	damage_taken = cfg["damage_taken"].to_long_long();
	expected_damage_inflicted = cfg["expected_damage_inflicted"].to_long_long();
	expected_damage_taken = cfg["expected_damage_taken"].to_long_long();

	turn_damage_inflicted = cfg["turn_damage_inflicted"].to_long_long();
	turn_damage_taken = cfg["turn_damage_taken"].to_long_long();
	turn_expected_damage_inflicted = cfg["turn_expected_damage_inflicted"].to_long_long();
	turn_expected_damage_taken = cfg["turn_expected_damage_taken"].to_long_long();

	save_id = cfg["save_id"].str();
}

scenario_context::scenario_context(const std::string& name)
{
	if(!mid_scenario || master_stats.empty()) {
		master_stats.emplace_back(name);
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
	attacker_side(get_team_save_id(a)),
	defender_side(get_team_save_id(d)),
	chance_to_hit_defender(a_cth),
	chance_to_hit_attacker(d_cth),
	attacker_res(),
	defender_res()
{
}

attack_context::~attack_context()
{
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

void attack_context::attack_expected_damage(double attacker_inflict_, double defender_inflict_)
{
	int attacker_inflict = round_double(attacker_inflict_ * stats::decimal_shift);
	int defender_inflict = round_double(defender_inflict_ * stats::decimal_shift);
	stats &att_stats = attacker_stats(), &def_stats = defender_stats();
	att_stats.expected_damage_inflicted += attacker_inflict;
	att_stats.expected_damage_taken     += defender_inflict;
	def_stats.expected_damage_inflicted += defender_inflict;
	def_stats.expected_damage_taken     += attacker_inflict;
	att_stats.turn_expected_damage_inflicted += attacker_inflict;
	att_stats.turn_expected_damage_taken     += defender_inflict;
	def_stats.turn_expected_damage_inflicted += defender_inflict;
	def_stats.turn_expected_damage_taken     += attacker_inflict;
}


void attack_context::attack_result(hit_result res, int damage, int drain)
{
	attacker_res.push_back(res == MISSES ? '0' : '1');
	stats &att_stats = attacker_stats(), &def_stats = defender_stats();

	if(res != MISSES) {
		// handle drain
		att_stats.damage_taken          -= drain;
		def_stats.damage_inflicted      -= drain;
		att_stats.turn_damage_taken     -= drain;
		def_stats.turn_damage_inflicted -= drain;

		att_stats.damage_inflicted      += damage;
		def_stats.damage_taken          += damage;
		att_stats.turn_damage_inflicted += damage;
		def_stats.turn_damage_taken     += damage;
	}

	if(res == KILLS) {
		++att_stats.killed[defender_type];
		++def_stats.deaths[defender_type];
	}
}

void attack_context::defend_result(hit_result res, int damage, int drain)
{
	defender_res.push_back(res == MISSES ? '0' : '1');
	stats &att_stats = attacker_stats(), &def_stats = defender_stats();

	if(res != MISSES) {
		//handle drain
		def_stats.damage_taken          -= drain;
		att_stats.damage_inflicted      -= drain;
		def_stats.turn_damage_taken     -= drain;
		att_stats.turn_damage_inflicted -= drain;

		att_stats.damage_taken          += damage;
		def_stats.damage_inflicted      += damage;
		att_stats.turn_damage_taken     += damage;
		def_stats.turn_damage_inflicted += damage;
	}

	if(res == KILLS) {
		++att_stats.deaths[attacker_type];
		++def_stats.killed[attacker_type];
	}
}

void recruit_unit(const unit& u)
{
	stats& s = get_stats(get_team_save_id(u));
	s.recruits[u.type().base_id()]++;
	s.recruit_cost += u.cost();
}

void recall_unit(const unit& u)
{
	stats& s = get_stats(get_team_save_id(u));
	s.recalls[u.type_id()]++;
	s.recall_cost += u.cost();
}

void un_recall_unit(const unit& u)
{
	stats& s = get_stats(get_team_save_id(u));
	s.recalls[u.type_id()]--;
	s.recall_cost -= u.cost();
}

void un_recruit_unit(const unit& u)
{
	stats& s = get_stats(get_team_save_id(u));
	s.recruits[u.type().base_id()]--;
	s.recruit_cost -= u.cost();
}

int un_recall_unit_cost(const unit& u)  // this really belongs elsewhere, perhaps in undo.cpp
{					// but I'm too lazy to do it at the moment
	return u.recall_cost();
}


void advance_unit(const unit& u)
{
	stats& s = get_stats(get_team_save_id(u));
	s.advanced_to[u.type_id()]++;
}

void reset_turn_stats(const std::string & save_id)
{
	stats &s = get_stats(save_id);
	s.turn_damage_inflicted = 0;
	s.turn_damage_taken = 0;
	s.turn_expected_damage_inflicted = 0;
	s.turn_expected_damage_taken = 0;
	s.save_id = save_id;
}

stats calculate_stats(const std::string & save_id)
{
	stats res;

	DBG_NG << "calculate_stats, side: " << save_id << " master_stats.size: " << master_stats.size() << "\n";
	// The order of this loop matters since the turn stats are taken from the
	// last stats merged.
	for ( size_t i = 0; i != master_stats.size(); ++i ) {
		team_stats_t::const_iterator find_it = master_stats[i].team_stats.find(save_id);
		if ( find_it != master_stats[i].team_stats.end() )
			merge_stats(res, find_it->second);
	}

	return res;
}


/**
 * Returns a list of names and stats for each scenario in the current campaign.
 * The front of the list is the oldest scenario; the back of the list is the
 * (most) current scenario.
 * Only scenarios with stats for the given @a side_id are included, but if no
 * scenarios are applicable, then a vector containing a single dummy entry will
 * be returned. (I.e., this never returns an empty vector.)
 * This list is intended for the statistics dialog and may become invalid if
 * new stats are recorded.
 */
levels level_stats(const std::string & save_id)
{
	static const stats null_stats;
	static const std::string null_name("");

	levels level_list;

	for ( size_t level = 0; level != master_stats.size(); ++level ) {
		const team_stats_t & team_stats = master_stats[level].team_stats;

		team_stats_t::const_iterator find_it = team_stats.find(save_id);
		if ( find_it != team_stats.end() )
			level_list.emplace_back(&master_stats[level].scenario_name, &find_it->second);
	}

	// Make sure we do return something (so other code does not have to deal
	// with an empty list).
	if ( level_list.empty() )
			level_list.emplace_back(&null_name, &null_stats);

	return level_list;
}


config write_stats()
{
	config res;
	res["mid_scenario"] = mid_scenario;

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
	mid_scenario = cfg["mid_scenario"].to_bool();

	for(const config &s : cfg.child_range("scenario")) {
		master_stats.emplace_back(s);
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

int sum_cost_str_int_map(const stats::str_int_map &m)
{
	int cost = 0;
	for (stats::str_int_map::const_iterator i = m.begin(); i != m.end(); ++i) {
		const unit_type *t = unit_types.find(i->first);
		if (!t) {
			ERR_NG << "Statistics refer to unknown unit type '" << i->first << "'. Discarding." << std::endl;
		} else {
			cost += i->second * t->cost();
		}
	}

	return cost;
}

} // end namespace statistics
