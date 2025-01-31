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
#include "global.hpp"
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

struct energy_bar
{
	/** Bar height in pixels. */
	int bar_h{};

	/** The extent to which the bar is filled, ranging [0, 1]. */
	float filled{};

	/** Color for the bar's filled area. */
	color_t fill_color{};

	/** Initial coordinates for the first bar drawn. */
	static constexpr point def_origin{14, 13};

	/** Default width for all bars. */
	static constexpr int def_w = 4;

	/** Distance between the top left corner of subsequent bars. */
	static constexpr int spacing = def_w + 1;

	/** Background color for the unfilled bar area. */
	static constexpr color_t background_color{0, 0, 0, 80};

	/** Border color surrounding the whole bar. */
	static constexpr color_t border_color{213, 213, 213, 200};

	/** Returns the height of the bar in pixels. */
	static constexpr auto get_height(int size, double scaling)
	{
		return int(size * scaling);
	};

	/** Returns the relative amount of the bar to be filled. */
	static constexpr auto get_filled(int size, int max)
	{
		return std::clamp(float(size) / max, 0.0f, 1.0f);
	};

	/** Returns the fill color with the appropriate alpha depending on the focused state. */
	static constexpr auto get_color(const color_t& c, bool focused)
	{
		return color_t{c.r, c.g, c.b, float_to_color(focused ? 1.0 : 0.8)};
	}
};

void draw_bar(int index, const energy_bar& data, const rect& bounds)
{
	// All absolute bar coordinates are normalized relative to the standard 72px hex.
	using game_config::tile_size;

	SDL_FPoint p1{
		float(energy_bar::def_origin.x + energy_bar::spacing * index) / tile_size,
		float(energy_bar::def_origin.y                              ) / tile_size,
	};

	// If the top of the bar sits 13px from the top of the scaled hex rect, the bottom
	// of the bar should extend no closer than 13px from the bottom.
	SDL_FPoint p2{
		std::min(p1.x + float(data.def_w) / tile_size, 1.0f - p1.x),
		std::min(p1.y + float(data.bar_h) / tile_size, 1.0f - p1.y)
	};

	// Full bar dimensions
	const SDL_FRect bar_rect = sdl::precise_subrect(bounds, p1, p2);
	draw::fill(bar_rect, energy_bar::border_color);

	// Size of a single pixel relative to the standard hex
	const float one_pixel = 1.0f / tile_size;

	SDL_FPoint bg1{ p1.x + one_pixel, p1.y + one_pixel };
	SDL_FPoint bg2{ p2.x - one_pixel, p2.y - one_pixel };

	// Full inner dimensions
	const SDL_FRect inner_rect = sdl::precise_subrect(bounds, bg1, bg2);
	draw::fill(inner_rect, energy_bar::background_color);

	SDL_FPoint fill1{ 0.0f, 1.0f - data.filled };
	SDL_FPoint fill2{ 1.0f, 1.0f };

	// Filled area, relative to the bottom of the inner bar area
	const SDL_FRect fill_rect = sdl::precise_subrect(inner_rect, fill1, fill2);
	draw::fill(fill_rect, data.fill_color);
}

} // anon namespace

unit_drawer::unit_drawer(display& thedisp)
	: disp(thedisp)
	, dc(disp.context())
	, map(dc.map())
	, halo_man(thedisp.get_halo_manager())
	, viewing_team_ref(disp.viewing_team())
	, playing_team_ref(disp.playing_team())
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
}

bool unit_drawer::selected_or_reachable(const map_location& loc) const
{
	const bool is_highlighted_enemy = units_that_can_reach_goal.count(loc) > 0;
	return loc == sel_hex || is_highlighted_enemy;
}

void unit_drawer::redraw_unit(const unit& u) const
{
	unit_animation_component & ac = u.anim_comp();
	map_location loc = u.get_location();

	int side = u.side();

	bool hidden = u.get_hidden();
	bool is_flying = u.is_flying();
	map_location::direction facing = u.facing();

	bool can_recruit = u.can_recruit();

	const bool is_selected_hex = selected_or_reachable(loc);

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
	const auto [xsrc, ysrc] = disp.get_location(loc);
	const auto [xdst, ydst] = disp.get_location(dst);

	// FIXME: double check whether the shift amount accounts for zoom level
	rect unit_rect = disp.get_location_rect(loc).shifted_by(0, adjusted_params.y);

	// We draw bars only if wanted, visible on the map view
	if(ac.draw_bars_ && unit_rect.overlaps(disp.map_outside_area())) {

		// Always show the ellipse for selected units
		if(prefs::get().show_side_colors() || is_selected_hex) {
			draw_ellipses(u, adjusted_params);
		}

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
		} else if(side != playing_team_ref.side()) {
			// We're looking at either the player's own unit or an ally's unit, but either way it
			// doesn't belong to the playing_team and isn't expected to move until after its next
			// turn refresh.
			auto os = orb_status::moved;
			if(side != viewing_team_ref.side())
				os = orb_status::allied;
			orb_img = get_orb_image(os);
		} else if(side != viewing_team_ref.side()) {
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

		if(orb_img) {
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

		for(const std::string& ov : u.overlays_abilities()) {
			if(texture tex = image::get_texture(ov)) {
				textures.push_back(std::move(tex));
			}
		};

		const bool bar_focus = (loc == mouse_hex || is_selected_hex);
		std::vector<energy_bar> bars;

		if(u.max_hitpoints() > 0) {
			bars.AGGREGATE_EMPLACE(
				energy_bar::get_height(u.max_hitpoints(), u.hp_bar_scaling()),
				energy_bar::get_filled(u.hitpoints(), u.max_hitpoints()),
				energy_bar::get_color(u.hp_color(), bar_focus)
			);
		}

		if(u.experience() > 0 && u.can_advance()) {
			bars.AGGREGATE_EMPLACE(
				energy_bar::get_height(u.max_experience(), u.xp_bar_scaling() / std::max(u.level(), 1)),
				energy_bar::get_filled(u.experience(), u.max_experience()),
				energy_bar::get_color(u.xp_color(), bar_focus)
			);
		}

		disp.drawing_buffer_add(drawing_layer::unit_bar, loc,
			[textures = std::move(textures), bars = std::move(bars), shift = point{xoff, yoff + adjusted_params.y}](
				const rect& dest) {
				const rect shifted = dest.shifted_by(shift);

				for(const texture& tex : textures) {
					draw::blit(tex, shifted);
				}

				for(std::size_t i = 0; i < bars.size(); ++i) { // bar bar bar
					draw_bar(i, bars[i], shifted);
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

void unit_drawer::draw_ellipses(const unit& u, const frame_parameters& params) const
{
	std::string ellipse = u.image_ellipse();
	if(ellipse == "none") {
		return;
	}

	auto path = formatter{};
	if(!ellipse.empty()) {
		path << ellipse;
	} else {
		path << "misc/ellipse";
	}

	// Build the path based on whether the unit has a ZoC can recruit
	if(u.can_recruit())
		path << "-leader";

	if(!u.emits_zoc())
		path << "-nozoc";

	if(selected_or_reachable(u.get_location()))
		path << "-selected";

	// Load the ellipse parts recolored to match team color
	const std::string ipf = formatter{} << "~RC(ellipse_red>" << team::get_side_color_id(u.side()) << ")";

	std::array images {
		image::get_texture(image::locator{path.str() + "-top.png", ipf}),
		image::get_texture(image::locator{path.str() + "-bottom.png", ipf})
	};

	// The division by 2 seems to have no real meaning,
	// It just works fine with the current center of ellipse
	// and prevent a too large adjust if submerge = 1.0
	const int y_shift = params.submerge > 0.0
		? params.y - static_cast<int>(params.submerge * hex_size_by_2)
		: params.y;

	disp.drawing_buffer_add(drawing_layer::unit_first, u.get_location(),
		[images = std::move(images), y_shift](const rect& dest) {
			for(const texture& tex : images) {
				if(tex) {
					draw::blit(tex, dest.shifted_by(0, y_shift));
				}
			}
		});
}
