/* $Id$ */
/*
   Copyright (C) 2010 - 2011 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "exceptions.hpp"
#include "game_errors.hpp"
#include "game_end_exceptions.hpp"
#include "video.hpp"

char const *game::exception::sticky;

void game::exception::rethrow()
{
	if (!sticky) return;
	throw game::exception("Unknown exception", "unknown");
}

