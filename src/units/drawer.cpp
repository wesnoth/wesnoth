/*
	Copyright (C) 2014 - 2025
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

struct energy_bar
{
	/** Bar height in pixels. */
	int bar_h{};

	/** Bar width in pixels. */
	int bar_w{};

	/** The extent to which the bar is filled, ranging [0, 1]. */
	float filled{};

	/** Color for the bar's filled area. */
	color_t fill_color{};

	/** Color for the unfilled part of the bar area */
	color_t empty_color{};

	/** Border color surrounding the whole bar. */
	color_t border_color{};

	/** Initial coordinates for the first bar drawn. */
	static constexpr point def_origin{ 14, 13 };

	/** Default width for all bars. */
	static constexpr int def_w = 6;

	/** The distance between the bars (in empty pixels).  */
	static constexpr int spacing = 1;

	/** The factor of the bar width that border should intrude into */
	static constexpr float border_factor = 0.1f;

	/** Percentage of the inner width used for edge rounding each side. */
	static constexpr float rounding_factor = 0.3f;

	/** Color for the unfilled bar area. */
	static constexpr color_t def_empty_color{ 180, 180, 180, 200 };

	/** Border color surrounding the whole bar. */
	static constexpr color_t def_border_color{ 0, 0, 0, 200 };

	/** Returns the height of the bar in pixels. */
	static constexpr auto get_height(int size, double scaling, double ui_zoom_factor)
	{
		return int(size * scaling * ui_zoom_factor);
	};

	/** Returns the width of the bar in pixels. */
	static constexpr auto get_width(double ui_zoom_factor)
	{
		return int(def_w * ui_zoom_factor);
	};

	/** Returns the relative amount of the bar to be filled. */
	static constexpr auto get_filled(int size, int max)
	{
		return std::clamp(float(size) / max, 0.0f, 1.0f);
	};

	/** Returns the fill color with the appropriate alpha depending on the focused/highlighted state. */
	static constexpr auto get_color(const color_t& c, bool focused)
	{
		float multiplier = focused ? 1.0f : 0.77f;
		return color_t{ c.r, c.g, c.b, static_cast<uint8_t>(c.a * multiplier) };
	}

	/** Returns the highlight string if focused, otherwise an empty string. */
	static std::string get_highlight_mod(bool focused)
	{
		return focused ? "~CS(30,30,30)" : "";
	}

	/** Adjusted zoom factor for scaling bars and other UI elements. */
	static double get_ui_zoom_factor(double zoom_factor)
	{
		return 3.0 / (3.0 + (zoom_factor - 1.0));
	}

	/** Heuristic divisor used to center orb/crown icons during zoom scaling. */
	static constexpr int centering_divisor = 5;
};

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

void draw_bar(int index, const energy_bar& data, const rect& bounds)
{
	using game_config::tile_size;

	// --- 1. HELPERS ---
	// Snaps a normalized value to the nearest physical pixel to prevent subpixel blurring.
	auto snap_to_pixel = [](float val, int dimension) {
		return std::round(val * static_cast<float>(dimension)) / static_cast<float>(dimension);
	};

	// Converts absolute pixel sizes to normalized (0.0 to 1.0) coordinates.
	auto to_norm_x = [&](int px) { return static_cast<float>(px) / tile_size; };
	auto to_norm_y = [&](int px) { return static_cast<float>(px) / tile_size; };

	// --- 2. DEFINE OUTER BOUNDARIES (top-left and bottom-right of bar) ---
	const int x_offset = energy_bar::def_origin.x + (data.bar_w + energy_bar::spacing) * index;
	const SDL_FPoint p1{
		snap_to_pixel(to_norm_x(x_offset), bounds.w),
		snap_to_pixel(to_norm_y(energy_bar::def_origin.y), bounds.h)
	};
	const SDL_FPoint p2{
		snap_to_pixel(std::min(p1.x + to_norm_x(data.bar_w), 1.0f), bounds.w),
		snap_to_pixel(std::min(p1.y + to_norm_y(data.bar_h), 1.0f), bounds.h)
	};
	const SDL_FRect outer_rect = sdl::precise_subrect(bounds, p1, p2);

	// --- 3. BORDER LINES & INSETS (4 thin rectangles) ---
	const float border_px = std::round(std::max(1.0f, outer_rect.w * data.border_factor));

	draw::fill(SDL_FRect{ outer_rect.x, outer_rect.y, outer_rect.w, border_px }, data.border_color);
	draw::fill(SDL_FRect{ outer_rect.x, outer_rect.y + outer_rect.h - border_px, outer_rect.w, border_px }, data.border_color);

	// Only draw left/right borders if there's enough space for the middle bar.
	const float available_w = outer_rect.w - (2.0f * border_px);
	const bool has_l = (available_w >= 0.0f);
	const bool has_r = (available_w >= 1.0f);
	const float side_y = outer_rect.y + border_px;
	const float side_h = outer_rect.h - (2.0f * border_px);

	if(has_l)
		draw::fill(SDL_FRect{ outer_rect.x, side_y, border_px, side_h }, data.border_color);
	if(has_r)
		draw::fill(SDL_FRect{ outer_rect.x + outer_rect.w - border_px, side_y, border_px, side_h }, data.border_color);

	// --- 4. FILLING THE INNER BAR (The "health" and "grey" parts of the "healthbar") ---
	const SDL_FRect inner_rect{
		outer_rect.x + (has_l * border_px),
		outer_rect.y + border_px,
		outer_rect.w - ((has_l + has_r) * border_px),
		outer_rect.h - (2.0f * border_px)
	};
	const float empty_ratio = 1.0f - static_cast<float>(data.filled);
	const SDL_FRect empty_part = sdl::precise_subrect(inner_rect, { 0.0f, 0.0f }, { 1.0f, empty_ratio });
	const SDL_FRect filled_part = sdl::precise_subrect(inner_rect, { 0.0f, empty_ratio }, { 1.0f, 1.0f });

	draw::fill(empty_part, data.empty_color);
	draw::fill(filled_part, data.fill_color);

	// --- 5. SEPARATOR LINE (between the "grey" and "health" in the "healthbar") ---
	const float min_gap_px = border_px * 1.5f;
	const float filled_px = static_cast<float>(data.filled) * inner_rect.h;
	const float empty_px = empty_ratio * inner_rect.h;

	// Only draw the separator if the fill level isn't too close to the top or bottom edges.
	if(filled_px >= min_gap_px && empty_px >= min_gap_px) {
		const SDL_FRect separator{ inner_rect.x, filled_part.y, inner_rect.w, border_px };
		draw::fill(separator, data.border_color);
	}

	// --- 6. ROUNDING OVERLAYS (simulating soft corners to left/right edges) ---
	int rounding_layers = std::min(20, static_cast<int>(std::floor(inner_rect.w * energy_bar::rounding_factor)));
	const float layer_thickness = 1.0f;

	for(int i = 0; i < rounding_layers; ++i) {
		float alpha_mult = (rounding_layers - i) / (rounding_layers + 0.5f);
		color_t overlay_color = {
			data.border_color.r,
			data.border_color.g,
			data.border_color.b,
			static_cast<uint8_t>(data.border_color.a * alpha_mult)
		};

		float inset = i * layer_thickness;
		SDL_FRect layer_rect = {
			inner_rect.x + inset,
			inner_rect.y,
			inner_rect.w - (2.0f * inset),
			inner_rect.h
		};

		if(layer_rect.w <= 0)
			break;

		draw::fill(SDL_FRect{ layer_rect.x, layer_rect.y, layer_thickness, layer_rect.h }, overlay_color);
		draw::fill(SDL_FRect{ layer_rect.x + layer_rect.w - layer_thickness, layer_rect.y, layer_thickness, layer_rect.h }, overlay_color);
	}
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

	const double ui_zoom_factor = energy_bar::get_ui_zoom_factor(zoom_factor);

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
	const terrain_type& terrain_info = map.get_terrain_info(loc);

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

		// Determine if this unit UI should be highlighted/focused
		const bool bar_focus = (loc == mouse_hex || is_selected_hex);

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
		const std::string focus_mod = energy_bar::get_highlight_mod(bar_focus);

		// 1. Orb icon
		if(orb_img) {
			std::string full_orb_path = orb_img->get_filename() + orb_img->get_modifications();
			if(texture tex = image::get_texture(full_orb_path + focus_mod)) {
				textures.push_back(std::move(tex));
			}
		}

		// 2. Crown, leader icon
		if(can_recruit) {
			if(texture tex = image::get_texture(u.leader_crown() + focus_mod)) {
				textures.push_back(std::move(tex));
			}
		}

		// 3. Other icons (Loyal unit icon, item/ablility icons etc.)
		for(const std::string& ov : u.overlays()) {
			if(texture tex = image::get_texture(ov + focus_mod)) {
				textures.push_back(std::move(tex));
			}
		}
		for(const std::string& ov : u.overlays_abilities()) {
			if(texture tex = image::get_texture(ov + focus_mod)) {
				textures.push_back(std::move(tex));
			}
		}

		std::vector<energy_bar> bars;

		if(u.max_hitpoints() > 0) {
			bars.AGGREGATE_EMPLACE(
				energy_bar::get_height(u.max_hitpoints(), u.hp_bar_scaling(), ui_zoom_factor),
				energy_bar::get_width(ui_zoom_factor),
				energy_bar::get_filled(u.hitpoints(), u.max_hitpoints()),
				energy_bar::get_color(u.hp_color(), bar_focus),
				energy_bar::get_color(energy_bar::def_empty_color, bar_focus),
				energy_bar::get_color(energy_bar::def_border_color, bar_focus)
			);
		}

		if(u.experience() > 0 && u.can_advance()) {
			bars.AGGREGATE_EMPLACE(
				energy_bar::get_height(u.max_experience(), u.xp_bar_scaling() / std::max(u.level(), 1), ui_zoom_factor),
				energy_bar::get_width(ui_zoom_factor),
				energy_bar::get_filled(u.experience(), u.max_experience()),
				energy_bar::get_color(u.xp_color(), bar_focus),
				energy_bar::get_color(energy_bar::def_empty_color, bar_focus),
				energy_bar::get_color(energy_bar::def_border_color, bar_focus)
			);
		}

		disp.drawing_buffer_add(drawing_layer::unit_bar, loc,
			[textures = std::move(textures), bars = std::move(bars), shift = point{ xoff, yoff + adjusted_params.y }, ui_zoom_factor](
				const rect& hex_rect)
			{
				// 1. Calculate the icon scaling (Orbs/Crowns)
				const int icon_size = static_cast<int>(hex_rect.w * ui_zoom_factor);

				// 2. Calculate offset to keep icons at the edge of the hex.
				const int edge_offset = (hex_rect.w - icon_size) / energy_bar::centering_divisor;

				// 3. Prepare the drawing areas
				const rect icon_dest = rect{ hex_rect.x + edge_offset, hex_rect.y + edge_offset, icon_size, icon_size }
				.shifted_by(shift);

				const rect bar_dest = hex_rect.shifted_by(shift);

				// 4. Draw icons (Orbs, Crowns, etc.)
				for(const texture& tex : textures) {
					draw::blit(tex, icon_dest);
				}

				// 5. Draw bars (HP, XP, etc.)
				for(std::size_t i = 0; i < bars.size(); ++i) {
					draw_bar(i, bars[i], bar_dest);
				}
			}
		);
	}

	// Smooth unit movements from terrain of different elevation.
	// Do this separately from above so that the health bar doesn't go up and down.

	const terrain_type& terrain_dst_info = map.get_terrain_info(dst);

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
