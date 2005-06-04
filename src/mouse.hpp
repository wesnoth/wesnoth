/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@comcast.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef MOUSE_H_INCLUDED
#define MOUSE_H_INCLUDED

namespace gui {

void scroll_inc();
void scroll_dec();

void scroll_reduce();

int scrollamount();

void scroll_reset();

}

#endif
