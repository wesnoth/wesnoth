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
#ifndef MINIMAP_HPP_INCLUDED
#define MINIMAP_HPP_INCLUDED

#include "map.hpp"
#include "sdl_utils.hpp"


class team;

namespace image {
	///function to create the minimap for a given map
	///the surface returned must be freed by the user
	surface getMinimap(int w, int h, const gamemap& map_, const viewpoint* vm=NULL);
}

#endif
