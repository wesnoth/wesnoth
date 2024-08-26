/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
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
#include "draw.hpp"
#include "game_board.hpp"
#include "gettext.hpp"
#include "picture.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "preferences/preferences.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "terrain/type_data.hpp"
#include "units/unit.hpp"

static lg::log_domain log_display("display");
#define DBG_DP LOG_STREAM(debug, log_display)
#define WRN_DP LOG_STREAM(warn, log_display)

namespace image {

std::function<rect(rect)> prep_minimap_for_rendering(
		const gamemap& map,
		const team* vw,
		const unit_map* units,
		const std::map<map_location, unsigned int>* reach_map,
		bool ignore_terrain_disabled)
{
	// Drawing mode flags.
	const bool preferences_minimap_draw_terrain   = prefs::get().minimap_draw_terrain() || ignore_terrain_disabled;
	const bool preferences_minimap_terrain_coding = prefs::get().minimap_terrain_coding();
	const bool preferences_minimap_draw_villages  = prefs::get().minimap_draw_villages();
	const bool preferences_minimap_draw_units     = prefs::get().minimap_draw_units();
	const bool preferences_minimap_unit_coding    = prefs::get().minimap_movement_coding();

	const int scale = (preferences_minimap_draw_terrain && preferences_minimap_terrain_coding) ? 24 : 4;

	DBG_DP << "Creating minimap: " << static_cast<int>(map.w() * scale * 0.75) << ", " << map.h() * scale;

	const std::size_t map_width  = static_cast<size_t>(std::max(0, map.w())) * scale * 3 / 4;
	const std::size_t map_height = static_cast<size_t>(std::max(0, map.h())) * scale;

	// No map!
	if(map_width == 0 || map_height == 0) {
		return nullptr;
	}

	// Nothing to draw!
	if(!preferences_minimap_draw_villages && !preferences_minimap_draw_terrain) {
		return nullptr;
	}

	const display* disp = display::get_singleton();
	const bool is_blindfolded = disp && disp->is_blindfolded();

	const auto shrouded = [&](const map_location& loc) {
		return is_blindfolded || (vw && vw->shrouded(loc));
	};

	const auto fogged = [&](const map_location& loc) {
		// Shrouded hex are not considered fogged (no need to fog a black image)
		return vw && !shrouded(loc) && vw->fogged(loc);
	};

	// Gets a destination rect for drawing at the given coordinates.
	// We need a balanced shift up and down of the hexes.
	// If not, only the bottom half-hexes are clipped and it looks asymmetrical.
	const auto get_dst_rect = [scale](const map_location& loc) {
		return rect {
			loc.x * scale             * 3 / 4                    - (scale / 4),
			loc.y * scale + scale / 4 * (is_odd(loc.x) ? 1 : -1) - (scale / 4),
			scale,
			scale
		};
	};

	// We want to draw the minimap with NN scaling.
	set_texture_scale_quality("nearest");

	// Create a temp texture a bit larger than we want. This allows us to compose the minimap and then
	// scale the whole result down the desired destination texture size.
	texture minimap(map_width, map_height, SDL_TEXTUREACCESS_TARGET);
	if(!minimap) {
		return nullptr;
	}

	{
		// Point rendering to the temp minimap texture.
		const draw::render_target_setter target_setter{minimap};

		// Clear the minimap texture, as some of it can be left transparent.
		draw::clear();

		//
		// Terrain
		//
		if(preferences_minimap_draw_terrain) {
			map.for_each_loc([&](const map_location& loc) {
				const bool highlighted = reach_map && reach_map->count(loc) != 0 && !shrouded(loc);

				const t_translation::terrain_code terrain = shrouded(loc) ? t_translation::VOID_TERRAIN : map[loc];
				const terrain_type& terrain_info = map.tdata()->get_terrain_info(terrain);

				// Destination rect for drawing the current hex.
				rect dest = get_dst_rect(loc);

				//
				// Draw map terrain using either terrain images...
				//
				if(preferences_minimap_terrain_coding) {
					if(!terrain_info.minimap_image().empty()) {
						const std::string base_file = "terrain/" + terrain_info.minimap_image() + ".png";
						const texture& tile = image::get_texture(base_file); // image::HEXED

						draw::blit(tile, dest);

						// NOTE: we skip the overlay when base is missing (to avoid hiding the error)
						if(tile && map.tdata()->get_terrain_info(terrain).is_combined()
							&& !terrain_info.minimap_image_overlay().empty())
						{
							const std::string overlay_file = "terrain/" + terrain_info.minimap_image_overlay() + ".png";
							const texture& overlay = image::get_texture(overlay_file); // image::HEXED

							// TODO: crop/center overlays?
							draw::blit(overlay, dest);
						}

						// FIXME: use shaders instead of textures for this once we can actually do that
						using namespace std::string_literals;

						if(fogged(loc)) {
							// Hex-shaped texture to apply #000000 at 40% opacity
							static const texture fog_overlay = image::get_texture("terrain/minimap-fog.png"s);
							draw::blit(fog_overlay, dest);
						}

						if(highlighted) {
							// Hex-shaped texture to apply #ffffff at 40% opacity
							static const texture fog_overlay = image::get_texture("terrain/minimap-highlight.png"s);
							draw::blit(fog_overlay, dest);
						}
					}
				} else {
					//
					// ... or color coding.
					//
					color_t col(0, 0, 0, 0);

					// Despite its name, game_config::team_rgb_range isn't just team colors,
					// it has "red", "lightblue", "cave", "reef", "fungus", etc.
					auto it = game_config::team_rgb_range.find(terrain_info.id());
					if(it != game_config::team_rgb_range.end()) {
						col = it->second.rep();
					}

					bool first = true;

					for(const auto& underlying_terrain : map.tdata()->underlying_union_terrain(terrain)) {
						const std::string& terrain_id = map.tdata()->get_terrain_info(underlying_terrain).id();

						it = game_config::team_rgb_range.find(terrain_id);
						if(it == game_config::team_rgb_range.end()) {
							return;
						}

						color_t tmp = it->second.rep();

						if(fogged(loc)) {
							tmp.r = std::max(0, tmp.r - 50);
							tmp.g = std::max(0, tmp.g - 50);
							tmp.b = std::max(0, tmp.b - 50);
						}

						if(highlighted) {
							tmp.r = std::min(255, tmp.r + 50);
							tmp.g = std::min(255, tmp.g + 50);
							tmp.b = std::min(255, tmp.b + 50);
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

					dest.w = scale * 3 / 4;
					draw::fill(dest, col);
				}
			});
		}

		//
		// Villages
		//
		if(preferences_minimap_draw_villages) {
			for(const map_location& loc : map.villages()) {
				if(is_blindfolded || (vw && (vw->shrouded(loc) || vw->fogged(loc)))) {
					continue;
				}

				color_t col(255, 255, 255);

				// TODO: Add a key to [game_config][colors] for this
				auto iter = game_config::team_rgb_range.find("white");
				if(iter != game_config::team_rgb_range.end()) {
					col = iter->second.min();
				}

				// Check needed for mp create dialog
				const int side_num = resources::gameboard ? resources::gameboard->village_owner(loc) : 0;

				if(side_num > 0) {
					if(preferences_minimap_unit_coding || !vw) {
						col = team::get_minimap_color(side_num);
					} else {
						if(vw->owns_village(loc)) {
							col = game_config::color_info(prefs::get().unmoved_color()).rep();
						} else if(vw->is_enemy(side_num)) {
							col = game_config::color_info(prefs::get().enemy_color()).rep();
						} else {
							col = game_config::color_info(prefs::get().allied_color()).rep();
						}
					}
				}

				rect dest = get_dst_rect(loc);
				dest.w = scale * 3 / 4;

				draw::fill(dest, col);
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
					auto status = orb_status::allied;

					if(is_enemy) {
						status = orb_status::enemy;
					} else if(vw && vw->side() == side) {
						status = disp->get_disp_context().unit_orb_status(u);
					} else {
						// no-op, status is already set to orb_status::allied;
					}

					col = game_config::color_info(orb_status_helper::get_orb_color(status)).rep();
				}

				rect fillrect = get_dst_rect(u_loc);
				fillrect.w = scale * 3 / 4;

				draw::fill(fillrect, col);
			}
		}
	}

	DBG_DP << "done generating minimap";

	return [minimap](rect dst) {
		const auto [raw_w, raw_h] = minimap.get_raw_size();

		// Check which dimensions needs to be shrunk more
		const double scale_ratio = std::min<double>(
			dst.w * 1.0 / raw_w,
			dst.h * 1.0 / raw_h
		);

		// Preserve map aspect ratio within the requested area
		const int scaled_w = static_cast<int>(raw_w * scale_ratio);
		const int scaled_h = static_cast<int>(raw_h * scale_ratio);

		// Attempt to center the map in the requested area
		dst.x = std::max(dst.x, dst.x + (dst.w - scaled_w) / 2);
		dst.y = std::max(dst.y, dst.y + (dst.h - scaled_h) / 2);
		dst.w = scaled_w;
		dst.h = scaled_h;

		draw::blit(minimap, dst);

		// Let the caller know where the minimap *actually* ended up being drawn
		return dst;
	};
}

}
