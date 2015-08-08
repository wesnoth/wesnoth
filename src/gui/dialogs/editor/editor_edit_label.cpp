/*
   Copyright (C) 2010 - 2015 by Fabian Müller <fabianmueller5@gmx.de>
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

#include "gui/dialogs/editor/editor_edit_label.hpp"

#include "gui/widgets/settings.hpp"

#include <SDL_video.h>
#include <boost/bind.hpp>

namespace gui2
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

teditor_edit_label::teditor_edit_label(std::string& text,
									   bool& immutable,
									   bool& visible_fog,
									   bool& visible_shroud,
									   SDL_Color& color,
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
	register_color_component("slider_red", &SDL_Color::r);
	register_color_component("slider_green", &SDL_Color::g);
	register_color_component("slider_blue", &SDL_Color::b);
}

void teditor_edit_label::register_color_component(std::string widget_id, Uint8 SDL_Color::* component) {
	register_integer(widget_id, true,
					 boost::bind(&teditor_edit_label::load_color_component, this, component),
					 boost::bind(&teditor_edit_label::save_color_component, this, component, _1));
}

int teditor_edit_label::load_color_component(Uint8 SDL_Color::* component) {
	return color_store.*component;
}

void teditor_edit_label::save_color_component(Uint8 SDL_Color::* component, const int value) {
	color_store.*component = value;
}
}
