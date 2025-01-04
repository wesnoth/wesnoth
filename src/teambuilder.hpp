/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
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

#include "config.hpp"

#include <deque>
#include <set>
#include <string>
#include <vector>

class team;
class game_board;

class team_builder
{
public:
	team_builder(const config& side_cfg, team& to_build, const config& level, game_board& board, int num);

	/**
	 * Very important! Delete copy constructor and declare a move constructor.
	 *
	 * @c `unit_configs_` may hold a pointer to a config object stored in @c `leader_configs_`.
	 * In that case, if a copy happens (such as when doing in-place construction that requires
	 * the vector be resized), these pointers will become invalid and lead to a crash.
	 */
	team_builder(const team_builder&) = delete;
	team_builder(team_builder&&) = default;

	/** Handles the first stage of team initialization (everything except unit construction). */
	void build_team_stage_one();

	/** Handles the second stage of team initialization ((some) unit construction). */
	void build_team_stage_two();
	/** Handles the third stage of team initialization (unit placement). */
	void build_team_stage_three();

private:
	std::deque<config> leader_configs_;
	// only used for objectives
	const config& level_;
	game_board& board_;
	std::set<std::string> seen_ids_;
	int side_;
	const config& side_cfg_;
	team& team_;
	std::vector<const config*> unit_configs_;

	void log_step(const char* s) const;
	void init();
	void gold();
	void new_team();
	void objectives();
	void previous_recruits();
	void handle_unit(const config& u, const char* origin);
	void handle_leader(const config& leader);
	void leader();
	void prepare_units();
	void place_units();
};
