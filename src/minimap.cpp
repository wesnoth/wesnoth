/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "minimap.hpp"

#include "color.hpp"
#include "display.hpp"
#include "game_board.hpp"
#include "gettext.hpp"
#include "picture.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "preferences/general.hpp"
#include "resources.hpp"
#include "sdl/render_utils.hpp"
#include "team.hpp"
#include "terrain/type_data.hpp"
#include "units/map.hpp"
#include "units/unit.hpp"

static lg::log_domain log_display("display");
#define DBG_DP LOG_STREAM(debug, log_display)
#define WRN_DP LOG_STREAM(warn, log_display)

namespace image
{
void render_minimap(unsigned dst_w,
		unsigned dst_h,
		const gamemap& map,
		const team* vw,
		const unit_map* units,
		const std::map<map_location, unsigned int>* reach_map,
		bool ignore_terrain_disabled)
{
	CVideo& video = CVideo::get_singleton();
	display* disp = display::get_singleton();

	const bool is_blindfolded = disp && disp->is_blindfolded();

	const terrain_type_data& tdata = *map.tdata();

	// Drawing mode flags.
	const bool preferences_minimap_draw_terrain   = preferences::minimap_draw_terrain() || ignore_terrain_disabled;
	const bool preferences_minimap_terrain_coding = preferences::minimap_terrain_coding();
	const bool preferences_minimap_draw_villages  = preferences::minimap_draw_villages();
	const bool preferences_minimap_draw_units     = preferences::minimap_draw_units();
	const bool preferences_minimap_unit_coding    = preferences::minimap_movement_coding();

	const int scale = (preferences_minimap_draw_terrain && preferences_minimap_terrain_coding) ? 24 : 4;

	DBG_DP << "Creating minimap: " << static_cast<int>(map.w() * scale * 0.75) << ", " << map.h() * scale << std::endl;

	const std::size_t map_width  = map.w() * scale * 3 / 4;
	const std::size_t map_height = map.h() * scale;

	// Gets a destination rect for drawing at the given coordinates.
	// We need a balanced shift up and down of the hexes.
	// If not, only the bottom half-hexes are clipped and it looks asymmetrical.
	auto get_dst_rect = [scale](int x, int y) ->SDL_Rect {
		return {
			x * scale             * 3 / 4                - (scale / 4),
			y * scale + scale / 4 * (is_odd(x) ? 1 : -1) - (scale / 4),
			scale,
			scale
		};
	};

	// No map!
	if(map_width == 0 || map_height == 0) {
		return;
	}

	// Nothing to draw!
	if(!preferences_minimap_draw_villages && !preferences_minimap_draw_terrain) {
		return;
	}

	// We want to draw the minimap with NN scaling.
	set_texture_scale_quality("nearest");

	// Create a temp surface a bit larger than we want. This allows us to compose the minimap and then
	// scale the whole result down the desired destination texture size.
	texture minimap(map_width, map_height, SDL_TEXTUREACCESS_TARGET);
	if(minimap.null()) {
		return;
	}

	{
		// Point rendering to the temp minimap texture.
		render_target_setter target_setter(minimap);

		//
		// Terrain
		//
		for(int y = 0; y <= map.total_height(); ++y) {
			for(int x = 0; x <= map.total_width(); ++x) {
				const map_location loc(x, y);

				if(!map.on_board_with_border(loc)) {
					continue;
				}

				const bool shrouded = is_blindfolded || (vw && vw->shrouded(loc));

				// Shrouded hex are not considered fogged (no need to fog a black image)
				const bool fogged = (vw != nullptr && !shrouded && vw->fogged(loc));

				const bool highlighted = reach_map && reach_map->count(loc) != 0 && !shrouded;

				const t_translation::terrain_code terrain = shrouded ? t_translation::VOID_TERRAIN : map[loc];
				const terrain_type& terrain_info = tdata.get_terrain_info(terrain);

				// Destination rect for drawing the current hex.
				SDL_Rect map_dst_rect = get_dst_rect(x, y);

				//
				// Draw map terrain...
				//
				if(preferences_minimap_draw_terrain) {
					if(preferences_minimap_terrain_coding) {
						//
						// ...either using terrain images...
						//
						if(!terrain_info.minimap_image().empty()) {
							const std::string base_file = "terrain/" + terrain_info.minimap_image() + ".png";
							const texture& tile = image::get_texture(base_file); // image::HEXED

							// TODO: handle fog (was a -50, -50, -50 adjust) and highlight (was a 50, 50, 50 adjust).
							video.render_copy(tile, nullptr, &map_dst_rect);

							// NOTE: we skip the overlay when base is missing (to avoid hiding the error)
							if(tile && tdata.get_terrain_info(terrain).is_combined()
								&& !terrain_info.minimap_image_overlay().empty())
							{
								const std::string overlay_file = "terrain/" + terrain_info.minimap_image_overlay() + ".png";

								const texture& overlay = image::get_texture(overlay_file); // image::HEXED

								// TODO: crop/center overlays?
								video.render_copy(overlay, nullptr, &map_dst_rect);
							}
						}
					} else {
						//
						// ... or color coding.
						//
						color_t col(0, 0, 0, 0);

						auto it = game_config::team_rgb_range.find(terrain_info.id());
						if(it != game_config::team_rgb_range.end()) {
							col = it->second.rep();
						}

						bool first = true;
						const t_translation::ter_list& underlying_terrains = tdata.underlying_union_terrain(terrain);

						for(const t_translation::terrain_code& underlying_terrain : underlying_terrains) {
							const std::string& terrain_id = tdata.get_terrain_info(underlying_terrain).id();

							it = game_config::team_rgb_range.find(terrain_id);
							if(it == game_config::team_rgb_range.end()) {
								continue;
							}

							color_t tmp = it->second.rep();

							if(fogged) {
								if(tmp.b < 50) {
									tmp.b = 0;
								} else {
									tmp.b -= 50;
								}

								if(tmp.g < 50) {
									tmp.g = 0;
								} else {
									tmp.g -= 50;
								}

								if(tmp.r < 50) {
									tmp.r = 0;
								} else {
									tmp.r -= 50;
								}
							}

							if(highlighted) {
								if(tmp.b > 205) {
									tmp.b = 255;
								} else {
									tmp.b += 50;
								}

								if(tmp.g > 205) {
									tmp.g = 255;
								} else {
									tmp.g += 50;
								}

								if(tmp.r > 205) {
									tmp.r = 255;
								} else {
									tmp.r += 50;
								}
							}

							if(first) {
								first = false;
								col = tmp;
							} else {
								col.r = col.r - (col.r - tmp.r) / 2;
								col.g = col.g - (col.g - tmp.g) / 2;
								col.b = col.b - (col.b - tmp.b) / 2;
							}
						}

						SDL_Rect fillrect {map_dst_rect.x, map_dst_rect.y, scale * 3 / 4, scale};
						sdl::fill_rectangle(fillrect, col);
					}
				}

				//
				// Draw village markers independent of terrain.
				//
				if(terrain_info.is_village() && preferences_minimap_draw_villages) {
					// Check needed for mp create dialog
					const int side = resources::gameboard
						? resources::gameboard->village_owner(loc)
						: -1;

					color_t col(255, 255, 255);

					// TODO: Add a key to [game_config][colors] for this
					auto iter = game_config::team_rgb_range.find("white");
					if(iter != game_config::team_rgb_range.end()) {
						col = iter->second.min();
					}

					if(!fogged && side > -1) {
						if(preferences_minimap_unit_coding || !vw) {
							col = team::get_minimap_color(side + 1);
						} else {
							if(vw->owns_village(loc)) {
								col = game_config::color_info(preferences::unmoved_color()).rep();
							} else if(vw->is_enemy(side + 1)) {
								col = game_config::color_info(preferences::enemy_color()).rep();
							} else {
								col = game_config::color_info(preferences::allied_color()).rep();
							}
						}
					}

					SDL_Rect fillrect {map_dst_rect.x, map_dst_rect.y, scale * 3 / 4, scale};
					sdl::fill_rectangle(fillrect, col);
				}
			}
		}

		//
		// Units
		//
		if(units && preferences_minimap_draw_units && !is_blindfolded) {
			for(const auto& u : *units) {
				const map_location& u_loc = u.get_location();
				const int side = u.side();
				const bool is_enemy = vw && vw->is_enemy(side);

				if((vw && vw->fogged(u_loc)) || (is_enemy && disp && u.invisible(u_loc)) || u.get_hidden()) {
					continue;
				}

				color_t col = team::get_minimap_color(side);

				if(!preferences_minimap_unit_coding) {
					if(is_enemy) {
						col = game_config::color_info(preferences::enemy_color()).rep();
					} else {
						if(vw && vw->side() == side) {
							if(u.movement_left() == u.total_movement()) {
								col = game_config::color_info(preferences::unmoved_color()).rep();
							} else if (u.movement_left() == 0) {
								col = game_config::color_info(preferences::moved_color()).rep();
							} else {
								col = game_config::color_info(preferences::partial_color()).rep();
							}
						} else {
							col = game_config::color_info(preferences::allied_color()).rep();
						}
					}
				}


				SDL_Rect fillrect = get_dst_rect(u_loc.x, u_loc.y);
				fillrect.w = scale * 3 / 4; // TODO: needed?

				sdl::fill_rectangle(fillrect, col);
			}
		}
	}

	texture::info src_info = minimap.get_info();

	const double wratio = dst_w * 1.0 / src_info.w;
	const double hratio = dst_h * 1.0 / src_info.h;

	const double ratio = std::min<double>(wratio, hratio);

	// TODO: maybe add arguments so we can set render origin?
	SDL_Rect final_dst_rect {
		0,
		0,
		static_cast<int>(src_info.w * ratio),
		static_cast<int>(src_info.h * ratio)
	};

	// Finally, render the composited minimap texture (scaled down) to the render target,
	// which should be the passed texture.
	video.render_copy(minimap, nullptr, &final_dst_rect);

	DBG_DP << "done generating minimap" << std::endl;
}

} // end namespace image
