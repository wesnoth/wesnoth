/*
	Copyright (C) 2008 - 2024
	by Tomasz Sniatowski <kailoran@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/action/action.hpp"
#include "editor/toolkit/brush.hpp"
#include "editor/editor_display.hpp"
#include "editor/action/mouse/mouse_action.hpp"

#include "gettext.hpp"
#include "editor/palette/terrain_palettes.hpp"

namespace editor {

bool mouse_action::has_context_menu() const
{
	return false;
}

void mouse_action::move(editor_display& disp, const map_location& hex)
{
	if (hex == previous_move_hex_) {
		return;
	}

	update_brush_highlights(disp, hex);
	previous_move_hex_ = hex;
}

void mouse_action::update_brush_highlights(editor_display& disp, const map_location& hex)
{
	disp.set_brush_locs(affected_hexes(disp, hex));
}

std::set<map_location> mouse_action::affected_hexes(
		editor_display& /*disp*/, const map_location& hex)
{
	std::set<map_location> res;
	res.insert(hex);
	return res;
}

std::unique_ptr<editor_action> mouse_action::drag_left(editor_display& /*disp*/,
		int /*x*/, int /*y*/, bool& /*partial*/, editor_action* /*last_undo*/)
{
	return nullptr;
}

std::unique_ptr<editor_action> mouse_action::drag_right(editor_display& /*disp*/,
		int /*x*/, int /*y*/, bool& /*partial*/, editor_action* /*last_undo*/)
{
	return nullptr;
}

std::unique_ptr<editor_action> mouse_action::drag_end_left(
		editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return nullptr;
}

std::unique_ptr<editor_action> mouse_action::drag_end_right(
		editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return nullptr;
}

std::unique_ptr<editor_action> mouse_action::up_right(
		editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return nullptr;
}

std::unique_ptr<editor_action> mouse_action::up_left(
		editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return nullptr;
}

std::unique_ptr<editor_action> mouse_action::key_event(
	editor_display& disp, const SDL_Event& event)
{
	if (!has_alt_modifier() && (event.key.keysym.sym >= '1' && event.key.keysym.sym <= '9')) {
		int side = event.key.keysym.sym - '0';
		if (side >= 1 && side <= gamemap::MAX_PLAYERS) {
			map_location pos = disp.get_map().starting_position(side);
			if (pos.valid()) {
				disp.scroll_to_tile(pos, display::WARP);
			}
		}
		return nullptr;
	}
	if (!disp.get_map().on_board(previous_move_hex_) || event.type != SDL_KEYUP) {
		return nullptr;
	}
	std::unique_ptr<editor_action> a;
	if ((has_alt_modifier() && (event.key.keysym.sym >= '1' && event.key.keysym.sym <= '9'))
	|| event.key.keysym.sym == SDLK_DELETE) {
		int res = event.key.keysym.sym - '0';
		if (res > gamemap::MAX_PLAYERS || event.key.keysym.sym == SDLK_DELETE) res = 0;
		const std::string* old_id = disp.get_map().is_special_location(previous_move_hex_);
		if (res == 0 && old_id != nullptr) {
			a = std::make_unique<editor_action_starting_position>(map_location(), *old_id);
		} else if (res > 0 && (old_id == nullptr || *old_id == std::to_string(res))) {
			a = std::make_unique<editor_action_starting_position>(previous_move_hex_, std::to_string(res));
		}
	}
	return a;
}

void mouse_action::set_mouse_overlay(editor_display& disp)
{
	disp.clear_mouseover_hex_overlay();
}

bool mouse_action::has_alt_modifier() const
{
	return key_[SDLK_RALT] || key_[SDLK_LALT];
}

bool mouse_action::has_shift_modifier() const
{
	return key_[SDLK_RSHIFT] || key_[SDLK_LSHIFT];
}

bool mouse_action::has_ctrl_modifier() const
{
#ifdef __APPLE__
	return key_[SDLK_RGUI] || key_[SDLK_LGUI];
#else
	return key_[SDLK_RCTRL] || key_[SDLK_LCTRL];
#endif
}

void mouse_action::set_terrain_mouse_overlay(
	editor_display& disp,
	const t_translation::terrain_code & fg,
	const t_translation::terrain_code & bg)
{
	const std::string fg_path = disp.get_map().get_terrain_info(fg).editor_image();
	const std::string bg_path = disp.get_map().get_terrain_info(bg).editor_image();
	const std::string blank_hex = "misc/blank-hex.png";

	if (!image::exists(fg_path) || !image::exists(bg_path)) {
		ERR_ED << "Missing terrain icon";
		disp.clear_mouseover_hex_overlay();
		return;
	}

	// For efficiency the size of the tile is cached.
	// We assume all tiles are of the same size.
	// The zoom factor can change, so it's not cached.
	// NOTE: when zooming and not moving the mouse, there are glitches.
	// Since the optimal alpha factor is unknown, it has to be calculated
	// on the fly, and caching the surfaces makes no sense yet.
	static const int size = image::get_size(blank_hex).x;
	static const int half_size = size / 2;
	static const int quarter_size = size / 4;
	static const int offset = 2;
	static const int new_size = half_size - 2;

	// Inserting scaled FG and BG hexes via IPF.
	std::stringstream path;
	// "hex~BLIT(fg~SCALE(ns,ns),2,qs)~BLIT(bg~SCALE(ns,ns),hs,qs)"
	path << blank_hex
		<< "~BLIT(" << fg_path << "~SCALE(" << new_size << "," << new_size
		<< ")," << offset << "," << quarter_size << ")"
		<< "~BLIT(" << bg_path << "~SCALE(" << new_size << "," << new_size
		<< ")," << half_size << "," << quarter_size << ")";

	// Set as mouseover overlay.
	disp.set_mouseover_hex_overlay(image::get_texture(path.str()));
}

std::set<map_location> brush_drag_mouse_action::affected_hexes(
	editor_display& /*disp*/, const map_location& hex)
{
	return get_brush().project(hex);
}

std::unique_ptr<editor_action> brush_drag_mouse_action::click_left(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	previous_drag_hex_ = hex;
	return click_perform_left(disp, affected_hexes(disp, hex));
}

std::unique_ptr<editor_action> brush_drag_mouse_action::click_right(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	previous_drag_hex_ = hex;
	return click_perform_right(disp, affected_hexes(disp, hex));
}

std::unique_ptr<editor_action> brush_drag_mouse_action::drag_left(editor_display& disp,
		int x, int y, bool& partial, editor_action* last_undo)
{
	return drag_generic<&brush_drag_mouse_action::click_perform_left>(disp, x, y, partial, last_undo);
}

std::unique_ptr<editor_action> brush_drag_mouse_action::drag_right(editor_display& disp,
		int x, int y, bool& partial, editor_action* last_undo)
{
	return drag_generic<&brush_drag_mouse_action::click_perform_right>(disp, x, y, partial, last_undo);
}

std::unique_ptr<editor_action> brush_drag_mouse_action::drag_end(
		editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return nullptr;
}

template <std::unique_ptr<editor_action> (brush_drag_mouse_action::*perform_func)(editor_display&, const std::set<map_location>&)>
std::unique_ptr<editor_action> brush_drag_mouse_action::drag_generic(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo)
{
	map_location hex = disp.hex_clicked_on(x, y);
	move(disp, hex);
	if (hex != previous_drag_hex_) {
		editor_action_extendable* last_undo_x = dynamic_cast<editor_action_extendable*>(last_undo);
		LOG_ED << "Last undo is " << last_undo << " and as x " << last_undo_x;
		partial = true;
		auto a = (this->*perform_func)(disp, affected_hexes(disp, hex));
		previous_drag_hex_ = hex;
		return a;
	} else {
		return nullptr;
	}
}

const brush& brush_drag_mouse_action::get_brush()
{
	assert(brush_);
	assert(*brush_);
	return **brush_;
}


std::unique_ptr<editor_action> mouse_action_paint::click_left(editor_display& disp, int x, int y)
{
	if (has_ctrl_modifier()) {
		map_location hex = disp.hex_clicked_on(x, y);
		terrain_palette_.select_fg_item(disp.get_map().get_terrain(hex));
		return nullptr;
	} else {
		return brush_drag_mouse_action::click_left(disp, x, y);
	}
}

std::unique_ptr<editor_action> mouse_action_paint::click_right(editor_display& disp, int x, int y)
{
	if (has_ctrl_modifier()) {
		map_location hex = disp.hex_clicked_on(x, y);
		terrain_palette_.select_bg_item(disp.get_map().get_terrain(hex));
		return nullptr;
	} else {
		return brush_drag_mouse_action::click_right(disp, x, y);
	}
}

std::unique_ptr<editor_action> mouse_action_paint::click_perform_left(
		editor_display& /*disp*/, const std::set<map_location>& hexes)
{
	if (has_ctrl_modifier()) return nullptr;
	// \todo why is this a chain and not an individual action? I guess that mouse drags were meant
	// to be optimised with one undo for multiple hexes, but it seems that was never added.
	auto chain = std::make_unique<editor_action_chain>();
	chain->append_action(std::make_unique<editor_action_paint_area>(
			hexes, terrain_palette_.selected_fg_item(), has_shift_modifier()));
	return chain;
}

std::unique_ptr<editor_action> mouse_action_paint::click_perform_right(
		editor_display& /*disp*/, const std::set<map_location>& hexes)
{
	if (has_ctrl_modifier()) return nullptr;
	// \todo why is this a chain and not an individual action?
	auto chain = std::make_unique<editor_action_chain>();
	chain->append_action(std::make_unique<editor_action_paint_area>(
			hexes, terrain_palette_.selected_bg_item(), has_shift_modifier()));
	return chain;
}

void mouse_action_paint::set_mouse_overlay(editor_display& disp)
{
	set_terrain_mouse_overlay(disp, terrain_palette_.selected_fg_item(),
			terrain_palette_.selected_bg_item());
}





bool mouse_action_paste::has_context_menu() const
{
	return true;
}

std::set<map_location> mouse_action_paste::affected_hexes(
	editor_display& /*disp*/, const map_location& hex)
{
	return paste_.get_offset_area(hex);
}

std::unique_ptr<editor_action> mouse_action_paste::click_left(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	return std::make_unique<editor_action_paste>(paste_, hex);
}

std::unique_ptr<editor_action> mouse_action_paste::click_right(editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return nullptr;
}

void mouse_action_paste::set_mouse_overlay(editor_display& disp)
{
	disp.set_mouseover_hex_overlay(
		image::get_texture(
			// center 60px icon on blank hex template
			image::locator(
				"misc/blank-hex.png",
				"~BLIT(icons/action/editor-paste_60.png,6,6)"
			)
		)
	);
}

std::set<map_location> mouse_action_fill::affected_hexes(
	editor_display& disp, const map_location& hex)
{
	return disp.get_map().get_contiguous_terrain_tiles(hex);
}

std::unique_ptr<editor_action> mouse_action_fill::click_left(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	if (has_ctrl_modifier()) {
		terrain_palette_.select_fg_item(disp.get_map().get_terrain(hex));
		return nullptr;
	} else {
		/** @todo only take the base terrain into account when searching for contiguous terrain when painting base only */
		//or use a different key modifier for that
		return std::make_unique<editor_action_fill>(hex, terrain_palette_.selected_fg_item(),
				has_shift_modifier());
	}
}

std::unique_ptr<editor_action> mouse_action_fill::click_right(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	if (has_ctrl_modifier()) {
		terrain_palette_.select_bg_item(disp.get_map().get_terrain(hex));
		return nullptr;
	} else {
		/** @todo only take the base terrain into account when searching for contiguous terrain when painting base only */
		//or use a different key modifier for that
		return std::make_unique<editor_action_fill>(hex, terrain_palette_.selected_bg_item(),
				has_shift_modifier());
	}
}

void mouse_action_fill::set_mouse_overlay(editor_display& disp)
{
	set_terrain_mouse_overlay(disp, terrain_palette_.selected_fg_item(),
			terrain_palette_.selected_bg_item());
}

std::unique_ptr<editor_action> mouse_action_starting_position::up_left(editor_display& disp, int x, int y)
{
	if (!click_) return nullptr;
	click_ = false;
	map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(hex)) {
		return nullptr;
	}
	auto player_starting_at_hex = disp.get_map().is_special_location(hex);

	if (has_ctrl_modifier()) {
		if (player_starting_at_hex) {
			location_palette_.add_item(*player_starting_at_hex);
		}
		return nullptr;
	}

	std::string new_player_at_hex = location_palette_.selected_item();
	std::unique_ptr<editor_action> a;

	if(!player_starting_at_hex || new_player_at_hex != *player_starting_at_hex) {
		// Set a starting position
		a = std::make_unique<editor_action_starting_position>(hex, new_player_at_hex);
	}
	else {
		// Erase current starting position
		a = std::make_unique<editor_action_starting_position>(map_location(), *player_starting_at_hex);
	}

	update_brush_highlights(disp, hex);

	return a;
}

std::unique_ptr<editor_action> mouse_action_starting_position::click_left(editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	click_ = true;
	return nullptr;
}

std::unique_ptr<editor_action> mouse_action_starting_position::up_right(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	auto player_starting_at_hex = disp.get_map().is_special_location(hex);
	if (player_starting_at_hex != nullptr) {
		return std::make_unique<editor_action_starting_position>(map_location(), *player_starting_at_hex);
	} else {
		return nullptr;
	}
}

std::unique_ptr<editor_action> mouse_action_starting_position::click_right(editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return nullptr;
}

void mouse_action_starting_position::set_mouse_overlay(editor_display& disp)
{
	disp.set_mouseover_hex_overlay(
		image::get_texture(
			// center 60px icon on blank hex template
			image::locator(
				"misc/blank-hex.png",
				"~BLIT(icons/action/editor-tool-starting-position_60.png,6,6)"
			)
		)
	);
}


} //end namespace editor
