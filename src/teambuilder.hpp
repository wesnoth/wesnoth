/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/



#ifndef TEAMBUILDER_HPP_INCLUDED
#define TEAMBUILDER_HPP_INCLUDED

#include <memory>
#include <vector>

class config;
class gamemap;
class team;
class team_builder;
class game_board;
typedef std::shared_ptr<team_builder> team_builder_ptr;

//create an object responsible for creating and populating a team from a config
team_builder_ptr create_team_builder(const config& side_cfg,
					 std::vector<team>& teams,
					 const config& level, game_board& board, int num);

//do first stage of team initialization (everything except unit placement)
void build_team_stage_one(team_builder_ptr tb_ptr);

//do second stage of team initialization (unit placement)
void build_team_stage_two(team_builder_ptr tb_ptr);

#endif
