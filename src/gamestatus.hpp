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

class gamestatus
{
public:
	gamestatus(int num_turns);
	enum TIME { DAWN, DAY1, DAY2, DUSK, NIGHT1, NIGHT2, NUM_TIMES };

	static const std::string& timeofdayDescription(TIME t);
	TIME timeofday() const;
	int turn() const;
	int number_of_turns() const;

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
	TIME timeofday_;
	int turn_;
	int numTurns_;
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
