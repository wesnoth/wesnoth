/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#pragma once

class surface;

namespace cursor
{

struct manager
{
	manager();
	~manager();
};

enum CURSOR_TYPE { NORMAL, WAIT, MOVE, ATTACK, HYPERLINK, MOVE_DRAG, ATTACK_DRAG, NO_CURSOR, NUM_CURSORS };

/**
 * Use the default parameter to reset cursors.
 * e.g. after a change in color cursor preferences
 */
void set(CURSOR_TYPE type = NUM_CURSORS);
void set_dragging(bool drag);
CURSOR_TYPE get();

void set_focus(bool focus);

struct setter
{
	setter(CURSOR_TYPE type);
	~setter();

private:
	CURSOR_TYPE old_;
};

} // end namespace cursor
