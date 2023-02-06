/*
	Copyright (C) 2008 - 2022
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

#include "editor/controller/editor_controller.hpp"
#include "editor/editor_display.hpp"
#include "floating_label.hpp"
#include "font/sdl_ttf_compat.hpp" // for pango_line_width
#include "lexical_cast.hpp"
#include "overlay.hpp"
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
	// If we're showing hexes near the north of the map, put the help string at the bottom of the screen.
	// Otherwise, put it at the top.
	if(help_handle_ != 0) {
		const bool place_at_top = get_visible_hexes().top[0] > 2;

		if(place_at_top != help_string_at_top_) {
			const auto& r = font::get_floating_label_rect(help_handle_);
			double delta = map_outside_area().h - r.h;
			if(place_at_top) {
				font::move_floating_label(help_handle_, 0.0, -delta);
			} else {
				font::move_floating_label(help_handle_, 0.0, delta);
			}
			help_string_at_top_ = place_at_top;
		}
	}
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

const time_of_day& editor_display::get_time_of_day(const map_location& /*loc*/) const
{
	return controller_.get_current_map_context().get_time_manager()->get_time_of_day();
}

display::overlay_map& editor_display::get_overlays()
{
	return controller_.get_current_map_context().get_overlays();
}

void editor_display::clear_help_string()
{
	font::remove_floating_label(help_handle_);
	help_handle_ = 0;
}

void editor_display::set_help_string(const std::string& str)
{
	clear_help_string();

	const color_t color{0, 0, 0, 0xbb};

	int size = font::SIZE_LARGE;

	while(size > 0) {
		if(font::pango_line_width(str, size) * 2 > video().get_width()) {
			size--;
		} else {
			break;
		}
	}

	const int border = 5;

	font::floating_label flabel(str);
	flabel.set_font_size(size);
	flabel.set_position(video().get_width() / 2, video().get_height());
	flabel.set_bg_color(color);
	flabel.set_border_size(border);

	help_handle_ = font::add_floating_label(flabel);

	// Put the label near the bottom of the screen. In pre_draw it'll be moved to the top if the
	// user is editing hexes at the south edge of the map.
	help_string_at_top_ = false;
	const auto& r = font::get_floating_label_rect(help_handle_);
	font::move_floating_label(help_handle_, 0.0, -double(r.h));
}

} //end namespace editor
