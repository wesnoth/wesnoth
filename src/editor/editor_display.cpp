/*
   Copyright (C) 2008 - 2018 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "reports.hpp"
#include "team.hpp"
#include "terrain/builder.hpp"
#include "units/map.hpp"

namespace wb {
	class manager;
}

namespace editor {

editor_display::editor_display(editor_controller& controller, reports& reports_object, const config& theme_cfg)
	: display(nullptr, std::shared_ptr<wb::manager>(), reports_object, theme_cfg, config())
	, brush_locations_()
	, controller_(controller)
{
	video().clear_screen();
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
	if (map().in_selection(loc)) {
		return image::BRIGHTENED;
	}
	return image::TOD_COLORED;
}

void editor_display::draw_hex(const map_location& loc)
{
	int xpos = get_location_x(loc);
	int ypos = get_location_y(loc);
	display::draw_hex(loc);
	if (map().on_board_with_border(loc)) {
		if (map().in_selection(loc)) {
			drawing_buffer_add(LAYER_FOG_SHROUD, loc, xpos, ypos,
				image::get_image("editor/selection-overlay.png", image::TOD_COLORED));
		}

		if (brush_locations_.find(loc) != brush_locations_.end()) {
			static const image::locator brush(game_config::images::editor_brush);
			drawing_buffer_add(LAYER_SELECTED_HEX, loc, xpos, ypos,
					image::get_image(brush, image::SCALED_TO_HEX));
		}
	}
}

const SDL_Rect& editor_display::get_clip_rect()
{
	return map_outside_area();
}

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

const tod_manager& editor_display::get_tod_man() const
{
	return *controller_.get_current_map_context().get_time_manager();
}

const time_of_day& editor_display::get_time_of_day(const map_location& /*loc*/) const
{
	return get_tod_man().get_time_of_day();
}

} //end namespace editor
