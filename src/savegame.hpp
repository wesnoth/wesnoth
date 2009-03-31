/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by Jörg Hinrichs, refactored from various
   places formerly created by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SAVEGAME_H_INCLUDED
#define SAVEGAME_H_INCLUDED

#include "global.hpp"
#include "gamestatus.hpp"

#include <string>

/** Autosave */
std::string save_autosave(unsigned turn, const config& snapshot, game_state& gamestate);

/** Normal midgame save */
void save_game(std::string& filename, const config& snapshot, game_state& gamestate);

/** Replay save created by player interaction */
void save_replay(std::string& filename, game_state& gamestate);

/** Replay save, created automatically at the end of a scenario */
void save_replay(game_state& gamestate);

#endif
