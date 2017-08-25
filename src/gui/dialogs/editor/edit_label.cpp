/*
   Copyright (C) 2010 - 2017 by Fabian Müller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

#include "utils/functional.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_edit_label
 *
 * == Edit label ==
 *
 * Dialog for editing gamemap labels.
 *
 * @begin{table}{dialog_widgets}
 *
 * title & & label & m &
 *         Dialog title label. $
 *
 * label & & text_box & m &
 *         Input field for the map label. $
 *
 * team_only_toggle & & toggle_button & m &
 *         Checkbox for whether to make the label visible to the player's team
 *         only or not. $
 *
 * @end{table}
 */

REGISTER_DIALOG(editor_edit_label)

editor_edit_label::editor_edit_label(std::string& text,
									   bool& immutable,
									   bool& visible_fog,
									   bool& visible_shroud,
									   color_t& color,
									   std::string& category)
	: color_store(color)
{
	// std::string text = label.text();
	// bool immutable = label.immutable();


	// std::string label     = old_label ? old_label->text()              : "";
	// std::string team_name = old_label ? old_label->team_name()         : "";
	// bool visible_shroud   = old_label ? old_label->visible_in_shroud() :
	// false;
	// bool visible_fog      = old_label ? old_label->visible_in_fog()    :
	// true;
	// bool immutable        = old_label ? old_label->immutable()         :
	// true;

	register_text("label", true, text, true);
	register_text("category", true, category, false);
	register_bool("immutable_toggle", true, immutable);
	register_bool("visible_fog_toggle", true, visible_fog);
	register_bool("visible_shroud_toggle", true, visible_shroud);
	register_color_component("slider_red", &color_t::r);
	register_color_component("slider_green", &color_t::g);
	register_color_component("slider_blue", &color_t::b);
}

void editor_edit_label::pre_show(window& win)
{
	win.add_to_tab_order(find_widget<text_box>(&win, "label", false, true));
	win.add_to_tab_order(find_widget<text_box>(&win, "category", false, true));
}

void editor_edit_label::register_color_component(std::string widget_id, uint8_t color_t::* component) {
	register_integer(widget_id, true,
					 std::bind(&editor_edit_label::load_color_component, this, component),
					 std::bind(&editor_edit_label::save_color_component, this, component, _1));
}

int editor_edit_label::load_color_component(uint8_t color_t::* component) {
	return color_store.*component;
}

void editor_edit_label::save_color_component(uint8_t color_t::* component, const int value) {
	color_store.*component = value;
}
} // namespace dialogs
} // namespace gui2
