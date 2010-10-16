/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TITLE_HPP_INCLUDED
#define TITLE_HPP_INCLUDED

class config;

/** Read the file with the tips-of-the-day. */
void read_tips_of_day(config& tips_of_day);

/** Go to the next tips-of-the-day */
void next_tip_of_day(config& tips_of_day, bool reverse = false);

/** Return the text for one of the tips-of-the-day. */
const config* get_tip_of_day(config& tips_of_day);

namespace gui {

}

#endif
