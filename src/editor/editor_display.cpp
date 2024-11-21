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

#include "draw.hpp"
#include "editor/controller/editor_controller.hpp"
#include "editor/editor_display.hpp"
#include "floating_label.hpp"
#include "font/sdl_ttf_compat.hpp" // for pango_line_width
#include "formula/string_utils.hpp"
#include "lexical_cast.hpp"
#include "overlay.hpp"
#include "team.hpp"
#include "terrain/builder.hpp"
#include "video.hpp"

namespace wb {
	class manager;
}

namespace editor {

editor_display::editor_display(editor_controller& controller, reports& reports_object)
	: display(nullptr, std::shared_ptr<wb::manager>(), reports_object, "editor", config())
	, brush_locations_()
	, controller_(controller)
	, mouseover_hex_overlay_()
{
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

void editor_display::draw_hex(const map_location& loc)
{
	display::draw_hex(loc);

	if(!get_map().on_board_with_border(loc) || map_screenshot_) {
		return;
	}

	if(get_map().in_selection(loc)) {
		drawing_buffer_add(drawing_layer::fog_shroud, loc,
			[tex = image::get_texture(image::locator{"editor/selection-overlay.png"}, image::TOD_COLORED)](const rect& d) {
				draw::blit(tex, d);
			});
	}

	if(brush_locations_.find(loc) != brush_locations_.end()) {
		static const image::locator brush(game_config::images::editor_brush);
		drawing_buffer_add(drawing_layer::selected_hex, loc, [tex = image::get_texture(brush, image::HEXED)](const rect& d) {
			draw::blit(tex, d);
		});
	}

	// Paint mouseover overlays
	if(mouseover_hex_overlay_ && loc == mouseoverHex_) {
		drawing_buffer_add(drawing_layer::mouseover_overlay, loc, [this](const rect& dest) {
			mouseover_hex_overlay_.set_alpha_mod(196);
			draw::blit(mouseover_hex_overlay_, dest);
			mouseover_hex_overlay_.set_alpha_mod(SDL_ALPHA_OPAQUE);
		});
	}
}

rect editor_display::get_clip_rect() const
{
	return map_outside_area();
}

void editor_display::layout()
{
	display::layout();

	config element;
	config::attribute_value& text = element.add_child("element")["text"];
	// Fill in the terrain report
	if (get_map().on_board_with_border(mouseoverHex_)) {
		text = get_map().get_terrain_editor_string(mouseoverHex_);
		refresh_report("terrain", &element);
		refresh_report("terrain_info");
		text = lexical_cast<std::string>(mouseoverHex_);
		refresh_report("position", &element);
	}

	if (context().teams().empty()) {
		text = int(get_map().villages().size());
		refresh_report("villages", &element);
	} else {
		refresh_report("villages");
		refresh_report("num_units");
	}

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

const time_of_day& editor_display::get_time_of_day(const map_location& /*loc*/) const
{
	return controller_.get_current_map_context().get_time_manager()->get_time_of_day();
}

display::overlay_map& editor_display::get_overlays()
{
	return controller_.get_current_map_context().get_overlays();
}

void editor_display::set_status(const std::string& str, const bool is_success)
{
	const color_t color{0, 0, 0, 0xbb};
	int size = font::SIZE_SMALL;
	point canvas_size = video::game_canvas_size();
	const int border = 3;

	std::string formatted_str;
	if (is_success) {
		formatted_str = VGETTEXT("<span color='#66ff00'><span face='DejaVuSans'>✔</span> $msg</span>", {{"msg", str}});
	} else {
		formatted_str = VGETTEXT("<span color='red'><span face='DejaVuSans'>✘</span> $msg</span>", {{"msg", str}});
	}

	using namespace std::chrono_literals;
	font::floating_label flabel(formatted_str);
	flabel.set_font_size(size);
	flabel.set_position(0, canvas_size.y);
	flabel.set_bg_color(color);
	flabel.set_border_size(border);
	flabel.set_lifetime(1000ms, 10ms);
	flabel.use_markup(true);

	const int f_handle = font::add_floating_label(flabel);
	const auto& r = font::get_floating_label_rect(f_handle);
	font::move_floating_label(f_handle, r.w, -r.h);
}

void editor_display::set_help_string_enabled(bool value)
{
	help_string_enabled_ = value;

	if (!value) {
		clear_help_string();
	} else if (!help_string_.empty()) {
		set_help_string(help_string_);
	}
}

void editor_display::clear_help_string()
{
	font::remove_floating_label(help_handle_);
	help_handle_ = 0;
}

void editor_display::set_help_string(const std::string& str)
{
	// Always update the internal string so we can toggle its visibility back
	// at any time without having to ask the current editor_palette.
	help_string_ = str;

	clear_help_string();

	if (!help_string_enabled_ || help_string_.empty()) {
		return;
	}

	const color_t color{0, 0, 0, 0xbb};

	int size = font::SIZE_LARGE;
	point canvas_size = video::game_canvas_size();

	while(size > 0) {
		if(font::pango_line_width(str, size) * 2 > canvas_size.x) {
			size--;
		} else {
			break;
		}
	}

	const int border = 5;

	font::floating_label flabel(str);
	flabel.set_font_size(size);
	flabel.set_position(canvas_size.x / 2, canvas_size.y);
	flabel.set_bg_color(color);
	flabel.set_border_size(border);

	help_handle_ = font::add_floating_label(flabel);

	// Put the label near the bottom of the screen. In layout() it'll be moved to the top if the
	// user is editing hexes at the south edge of the map.
	help_string_at_top_ = false;
	const auto& r = font::get_floating_label_rect(help_handle_);
	font::move_floating_label(help_handle_, 0.0, -double(r.h));
}

} //end namespace editor
