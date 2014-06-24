/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_state.hpp"

#include "log.hpp"
#include "map.hpp"
#include "pathfind/teleport.hpp"

#include <boost/foreach.hpp>

#include <algorithm>
#include <set>

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

game_state::game_state(const config & level, const config & game_config) :
	level_(level),
	gamedata_(level_),
	board_(game_config,level_),
	tod_manager_(level_),
	pathfind_manager_()
{}

game_state::~game_state() {}

static int placing_score(const config& side, const gamemap& map, const map_location& pos)
{
	int positions = 0, liked = 0;
	const t_translation::t_list terrain = t_translation::read_list(side["terrain_liked"]);

	for(int i = pos.x-8; i != pos.x+8; ++i) {
		for(int j = pos.y-8; j != pos.y+8; ++j) {
			const map_location pos(i,j);
			if(map.on_board(pos)) {
				++positions;
				if(std::count(terrain.begin(),terrain.end(),map[pos])) {
					++liked;
				}
			}
		}
	}

	return (100*liked)/positions;
}

struct placing_info {

	placing_info() :
		side(0),
		score(0),
		pos()
	{
	}

	int side, score;
	map_location pos;
};

static bool operator<(const placing_info& a, const placing_info& b) { return a.score > b.score; }


void game_state::place_sides_in_preferred_locations()
{
	std::vector<placing_info> placings;

	int num_pos = board_.map().num_valid_starting_positions();

	int side_num = 1;
	BOOST_FOREACH(const config &side, level_.child_range("side"))
	{
		for(int p = 1; p <= num_pos; ++p) {
			const map_location& pos = board_.map().starting_position(p);
			int score = placing_score(side, board_.map(), pos);
			placing_info obj;
			obj.side = side_num;
			obj.score = score;
			obj.pos = pos;
			placings.push_back(obj);
		}
		++side_num;
	}

	std::sort(placings.begin(),placings.end());
	std::set<int> placed;
	std::set<map_location> positions_taken;

	for (std::vector<placing_info>::const_iterator i = placings.begin(); i != placings.end() && int(placed.size()) != side_num - 1; ++i) {
		if(placed.count(i->side) == 0 && positions_taken.count(i->pos) == 0) {
			placed.insert(i->side);
			positions_taken.insert(i->pos);
			board_.map_->set_starting_position(i->side,i->pos);
			LOG_NG << "placing side " << i->side << " at " << i->pos << std::endl;
		}
	}
}
