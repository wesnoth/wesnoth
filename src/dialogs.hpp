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
#ifndef DIALOGS_H_INCLUDED
#define DIALOGS_H_INCLUDED

#include "actions.hpp"
#include "config.hpp"
#include "display.hpp"
#include "show_dialog.hpp"
#include "unit.hpp"

namespace dialogs
{
void advance_unit(const game_data& info,
                  std::map<gamemap::location,unit>& units,
				  const gamemap::location& loc,
				  display& gui, bool random_choice=false);

void show_objectives(display& disp, config& level_info);

// Ask user if I should really save the game and what name I should use
// returns 0 iff user wants to save the game
int get_save_name(display & disp, const std::string& caption, 
				const std::string& message, std::string* name,
				gui::DIALOG_TYPE dialog_type=gui::YES_NO);

//allow user to select the game they want to load. Returns the name
//of the save they want to load. Stores whether the user wants to show
//a replay of the game in show_replay. If show_replay is NULL, then
//the user will not be asked if they want to show a replay.
std::string load_game_dialog(display& disp, bool* show_replay);

void unit_speak(const config& message_info, display& disp, const unit_map& units);

}

#endif
