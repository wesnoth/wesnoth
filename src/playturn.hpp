/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef PLAYTURN_HPP_INCLUDED
#define PLAYTURN_HPP_INCLUDED

#include "global.hpp"

#include "actions.hpp"
#include "config.hpp"
#include "display.hpp"
#include "gamestatus.hpp"
#include "generic_event.hpp"
#include "team.hpp"
#include "unit.hpp"

#include <map>
#include <vector>

class turn_info
{
public:
	turn_info(const game_data& gameinfo, game_state& state_of_game,
	          const gamestatus& status, display& gui, gamemap& map,
		  std::vector<team>& teams, unsigned int team_num, unit_map& units,
		  undo_list& undo_stack);

	~turn_info();

	void turn_slice();

	enum PROCESS_DATA_RESULT { PROCESS_CONTINUE, PROCESS_RESTART_TURN, PROCESS_END_TURN };

private:
	void change_side_controller(const std::string& side, const std::string& player, bool own_side=false);

	const game_data& gameinfo_;
	game_state& state_of_game_;
	const gamestatus& status_;
	display& gui_;
	gamemap& map_;
	std::vector<team>& teams_;
	unsigned int team_num_;
	unit_map& units_;

	undo_list& undo_stack_;
};

#endif
