/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "global.hpp"
#include "minimap.hpp"

#include "game_board.hpp"
#include "gettext.hpp"
#include "image.hpp"
#include "log.hpp"
#include "map.hpp"
#include "resources.hpp"
#include "sdl/utils.hpp"
#include "team.hpp"
#include "terrain_type_data.hpp"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"

#include "game_display.hpp"

#include <boost/foreach.hpp>

#include "preferences.hpp"

static lg::log_domain log_display("display");
#define DBG_DP LOG_STREAM(debug, log_display)
#define WRN_DP LOG_STREAM(warn, log_display)


namespace image {

surface getMinimap(int w, int h, const gamemap &map, const team *vw, const std::map<map_location,unsigned int> *reach_map)
{
	const terrain_type_data & tdata = *map.tdata();

	const int scale = 8;

	DBG_DP << "creating minimap " << int(map.w()*scale*0.75) << "," << map.h()*scale << "\n";

	const bool preferences_minimap_draw_terrain = preferences::minimap_draw_terrain();
	const bool preferences_minimap_terrain_coding = preferences::minimap_terrain_coding();
	const bool preferences_minimap_draw_villages = preferences::minimap_draw_villages();
	const bool preferences_minimap_unit_coding = preferences::minimap_movement_coding();

	const size_t map_width = map.w()*scale*3/4;
	const size_t map_height = map.h()*scale;
	if(map_width == 0 || map_height == 0) {
		return surface(NULL);
	}

	if(!preferences_minimap_draw_villages && !preferences_minimap_draw_terrain)
	{
		//return if there is nothing to draw.
		//(optimisation)
		double ratio = std::min<double>( w*1.0 / map_width, h*1.0 / map_height);
		return create_neutral_surface(map_width * ratio, map_height * ratio);
	}

	surface minimap(create_neutral_surface(map_width, map_height));
	if(minimap == NULL)
		return surface(NULL);

	typedef mini_terrain_cache_map cache_map;
	cache_map *normal_cache = &mini_terrain_cache;
	cache_map *fog_cache = &mini_fogged_terrain_cache;
	cache_map *highlight_cache = &mini_highlighted_terrain_cache;

	for(int y = 0; y != map.total_height(); ++y)
		for(int x = 0; x != map.total_width(); ++x) {

			const map_location loc(x,y);
			if(!map.on_board(loc))
				continue;

			const bool highlighted = reach_map && reach_map->count(loc) != 0;

			const bool shrouded = (resources::screen != NULL && resources::screen->is_blindfolded()) || (vw != NULL && vw->shrouded(loc));
			// shrouded hex are not considered fogged (no need to fog a black image)
			const bool fogged = (vw != NULL && !shrouded && vw->fogged(loc));

			const t_translation::t_terrain terrain = shrouded ?
					t_translation::VOID_TERRAIN : map[loc];
			const terrain_type& terrain_info = tdata.get_terrain_info(terrain);

			// we need a balanced shift up and down of the hexes.
			// if not, only the bottom half-hexes are clipped
			// and it looks asymmetrical.

			// also do 1-pixel shift because the scaling
			// function seems to do it with its rounding
			SDL_Rect maprect = sdl::create_rect(
					x * scale * 3 / 4 - 1
					, y * scale + scale / 4 * (is_odd(x) ? 1 : -1) - 1
					, 0
					, 0);

			if (preferences_minimap_draw_terrain) {

				if (preferences_minimap_terrain_coding) {

					surface surf(NULL);

					bool need_fogging = false;
					bool need_highlighting = false;

					cache_map* cache = fogged ? fog_cache : normal_cache;
					if (highlighted)
						cache = highlight_cache;
					cache_map::iterator i = cache->find(terrain);

					if (fogged && i == cache->end()) {
						// we don't have the fogged version in cache
						// try the normal cache and ask fogging the image
						cache = normal_cache;
						i = cache->find(terrain);
						need_fogging = true;
					}

					if (highlighted && i == cache->end()) {
						// we don't have the highlighted version in cache
						// try the normal cache and ask fogging the image
						cache = normal_cache;
						i = cache->find(terrain);
						need_highlighting = true;
					}

					if(i == cache->end() && !terrain_info.minimap_image().empty()) {
						std::string base_file =
								"terrain/" + terrain_info.minimap_image() + ".png";
						surface tile = get_image(base_file,image::HEXED);

						//Compose images of base and overlay if necessary
						// NOTE we also skip overlay when base is missing (to avoid hiding the error)
						if(tile != NULL && tdata.get_terrain_info(terrain).is_combined() && !terrain_info.minimap_image_overlay().empty()) {
							std::string overlay_file =
									"terrain/" + terrain_info.minimap_image_overlay() + ".png";
							surface overlay = get_image(overlay_file,image::HEXED);

							if(overlay != NULL && overlay != tile) {
								surface combined = create_neutral_surface(tile->w, tile->h);
								SDL_Rect r = sdl::create_rect(0,0,0,0);
								sdl_blit(tile, NULL, combined, &r);
								r.x = std::max(0, (tile->w - overlay->w)/2);
								r.y = std::max(0, (tile->h - overlay->h)/2);
								//blit_surface needs neutral surface
								surface overlay_neutral = make_neutral_surface(overlay);
								blit_surface(overlay_neutral, NULL, combined, &r);
								tile = combined;
							}
						}

						surf = scale_surface_sharp(tile, scale, scale);

						i = normal_cache->insert(cache_map::value_type(terrain,surf)).first;
					}

					surf = i->second;

					if (need_fogging) {
						surf = adjust_surface_color(surf,-50,-50,-50);
						fog_cache->insert(cache_map::value_type(terrain,surf));
					}

					if (need_highlighting) {
						surf = adjust_surface_color(surf,50,50,50);
						highlight_cache->insert(cache_map::value_type(terrain,surf));
					}

					if(surf != NULL)
						sdl_blit(surf, NULL, minimap, &maprect);

				} else {

					SDL_Color col;
					std::map<std::string, color_range>::const_iterator it = game_config::team_rgb_range.find(terrain_info.id());
					if (it == game_config::team_rgb_range.end()) {
						col = create_color(0,0,0,0);
					} else
						col = int_to_color(it->second.rep());

					bool first = true;
					const t_translation::t_list& underlying_terrains = tdata.underlying_union_terrain(terrain);
					BOOST_FOREACH(const t_translation::t_terrain& underlying_terrain, underlying_terrains) {

						const std::string& terrain_id = tdata.get_terrain_info(underlying_terrain).id();
						std::map<std::string, color_range>::const_iterator it = game_config::team_rgb_range.find(terrain_id);
						if (it == game_config::team_rgb_range.end())
							continue;

						SDL_Color tmp = int_to_color(it->second.rep());

						if (fogged) {
							if (tmp.b < 50) tmp.b = 0;
							else tmp.b -= 50;
							if (tmp.g < 50) tmp.g = 0;
							else tmp.g -= 50;
							if (tmp.r < 50) tmp.r = 0;
							else tmp.r -= 50;
						}

						if (highlighted) {
							if (tmp.b > 205) tmp.b = 255;
							else tmp.b += 50;
							if (tmp.g > 205) tmp.g = 255;
							else tmp.g += 50;
							if (tmp.r > 205) tmp.r = 255;
							else tmp.r += 50;
						}

						if (first) {
							first = false;
							col = tmp;
						} else {
							col.r = col.r - (col.r - tmp.r)/2;
							col.g = col.g - (col.g - tmp.g)/2;
							col.b = col.b - (col.b - tmp.b)/2;
						}
					}
					SDL_Rect fillrect = sdl::create_rect(maprect.x, maprect.y, scale * 3/4, scale);
					const Uint32 mapped_col = SDL_MapRGB(minimap->format,col.r,col.g,col.b);
					sdl::fill_rect(minimap, &fillrect, mapped_col);
				}
			}

			if (terrain_info.is_village() && preferences_minimap_draw_villages) {

				int side = (resources::gameboard ? resources::gameboard->village_owner(loc) : -1); //check needed for mp create dialog

				SDL_Color col = int_to_color(game_config::team_rgb_range.find("white")->second.min());

				if (!fogged) {
					if (side > -1) {

						if (preferences_minimap_unit_coding || !vw ) {
							col = team::get_minimap_color(side + 1);
						} else {

							if (vw->owns_village(loc))
								col = int_to_color(game_config::color_info(preferences::unmoved_color()).rep());
							else if (vw->is_enemy(side + 1))
								col = int_to_color(game_config::color_info(preferences::enemy_color()).rep());
							else
								col = int_to_color(game_config::color_info(preferences::allied_color()).rep());
						}
					}
				}

				SDL_Rect fillrect = sdl::create_rect(
						maprect.x
						, maprect.y
						, scale * 3/4
						, scale
				);

				const Uint32 mapped_col = SDL_MapRGB(minimap->format,col.r,col.g,col.b);
				sdl::fill_rect(minimap, &fillrect, mapped_col);

			}

		}

	double wratio = w*1.0 / minimap->w;
	double hratio = h*1.0 / minimap->h;
	double ratio = std::min<double>(wratio, hratio);

	minimap = scale_surface_sharp(minimap,
		static_cast<int>(minimap->w * ratio), static_cast<int>(minimap->h * ratio));

	DBG_DP << "done generating minimap\n";

	return minimap;
}

#ifdef SDL_GPU
SDL_Rect draw_minimap(CVideo &video, const SDL_Rect &area, const gamemap &map, const team *vw, const std::map<map_location, unsigned int> *reach_map)
{
	const terrain_type_data & tdata = *map.tdata();

	const float width = map.w() * 72 * 3/4;
	const float height = map.h() * 72 + 72 * 1/4;
	const float scale_factor = std::min(area.w / width, area.h / height);
	const float tile_size = 72 * scale_factor;
	const int xoff = area.x + (area.w - scale_factor * width) / 2;
	const int yoff = area.y + (area.h - scale_factor * height) / 2 + 1;

	const bool draw_terrain = preferences::minimap_draw_terrain();
	const bool terrain_coding = preferences::minimap_terrain_coding();
	const bool draw_villages = preferences::minimap_draw_villages();

	for(int y = 0; y != map.total_height(); ++y) {
		for(int x = 0; x != map.total_width(); ++x) {
			map_location loc(x, y);
			if(!map.on_board(loc))
				continue;

			const bool highlighted = reach_map && reach_map->count(loc) != 0;
			const bool shrouded = (resources::screen != NULL && resources::screen->is_blindfolded()) || (vw != NULL && vw->shrouded(loc));
			// shrouded hex are not considered fogged (no need to fog a black image)
			const bool fogged = (vw != NULL && !shrouded && vw->fogged(loc));

			const t_translation::t_terrain terrain = shrouded ?
					t_translation::VOID_TERRAIN : map[loc];
			const terrain_type& terrain_info = tdata.get_terrain_info(terrain);

			const int xpos = x * tile_size * 3/4 + xoff;
			const int ypos = y * tile_size + tile_size / 4 * (is_odd(x) ? 2 : 0) + yoff;

			if (draw_terrain) {
				if (terrain_coding) {
					sdl::timage img = image::get_texture("terrain/" + terrain_info.minimap_image() + ".png", image::HEXED);
					//TODO: proper color mod values once blending is implemented
					if (fogged) {
						img.set_color_mod(100, 100, 100);
					}
					if (highlighted) {
						img.set_color_mod(150, 150, 150);
					}

					img.set_scale(scale_factor, scale_factor);

					video.draw_texture(img, xpos, ypos);

					if (terrain_info.is_combined()) {
						sdl::timage overlay = image::get_texture("terrain/" + terrain_info.minimap_image_overlay() + ".png", image::HEXED);
						if (fogged) {
							overlay.set_color_mod(100, 100, 100);
						}
						if (highlighted) {
							overlay.set_color_mod(150, 150, 150);
						}

						overlay.set_scale(scale_factor, scale_factor);
						video.draw_texture(overlay, xpos, ypos);
					}
				} else {
					SDL_Color col;
					std::map<std::string, color_range>::const_iterator it = game_config::team_rgb_range.find(terrain_info.id());
					if (it == game_config::team_rgb_range.end()) {
						col = create_color(0,0,0,255);
					} else
						col = int_to_color(it->second.rep());

					bool first = true;
					const t_translation::t_list& underlying_terrains = tdata.underlying_union_terrain(terrain);
					BOOST_FOREACH(const t_translation::t_terrain& underlying_terrain, underlying_terrains) {

						const std::string& terrain_id = tdata.get_terrain_info(underlying_terrain).id();
						std::map<std::string, color_range>::const_iterator it = game_config::team_rgb_range.find(terrain_id);
						if (it == game_config::team_rgb_range.end())
							continue;

						SDL_Color tmp = int_to_color(it->second.rep());

						if (fogged) {
							if (tmp.b < 50) tmp.b = 0;
							else tmp.b -= 50;
							if (tmp.g < 50) tmp.g = 0;
							else tmp.g -= 50;
							if (tmp.r < 50) tmp.r = 0;
							else tmp.r -= 50;
						}

						if (highlighted) {
							if (tmp.b > 205) tmp.b = 255;
							else tmp.b += 50;
							if (tmp.g > 205) tmp.g = 255;
							else tmp.g += 50;
							if (tmp.r > 205) tmp.r = 255;
							else tmp.r += 50;
						}

						if (first) {
							first = false;
							col = tmp;
						} else {
							col.r = col.r - (col.r - tmp.r)/2;
							col.g = col.g - (col.g - tmp.g)/2;
							col.b = col.b - (col.b - tmp.b)/2;
						}
					}
					SDL_Rect fillrect = sdl::create_rect(xpos, ypos, tile_size * 3/4, tile_size);
					sdl::fill_rect(video, fillrect, col);
				}
			}


			if (terrain_info.is_village() && draw_villages) {

				int side = (resources::gameboard ? resources::gameboard->village_owner(loc) : -1); //check needed for mp create dialog

				SDL_Color col = int_to_color(game_config::team_rgb_range.find("white")->second.min());

				if (!fogged) {
					if (side > -1) {

						if (!preferences::minimap_movement_coding() || !vw ) {
							col = team::get_minimap_color(side + 1);
						} else {

							if (vw->owns_village(loc))
								col = int_to_color(game_config::color_info(preferences::unmoved_color()).rep());
							else if (vw->is_enemy(side + 1))
								col = int_to_color(game_config::color_info(preferences::enemy_color()).rep());
							else
								col = int_to_color(game_config::color_info(preferences::allied_color()).rep());
						}
					}
				}

				SDL_Rect fillrect = sdl::create_rect(
						xpos
						, ypos
						, tile_size * 3/4
						, tile_size
				);

				sdl::fill_rect(video, fillrect, col);
			}
		}
	}

	return sdl::create_rect(xoff, yoff, width * scale_factor, height * scale_factor);
}
#endif

}
