/*
	Copyright (C) 2010 - 2024
	by Fabian Müller <fabianmueller5@gmx.de>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/editor/edit_label.hpp"

#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

#include <functional>

namespace gui2::dialogs
{

REGISTER_DIALOG(editor_edit_label)

editor_edit_label::editor_edit_label(std::string& text,
									   bool& immutable,
									   bool& visible_fog,
									   bool& visible_shroud,
									   color_t& color,
									   std::string& category)
	: modal_dialog(window_id())
	, color_store(color)
{
	register_text("label", true, text, true);
	register_text("category", true, category, false);
	register_bool("immutable_toggle", true, immutable);
	register_bool("visible_fog_toggle", true, visible_fog);
	register_bool("visible_shroud_toggle", true, visible_shroud);
	register_color_component("slider_red", &color_t::r);
	register_color_component("slider_green", &color_t::g);
	register_color_component("slider_blue", &color_t::b);
}

void editor_edit_label::pre_show()
{
	add_to_tab_order(find_widget<text_box>("label", false, true));
	add_to_tab_order(find_widget<text_box>("category", false, true));
}

void editor_edit_label::register_color_component(const std::string& widget_id, uint8_t color_t::* component) {
	register_integer(widget_id, true,
					 std::bind(&editor_edit_label::load_color_component, this, component),
					 std::bind(&editor_edit_label::save_color_component, this, component, std::placeholders::_1));
}

int editor_edit_label::load_color_component(uint8_t color_t::* component) {
	return color_store.*component;
}

void editor_edit_label::save_color_component(uint8_t color_t::* component, const int value) {
	color_store.*component = value;
}
} // namespace dialogs
