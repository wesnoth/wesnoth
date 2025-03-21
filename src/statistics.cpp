/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
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
 *  Manage statistics: recruitments, recalls, kills, losses, etc.
 */

#include "statistics.hpp"
#include "game_board.hpp"
#include "log.hpp"
#include "resources.hpp" // Needed for teams, to get team save_id for a unit
#include "team.hpp" // Needed to get team save_id
#include "units/types.hpp"
#include "units/unit.hpp"

#include <cmath>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

namespace
{

std::string get_team_save_id(const unit& u)
{
	assert(resources::gameboard);
	return resources::gameboard->get_team(u.side()).save_id_or_number();
}

}

statistics_t::statistics_t(statistics_record::campaign_stats_t& record)
	: record_(record)
{

}

statistics_attack_context::statistics_attack_context(
	statistics_t& stats, const unit& a, const unit& d, int a_cth, int d_cth)
	: stats_(&stats)
	, attacker_type(a.type_id())
	, defender_type(d.type_id())
	, attacker_side(get_team_save_id(a))
	, defender_side(get_team_save_id(d))
	, chance_to_hit_defender(a_cth)
	, chance_to_hit_attacker(d_cth)
	, attacker_res()
	, defender_res()
{
}

statistics_attack_context::~statistics_attack_context()
{
	std::string attacker_key = "s" + attacker_res;
	std::string defender_key = "s" + defender_res;

	attacker_stats().attacks_inflicted[chance_to_hit_defender][attacker_key]++;
	defender_stats().defends_inflicted[chance_to_hit_attacker][defender_key]++;

	attacker_stats().attacks_taken[chance_to_hit_attacker][defender_key]++;
	defender_stats().defends_taken[chance_to_hit_defender][attacker_key]++;
}

statistics_attack_context::stats& statistics_attack_context::attacker_stats()
{
	return stats_->get_stats(attacker_side);
}

statistics_attack_context::stats& statistics_attack_context::defender_stats()
{
	return stats_->get_stats(defender_side);
}

void statistics_attack_context::attack_expected_damage(double attacker_inflict_, double defender_inflict_)
{
	int attacker_inflict = std::round(attacker_inflict_ * stats::decimal_shift);
	int defender_inflict = std::round(defender_inflict_ * stats::decimal_shift);
	stats &att_stats = attacker_stats(), &def_stats = defender_stats();
	att_stats.expected_damage_inflicted += attacker_inflict;
	att_stats.expected_damage_taken += defender_inflict;
	def_stats.expected_damage_inflicted += defender_inflict;
	def_stats.expected_damage_taken += attacker_inflict;
	att_stats.turn_expected_damage_inflicted += attacker_inflict;
	att_stats.turn_expected_damage_taken += defender_inflict;
	def_stats.turn_expected_damage_inflicted += defender_inflict;
	def_stats.turn_expected_damage_taken += attacker_inflict;
}

void statistics_attack_context::attack_result(hit_result res, int cth, int damage, int drain)
{
	attacker_res.push_back(res == MISSES ? '0' : '1');
	stats &att_stats = attacker_stats(), &def_stats = defender_stats();

	if(res != MISSES) {
		++att_stats.by_cth_inflicted[cth].hits;
		++att_stats.turn_by_cth_inflicted[cth].hits;
		++def_stats.by_cth_taken[cth].hits;
		++def_stats.turn_by_cth_taken[cth].hits;
	}
	++att_stats.by_cth_inflicted[cth].strikes;
	++att_stats.turn_by_cth_inflicted[cth].strikes;
	++def_stats.by_cth_taken[cth].strikes;
	++def_stats.turn_by_cth_taken[cth].strikes;

	if(res != MISSES) {
		// handle drain
		att_stats.damage_taken -= drain;
		def_stats.damage_inflicted -= drain;
		att_stats.turn_damage_taken -= drain;
		def_stats.turn_damage_inflicted -= drain;

		att_stats.damage_inflicted += damage;
		def_stats.damage_taken += damage;
		att_stats.turn_damage_inflicted += damage;
		def_stats.turn_damage_taken += damage;
	}

	if(res == KILLS) {
		++att_stats.killed[defender_type];
		++def_stats.deaths[defender_type];
	}
}

void statistics_attack_context::defend_result(hit_result res, int cth, int damage, int drain)
{
	defender_res.push_back(res == MISSES ? '0' : '1');
	stats &att_stats = attacker_stats(), &def_stats = defender_stats();

	if(res != MISSES) {
		++def_stats.by_cth_inflicted[cth].hits;
		++def_stats.turn_by_cth_inflicted[cth].hits;
		++att_stats.by_cth_taken[cth].hits;
		++att_stats.turn_by_cth_taken[cth].hits;
	}
	++def_stats.by_cth_inflicted[cth].strikes;
	++def_stats.turn_by_cth_inflicted[cth].strikes;
	++att_stats.by_cth_taken[cth].strikes;
	++att_stats.turn_by_cth_taken[cth].strikes;

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

void statistics_t::recruit_unit(const unit& u)
{
	stats& s = get_stats(get_team_save_id(u));
	s.recruits[u.type().parent_id()]++;
	s.recruit_cost += u.cost();
}

void statistics_t::recall_unit(const unit& u)
{
	stats& s = get_stats(get_team_save_id(u));
	s.recalls[u.type_id()]++;
	s.recall_cost += u.cost();
}

void statistics_t::un_recall_unit(const unit& u)
{
	stats& s = get_stats(get_team_save_id(u));
	s.recalls[u.type_id()]--;
	s.recall_cost -= u.cost();
}

void statistics_t::un_recruit_unit(const unit& u)
{
	stats& s = get_stats(get_team_save_id(u));
	s.recruits[u.type().parent_id()]--;
	s.recruit_cost -= u.cost();
}

void statistics_t::advance_unit(const unit& u)
{
	stats& s = get_stats(get_team_save_id(u));
	s.advanced_to[u.type_id()]++;
}

void statistics_t::reset_turn_stats(const std::string& save_id)
{
	stats& s = get_stats(save_id);
	s.turn_damage_inflicted = 0;
	s.turn_damage_taken = 0;
	s.turn_expected_damage_inflicted = 0;
	s.turn_expected_damage_taken = 0;
	s.turn_by_cth_inflicted.clear();
	s.turn_by_cth_taken.clear();
	s.save_id = save_id;
}

statistics_t::stats statistics_t::calculate_stats(const std::string& save_id)
{
	stats res;

	DBG_NG << "calculate_stats, side: " << save_id << " master_stats.size: " << master_stats().size();
	// The order of this loop matters since the turn stats are taken from the
	// last stats merged.
	for(std::size_t i = 0; i != master_stats().size(); ++i) {
		auto find_it = master_stats()[i].team_stats.find(save_id);
		if(find_it != master_stats()[i].team_stats.end()) {
			res.merge_with(find_it->second);
		}
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
statistics_t::levels statistics_t::level_stats(const std::string& save_id)
{
	static const stats null_stats;
	static const std::string null_name("");

	levels level_list;

	for(std::size_t level = 0; level != master_stats().size(); ++level) {
		const auto& team_stats = master_stats()[level].team_stats;

		auto find_it = team_stats.find(save_id);
		if(find_it != team_stats.end()) {
			level_list.emplace_back(&master_stats()[level].scenario_name, &find_it->second);
		}
	}

	// Make sure we do return something (so other code does not have to deal
	// with an empty list).
	if(level_list.empty()) {
		level_list.emplace_back(&null_name, &null_stats);
	}

	return level_list;
}

statistics_t::stats& statistics_t::get_stats(const std::string& save_id)
{
	if(master_stats().empty()) {
		master_stats().emplace_back(std::string());
	}

	return master_stats().back().team_stats[save_id];
}

int statistics_t::sum_str_int_map(const std::map<std::string, int>& m)
{
	int res = 0;
	for(const auto& pair: m) {
		res += pair.second;
	}

	return res;
}

int statistics_t::sum_cost_str_int_map(const std::map<std::string, int>& m)
{
	int cost = 0;
	for(const auto& pair : m) {
		const unit_type* t = unit_types.find(pair.first);
		if(!t) {
			ERR_NG << "Statistics refer to unknown unit type '" << pair.first << "'. Discarding.";
		} else {
			cost += pair.second * t->cost();
		}
	}

	return cost;
}
