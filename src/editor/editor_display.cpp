/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor_display.hpp"
#include "builder.hpp"

namespace editor {

editor_display::editor_display(CVideo& video, const editor_map& map,
		const config& theme_cfg, const config& level)
	: display(video, &map, theme_cfg, level)
	, brush_locations_()
	, toolbar_hint_()
{
	clear_screen();
}

void editor_display::add_brush_loc(const map_location& hex)
{
	brush_locations_.insert(hex);
	invalidate(hex);
}

void editor_display::set_brush_locs(const std::set<map_location>& hexes)
{
	invalidate(brush_locations_);
	brush_locations_ = hexes;
	invalidate(brush_locations_);
}

void editor_display::clear_brush_locs()
{
	invalidate(brush_locations_);
	brush_locations_.clear();
}

void editor_display::remove_brush_loc(const map_location& hex)
{
	brush_locations_.erase(hex);
	invalidate(hex);
}

void editor_display::rebuild_terrain(const map_location &loc) {
	builder_->rebuild_terrain(loc);
}

void editor_display::pre_draw()
{
}

image::TYPE editor_display::get_image_type(const map_location& loc)
{
	if (brush_locations_.find(loc) != brush_locations_.end()) {
		return image::BRIGHTENED;
	} else if (map().in_selection(loc)) {
		return image::SEMI_BRIGHTENED;
	}
	return image::SCALED_TO_HEX;
}

void editor_display::draw_hex(const map_location& loc)
{
	int xpos = get_location_x(loc);
	int ypos = get_location_y(loc);
	tblit blit(xpos, ypos);
	display::draw_hex(loc);
	if (map().on_board_with_border(loc)) {
		if (map().in_selection(loc)) {
			drawing_buffer_add(LAYER_FOG_SHROUD, loc, tblit(xpos, ypos,
				image::get_image("editor/selection-overlay.png", image::SCALED_TO_HEX)));
		}

		if(loc == mouseoverHex_ && !game_config::images::mouseover.empty()) {
			static const image::locator mouseover(game_config::images::mouseover);
			drawing_buffer_add(LAYER_MOVE_INFO, loc, tblit(xpos, ypos,
					image::get_image(mouseover, image::UNMASKED)));
		}
	}
}

const SDL_Rect& editor_display::get_clip_rect()
{
	return map_outside_area();
}

void editor_display::draw_sidebar()
{
	// Fill in the terrain report
	if(get_map().on_board_with_border(mouseoverHex_)) {
		refresh_report(reports::TERRAIN, reports::report(get_map().get_terrain_string(mouseoverHex_)));
		refresh_report(reports::POSITION, reports::report(lexical_cast<std::string>(mouseoverHex_)));
	}
	refresh_report(reports::VILLAGES, reports::report(lexical_cast<std::string>(get_map().villages().size())));
	refresh_report(reports::EDITOR_TOOL_HINT, reports::report(toolbar_hint_));
}

} //end namespace editor
