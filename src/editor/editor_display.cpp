/*
   Copyright (C) 2008 - 2018 by Tomasz Sniatowski <kailoran@gmail.com>
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

#include "editor/controller/editor_controller.hpp"
#include "editor/editor_display.hpp"
#include "lexical_cast.hpp"
#include "ogl/utils.hpp"
#include "overlay.hpp"
#include "reports.hpp"
#include "sdl/texture.hpp"
#include "team.hpp"
#include "terrain/builder.hpp"
#include "units/map.hpp"

namespace wb {
	class manager;
}

namespace editor {

editor_display::editor_display(editor_controller& controller, const config& theme_cfg)
	: display(nullptr, std::shared_ptr<wb::manager>(), theme_cfg, config())
	, brush_locations_()
	, controller_(controller)
{
#ifdef USE_GL_RENDERING
	gl::clear_screen();
#else
	video().clear_screen();
#endif
}

void editor_display::add_brush_loc(const map_location& hex)
{
	brush_locations_.insert(hex);
}

void editor_display::set_brush_locs(const std::set<map_location>& hexes)
{
	brush_locations_ = hexes;
}

void editor_display::clear_brush_locs()
{
	brush_locations_.clear();
}

void editor_display::remove_brush_loc(const map_location& hex)
{
	brush_locations_.erase(hex);
}

void editor_display::rebuild_terrain(const map_location &loc) {
	builder_->rebuild_terrain(loc);
}

void editor_display::pre_draw()
{
}

image::TYPE editor_display::get_image_type(const map_location& loc)
{
	if (map().in_selection(loc)) {
		return image::BRIGHTENED;
	}
	return image::TOD_COLORED;
}

void editor_display::draw_hex_cursor(const map_location& loc)
{
	if(!map().on_board_with_border(loc)) {
		return;
	}

	static texture brush(image::get_texture(game_config::images::editor_brush));

	for(const map_location& b_loc : brush_locations_) {
		render_scaled_to_zoom(brush, b_loc); // LAYER_SELECTED_HEX
	}
}

void editor_display::draw_hex_overlays()
{
	static texture selection_overlay(image::get_texture("editor/selection-overlay.png"));

	for(const map_location& s_loc : map().selection()) {
		render_scaled_to_zoom(selection_overlay, s_loc); // LAYER_FOG_SHROUD
	}
}

const SDL_Rect editor_display::get_clip_rect()
{
	return map_outside_area();
}

// USE AS REFERENCE FOR WHAT NEEDS TO BE UPDATED IN GUI2 UI
#if 0
void editor_display::draw_sidebar()
{
	config element;
	config::attribute_value &text = element.add_child("element")["text"];
	// Fill in the terrain report
	if (get_map().on_board_with_border(mouseoverHex_)) {
		text = get_map().get_terrain_editor_string(mouseoverHex_);
		refresh_report("terrain", &element);
		refresh_report("terrain_info");
		text = lexical_cast<std::string>(mouseoverHex_);
		refresh_report("position", &element);
	}

	if (dc_->teams().empty()) {
		text = int(get_map().villages().size());
		refresh_report("villages", &element);
	} else {
		refresh_report("villages");
		refresh_report("num_units");
	}
}
#endif

const time_of_day& editor_display::get_time_of_day(const map_location& /*loc*/) const
{
	return controller_.get_current_map_context().get_time_manager()->get_time_of_day();
}

display::overlay_map& editor_display::get_overlays()
{
	return controller_.get_current_map_context().get_overlays();
}

} //end namespace editor
