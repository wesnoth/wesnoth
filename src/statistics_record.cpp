/*
	Copyright (C) 2023 - 2024
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
 *  @file
 *  Manage statistics: saving and reading data.
 */

#include "statistics_record.hpp"
#include "log.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/string_utils.hpp"


static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

namespace statistics_record
{

static config write_str_int_map(const stats_t::str_int_map& m)
{
	config res;
	for(stats_t::str_int_map::const_iterator i = m.begin(); i != m.end(); ++i) {
		std::string n = std::to_string(i->second);
		if(res.has_attribute(n)) {
			res[n] = res[n].str() + "," + i->first;
		} else {
			res[n] = i->first;
		}
	}

	return res;
}

static void write_str_int_map(config_writer& out, const stats_t::str_int_map& m)
{
	using reverse_map = std::multimap<int, std::string>;
	reverse_map rev;
	std::transform(m.begin(), m.end(), std::inserter(rev, rev.begin()),
		[](const stats_t::str_int_map::value_type& p) { return std::pair(p.second, p.first); });
	reverse_map::const_iterator i = rev.begin(), j;
	while(i != rev.end()) {
		j = rev.upper_bound(i->first);
		std::vector<std::string> vals;
		std::transform(i, j, std::back_inserter(vals), [](const reverse_map::value_type& p) { return p.second; });
		out.write_key_val(std::to_string(i->first), utils::join(vals));
		i = j;
	}
}

static stats_t::str_int_map read_str_int_map(const config& cfg)
{
	stats_t::str_int_map m;
	for(const auto& [key, value] : cfg.attribute_range()) {
		try {
			for(const std::string& val : utils::split(value)) {
				m[val] = std::stoi(key);
			}
		} catch(const std::invalid_argument&) {
			ERR_NG << "Invalid statistics entry; skipping";
		}
	}

	return m;
}

static config write_battle_result_map(const stats_t::battle_result_map& m)
{
	config res;
	for(stats_t::battle_result_map::const_iterator i = m.begin(); i != m.end(); ++i) {
		config& new_cfg = res.add_child("sequence");
		new_cfg = write_str_int_map(i->second);
		new_cfg["_num"] = i->first;
	}

	return res;
}

static void write_battle_result_map(config_writer& out, const stats_t::battle_result_map& m)
{
	for(stats_t::battle_result_map::const_iterator i = m.begin(); i != m.end(); ++i) {
		out.open_child("sequence");
		write_str_int_map(out, i->second);
		out.write_key_val("_num", i->first);
		out.close_child("sequence");
	}
}

static stats_t::battle_result_map read_battle_result_map(const config& cfg)
{
	stats_t::battle_result_map m;
	for(const config& i : cfg.child_range("sequence")) {
		config item = i;
		int key = item["_num"].to_int();
		item.remove_attribute("_num");
		m[key] = read_str_int_map(item);
	}

	return m;
}

static config write_by_cth_map(const stats_t::hitrate_map& m)
{
	config res;
	for(const auto& i : m) {
		res.add_child("hitrate_map_entry", config{"cth", i.first, "stats", i.second.write()});
	}
	return res;
}

static void merge_battle_result_maps(stats_t::battle_result_map& a, const stats_t::battle_result_map& b);

static stats_t::hitrate_map read_by_cth_map_from_battle_result_maps(
	const stats_t::battle_result_map& attacks, const stats_t::battle_result_map& defends)
{
	stats_t::hitrate_map m;

	stats_t::battle_result_map merged = attacks;
	merge_battle_result_maps(merged, defends);

	for(const auto& i : merged) {
		int cth = i.first;
		const stats_t::battle_sequence_frequency_map& frequency_map = i.second;
		for(const auto& j : frequency_map) {
			const std::string& res = j.first; // see attack_context::~attack_context()
			const int occurrences = j.second;
			unsigned int misses = std::count(res.begin(), res.end(), '0');
			unsigned int hits = std::count(res.begin(), res.end(), '1');
			if(misses + hits == 0) {
				continue;
			}
			misses *= occurrences;
			hits *= occurrences;
			m[cth].strikes += misses + hits;
			m[cth].hits += hits;
		}
	}

	return m;
}

static stats_t::hitrate_map read_by_cth_map(const config& cfg)
{
	stats_t::hitrate_map m;
	for(const config& i : cfg.child_range("hitrate_map_entry")) {
		m.emplace(i["cth"].to_int(), stats_t::hitrate_t(i.mandatory_child("stats")));
	}
	return m;
}

static void merge_str_int_map(stats_t::str_int_map& a, const stats_t::str_int_map& b)
{
	for(stats_t::str_int_map::const_iterator i = b.begin(); i != b.end(); ++i) {
		a[i->first] += i->second;
	}
}

static void merge_battle_result_maps(stats_t::battle_result_map& a, const stats_t::battle_result_map& b)
{
	for(stats_t::battle_result_map::const_iterator i = b.begin(); i != b.end(); ++i) {
		merge_str_int_map(a[i->first], i->second);
	}
}

static void merge_cth_map(stats_t::hitrate_map& a, const stats_t::hitrate_map& b)
{
	for(const auto& i : b) {
		a[i.first].hits += i.second.hits;
		a[i.first].strikes += i.second.strikes;
	}
}


stats_t::stats_t()
	: recruits()
	, recalls()
	, advanced_to()
	, deaths()
	, killed()
	, recruit_cost(0)
	, recall_cost(0)
	, attacks_inflicted()
	, defends_inflicted()
	, attacks_taken()
	, defends_taken()
	, damage_inflicted(0)
	, damage_taken(0)
	, turn_damage_inflicted(0)
	, turn_damage_taken(0)
	, by_cth_inflicted()
	, by_cth_taken()
	, turn_by_cth_inflicted()
	, turn_by_cth_taken()
	, expected_damage_inflicted(0)
	, expected_damage_taken(0)
	, turn_expected_damage_inflicted(0)
	, turn_expected_damage_taken(0)
	, save_id()
{
}

stats_t::stats_t(const config& cfg)
	: recruits()
	, recalls()
	, advanced_to()
	, deaths()
	, killed()
	, recruit_cost(0)
	, recall_cost(0)
	, attacks_inflicted()
	, defends_inflicted()
	, attacks_taken()
	, defends_taken()
	, damage_inflicted(0)
	, damage_taken(0)
	, turn_damage_inflicted(0)
	, turn_damage_taken(0)
	, by_cth_inflicted()
	, by_cth_taken()
	, turn_by_cth_inflicted()
	, turn_by_cth_taken()
	, expected_damage_inflicted(0)
	, expected_damage_taken(0)
	, turn_expected_damage_inflicted(0)
	, turn_expected_damage_taken(0)
	, save_id()
{
	read(cfg);
}

config stats_t::write() const
{
	config res;
	res.add_child("recruits", write_str_int_map(recruits));
	res.add_child("recalls", write_str_int_map(recalls));
	res.add_child("advances", write_str_int_map(advanced_to));
	res.add_child("deaths", write_str_int_map(deaths));
	res.add_child("killed", write_str_int_map(killed));
	res.add_child("attacks", write_battle_result_map(attacks_inflicted));
	res.add_child("defends", write_battle_result_map(defends_inflicted));
	res.add_child("attacks_taken", write_battle_result_map(attacks_taken));
	res.add_child("defends_taken", write_battle_result_map(defends_taken));
	// Don't serialize by_cth_inflicted / by_cth_taken; they're deserialized from attacks_inflicted/defends_inflicted.
	res.add_child("turn_by_cth_inflicted", write_by_cth_map(turn_by_cth_inflicted));
	res.add_child("turn_by_cth_taken", write_by_cth_map(turn_by_cth_taken));

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

void stats_t::write(config_writer& out) const
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
	write_battle_result_map(out, attacks_inflicted);
	out.close_child("attacks");
	out.open_child("defends");
	write_battle_result_map(out, defends_inflicted);
	out.close_child("defends");
	out.open_child("attacks_taken");
	write_battle_result_map(out, attacks_taken);
	out.close_child("attacks_taken");
	out.open_child("defends_taken");
	write_battle_result_map(out, defends_taken);
	out.close_child("defends_taken");
	// Don't serialize by_cth_inflicted / by_cth_taken; they're deserialized from attacks_inflicted/defends.
	out.open_child("turn_by_cth_inflicted");
	out.write(write_by_cth_map(turn_by_cth_inflicted));
	out.close_child("turn_by_cth_inflicted");
	out.open_child("turn_by_cth_taken");
	out.write(write_by_cth_map(turn_by_cth_taken));
	out.close_child("turn_by_cth_taken");

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

void stats_t::read(const config& cfg)
{
	if(const auto c = cfg.optional_child("recruits")) {
		recruits = read_str_int_map(c.value());
	}
	if(const auto c = cfg.optional_child("recalls")) {
		recalls = read_str_int_map(c.value());
	}
	if(const auto c = cfg.optional_child("advances")) {
		advanced_to = read_str_int_map(c.value());
	}
	if(const auto c = cfg.optional_child("deaths")) {
		deaths = read_str_int_map(c.value());
	}
	if(const auto c = cfg.optional_child("killed")) {
		killed = read_str_int_map(c.value());
	}
	if(const auto c = cfg.optional_child("recalls")) {
		recalls = read_str_int_map(c.value());
	}
	if(const auto c = cfg.optional_child("attacks")) {
		attacks_inflicted = read_battle_result_map(c.value());
	}
	if(const auto c = cfg.optional_child("defends")) {
		defends_inflicted = read_battle_result_map(c.value());
	}
	if(const auto c = cfg.optional_child("attacks_taken")) {
		attacks_taken = read_battle_result_map(c.value());
	}
	if(const auto c = cfg.optional_child("defends_taken")) {
		defends_taken = read_battle_result_map(c.value());
	}
	by_cth_inflicted = read_by_cth_map_from_battle_result_maps(attacks_inflicted, defends_inflicted);
	// by_cth_taken will be an empty map in old (pre-#4070) savefiles that don't have
	// [attacks_taken]/[defends_taken] tags in their [statistics] tags
	by_cth_taken = read_by_cth_map_from_battle_result_maps(attacks_taken, defends_taken);
	if(const auto c = cfg.optional_child("turn_by_cth_inflicted")) {
		turn_by_cth_inflicted = read_by_cth_map(c.value());
	}
	if(const auto c = cfg.optional_child("turn_by_cth_taken")) {
		turn_by_cth_taken = read_by_cth_map(c.value());
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

void stats_t::merge_with(const stats_t& b)
{
	stats_t& a = *this;
	DBG_NG << "Merging statistics";
	merge_str_int_map(a.recruits, b.recruits);
	merge_str_int_map(a.recalls, b.recalls);
	merge_str_int_map(a.advanced_to, b.advanced_to);
	merge_str_int_map(a.deaths, b.deaths);
	merge_str_int_map(a.killed, b.killed);

	merge_cth_map(a.by_cth_inflicted, b.by_cth_inflicted);
	merge_cth_map(a.by_cth_taken, b.by_cth_taken);

	merge_battle_result_maps(a.attacks_inflicted, b.attacks_inflicted);
	merge_battle_result_maps(a.defends_inflicted, b.defends_inflicted);
	merge_battle_result_maps(a.attacks_taken, b.attacks_taken);
	merge_battle_result_maps(a.defends_taken, b.defends_taken);

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
	a.turn_by_cth_inflicted = b.turn_by_cth_inflicted;
	a.turn_by_cth_taken = b.turn_by_cth_taken;
}


scenario_stats_t::scenario_stats_t(const config& cfg)
	: team_stats()
	, scenario_name(cfg["scenario"])
{
	for(const config& team : cfg.child_range("team")) {
		team_stats[team["save_id"]] = stats_t(team);
	}
}

config scenario_stats_t::write() const
{
	config res;
	res["scenario"] = scenario_name;
	for(team_stats_t::const_iterator i = team_stats.begin(); i != team_stats.end(); ++i) {
		res.add_child("team", i->second.write());
	}

	return res;
}

void scenario_stats_t::write(config_writer& out) const
{
	out.write_key_val("scenario", scenario_name);
	for(team_stats_t::const_iterator i = team_stats.begin(); i != team_stats.end(); ++i) {
		out.open_child("team");
		i->second.write(out);
		out.close_child("team");
	}
}

config stats_t::hitrate_t::write() const
{
	return config("hits", hits, "strikes", strikes);
}

stats_t::hitrate_t::hitrate_t(const config& cfg)
	: strikes(cfg["strikes"].to_int())
	, hits(cfg["hits"].to_int())
{
}

config campaign_stats_t::to_config() const
{
	config res;

	for(std::vector<scenario_stats_t>::const_iterator i = master_record.begin(); i != master_record.end(); ++i) {
		res.add_child("scenario", i->write());
	}

	return res;
}

void campaign_stats_t::write(config_writer& out) const
{
	for(std::vector<scenario_stats_t>::const_iterator i = master_record.begin(); i != master_record.end(); ++i) {
		out.open_child("scenario");
		i->write(out);
		out.close_child("scenario");
	}
}

void campaign_stats_t::read(const config& cfg, bool append)
{
	if(!append) {
		master_record.clear();
	}
	for(const config& s : cfg.child_range("scenario")) {
		master_record.emplace_back(s);
	}
}

void campaign_stats_t::new_scenario(const std::string& name)
{
	master_record.emplace_back(name);
}

void campaign_stats_t::clear_current_scenario()
{
	if(master_record.empty() == false) {
		master_record.back().team_stats.clear();
	}
}

} // namespace statistics_record

std::ostream& operator<<(std::ostream& outstream, const statistics_record::stats_t::hitrate_t& by_cth)
{
	outstream << "[" << by_cth.hits << "/" << by_cth.strikes << "]";
	return outstream;
}
