/*
   Copyright (C) 2014 by 
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

#include "gui/widgets/num_box.hpp"

#include "gui/auxiliary/widget_definition/text_box.hpp"
#include "gui/auxiliary/window_builder/num_box.hpp"
#include "gui/widgets/detail/register.tpp"
#include "gui/widgets/settings.hpp"
#include "clipboard.hpp"
#include "serialization/string_utils.hpp"

#include <boost/bind.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

namespace gui2 {

REGISTER_WIDGET3(ttext_box_definition, num_box, "text_box_definition")

void tnum_box::validate(const int value, const int cursor_shift)
{
	if (value > get_maximum_value()) {
		if (set_validated(get_maximum_value() , cursor_shift)) {
			return;
		}
	} else if (value < get_minimum_value()) {
		if (set_validated(get_minimum_value() , cursor_shift)) {
			return;
		}
	} else {
		if (set_validated(value, cursor_shift)) {
			return;
		}
	}
	if (cursor_shift) {
		set_cursor(get_selection_start() + cursor_shift, false);
	}
	ttext_box::set_value(boost::lexical_cast<std::string>(get_value()));
}

void tnum_box::set_value(const std::string& text)
{
	try {
		validate(boost::lexical_cast<int>(text));
	}
	catch (boost::bad_lexical_cast) { }
}

void tnum_box::insert_char(const Uint16 unicode)
{
	std::string restore = get_text();
	delete_selection();
	if (!text_.insert_unicode(get_selection_start(), unicode)) {
		return;
	}
	try {
		validate(boost::lexical_cast<int>(get_text()) , 1);
	}
	catch (boost::bad_lexical_cast) {
		ttext_::set_value(restore);
		return;
	}
}

void tnum_box::paste_selection(const bool mouse)
{
	std::string paste = copy_from_clipboard(mouse);
	
	// trim leading and trailing whitespaces
	boost::algorithm::trim(paste);
	// is something left?
	if(paste.empty()) return;
	std::string restore = get_text();
	delete_selection();
	unsigned len = text_.insert_text(get_selection_start(), paste);
	try {
		validate(boost::lexical_cast<int>(get_text()), len);
	}
	catch (boost::bad_lexical_cast) {
		ttext_::set_value(restore);
		return;
	}
}


void tnum_box::delete_char(const bool before_cursor)
{
	ttext_box::delete_char(before_cursor);
	validate(boost::lexical_cast<int>(get_text()));
}

void tnum_box::handle_key_backspace(SDLMod /*modifier*/, bool& handled)
{
	handled = true;
	if (get_selection_length() != 0) {
		delete_selection();
	}
	else if (get_selection_start()) {
		delete_char(true);
	}
	validate(boost::lexical_cast<int>(get_text()));
}

void tnum_box::handle_key_delete(SDLMod /*modifier*/, bool& handled)
{
	handled = true;
	if (get_selection_length() != 0) {
		delete_selection();
	}
	else if (get_selection_start() < utils::string_to_wstring(get_text()).size()) {
		delete_char(false);
	}
	validate(boost::lexical_cast<int>(get_text()));
}

const std::string& tnum_box::get_control_type() const
{
	static const std::string type = "num_box";
	return type;
}

} // namespace gui2
