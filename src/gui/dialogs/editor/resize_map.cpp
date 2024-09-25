/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/editor/resize_map.hpp"

#include "gui/auxiliary/field.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/slider.hpp"

#include <functional>

namespace gui2::dialogs
{

REGISTER_DIALOG(editor_resize_map)

editor_resize_map::editor_resize_map(int& width,
									   int& height,
									   EXPAND_DIRECTION& expand_direction,
									   bool& copy_edge_terrain)
	: modal_dialog(window_id())
	, width_(register_integer("width", true, width))
	, height_(register_integer("height", true, height))
	, old_width_(width)
	, old_height_(height)
	, expand_direction_(expand_direction)
	, direction_buttons_{}
{
	register_bool("copy_edge_terrain", false, copy_edge_terrain);

	register_label("old_width", false, std::to_string(width));
	register_label("old_height", false, std::to_string(height));
}

void editor_resize_map::pre_show()
{
	slider& height = find_widget<slider>("height");
	connect_signal_notify_modified(
			height,
			std::bind(&editor_resize_map::update_expand_direction, this));

	slider& width = find_widget<slider>("width");
	connect_signal_notify_modified(
			width,
			std::bind(&editor_resize_map::update_expand_direction, this));

	std::string name_prefix = "expand";
	for(int i = 0; i < 9; ++i) {
		std::string name = name_prefix + std::to_string(i);
		direction_buttons_[i]
				= find_widget<toggle_button>(name, false, true);

		connect_signal_notify_modified(*direction_buttons_[i],
			std::bind(&editor_resize_map::update_expand_direction, this));
	}
	direction_buttons_[0]->set_value(true);
	update_expand_direction();
}

/**
 * Convert a coordinate on a 3  by 3 grid to an index, return 9 for out of
 * bounds
 */
static int resize_grid_xy_to_idx(const int x, const int y)
{
	if(x < 0 || x > 2 || y < 0 || y > 2) {
		return 9;
	} else {
		return y * 3 + x;
	}
}

void editor_resize_map::set_direction_icon(int index, const std::string& icon)
{
	if(index < 9) {
		direction_buttons_[index]->set_icon_name("icons/arrows/arrows_blank_"
												 + icon + "_30.png");
	}
}

void editor_resize_map::update_expand_direction()
{
	for(int i = 0; i < 9; ++i) {
		if(direction_buttons_[i]->get_value()
		   && static_cast<int>(expand_direction_) != i) {

			expand_direction_ = static_cast<EXPAND_DIRECTION>(i);
			break;
		}
	}
	for(int i = 0; i < static_cast<int>(expand_direction_); ++i) {
		direction_buttons_[i]->set_value(false);
		set_direction_icon(i, "none");
	}
	direction_buttons_[expand_direction_]->set_value(true);
	for(int i = expand_direction_ + 1; i < 9; ++i) {
		direction_buttons_[i]->set_value(false);
		set_direction_icon(i, "none");
	}

	int xdiff = width_->get_widget_value() - old_width_;
	int ydiff = height_->get_widget_value() - old_height_;
	int x = static_cast<int>(expand_direction_) % 3;
	int y = static_cast<int>(expand_direction_) / 3;
	set_direction_icon(expand_direction_, "center");
	if(xdiff != 0) {
		int left = resize_grid_xy_to_idx(x - 1, y);
		int right = resize_grid_xy_to_idx(x + 1, y);
		if(xdiff < 0)
			std::swap(left, right);
		set_direction_icon(left, "left");
		set_direction_icon(right, "right");
	}
	if(ydiff != 0) {
		int top = resize_grid_xy_to_idx(x, y - 1);
		int bottom = resize_grid_xy_to_idx(x, y + 1);
		if(ydiff < 0)
			std::swap(top, bottom);
		set_direction_icon(top, "up");
		set_direction_icon(bottom, "down");
	}
	if(xdiff < 0 || ydiff < 0 || (xdiff > 0 && ydiff > 0)) {
		int nw = resize_grid_xy_to_idx(x - 1, y - 1);
		int ne = resize_grid_xy_to_idx(x + 1, y - 1);
		int sw = resize_grid_xy_to_idx(x - 1, y + 1);
		int se = resize_grid_xy_to_idx(x + 1, y + 1);
		if(xdiff < 0 || ydiff < 0) {
			std::swap(nw, se);
			std::swap(ne, sw);
		}
		set_direction_icon(nw, "topleft");
		set_direction_icon(ne, "topright");
		set_direction_icon(sw, "bottomleft");
		set_direction_icon(se, "bottomright");
	}
}

} // namespace dialogs
