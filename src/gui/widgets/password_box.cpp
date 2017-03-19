/*
   Copyright (C) 2009 - 2017 by Thomas Baumhauer
   <thomas.baumhauer@NOSPAMgmail.com>
   Copyright (C) 2009 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/password_box.hpp"

#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"

#include "desktop/clipboard.hpp"
#include "utils/functional.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET3(text_box_definition, password_box, "text_box_definition")

void password_box::set_value(const std::string& text)
{
	real_value_ = text;
	size_t sz = utf8::size(text);
	utf8::string passwd;
	for(size_t i = 0; i < sz; i++) {
		passwd.append(font::unicode_bullet);
	}
	text_box::set_value(passwd);
}

void password_box::delete_selection()
{
	int len = get_selection_length();
	if(len == 0) {
		return;
	}
	unsigned start = get_selection_start();
	if(len < 0) {
		len = -len;
		start -= len;
	}

	utf8::erase(real_value_, start, len);
	set_value(real_value_);
	set_cursor(start, false);
}

void password_box::insert_char(const utf8::string& unicode)
{
	int len = get_selection_length();
	unsigned sel = get_selection_start();
	if(len < 0) {
		len = -len;
		sel -= len;
	}

	size_t sz = utf8::size(unicode);
	if(sz == 1) {
		text_box::insert_char(font::unicode_bullet);
	} else {
		utf8::string passwd;
		for(size_t i = 0; i < sz; i++) {
			passwd.append(font::unicode_bullet);
		}
		text_box::insert_char(passwd);
		set_cursor(sel + sz, false);
	}
	utf8::insert(real_value_, sel, unicode);
}

void password_box::paste_selection(const bool mouse)
{
	const std::string& text = desktop::clipboard::copy_from_clipboard(mouse);
	if(text.empty()) {
		return;
	}
	insert_char(text);
}

const std::string& password_box::get_control_type() const
{
	static const std::string type = "password_box";
	return type;
}

// }---------- BUILDER -----------{

namespace implementation
{

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_password_box
 *
 * == Password box ==
 *
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="password_box"}{min=0}{max=-1}{super="generic/widget_instance"}
 * @begin{table}{config}
 *     label & t_string & "" &         The initial text of the password box. $
 * @end{table}
 * @end{tag}{name="password_box"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

builder_password_box::builder_password_box(const config& cfg)
	: builder_styled_widget(cfg), history_(cfg["history"])
{
}

widget* builder_password_box::build() const
{
	password_box* widget = new password_box();

	init_control(widget);

	// A password box doesn't have a label but a text.
	// It also has no history.
	widget->set_value(label_string);

	DBG_GUI_G << "Window builder: placed password box '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
