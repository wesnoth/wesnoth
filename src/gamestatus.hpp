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
#ifndef GAME_STATUS_HPP_INCLUDED
#define GAME_STATUS_HPP_INCLUDED

#include "unit.hpp"
#include "unit_types.hpp"

#include <string>
#include <vector>

struct time_of_day
{
	explicit time_of_day(config& cfg);

	int lawful_bonus;
	std::string image;
	std::string name;
	int red, green, blue;
};

class gamestatus
{
public:
	gamestatus(config& time_cfg, int num_turns);

	const time_of_day& get_time_of_day(bool illuminated=false) const;
	size_t turn() const;
	size_t number_of_turns() const;

	bool next_turn();

	struct load_game_failed {
		load_game_failed() {}
		load_game_failed(const std::string& msg) : message(msg) {}

		std::string message;
	};

	struct game_error {
		game_error(const std::string& msg) : message(msg) {}
		std::string message;
	};

private:
	std::vector<time_of_day> times_, illuminatedTimes_;

	size_t turn_;
	size_t numTurns_;
};

struct game_state
{
	game_state() : gold(-1), difficulty("NORMAL") {}
	std::string label;
	std::string version;
	std::string campaign_type;
	int scenario;
	int gold;
	std::vector<unit> available_units;
	std::map<std::string,std::string> variables;
	std::string difficulty;

	config replay_data;
	config starting_pos;
};

std::vector<std::string> get_saves_list();

void load_game(game_data& data, const std::string& name, game_state& state);
void save_game(const game_state& state);

#endif
