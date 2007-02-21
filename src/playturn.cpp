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

#include "playturn.hpp"

#include "game_config.hpp"
#include "gettext.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "show_dialog.hpp"
#include "sound.hpp"

turn_info::turn_info(const game_data& gameinfo, game_state& state_of_game,
                     const gamestatus& status, display& gui, gamemap& map,
		     std::vector<team>& teams, unsigned int team_num, unit_map& units,
			 undo_list& undo_stack)
  : gameinfo_(gameinfo), state_of_game_(state_of_game), status_(status),
    gui_(gui), map_(map), teams_(teams), team_num_(team_num),
    units_(units), undo_stack_(undo_stack)
{}

turn_info::~turn_info(){
	undo_stack_.clear();
}

void turn_info::turn_slice()
{
	events::pump();
	events::raise_process_event();
	events::raise_draw_event();
}

void turn_info::change_side_controller(const std::string& side, const std::string& player, bool own_side)
{
	config cfg;
	config& change = cfg.add_child("change_controller");
	change["side"] = side;
	change["player"] = player;

	if(own_side) {
		change["own_side"] = "yes";
	}
}
