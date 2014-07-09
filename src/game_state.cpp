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

#include "game_data.hpp"
#include "loadscreen.hpp"
#include "log.hpp"
#include "map.hpp"
#include "pathfind/teleport.hpp"
#include "game_preferences.hpp"
#include "random_new_deterministic.hpp"

#include <boost/foreach.hpp>
#include <SDL_timer.h>

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
	pathfind_manager_(),
	first_human_team_(-1)
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

void game_state::init(const int ticks)
{
	if (level_["modify_placing"].to_bool()) {
		LOG_NG << "modifying placing..." << std::endl;
		place_sides_in_preferred_locations();
	}

	LOG_NG << "initialized time of day regions... "    << (SDL_GetTicks() - ticks) << std::endl;
	BOOST_FOREACH(const config &t, level_.child_range("time_area")) {
		tod_manager_.add_time_area(board_.map(),t);
	}

	LOG_NG << "initialized teams... "    << (SDL_GetTicks() - ticks) << std::endl;
	loadscreen::start_stage("init teams");

	board_.teams_.resize(level_.child_count("side"));

	std::vector<team_builder_ptr> team_builders;

	int team_num = 0;
	BOOST_FOREACH(const config &side, level_.child_range("side"))
	{
		if (first_human_team_ == -1) {
			const std::string &controller = side["controller"];
			if (controller == "human" &&
			    side["id"] == preferences::login()) {
				first_human_team_ = team_num;
			} else if (controller == "human") {
				first_human_team_ = team_num;
			}
		}
		team_builder_ptr tb_ptr = gamedata_.create_team_builder(side,
			board_.teams_, level_, *board_.map_);
		++team_num;
		gamedata_.build_team_stage_one(tb_ptr);
		team_builders.push_back(tb_ptr);
	}
	{
		//sync traits of start units and the random start time.
		random_new::set_random_determinstic deterministic(gamedata_.rng());

		tod_manager_.resolve_random(*random_new::generator);

		BOOST_FOREACH(team_builder_ptr tb_ptr, team_builders)
		{
			gamedata_.build_team_stage_two(tb_ptr);
		}
	}

	pathfind_manager_.reset(new pathfind::manager(level_));
}

config game_state::to_config() const
{
	config cfg;

	board_.write_config(cfg);

	cfg.merge_with(tod_manager_.to_config());

	//write out the current state of the map
	cfg.merge_with(pathfind_manager_->to_config());

	gamedata_.write_snapshot(cfg);

	return cfg;
}
