/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef MINIMAP_HPP_INCLUDED
#define MINIMAP_HPP_INCLUDED

#include <cstddef>
#include <map>

class gamemap;
class surface;
class team;
struct map_location;
class gamemap;

#ifdef SDL_GPU
class CVideo;
struct SDL_Rect;
#endif

namespace image {
	///function to create the minimap for a given map
	///the surface returned must be freed by the user
#ifdef SDL_GPU
	SDL_Rect draw_minimap(CVideo &video, const SDL_Rect &area, const gamemap &map, const team *vw = nullptr, const std::map<map_location,unsigned int> *reach_map = nullptr);
#endif
	surface getMinimap(int w, int h, const gamemap &map_, const team *vm = nullptr, const std::map<map_location,unsigned int> *reach_map = nullptr);
}

#endif
