/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "drawable_unit.hpp"

#include "display.hpp"
#include "display_context.hpp"
#include "game_preferences.hpp"
#include "halo.hpp"
#include "map.hpp"
#include "team.hpp"
#include "unit_animation.hpp"
#include "unit_frame.hpp"

#include <boost/foreach.hpp>

void drawable_unit::redraw_unit (display & disp) const
{
	const display_context & dc = disp.get_disp_context();
	const gamemap &map = dc.map();
	const std::vector<team> &teams = dc.teams();

	const team & viewing_team = teams[disp.viewing_team()];

	if ( hidden_ || disp.is_blindfolded() || !is_visible_to_team(viewing_team,map, disp.show_everything()) )
	{
		clear_haloes();
		if(anim_) {
			anim_->update_last_draw_time();
		}
		return;
	}

	if (!anim_) {
		set_standing();
		if (!anim_) return;
	}

	if (refreshing_) return;
	refreshing_ = true;

	anim_->update_last_draw_time();
	frame_parameters params;
	const t_translation::t_terrain terrain = map.get_terrain(loc_);
	const terrain_type& terrain_info = map.get_terrain_info(terrain);

	// do not set to 0 so we can distinguish the flying from the "not on submerge terrain"
	// instead use -1.0 (as in "negative depth", it will be ignored by rendering)
	params.submerge= is_flying() ? -1.0 : terrain_info.unit_submerge();

	if (invisible(loc_) &&
			params.highlight_ratio > 0.5) {
		params.highlight_ratio = 0.5;
	}
	if (loc_ == disp.selected_hex() && params.highlight_ratio == 1.0) {
		params.highlight_ratio = 1.5;
	}

	int height_adjust = static_cast<int>(terrain_info.unit_height_adjust() * disp.get_zoom_factor());
	if (is_flying() && height_adjust < 0) {
		height_adjust = 0;
	}
	params.y -= height_adjust;
	params.halo_y -= height_adjust;

	int red = 0,green = 0,blue = 0,tints = 0;
	double blend_ratio = 0;
	// Add future colored states here
	if(get_state(STATE_POISONED)) {
		green += 255;
		blend_ratio += 0.25;
		tints += 1;
	}
	if(get_state(STATE_SLOWED)) {
		red += 191;
		green += 191;
		blue += 255;
		blend_ratio += 0.25;
		tints += 1;
	}
	if(tints > 0) {
		params.blend_with = disp.rgb((red/tints),(green/tints),(blue/tints));
		params.blend_ratio = ((blend_ratio/tints));
	}

	//hackish : see unit_frame::merge_parameters
	// we use image_mod on the primary image
	// and halo_mod on secondary images and all haloes
	params.image_mod = image_mods();
	params.halo_mod = TC_image_mods();
	params.image= absolute_image();


	if(get_state(STATE_PETRIFIED)) params.image_mod +="~GS()";
	params.primary_frame = t_true;


	const frame_parameters adjusted_params = anim_->get_current_params(params);

	const map_location dst = loc_.get_direction(facing_);
	const int xsrc = disp.get_location_x(loc_);
	const int ysrc = disp.get_location_y(loc_);
	const int xdst = disp.get_location_x(dst);
	const int ydst = disp.get_location_y(dst);
	int d2 = disp.hex_size() / 2;

	const int x = static_cast<int>(adjusted_params.offset * xdst + (1.0-adjusted_params.offset) * xsrc) + d2;
	const int y = static_cast<int>(adjusted_params.offset * ydst + (1.0-adjusted_params.offset) * ysrc) + d2;

	if(unit_halo_ == halo::NO_HALO && !image_halo().empty()) {
		unit_halo_ = halo::add(0, 0, image_halo()+TC_image_mods(), map_location(-1, -1));
	}
	if(unit_halo_ != halo::NO_HALO && image_halo().empty()) {
		halo::remove(unit_halo_);
		unit_halo_ = halo::NO_HALO;
	} else if(unit_halo_ != halo::NO_HALO) {
		halo::set_location(unit_halo_, x, y - height_adjust);
	}



	// We draw bars only if wanted, visible on the map view
	bool draw_bars = draw_bars_ ;
	if (draw_bars) {
		const int d = disp.hex_size();
		SDL_Rect unit_rect = sdl::create_rect(xsrc, ysrc +adjusted_params.y, d, d);
		draw_bars = sdl::rects_overlap(unit_rect, disp.map_outside_area());
	}

	surface ellipse_front(NULL);
	surface ellipse_back(NULL);
	int ellipse_floating = 0;
	// Always show the ellipse for selected units
	if(draw_bars && (preferences::show_side_colors() || disp.selected_hex() == loc_)) {
		if(adjusted_params.submerge > 0.0) {
			// The division by 2 seems to have no real meaning,
			// It just works fine with the current center of ellipse
			// and prevent a too large adjust if submerge = 1.0
			ellipse_floating = static_cast<int>(adjusted_params.submerge * disp.hex_size() / 2);
		}

		std::string ellipse=image_ellipse();
		if(ellipse.empty()){
			ellipse="misc/ellipse";
		}

		if(ellipse != "none") {
			// check if the unit has a ZoC or can recruit
			const char* const nozoc = emit_zoc_ ? "" : "nozoc-";
			const char* const leader = can_recruit() ? "leader-" : "";
			const char* const selected = disp.selected_hex() == loc_ ? "selected-" : "";

			// Load the ellipse parts recolored to match team color
			char buf[100];
			std::string tc=team::get_side_color_index(side_);

			snprintf(buf,sizeof(buf),"%s-%s%s%stop.png~RC(ellipse_red>%s)",ellipse.c_str(),leader,nozoc,selected,tc.c_str());
			ellipse_back.assign(image::get_image(image::locator(buf), image::SCALED_TO_ZOOM));
			snprintf(buf,sizeof(buf),"%s-%s%s%sbottom.png~RC(ellipse_red>%s)",ellipse.c_str(),leader,nozoc,selected,tc.c_str());
			ellipse_front.assign(image::get_image(image::locator(buf), image::SCALED_TO_ZOOM));
		}
	}

	if (ellipse_back != NULL) {
		//disp.drawing_buffer_add(display::LAYER_UNIT_BG, loc,
		disp.drawing_buffer_add(display::LAYER_UNIT_FIRST, loc_,
			xsrc, ysrc +adjusted_params.y-ellipse_floating, ellipse_back);
	}

	if (ellipse_front != NULL) {
		//disp.drawing_buffer_add(display::LAYER_UNIT_FG, loc,
		disp.drawing_buffer_add(display::LAYER_UNIT_FIRST, loc_,
			xsrc, ysrc +adjusted_params.y-ellipse_floating, ellipse_front);
	}
	if(draw_bars) {
		const image::locator* orb_img = NULL;
		/*static*/ const image::locator partmoved_orb(game_config::images::orb + "~RC(magenta>" +
						preferences::partial_color() + ")"  );
		/*static*/ const image::locator moved_orb(game_config::images::orb + "~RC(magenta>" +
						preferences::moved_color() + ")"  );
		/*static*/ const image::locator ally_orb(game_config::images::orb + "~RC(magenta>" +
						preferences::allied_color() + ")"  );
		/*static*/ const image::locator enemy_orb(game_config::images::orb + "~RC(magenta>" +
						preferences::enemy_color() + ")"  );
		/*static*/ const image::locator unmoved_orb(game_config::images::orb + "~RC(magenta>" +
						preferences::unmoved_color() + ")"  );

		const std::string* energy_file = &game_config::images::energy;

		if(size_t(side()) != disp.viewing_team()+1) {
			if(disp.team_valid() &&
			   viewing_team.is_enemy(side())) {
				if (preferences::show_enemy_orb() && !get_state(STATE_PETRIFIED))
					orb_img = &enemy_orb;
				else
					orb_img = NULL;
			} else {
				if (preferences::show_allied_orb())
					orb_img = &ally_orb;
				else orb_img = NULL;
			}
		} else {
			if (preferences::show_moved_orb())
				orb_img = &moved_orb;
			else orb_img = NULL;

			if(disp.playing_team() == disp.viewing_team() && !user_end_turn()) {
				if (movement_left() == total_movement()) {
					if (preferences::show_unmoved_orb())
						orb_img = &unmoved_orb;
					else orb_img = NULL;
				} else if ( dc.unit_can_move(*this) ) {
					if (preferences::show_partial_orb())
						orb_img = &partmoved_orb;
					else orb_img = NULL;
				}
			}
		}

		if (orb_img != NULL) {
			surface orb(image::get_image(*orb_img,image::SCALED_TO_ZOOM));
			disp.drawing_buffer_add(display::LAYER_UNIT_BAR,
				loc_, xsrc, ysrc +adjusted_params.y, orb);
		}

		double unit_energy = 0.0;
		if(max_hitpoints() > 0) {
			unit_energy = double(hitpoints())/double(max_hitpoints());
		}
		const int bar_shift = static_cast<int>(-5*disp.get_zoom_factor());
		const int hp_bar_height = static_cast<int>(max_hitpoints() * hp_bar_scaling_);

		const fixed_t bar_alpha = (loc_ == disp.mouseover_hex() || loc_ == disp.selected_hex()) ? ftofxp(1.0): ftofxp(0.8);

		disp.draw_bar(*energy_file, xsrc+bar_shift, ysrc +adjusted_params.y,
			loc_, hp_bar_height, unit_energy,hp_color(), bar_alpha);

		if(experience() > 0 && can_advance()) {
			const double filled = double(experience())/double(max_experience());

			const int xp_bar_height = static_cast<int>(max_experience() * xp_bar_scaling_ / std::max<int>(level_,1));

			SDL_Color color=xp_color();
			disp.draw_bar(*energy_file, xsrc, ysrc +adjusted_params.y,
				loc_, xp_bar_height, filled, color, bar_alpha);
		}

		if (can_recruit()) {
			surface crown(image::get_image(leader_crown(),image::SCALED_TO_ZOOM));
			if(!crown.null()) {
				//if(bar_alpha != ftofxp(1.0)) {
				//	crown = adjust_surface_alpha(crown, bar_alpha);
				//}
				disp.drawing_buffer_add(display::LAYER_UNIT_BAR,
					loc_, xsrc, ysrc +adjusted_params.y, crown);
			}
		}

		for(std::vector<std::string>::const_iterator ov = overlays().begin(); ov != overlays().end(); ++ov) {
			const surface ov_img(image::get_image(*ov, image::SCALED_TO_ZOOM));
			if(ov_img != NULL) {
				disp.drawing_buffer_add(display::LAYER_UNIT_BAR,
					loc_, xsrc, ysrc +adjusted_params.y, ov_img);
			}
		}
	}

	// Smooth unit movements from terrain of different elevation.
	// Do this separately from above so that the health bar doesn't go up and down.

	const t_translation::t_terrain terrain_dst = map.get_terrain(dst);
	const terrain_type& terrain_dst_info = map.get_terrain_info(terrain_dst);

	int height_adjust_unit = static_cast<int>((terrain_info.unit_height_adjust() * (1.0 - adjusted_params.offset) +
											  terrain_dst_info.unit_height_adjust() * adjusted_params.offset) *
											  disp.get_zoom_factor());
	if (is_flying() && height_adjust_unit < 0) {
		height_adjust_unit = 0;
	}
	params.y -= height_adjust_unit - height_adjust;
	params.halo_y -= height_adjust_unit - height_adjust;

	anim_->redraw(params);
	refreshing_ = false;
}

