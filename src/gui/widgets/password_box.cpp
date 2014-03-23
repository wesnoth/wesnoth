/*
   Copyright (C) 2009 - 2014 by Thomas Baumhauer
   <thomas.baumhauer@NOSPAMgmail.com>
   Copyright (C) 2009 - 2014 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/text_box.hpp"
#include "gui/auxiliary/window_builder/password_box.hpp"
#include "gui/widgets/detail/register.tpp"
#include "gui/widgets/settings.hpp"
#include "serialization/string_utils.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

REGISTER_WIDGET3(ttext_box_definition, password_box, "text_box_definition")

namespace
{

size_t get_text_length(const std::string& str)
{
	return utf8::size(str);
}

} // namespace

void tpassword_box::set_value(const std::string& text)
{
	ttext_box::set_value(text);
	real_value_ = get_value();
	ttext_box::set_value(std::string(get_text_length(real_value_), '*'));
}

void tpassword_box::insert_char(const Uint16 unicode)
{
	pre_function();
	ttext_box::insert_char(unicode);
	post_function();
}

void tpassword_box::delete_char(const bool before_cursor)
{
	pre_function();
	ttext_box::delete_char(before_cursor);
	post_function();
}

void tpassword_box::handle_key_backspace(SDLMod /*modifier*/, bool& handled)
{
	pre_function();

	// Copy & paste from ttext_::handle_key_backspace()
	DBG_GUI_E << LOG_SCOPE_HEADER << '\n';

	handled = true;
	if(get_selection_length() != 0) {
		delete_selection();
	} else if(get_selection_start()) {
		delete_char(true);
	}

	post_function();
}

void tpassword_box::handle_key_delete(SDLMod /*modifier*/, bool& handled)
{
	pre_function();

	// Copy & paste from ttext_::handle_key_delete()
	DBG_GUI_E << LOG_SCOPE_HEADER << '\n';

	handled = true;
	if(get_selection_length() != 0) {
		delete_selection();
	} else if(get_selection_start() < get_text_length(text())) {
		delete_char(false);
	}

	post_function();
}

void tpassword_box::paste_selection(const bool mouse)
{
	pre_function();
	ttext_box::paste_selection(mouse);
	post_function();
}

void tpassword_box::pre_function()
{
	// ttext_box::set_value() will reset the selection,
	// we therefore have to remember it
	size_t selection_start = get_selection_start();
	size_t selection_length = get_selection_length();

	// Tell ttext_box the actual input of this box
	ttext_box::set_value(real_value_);

	// Restore the selection
	set_selection_start(selection_start);
	set_selection_length(selection_length);
}

void tpassword_box::post_function()
{
	// See above
	size_t selection_start = get_selection_start();
	size_t selection_length = get_selection_length();

	// Get the input back and make ttext_box forget it
	real_value_ = get_value();
	ttext_box::set_value(std::string(get_text_length(real_value_), '*'));

	// See above
	set_selection_start(selection_start);
	set_selection_length(selection_length);

	// Why do the selection functions not update
	// the canvas?
	update_canvas();
	set_is_dirty(true);
}

const std::string& tpassword_box::get_control_type() const
{
	static const std::string type = "password_box";
	return type;
}

} // namespace gui2
