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

#pragma once

#include "statistics_record.hpp"

class unit;
#include <string>
#include <map>
#include <vector>

class statistics_t
{
public:
	using stats = statistics_record::stats_t;

	statistics_t(statistics_record::campaign_stats_t& record);

	void recruit_unit(const unit& u);
	void recall_unit(const unit& u);
	void un_recall_unit(const unit& u);
	void un_recruit_unit(const unit& u);

	void advance_unit(const unit& u);

	void reset_turn_stats(const std::string & save_id);
	stats calculate_stats(const std::string & save_id);
	/** Stats (and name) for each scenario. The pointers are never nullptr. */
	typedef std::vector< std::pair<const std::string *, const stats *>> levels;
	/** Returns a list of names and stats for each scenario in the current campaign. */
	levels level_stats(const std::string & save_id);
	/// returns the stats for the given side in the current scenario.
	stats& get_stats(const std::string &save_id);

	static int sum_str_int_map(const std::map<std::string,int>& m);
	static int sum_cost_str_int_map(const std::map<std::string,int>& m);
private:
	statistics_record::campaign_stats_t& record_;

	auto& master_stats() {
		return record_.master_record;
	}
};

struct statistics_attack_context
{
	using stats = statistics_t::stats;

	statistics_attack_context(statistics_t& stats, const unit& a, const unit& d, int a_cth, int d_cth);
	~statistics_attack_context();
	enum hit_result { MISSES, HITS, KILLS };

	void attack_expected_damage(double attacker_inflict, double defender_inflict);
	void attack_result(hit_result res, int cth, int damage, int drain);
	void defend_result(hit_result res, int cth, int damage, int drain);
private:

	/// never nullptr
	statistics_t* stats_;

	std::string attacker_type, defender_type;
	std::string attacker_side, defender_side;
	int chance_to_hit_defender, chance_to_hit_attacker;
	std::string attacker_res, defender_res;

	stats& attacker_stats();
	stats& defender_stats();
};
