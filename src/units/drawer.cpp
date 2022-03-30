/*
	Copyright (C) 2014 - 2022
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "units/drawer.hpp"

#include "color.hpp"
#include "display.hpp"
#include "display_context.hpp"
#include "formatter.hpp"
#include "game_display.hpp"
#include "halo.hpp"
#include "map/location.hpp"
#include "map/map.hpp"
#include "picture.hpp"
#include "preferences/game.hpp"
#include "sdl/surface.hpp"
#include "team.hpp"
#include "units/animation.hpp"
#include "units/animation_component.hpp"
#include "units/frame.hpp"
#include "units/types.hpp"
#include "units/unit.hpp"

// Map of different energy bar surfaces and their dimensions.
static std::map<surface, SDL_Rect> energy_bar_rects;

namespace
{
/**
 * Wrapper which will assemble the image path (including IPF for the color from get_orb_color) for a given orb.
 * Returns nullptr if the preferences have been configured to hide this orb.
 */
std::unique_ptr<image::locator> get_orb_image(orb_status os)
{
	if(os == orb_status::disengaged) {
		if(orb_status_helper::prefs_show_orb(os)) {
			auto partial_color = orb_status_helper::get_orb_color(orb_status::partial);
			auto moved_color = orb_status_helper::get_orb_color(orb_status::moved);
			return std::make_unique<image::locator>(game_config::images::orb_two_color + "~RC(ellipse_red>"
				+ moved_color + ")~RC(magenta>" + partial_color + ")");
		}
		os = orb_status::partial;
	}

	if(!orb_status_helper::prefs_show_orb(os))
		return nullptr;
	auto color = orb_status_helper::get_orb_color(os);
	return std::make_unique<image::locator>(game_config::images::orb + "~RC(magenta>" + color + ")");
}
}

unit_drawer::unit_drawer(display & thedisp) :
	disp(thedisp),
	dc(disp.get_disp_context()),
	map(dc.map()),
	teams(dc.teams()),
	halo_man(thedisp.get_halo_manager()),
	viewing_team(disp.viewing_team()),
	playing_team(disp.playing_team()),
	viewing_team_ref(teams[viewing_team]),
	playing_team_ref(teams[playing_team]),
	is_blindfolded(disp.is_blindfolded()),
	show_everything(disp.show_everything()),
	sel_hex(disp.selected_hex()),
	mouse_hex(disp.mouseover_hex()),
	zoom_factor(disp.get_zoom_factor()),
	hex_size(disp.hex_size()),
	hex_size_by_2(disp.hex_size()/2)
{
	if(const game_display* game_display = dynamic_cast<class game_display*>(&disp)) {
		units_that_can_reach_goal = game_display->units_that_can_reach_goal();
	}

	// This used to be checked in the drawing code, where it simply triggered skipping some logic.
	// However, I think it's obsolete, and that the initialization of viewing_team_ref would already
	// be undefined behavior in the situation where this assert fails.
	assert(disp.team_valid());
}

void unit_drawer::redraw_unit (const unit & u) const
{
	unit_animation_component & ac = u.anim_comp();
	map_location loc = u.get_location();

	int side = u.side();

	bool hidden = u.get_hidden();
	bool is_flying = u.is_flying();
	map_location::DIRECTION facing = u.facing();
	int hitpoints = u.hitpoints();
	int max_hitpoints = u.max_hitpoints();

	bool can_recruit = u.can_recruit();
	bool can_advance = u.can_advance();

	int experience = u.experience();
	int max_experience = u.max_experience();

	bool emit_zoc = u.emits_zoc();

	color_t hp_color=u.hp_color();
	color_t xp_color=u.xp_color();

	std::string ellipse=u.image_ellipse();

	const bool is_highlighted_enemy = units_that_can_reach_goal.count(loc) > 0;
	const bool is_selected_hex = (loc == sel_hex || is_highlighted_enemy);

	if(hidden || is_blindfolded || !u.is_visible_to_team(viewing_team_ref, show_everything)) {
		ac.clear_haloes();
		if(ac.anim_) {
			ac.anim_->update_last_draw_time();
		}
		return;
	}

	if (!ac.anim_) {
		ac.set_standing();
		if (!ac.anim_) return;
	}

	if (ac.refreshing_) return;
	ac.refreshing_ = true;

	ac.anim_->update_last_draw_time();
	frame_parameters params;
	const t_translation::terrain_code terrain = map.get_terrain(loc);
	const terrain_type& terrain_info = map.get_terrain_info(terrain);

	// do not set to 0 so we can distinguish the flying from the "not on submerge terrain"
	// instead use -1.0 (as in "negative depth", it will be ignored by rendering)
	params.submerge= is_flying ? -1.0 : terrain_info.unit_submerge();

	if(u.invisible(loc) && params.highlight_ratio > 0.6) {
		params.highlight_ratio = 0.6;
	}
	if (is_selected_hex && params.highlight_ratio == 1.0) {
		params.highlight_ratio = 1.5;
	}

	int height_adjust = static_cast<int>(terrain_info.unit_height_adjust() * zoom_factor);
	if (is_flying && height_adjust < 0) {
		height_adjust = 0;
	}
	params.y -= height_adjust;
	params.halo_y -= height_adjust;

	int red = 0,green = 0,blue = 0,tints = 0;
	double blend_ratio = 0;
	// Add future colored states here
	if(u.poisoned()) {
		green += 255;
		blend_ratio += 0.25;
		tints += 1;
	}
	if(u.slowed()) {
		red += 191;
		green += 191;
		blue += 255;
		blend_ratio += 0.25;
		tints += 1;
	}
	if(tints > 0) {
		params.blend_with = color_t((red/tints),(green/tints),(blue/tints));
		params.blend_ratio = ((blend_ratio/tints));
	}

	//hackish : see unit_frame::merge_parameters
	// we use image_mod on the primary image
	// and halo_mod on secondary images and all haloes
	params.image_mod = u.image_mods();
	params.halo_mod = u.TC_image_mods();
	params.image= u.default_anim_image();


	if(u.incapacitated()) params.image_mod +="~GS()";
	params.primary_frame = true;


	const frame_parameters adjusted_params = ac.anim_->get_current_params(params);

	const map_location dst = loc.get_direction(facing);
	const int xsrc = disp.get_location_x(loc);
	const int ysrc = disp.get_location_y(loc);
	const int xdst = disp.get_location_x(dst);
	const int ydst = disp.get_location_y(dst);

	const int x = static_cast<int>(adjusted_params.offset * xdst + (1.0-adjusted_params.offset) * xsrc) + hex_size_by_2;
	const int y = static_cast<int>(adjusted_params.offset * ydst + (1.0-adjusted_params.offset) * ysrc) + hex_size_by_2;

	bool has_halo = ac.unit_halo_ && ac.unit_halo_->valid();
	if(!has_halo && !u.image_halo().empty()) {
		ac.unit_halo_ = halo_man.add(x, y - height_adjust, u.image_halo()+u.TC_image_mods(), map_location(-1, -1));
	}
	if(has_halo && u.image_halo().empty()) {
		halo_man.remove(ac.unit_halo_);
		ac.unit_halo_.reset();
	} else if(has_halo) {
		halo_man.set_location(ac.unit_halo_, x, y - height_adjust);
	}

	// We draw bars only if wanted, visible on the map view
	bool draw_bars = ac.draw_bars_ ;
	if (draw_bars) {
		SDL_Rect unit_rect {xsrc, ysrc +adjusted_params.y, hex_size, hex_size};
		draw_bars = sdl::rects_overlap(unit_rect, disp.map_outside_area());
	}
	surface ellipse_front(nullptr);
	surface ellipse_back(nullptr);
	int ellipse_floating = 0;
	// Always show the ellipse for selected units
	if(draw_bars && (preferences::show_side_colors() || is_selected_hex)) {
		if(adjusted_params.submerge > 0.0) {
			// The division by 2 seems to have no real meaning,
			// It just works fine with the current center of ellipse
			// and prevent a too large adjust if submerge = 1.0
			ellipse_floating = static_cast<int>(adjusted_params.submerge * hex_size_by_2);
		}

		if(ellipse.empty()){
			ellipse="misc/ellipse";
		}

		if(ellipse != "none") {
			// check if the unit has a ZoC or can recruit
			const std::string nozoc    = !emit_zoc      ? "nozoc-"    : "";
			const std::string leader   = can_recruit    ? "leader-"   : "";
			const std::string selected = is_selected_hex? "selected-" : "";
			const std::string tc       = team::get_side_color_id(side);

			const std::string ellipse_top = formatter() << ellipse << "-" << leader << nozoc << selected << "top.png~RC(ellipse_red>" << tc << ")";
			const std::string ellipse_bot = formatter() << ellipse << "-" << leader << nozoc << selected << "bottom.png~RC(ellipse_red>" << tc << ")";

			// Load the ellipse parts recolored to match team color
			ellipse_back = image::get_image(image::locator(ellipse_top), image::SCALED_TO_ZOOM);
			ellipse_front = image::get_image(image::locator(ellipse_bot), image::SCALED_TO_ZOOM);
		}
	}
	if (ellipse_back != nullptr) {
		//disp.drawing_buffer_add(display::LAYER_UNIT_BG, loc,
		disp.drawing_buffer_add(display::LAYER_UNIT_FIRST, loc,
			xsrc, ysrc +adjusted_params.y-ellipse_floating, ellipse_back);
	}

	if (ellipse_front != nullptr) {
		//disp.drawing_buffer_add(display::LAYER_UNIT_FG, loc,
		disp.drawing_buffer_add(display::LAYER_UNIT_FIRST, loc,
			xsrc, ysrc +adjusted_params.y-ellipse_floating, ellipse_front);
	}
	if(draw_bars) {
		const auto& type_cfg = u.type().get_cfg();
		const auto& cfg_offset_x = type_cfg["bar_offset_x"];
		const auto& cfg_offset_y = type_cfg["bar_offset_y"];
		int xoff;
		int yoff;
		if(cfg_offset_x.empty() && cfg_offset_y.empty()) {
			const surface unit_img = image::get_image(u.default_anim_image(), image::SCALED_TO_ZOOM);
			xoff = !unit_img ? 0 : (hex_size - unit_img->w)/2;
			yoff = !unit_img ? 0 : (hex_size - unit_img->h)/2;
		}
		else {
			xoff = cfg_offset_x.to_int();
			yoff = cfg_offset_y.to_int();
		}

		const std::string* energy_file = &game_config::images::energy;

		using namespace orb_status_helper;
		std::unique_ptr<image::locator> orb_img = nullptr;
		if(viewing_team_ref.is_enemy(side)) {
			if(!u.incapacitated())
				orb_img = get_orb_image(orb_status::enemy);
		} else if(static_cast<std::size_t>(side) != playing_team + 1) {
			// We're looking at either the player's own unit or an ally's unit, but either way it
			// doesn't belong to the playing_team and isn't expected to move until after its next
			// turn refresh.
			auto os = orb_status::moved;
			if(static_cast<std::size_t>(side) != viewing_team + 1)
				os = orb_status::allied;
			orb_img = get_orb_image(os);
		} else {
			// We're looking at either the player's own unit, or an ally's unit, during the unit's
			// owner's turn.
			auto os = dc.unit_orb_status(u);
			orb_img = get_orb_image(os);
		}

		if(orb_img != nullptr) {
			surface orb(image::get_image(*orb_img, image::SCALED_TO_ZOOM));
			disp.drawing_buffer_add(display::LAYER_UNIT_BAR, loc, xsrc + xoff, ysrc + yoff + adjusted_params.y, orb);
		}

		double unit_energy = 0.0;
		if(max_hitpoints > 0) {
			unit_energy = static_cast<double>(hitpoints)/static_cast<double>(max_hitpoints);
		}
		const int bar_shift = static_cast<int>(-5*zoom_factor);
		const int hp_bar_height = static_cast<int>(max_hitpoints * u.hp_bar_scaling());

		const int32_t bar_alpha = (loc == mouse_hex || is_selected_hex) ? floating_to_fixed_point(1.0): floating_to_fixed_point(0.8);

		draw_bar(*energy_file, xsrc+xoff+bar_shift, ysrc+yoff+adjusted_params.y,
			loc, hp_bar_height, unit_energy,hp_color, bar_alpha);

		if(experience > 0 && can_advance) {
			const double filled = static_cast<double>(experience) / static_cast<double>(max_experience);
			const int xp_bar_height = static_cast<int>(max_experience * u.xp_bar_scaling() / std::max<int>(u.level(),1));

			draw_bar(*energy_file, xsrc+xoff, ysrc+yoff+adjusted_params.y,
				loc, xp_bar_height, filled, xp_color, bar_alpha);
		}

		if (can_recruit) {
			surface crown(image::get_image(u.leader_crown(),image::SCALED_TO_ZOOM));
			if(crown) {
				disp.drawing_buffer_add(display::LAYER_UNIT_BAR,
					loc, xsrc+xoff, ysrc+yoff+adjusted_params.y, crown);
			}
		}

		for(const std::string& ov : u.overlays()) {
			const surface ov_img(image::get_image(ov, image::SCALED_TO_ZOOM));
			if(ov_img != nullptr) {
				disp.drawing_buffer_add(display::LAYER_UNIT_BAR,
					loc, xsrc+xoff, ysrc+yoff+adjusted_params.y, ov_img);
			}
		}
	}

	// Smooth unit movements from terrain of different elevation.
	// Do this separately from above so that the health bar doesn't go up and down.

	const t_translation::terrain_code terrain_dst = map.get_terrain(dst);
	const terrain_type& terrain_dst_info = map.get_terrain_info(terrain_dst);

	// height_adjust_unit is not scaled by zoom_factor here otherwise it results in a squared offset that results in #5974
	// It appears the tiles and units are scaled together somewhere else
	int height_adjust_unit = static_cast<int>(terrain_info.unit_height_adjust() * (1.0 - adjusted_params.offset) +
											  terrain_dst_info.unit_height_adjust() * adjusted_params.offset);
	if (is_flying && height_adjust_unit < 0) {
		height_adjust_unit = 0;
	}
	params.y -= height_adjust_unit - height_adjust;
	params.halo_y -= height_adjust_unit - height_adjust;

	ac.anim_->redraw(params, halo_man);
	ac.refreshing_ = false;
}

void unit_drawer::draw_bar(const std::string& image, int xpos, int ypos,
		const map_location& loc, std::size_t height, double filled,
		const color_t& col, int32_t alpha) const
{

	filled = std::min<double>(std::max<double>(filled,0.0),1.0);
	height = static_cast<std::size_t>(height*zoom_factor);

	surface surf(image::get_image(image,image::SCALED_TO_HEX));

	// We use UNSCALED because scaling (and bilinear interpolation)
	// is bad for calculate_energy_bar.
	// But we will do a geometric scaling later.
	surface bar_surf(image::get_image(image));
	if(surf == nullptr || bar_surf == nullptr) {
		return;
	}

	// calculate_energy_bar returns incorrect results if the surface colors
	// have changed (for example, due to bilinear interpolation)
	const SDL_Rect& unscaled_bar_loc = calculate_energy_bar(bar_surf);

	SDL_Rect bar_loc;
	if (surf->w == bar_surf->w && surf->h == bar_surf->h)
		bar_loc = unscaled_bar_loc;
	else {
		const int32_t xratio = fixed_point_divide(surf->w,bar_surf->w);
		const int32_t yratio = fixed_point_divide(surf->h,bar_surf->h);
		const SDL_Rect scaled_bar_loc {
			    fixed_point_to_int(unscaled_bar_loc.x * xratio)
			  , fixed_point_to_int(unscaled_bar_loc.y * yratio + 127)
			  , fixed_point_to_int(unscaled_bar_loc.w * xratio + 255)
			  , fixed_point_to_int(unscaled_bar_loc.h * yratio + 255)
		};
		bar_loc = scaled_bar_loc;
	}

	if(height > static_cast<std::size_t>(bar_loc.h)) {
		height = bar_loc.h;
	}

	const std::size_t skip_rows = bar_loc.h - height;

	SDL_Rect top {0, 0, surf->w, bar_loc.y};
	SDL_Rect bot = sdl::create_rect(0, bar_loc.y + skip_rows, surf->w, 0);
	bot.h = surf->w - bot.y;

	disp.drawing_buffer_add(display::LAYER_UNIT_BAR, loc, xpos, ypos, surf, top);
	disp.drawing_buffer_add(display::LAYER_UNIT_BAR, loc, xpos, ypos + top.h, surf, bot);

	std::size_t unfilled = static_cast<std::size_t>(height * (1.0 - filled));

	if(unfilled < height && alpha >= floating_to_fixed_point(0.3)) {
		const uint8_t r_alpha = std::min<unsigned>(fixed_point_multiply(alpha,255),255);
		surface filled_surf(bar_loc.w, height - unfilled);
		SDL_Rect filled_area = sdl::create_rect(0, 0, bar_loc.w, height-unfilled);
		sdl::fill_surface_rect(filled_surf,&filled_area,SDL_MapRGBA(bar_surf->format,col.r,col.g,col.b, r_alpha));
		disp.drawing_buffer_add(display::LAYER_UNIT_BAR, loc, xpos + bar_loc.x, ypos + bar_loc.y + unfilled, filled_surf);
	}
}

struct is_energy_color {
	bool operator()(uint32_t color) const { return (color&0xFF000000) > 0x10000000 &&
	                                              (color&0x00FF0000) < 0x00100000 &&
												  (color&0x0000FF00) < 0x00001000 &&
												  (color&0x000000FF) < 0x00000010; }
};

const SDL_Rect& unit_drawer::calculate_energy_bar(surface surf) const
{
	const std::map<surface,SDL_Rect>::const_iterator i = energy_bar_rects.find(surf);
	if(i != energy_bar_rects.end()) {
		return i->second;
	}

	int first_row = -1, last_row = -1, first_col = -1, last_col = -1;

	const_surface_lock image_lock(surf);
	const uint32_t* const begin = image_lock.pixels();

	for(int y = 0; y != surf->h; ++y) {
		const uint32_t* const i1 = begin + surf->w*y;
		const uint32_t* const i2 = i1 + surf->w;
		const uint32_t* const itor = std::find_if(i1,i2,is_energy_color());
		const int count = std::count_if(itor,i2,is_energy_color());

		if(itor != i2) {
			if(first_row == -1) {
				first_row = y;
			}

			first_col = itor - i1;
			last_col = first_col + count;
			last_row = y;
		}
	}

	const SDL_Rect res {
			  first_col
			, first_row
			, last_col-first_col
			, last_row+1-first_row
	};
	energy_bar_rects.emplace(surf, res);
	return calculate_energy_bar(surf);
}
