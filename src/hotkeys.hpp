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
#ifndef HOTKEYS_HPP_INCLUDED
#define HOTKEYS_HPP_INCLUDED

#include "config.hpp"
#include "display.hpp"
#include "key.hpp"

enum HOTKEY_COMMAND { HOTKEY_CYCLE_UNITS, HOTKEY_END_UNIT_TURN, HOTKEY_LEADER,
                      HOTKEY_UNDO, HOTKEY_REDO,
                      HOTKEY_ZOOM_IN, HOTKEY_ZOOM_OUT, HOTKEY_ZOOM_DEFAULT,
                      HOTKEY_FULLSCREEN, HOTKEY_ACCELERATED,
                      HOTKEY_TERRAIN_TABLE, HOTKEY_ATTACK_RESISTANCE,
                      HOTKEY_UNIT_DESCRIPTION, HOTKEY_SAVE_GAME,
                      HOTKEY_RECRUIT, HOTKEY_RECALL, HOTKEY_ENDTURN,
                      HOTKEY_NULL };

void add_hotkeys(config& cfg);

HOTKEY_COMMAND check_keys(display& disp);

#endif
