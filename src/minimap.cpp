/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "global.hpp"
#include "minimap.hpp"

#include "gettext.hpp"
#include "image.hpp"
#include "log.hpp"
#include "map.hpp"
#include "sdl_utils.hpp"

#include "team.hpp"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"

static lg::log_domain log_display("display");
#define DBG_DP LOG_STREAM(debug, log_display)
#define WRN_DP LOG_STREAM(warn, log_display)


namespace image {

surface getMinimap(int w, int h, const gamemap &map, const team *vw)
{
	const int scale = 8;

	DBG_DP << "creating minimap " << int(map.w()*scale*0.75) << "," << int(map.h()*scale) << "\n";

	const size_t map_width = map.w()*scale*3/4;
	const size_t map_height = map.h()*scale;
	if(map_width == 0 || map_height == 0) {
		return surface(NULL);
	}

	surface minimap(create_neutral_surface(map_width, map_height));
	if(minimap == NULL)
		return surface(NULL);

	typedef mini_terrain_cache_map cache_map;
	cache_map *normal_cache = &mini_terrain_cache;
	cache_map *fog_cache = &mini_fogged_terrain_cache;

	for(int y = 0; y != map.total_height(); ++y) {
		for(int x = 0; x != map.total_width(); ++x) {

			surface surf(NULL);

			const map_location loc(x,y);
			if(map.on_board(loc)) {
				const bool shrouded = vw != NULL && vw->shrouded(loc);
				// shrouded hex are not considered fogged (no need to fog a black image)
				const bool fogged = vw != NULL && !shrouded && vw->fogged(loc);
				const t_translation::t_terrain terrain = shrouded ?
					t_translation::VOID_TERRAIN : map[loc];

				bool need_fogging = false;

				cache_map* cache = fogged ? fog_cache : normal_cache;
				cache_map::iterator i = cache->find(terrain);

				if (fogged && i == cache->end()) {
					// we don't have the fogged version in cache
					// try the normal cache and ask fogging the image
					cache = normal_cache;
					i = cache->find(terrain);
					need_fogging = true;
				}

				if(i == cache->end()) {
					surface tile(get_image("terrain/" + map.get_terrain_info(terrain).minimap_image() + ".png",image::HEXED));

					if(tile == 0) {
						utils::string_map symbols;
						symbols["terrain"] = t_translation::write_terrain_code(terrain);
						const std::string msg =
							vgettext("Could not get image for terrain: $terrain.", symbols);
						VALIDATE(false, msg);
					}

					//Compose images of base and overlay if neccessary
					if(map.get_terrain_info(terrain).is_combined()) {
						surface overlay(get_image("terrain/" + map.get_terrain_info(terrain).minimap_image_overlay() + ".png", image::HEXED));
						if(overlay != 0 && overlay != tile) {
							surface combined = create_compatible_surface(tile, tile->w, tile->h);
							SDL_Rect r;
							r.x = 0;
							r.y = 0;
							SDL_BlitSurface(tile, NULL, combined, &r);
							r.x = std::max(0, (tile->w - overlay->w)/2);
							r.y = std::max(0, (tile->h - overlay->h)/2);
                            if ((overlay->flags & SDL_RLEACCEL) == 0) {
                                blit_surface(overlay, NULL, combined, &r);
                            } else {
                                WRN_DP << map.get_terrain_info(terrain).minimap_image_overlay() << ".png overlay is RLE-encoded, creating a neutral surface\n";
                                surface overlay_neutral = make_neutral_surface(overlay);
							    blit_surface(overlay_neutral, NULL, combined, &r);
                            }
							tile = combined;
						}

					}

					surf = surface(scale_surface_blended(tile,scale,scale));

					VALIDATE(surf != NULL, _("Error creating or aquiring an image."));

					i = normal_cache->insert(cache_map::value_type(terrain,surf)).first;
				}

				surf = i->second;

				if (need_fogging) {
					surf = surface(adjust_surface_color(surf,-50,-50,-50));
					fog_cache->insert(cache_map::value_type(terrain,surf));
				}

				VALIDATE(surf != NULL, _("Error creating or aquiring an image."));

				// we need a balanced shift up and down of the hexes.
				// if not, only the bottom half-hexes are clipped
				// and it looks asymmetrical.

				// also do 1-pixel shift because the scaling
				// function seems to do it with its rounding
				SDL_Rect maprect = {x * scale*3/4 - 1,
					y*scale + scale/4 * (is_odd(x) ? 1 : -1) - 1,
					0, 0};
				SDL_BlitSurface(surf, NULL, minimap, &maprect);
			}
		}
	}

	double wratio = w*1.0 / minimap->w;
	double hratio = h*1.0 / minimap->h;
	double ratio = std::min<double>(wratio, hratio);

	minimap = scale_surface(minimap,
		static_cast<int>(minimap->w * ratio), static_cast<int>(minimap->h * ratio));

	DBG_DP << "done generating minimap\n";

	return minimap;
}
}
