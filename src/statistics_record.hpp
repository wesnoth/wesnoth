/*
	Copyright (C) 2003 - 2024
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

class config;
class config_writer;

#include <string>
#include <map>
#include <vector>

namespace statistics_record
{
	struct stats_t
	{
		stats_t();
		explicit stats_t(const config& cfg);

		config write() const;
		void write(config_writer &out) const;
		void read(const config& cfg);

		void merge_with(const stats_t& other);

		typedef std::map<std::string,int> str_int_map;
		str_int_map recruits, recalls, advanced_to, deaths, killed;
		int recruit_cost, recall_cost;

		/*
		 *  A type that will map a string of hit/miss to the number of times
		 *  that sequence has occurred.
		 */
		typedef str_int_map battle_sequence_frequency_map;

		/** A type that will map different % chances to hit to different results. */
		typedef std::map<int,battle_sequence_frequency_map> battle_result_map;

		/** Statistics of this side's attacks on its own turns. */
		battle_result_map attacks_inflicted;
		/** Statistics of this side's attacks on enemies' turns. */
		battle_result_map defends_inflicted;
		/** Statistics of enemies' counter attacks on this side's turns. */
		battle_result_map attacks_taken;
		/** Statistics of enemies' attacks against this side on their turns. */
		battle_result_map defends_taken;

		long long damage_inflicted, damage_taken;
		long long turn_damage_inflicted, turn_damage_taken;

		struct hitrate_t
		{
			int strikes; //< Number of strike attempts at the given CTH
			int hits; //< Number of strikes that hit at the given CTH
			hitrate_t() = default;
			explicit hitrate_t(const config& cfg);
			config write() const;
		};
		/** A type that maps chance-to-hit percentage to number of hits and strikes at that CTH. */
		typedef std::map<int, hitrate_t> hitrate_map;
		hitrate_map by_cth_inflicted, by_cth_taken;
		hitrate_map turn_by_cth_inflicted, turn_by_cth_taken;

		static const int decimal_shift = 1000;

		// Expected value for damage inflicted/taken * 1000, based on
		// probability to hit,
		// Use this long term to see how lucky a side is.

		long long expected_damage_inflicted, expected_damage_taken;
		long long turn_expected_damage_inflicted, turn_expected_damage_taken;
		std::string save_id;
	};


	using team_stats_t = std::map<std::string, stats_t>;

	struct scenario_stats_t
	{
		explicit scenario_stats_t(const std::string& name) :
			team_stats(),
			scenario_name(name)
		{}

		explicit scenario_stats_t(const config& cfg);

		config write() const;
		void write(config_writer &out) const;

		team_stats_t team_stats;
		std::string scenario_name;
	};

	struct campaign_stats_t
	{
		campaign_stats_t() = default;
		explicit campaign_stats_t(const config& cfg)
			: master_record()
		{
			read(cfg);
		}
		config to_config() const;
		void write(config_writer &out) const;
		void read(const config& cfg, bool append = false);
		/** Adds an entry for anew scenario to wrte to. */
		void new_scenario(const std::string & scenario_name);
		/** Delete the current scenario from the stats. */
		void clear_current_scenario();

		std::vector<scenario_stats_t> master_record;
	};

}

std::ostream& operator<<(std::ostream& outstream, const statistics_record::stats_t::hitrate_t& by_cth);
