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

#ifndef INCL_GAME_STATE_HPP_
#define INCL_GAME_STATE_HPP_

class config;

#include "filter_context.hpp"
#include "game_board.hpp"
#include "game_data.hpp"
#include "tod_manager.hpp"

#include <boost/scoped_ptr.hpp>

namespace pathfind { class manager; }

class game_state : public filter_context
{
public:
	const config& level_;
	game_data gamedata_;
	game_board board_;
	tod_manager tod_manager_;
	boost::scoped_ptr<pathfind::manager> pathfind_manager_;

	int first_human_team_; //needed to initialize the viewpoint during setup

	game_state(const config & level, const config & game_config);

	~game_state();

	void place_sides_in_preferred_locations();

	void init(int ticks);

	config to_config() const;

	virtual const display_context & get_disp_context() const { return board_; }
	virtual const tod_manager & get_tod_man() const { return tod_manager_; }
};

#endif
