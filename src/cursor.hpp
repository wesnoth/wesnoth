/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file cursor.hpp
//!

#ifndef CURSOR_HPP_INCLUDED
#define CURSOR_HPP_INCLUDED

#include "SDL.h"
#include "sdl_utils.hpp"

namespace cursor
{

struct manager
{
	manager();
	~manager();
};

enum CURSOR_TYPE { NORMAL, WAIT, MOVE, ATTACK, HYPERLINK, MOVE_DRAG, ATTACK_DRAG, NO_CURSOR, NUM_CURSORS };

//! This function uses temporary the b&w cursors while the display is busy.
// If used, the colour-cursors will be reused when the display is ready again.
void temporary_use_bw();

//! Use the default parameter to reset cursors.
// e.g. after a change in color cursor preferences
void set(CURSOR_TYPE type = NUM_CURSORS);
void set_dragging(bool drag);
CURSOR_TYPE get();

void draw(surface screen);
void undraw(surface screen);

void set_focus(bool focus);

struct setter
{
	setter(CURSOR_TYPE type);
	~setter();

private:
	CURSOR_TYPE old_;
};

} // end namespace cursor

#endif
