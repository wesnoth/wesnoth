/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"
#include "image.hpp"
#include "log.hpp"
#include "minimap.hpp"
#include "team.hpp"
#include "wassert.hpp"

#define LOG_DP LOG_STREAM(info, display)
#define ERR_DP LOG_STREAM(err, display)

namespace image {

surface getMinimap(int w, int h, const gamemap& map, const viewpoint* vw)
{
	const int scale = 8;

	LOG_DP << "creating minimap " << int(map.w()*scale*0.75) << "," << int(map.h()*scale) << "\n";

	const size_t map_width = map.w()*scale*3/4;
	const size_t map_height = map.h()*scale;
	if(map_width == 0 || map_height == 0) {
		return surface(NULL);
	}

	surface minimap(SDL_CreateRGBSurface(SDL_SWSURFACE,
	                               map_width,map_height,
	                               pixel_format->BitsPerPixel,
	                               pixel_format->Rmask,
	                               pixel_format->Gmask,
	                               pixel_format->Bmask,
	                               pixel_format->Amask));
	if(minimap == NULL)
		return surface(NULL);

	typedef mini_terrain_cache_map cache_map;
	cache_map& cache = mini_terrain_cache;
	cache_map& fog_cache = mini_fogged_terrain_cache;

	for(int y = 0; y != map.h(); ++y) {
		for(int x = 0; x != map.w(); ++x) {

			surface surf(NULL);

			const gamemap::location loc(x,y);
			if(map.on_board(loc)) {
				const bool shrouded = vw != NULL && vw->shrouded(x,y);
				const bool fogged = vw != NULL && vw->fogged(x,y) && !shrouded;
				const t_translation::t_letter terrain = shrouded ? t_translation::VOID_TERRAIN : map[x][y];

				cache_map::iterator i;
				bool need_fogging = false;

				if (fogged) {
					i = fog_cache.find(terrain);
				}
				if (!fogged || i == fog_cache.end()) {
					i = cache.find(terrain);
					need_fogging = fogged;
				}

				if(i == cache.end()) {
					surface tile(get_image("terrain/" + map.get_terrain_info(terrain).minimap_image() + ".png",image::HEXED));

					if(tile == NULL) {
						ERR_DP << "could not get image for terrrain '"
						          << terrain << "'\n";
						continue;
					}

					surf = surface(scale_surface_blended(tile,scale,scale));

					if(surf == NULL) {
						continue;
					}
					i = mini_terrain_cache.insert(cache_map::value_type(terrain,surf)).first;
				}

				surf = i->second;
				
				if (need_fogging) {
					surf = surface(adjust_surface_colour(surf,-50,-50,-50));
					mini_fogged_terrain_cache.insert(cache_map::value_type(terrain,surf));
				}

				wassert(surf != NULL);

				SDL_Rect maprect = {x*scale*3/4,y*scale + (is_odd(x) ? scale/2 : 0),0,0};
				SDL_BlitSurface(surf, NULL, minimap, &maprect);
			}
		}
	}

	if((minimap->w != w || minimap->h != h) && w != 0 && h != 0) {
		const surface surf(minimap);

#if 0
		// preserve the aspect ratio of the original map rather than
		// distorting it to fit the minimap window.
		//
		// This needs more work.  There are at least two issues:
		// (1) the part of the minimap window outside the scaled map
		// needs to be blacked out/invalidated.
		// (2) the rather nasty code in draw_minimap_units needs to 
		// change.
		float sw = 1.0, sh = 1.0;

		if (minimap->h < minimap->w) sh = (minimap->h*1.0)/minimap->w;
		if (minimap->w < minimap->h) sw = (minimap->w*1.0)/minimap->h;
		w = int(w * sw);
		h = int(h * sh);
#endif

		minimap = surface(scale_surface(surf,w,h));
	}

	LOG_DP << "done generating minimap\n";

	return minimap;
}
}
