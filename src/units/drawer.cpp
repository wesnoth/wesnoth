/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "halo.hpp"
#include "map/location.hpp"
#include "map/map.hpp"
#include "preferences/game.hpp"
#include "sdl/rect.hpp"
#include "sdl/texture.hpp"
#include "team.hpp"
#include "units/animation.hpp"
#include "units/animation_component.hpp"
#include "units/frame.hpp"
#include "units/unit.hpp"

unit_drawer::unit_drawer(display& thedisp)
	: disp(thedisp)
	, dc(disp.get_disp_context())
	, map(dc.map())
	, teams(dc.teams())
	, halo_man(thedisp.get_halo_manager())
	, viewing_team(disp.viewing_team())
	, playing_team(disp.playing_team())
	, viewing_team_ref(teams[viewing_team])
	, playing_team_ref(teams[playing_team])
	, is_blindfolded(disp.is_blindfolded())
	, show_everything(disp.show_everything())
	, sel_hex(disp.selected_hex())
	, mouse_hex(disp.mouseover_hex())
	, zoom_factor(disp.get_zoom_factor())
	, hex_size(disp.hex_size())
	, hex_size_by_2(disp.hex_size() / 2)
{
}

// NOTE: surfaces were scaled to zoom
void unit_drawer::redraw_unit(const unit & u) const
{
	unit_animation_component & ac = u.anim_comp();
	map_location loc = u.get_location();

	int side = u.side();

	bool hidden = u.get_hidden();
	bool is_flying = u.is_flying();

	map_location::DIRECTION facing = u.facing();

	int hitpoints = u.hitpoints();
	int max_hitpoints = u.max_hitpoints();
	int movement_left = u.movement_left();
	int total_movement = u.total_movement();

	bool can_recruit = u.can_recruit();
	bool can_advance = u.can_advance();

	int experience = u.experience();
	int max_experience = u.max_experience();

	bool emit_zoc = u.emits_zoc();

	color_t hp_color = u.hp_color();
	color_t xp_color = u.xp_color();

	std::string ellipse=u.image_ellipse();

	if(hidden || is_blindfolded || !u.is_visible_to_team(viewing_team_ref, dc, show_everything)) {
		ac.clear_haloes();
		if(ac.anim_) {
			ac.anim_->update_last_draw_time();
		}

		return;
	}

	if(!ac.anim_) {
		ac.set_standing();
		if(!ac.anim_) {
			return;
		}
	}

	if(ac.refreshing_) {
		return;
	}

	ac.refreshing_ = true;

	ac.anim_->update_last_draw_time();

	const t_translation::terrain_code terrain = map.get_terrain(loc);
	const terrain_type& terrain_info = map.get_terrain_info(terrain);

	frame_parameters params;

	// Do not set to 0 so we can distinguish the flying from the "not on submerge terrain"
	// instead use -1.0 (as in "negative depth", it will be ignored by rendering)
	params.submerge= is_flying ? -1.0 : terrain_info.unit_submerge();

	if(u.invisible(loc, dc) && params.highlight_ratio > 0.6) {
		params.highlight_ratio = 0.6;
	}

	if(loc == sel_hex && params.highlight_ratio == 1.0) {
		params.highlight_ratio = 1.5;
	}

	int height_adjust = static_cast<int>(terrain_info.unit_height_adjust() * zoom_factor);
	if(is_flying && height_adjust < 0) {
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
		params.blend_with = color_t((red / tints), (green / tints), (blue / tints));
		params.blend_ratio = blend_ratio / tints;
	}

	// Hackish: see unit_frame::merge_parameters.
	// We use image_mod on the primary image and halo_mod on secondary images and all haloes
	params.image_mod = u.image_mods();
	params.halo_mod = u.TC_image_mods();
	params.image = u.default_anim_image();

	if(u.incapacitated()) {
		params.image_mod +="~GS()";
	}

	params.primary_frame = true;

	const frame_parameters adjusted_params = ac.anim_->get_current_params(params);

	const map_location dst = loc.get_direction(facing);
	const int xsrc = disp.get_location_x(loc);
	const int ysrc = disp.get_location_y(loc);
	const int xdst = disp.get_location_x(dst);
	const int ydst = disp.get_location_y(dst);

	const int x = static_cast<int>(adjusted_params.offset * xdst + (1.0 - adjusted_params.offset) * xsrc) + hex_size_by_2;
	const int y = static_cast<int>(adjusted_params.offset * ydst + (1.0 - adjusted_params.offset) * ysrc) + hex_size_by_2;

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
	if(draw_bars) {
		SDL_Rect unit_rect {xsrc, ysrc + adjusted_params.y, hex_size, hex_size};
		draw_bars = sdl::rects_overlap(unit_rect, disp.map_outside_area());
	}

	texture ellipse_front(nullptr);
	texture ellipse_back(nullptr);

	int ellipse_floating = 0;

	// Always show the ellipse for selected units.
	if(draw_bars && (preferences::show_side_colors() || sel_hex == loc)) {
		if(adjusted_params.submerge > 0.0) {
			// The division by 2 seems to have no real meaning. It just works fine with the current center
			// of the ellipse and prevents an overadjustment if submerge == 1.0.
			ellipse_floating = static_cast<int>(adjusted_params.submerge * hex_size_by_2);
		}

		if(ellipse.empty()) {
			ellipse = "misc/ellipse";
		}

		if(ellipse != "none") {
			// check if the unit has a ZoC or can recruit
			const std::string nozoc    = !emit_zoc      ? "nozoc-"    : "";
			const std::string leader   = can_recruit    ? "leader-"   : "";
			const std::string selected = sel_hex == loc ? "selected-" : "";
			const std::string tc       = team::get_side_color_index(side);

			const std::string ellipse_top = formatter() <<
				ellipse << "-" << leader << nozoc << selected << "top.png~RC(ellipse_red>" << tc << ")";

			const std::string ellipse_bot = formatter() <<
				ellipse << "-" << leader << nozoc << selected << "bottom.png~RC(ellipse_red>" << tc << ")";

			// Load the ellipse parts recolored to match team color
			ellipse_back = image::get_texture(ellipse_top /*, image::SCALED_TO_ZOOM)*/);
			ellipse_front = image::get_texture(ellipse_bot /*, image::SCALED_TO_ZOOM)*/);
		}
	}

	if(ellipse_back != nullptr) {
		//disp.drawing_buffer_add(drawing_buffer::LAYER_UNIT_BG, loc,
		//disp.drawing_buffer_add(drawing_buffer::LAYER_UNIT_FIRST, loc,
		//	xsrc, ysrc +adjusted_params.y-ellipse_floating, ellipse_back);

		disp.render_scaled_to_zoom(ellipse_back, xsrc, ysrc + adjusted_params.y - ellipse_floating);
	}

	if(ellipse_front != nullptr) {
		//disp.drawing_buffer_add(drawing_buffer::LAYER_UNIT_FG, loc,
		//disp.drawing_buffer_add(drawing_buffer::LAYER_UNIT_FIRST, loc,
		//	xsrc, ysrc +adjusted_params.y-ellipse_floating, ellipse_front);

		disp.render_scaled_to_zoom(ellipse_front, xsrc, ysrc + adjusted_params.y - ellipse_floating);
	}

	// Smooth unit movements from terrain of different elevation.
	// TODO: see if having this here actually makes the healthbars bob up and and down.

	const t_translation::terrain_code terrain_dst = map.get_terrain(dst);
	const terrain_type& terrain_dst_info = map.get_terrain_info(terrain_dst);

	int height_adjust_unit = static_cast<int>((
		terrain_info.unit_height_adjust()     * (1.0 - adjusted_params.offset) +
		terrain_dst_info.unit_height_adjust() *        adjusted_params.offset
	) * zoom_factor);

	if(is_flying && height_adjust_unit < 0) {
		height_adjust_unit = 0;
	}

	params.y -= height_adjust_unit - height_adjust;
	params.halo_y -= height_adjust_unit - height_adjust;

	ac.anim_->redraw(params, halo_man);

	if(draw_bars) {
		const texture unit_img = image::get_texture(u.default_anim_image());

		const texture::info unit_img_info = unit_img.get_info();

		const int xoff = unit_img.null() ? hex_size_by_2 : (hex_size - unit_img_info.w) / 2;
		const int yoff = unit_img.null() ? hex_size_by_2 : (hex_size - unit_img_info.h) / 2;

		const image::locator partmoved_orb(
			game_config::images::orb + "~RC(magenta>" + preferences::partial_color() + ")");
		const image::locator moved_orb(
			game_config::images::orb + "~RC(magenta>" + preferences::moved_color()   + ")");
		const image::locator ally_orb(
			game_config::images::orb + "~RC(magenta>" + preferences::allied_color()  + ")");
		const image::locator enemy_orb(
			game_config::images::orb + "~RC(magenta>" + preferences::enemy_color()   + ")");
		const image::locator unmoved_orb(
			game_config::images::orb + "~RC(magenta>" + preferences::unmoved_color() + ")");

		const image::locator* orb_img = nullptr;

		if(static_cast<size_t>(side) != viewing_team + 1) {
			if(disp.team_valid() && viewing_team_ref.is_enemy(side)) {
				if(preferences::show_enemy_orb() && !u.incapacitated()) {
					orb_img = &enemy_orb;
				}
			} else {
				if(preferences::show_allied_orb()) {
					orb_img = &ally_orb;
				}
			}
		} else {
			if(preferences::show_moved_orb()) {
				orb_img = &moved_orb;
			}

			if(playing_team == viewing_team && !u.user_end_turn()) {
				if(movement_left == total_movement) {
					if(preferences::show_unmoved_orb()) {
						orb_img = &unmoved_orb;
					}
				} else if(dc.unit_can_move(u)) {
					if(preferences::show_partial_orb()) {
						orb_img = &partmoved_orb;
					}
				}
			}
		}

		if(orb_img != nullptr) {
			texture orb(image::get_texture(*orb_img));
			//disp.drawing_buffer_add(drawing_buffer::LAYER_UNIT_BAR,
			//	loc, xsrc + xoff, ysrc + yoff + adjusted_params.y, orb);
			disp.render_scaled_to_zoom(orb, xsrc + xoff, ysrc + yoff + adjusted_params.y);
		}

		double unit_energy = 0.0;
		if(max_hitpoints > 0) {
			unit_energy = double(hitpoints)/double(max_hitpoints);
		}

		const int bar_shift = static_cast<int>(-5*zoom_factor);
		const int hp_bar_height = static_cast<int>(max_hitpoints * u.hp_bar_scaling());

		// TODO: stop using fixed-point stuff.
		const fixed_t bar_alpha = (loc == mouse_hex || loc == sel_hex) ? ftofxp(1.0): ftofxp(0.8);

		// HP bar
		draw_bar(
			xsrc + xoff + bar_shift,
			ysrc + yoff + adjusted_params.y,
			hp_bar_height, unit_energy, hp_color, bar_alpha
		);

		if(experience > 0 && can_advance) {
			const double filled = double(experience) / double(max_experience);
			const int xp_bar_height = static_cast<int>(max_experience * u.xp_bar_scaling() / std::max<int>(u.level(),1));

			// XP bar
			draw_bar(
				xsrc + xoff,
				ysrc + yoff + adjusted_params.y,
				xp_bar_height, filled, xp_color, bar_alpha
			);
		}

		if(can_recruit) {
			texture crown(image::get_texture(u.leader_crown()));
			if(crown != nullptr) {
				//if(bar_alpha != ftofxp(1.0)) {
				//	crown = adjust_surface_alpha(crown, bar_alpha);
				//}
				//disp.drawing_buffer_add(drawing_buffer::LAYER_UNIT_BAR,
				//	loc, xsrc+xoff, ysrc+yoff+adjusted_params.y, crown);
				disp.render_scaled_to_zoom(crown, xsrc + xoff, ysrc + yoff + adjusted_params.y);
			}
		}

		for(const auto& ov : u.overlays()) {
			const texture ov_img(image::get_texture(ov));
			if(ov_img != nullptr) {
				//disp.drawing_buffer_add(drawing_buffer::LAYER_UNIT_BAR,
				//	loc, xsrc+xoff, ysrc+yoff+adjusted_params.y, ov_img);
				disp.render_scaled_to_zoom(ov_img, xsrc + xoff, ysrc + yoff + adjusted_params.y);
			}
		}
	}

	ac.refreshing_ = false;
}

void unit_drawer::draw_bar(int xpos, int ypos, size_t height, double filled, color_t col, fixed_t alpha) const
{
	filled = utils::clamp<double>(filled, 0.0, 1.0);
	height = static_cast<size_t>(height * zoom_factor);

	const size_t unfilled = static_cast<size_t>(height * (1.0 - filled));

	// Magic width number
	static unsigned int bar_width = 4;

	static const color_t bar_color_bg {0, 0, 0 , 80};
	static const color_t bar_color_border {213, 213, 213, 200};

	// Override the filled area's color's alpha.
	col.a = std::min<unsigned>(alpha, 255);

	// Bar dimensions.
	// Note about the magic x, y additions: we used to use an image for the bar instead of drawing
	// it procedurally. Its x,y position within the file was 19,13, so we offset the origin by that
	// much to make it line up with the crowns as before. Should probably compensate for this better
	// in the future.
	SDL_Rect bar_rect;
	bar_rect.x = xpos + 19;
	bar_rect.y = ypos + 13;
	bar_rect.w = bar_width;
	bar_rect.h = height;

	// Filled area dimensions.
	SDL_Rect fill_rect;
	fill_rect.x = bar_rect.x;
	fill_rect.y = bar_rect.y + unfilled;
	fill_rect.w = bar_rect.w;
	fill_rect.h = bar_rect.h - unfilled;

	// Tinted background.
	sdl::fill_rectangle(bar_rect, bar_color_bg);

	// Filled area.
	sdl::fill_rectangle(fill_rect, col);

	// Bar outline.
	sdl::draw_rectangle(bar_rect, bar_color_border);
}
