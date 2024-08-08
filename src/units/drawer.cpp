/*
	Copyright (C) 2014 - 2024
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
#include "draw.hpp"
#include "formatter.hpp"
#include "game_display.hpp"
#include "halo.hpp"
#include "log.hpp"
#include "map/location.hpp"
#include "map/map.hpp"
#include "picture.hpp"
#include "preferences/preferences.hpp"
#include "team.hpp"
#include "units/animation.hpp"
#include "units/animation_component.hpp"
#include "units/frame.hpp"
#include "units/types.hpp"
#include "units/unit.hpp"

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)

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

/**
 * Assemble a two-color orb for an ally's unit, during that ally's turn.
 * Returns nullptr if the preferences both of these orbs and ally orbs in general are off.
 * Returns a one-color orb (using the ally color) in several circumstances.
 */
std::unique_ptr<image::locator> get_playing_ally_orb_image(orb_status os)
{
	if(!prefs::get().show_status_on_ally_orb())
		return get_orb_image(orb_status::allied);

	// This is conditional on prefs_show_orb, because a user might want to disable the standard
	// partial orb, but keep it enabled as a reminder for units in the disengaged state.
	if(os == orb_status::disengaged && !orb_status_helper::prefs_show_orb(orb_status::disengaged)) {
		os = orb_status::partial;
	}

	if(!orb_status_helper::prefs_show_orb(os))
		return get_orb_image(orb_status::allied);

	auto allied_color = orb_status_helper::get_orb_color(orb_status::allied);
	auto status_color = orb_status_helper::get_orb_color(os);
	return std::make_unique<image::locator>(game_config::images::orb_two_color + "~RC(ellipse_red>"
			+ allied_color + ")~RC(magenta>" + status_color + ")");
}

void draw_bar(int xpos, int ypos, int bar_height, double filled, const color_t& col)
{
	// Magic width number
	static constexpr unsigned int bar_width = 4;

	static constexpr color_t bar_color_bg{0, 0, 0, 80};
	static constexpr color_t bar_color_border{213, 213, 213, 200};

	// We used to use an image for the bar instead of drawing it procedurally. Its x,y position
	// within the file was 19,13, so we offset the origin by that much to make it line up with
	// the crowns as before. Should probably compensate for this better in the future.
	const point offset = display::scaled_to_zoom(point{19, 13});

	// Full bar dimensions.
	rect bar_rect = display::scaled_to_zoom({
		xpos + offset.x,
		ypos + offset.y,
		bar_width,
		bar_height
	});

	// Bar dimensions should not overflow 80% of the scaled hex dimensions.
	// The 80% comes from an approximation of the length of a segment drawn
	// inside a regular hexagon that runs parallel to its outer left side.
	bar_rect.w = std::clamp<int>(bar_rect.w, 0, display::hex_size() * 0.80 - offset.x);
	bar_rect.h = std::clamp<int>(bar_rect.h, 0, display::hex_size() * 0.80 - offset.y);

	filled = std::clamp<double>(filled, 0.0, 1.0);
	const int unfilled = static_cast<std::size_t>(bar_rect.h * (1.0 - filled));

	// Filled area dimensions.
	const rect fill_rect {
		bar_rect.x,
		bar_rect.y + unfilled,
		bar_rect.w,
		bar_rect.h - unfilled
	};

	// Tinted background.
	draw::fill(bar_rect, bar_color_bg);

	// Filled area.
	draw::fill(fill_rect, col);

	// Bar outline.
	draw::rect(bar_rect, bar_color_border);
}
}

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
	if(const game_display* game_display = dynamic_cast<class game_display*>(&disp)) {
		units_that_can_reach_goal = game_display->units_that_can_reach_goal();
	}

	// This used to be checked in the drawing code, where it simply triggered skipping some logic.
	// However, I think it's obsolete, and that the initialization of viewing_team_ref would already
	// be undefined behavior in the situation where this assert fails.
	assert(disp.team_valid());
}

void unit_drawer::redraw_unit(const unit& u) const
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

	// Override the filled area's color's alpha.
	hp_color.a = (loc == mouse_hex || is_selected_hex) ? 255u : float_to_color(0.8);
	xp_color.a = hp_color.a;

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

	// We draw bars only if wanted, visible on the map view
	bool draw_bars = ac.draw_bars_ ;
	if (draw_bars) {
		rect unit_rect {xsrc, ysrc +adjusted_params.y, hex_size, hex_size};
		draw_bars = unit_rect.overlaps(disp.map_outside_area());
	}

	texture ellipse_front;
	texture ellipse_back;
	int ellipse_floating = 0;
	// Always show the ellipse for selected units
	if(draw_bars && (prefs::get().show_side_colors() || is_selected_hex)) {
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
			ellipse_back = image::get_texture(image::locator(ellipse_top));
			ellipse_front = image::get_texture(image::locator(ellipse_bot));
		}
	}

	disp.drawing_buffer_add(drawing_layer::unit_first, loc, [=, adj_y = adjusted_params.y](const rect& d) {
		// Both front and back have the same origin
		const point origin { d.x, d.y + adj_y - ellipse_floating };

		if(ellipse_back) {
			draw::blit(ellipse_back, display::scaled_to_zoom({origin.x, origin.y, ellipse_back.w(), ellipse_back.h()}));
		}

		if(ellipse_front) {
			draw::blit(ellipse_front, display::scaled_to_zoom({origin.x, origin.y, ellipse_front.w(), ellipse_front.h()}));
		}
	});

	if(draw_bars) {
		const auto& type_cfg = u.type().get_cfg();
		const auto& cfg_offset_x = type_cfg["bar_offset_x"];
		const auto& cfg_offset_y = type_cfg["bar_offset_y"];
		int xoff;
		int yoff;
		if(cfg_offset_x.empty() && cfg_offset_y.empty()) {
			const point s = display::scaled_to_zoom(
				image::get_size(u.default_anim_image())
			);
			xoff = !s.x ? 0 : (hex_size - s.x)/2;
			yoff = !s.y ? 0 : (hex_size - s.x)/2;
		}
		else {
			xoff = cfg_offset_x.to_int();
			yoff = cfg_offset_y.to_int();
		}

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
		} else if(static_cast<std::size_t>(side) != viewing_team + 1) {
			// We're looking at an ally's unit, during that ally's turn.
			auto os = dc.unit_orb_status(u);
			orb_img = get_playing_ally_orb_image(os);
		} else {
			// We're looking at the player's own unit, during the player's turn.
			auto os = dc.unit_orb_status(u);
			orb_img = get_orb_image(os);
		}

		// All the various overlay textures to draw with the HP/XP bars
		std::vector<texture> textures;

		if(orb_img != nullptr) {
			textures.push_back(image::get_texture(*orb_img));
		}

		if(can_recruit) {
			if(texture tex = image::get_texture(u.leader_crown())) {
				textures.push_back(std::move(tex));
			}
		}

		for(const std::string& ov : u.overlays()) {
			if(texture tex = image::get_texture(ov)) {
				textures.push_back(std::move(tex));
			}
		};

		const std::vector<std::string> overlays_abilities = u.overlays_abilities();
		for(const std::string& ov : overlays_abilities) {
			if(texture tex = image::get_texture(ov)) {
				textures.push_back(std::move(tex));
			}
		};

		disp.drawing_buffer_add(drawing_layer::unit_bar, loc, [=,
			textures      = std::move(textures),
			adj_y         = adjusted_params.y,
			//origin        = point{xsrc + xoff, ysrc + yoff + adjusted_params.y},
			bar_hp_height = static_cast<int>(max_hitpoints  * u.hp_bar_scaling()),
			bar_xp_height = static_cast<int>(max_experience * u.xp_bar_scaling() / std::max<int>(u.level(), 1))
		](const rect& d) {
			const point origin { d.x + xoff, d.y + yoff + adj_y };

			for(const texture& tex : textures) {
				draw::blit(tex, display::scaled_to_zoom({origin.x, origin.y, tex.w(), tex.h()}));
			}

			if(max_hitpoints > 0) {
				// Offset slightly to make room for the XP bar
				const int hp_offset = static_cast<int>(-5 * display::get_zoom_factor());

				double filled = static_cast<double>(hitpoints) / static_cast<double>(max_hitpoints);
				draw_bar(origin.x + hp_offset, origin.y, bar_hp_height, filled, hp_color);
			}

			if(experience > 0 && can_advance) {
				double filled = static_cast<double>(experience) / static_cast<double>(max_experience);
				draw_bar(origin.x, origin.y, bar_xp_height, filled, xp_color);
			}
		});
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
	// TODO: params.halo_y is not used. Why is it set?

	const int halo_x =
		static_cast<int>(
			adjusted_params.offset * xdst
			+ (1.0 - adjusted_params.offset) * xsrc
		)
		+ hex_size_by_2;
	const int halo_y =
		static_cast<int>(
			adjusted_params.offset * ydst
			+ (1.0 - adjusted_params.offset) * ysrc
		)
		+ hex_size_by_2 - height_adjust_unit * zoom_factor;

	bool has_halo = ac.unit_halo_ && ac.unit_halo_->valid();
	if(!has_halo && !u.image_halo().empty()) {
		ac.unit_halo_ = halo_man.add(
			halo_x, halo_y,
			u.image_halo() + u.TC_image_mods(),
			map_location(-1, -1)
		);
	}
	if(has_halo && u.image_halo().empty()) {
		halo_man.remove(ac.unit_halo_);
		ac.unit_halo_.reset();
	} else if(has_halo) {
		halo_man.set_location(ac.unit_halo_, halo_x, halo_y);
	}

	const std::vector<std::string> halos_abilities = u.halo_abilities();
	bool has_abil_halo = !ac.abil_halos_.empty() && ac.abil_halos_.front()->valid();
	if(!has_abil_halo && !halos_abilities.empty()) {
		for(const std::string& halo_ab : halos_abilities){
			halo::handle abil_halo = halo_man.add(
				halo_x, halo_y,
				halo_ab + u.TC_image_mods(),
				map_location(-1, -1)
			);
			if(abil_halo->valid()){
				ac.abil_halos_.push_back(abil_halo);
			}
		}
	}
	if(has_abil_halo && (ac.abil_halos_ref_ != halos_abilities || halos_abilities.empty())){
		for(halo::handle& abil_halo : ac.abil_halos_){
			halo_man.remove(abil_halo);
		}
		ac.abil_halos_.clear();
		if(!halos_abilities.empty()){
			for(const std::string& halo_ab : halos_abilities){
				halo::handle abil_halo = halo_man.add(
					halo_x, halo_y,
					halo_ab + u.TC_image_mods(),
					map_location(-1, -1)
				);
				if(abil_halo->valid()){
					ac.abil_halos_.push_back(abil_halo);
				}
			}
		}
	} else if(has_abil_halo){
		for(halo::handle& abil_halo : ac.abil_halos_){
			halo_man.set_location(abil_halo, halo_x, halo_y);
		}
	}

	ac.abil_halos_ref_ = halos_abilities;

	ac.anim_->redraw(params, halo_man);
	ac.refreshing_ = false;
}
