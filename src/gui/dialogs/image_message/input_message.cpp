/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/image_message/input_message.hpp"

#include "foreach.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/auxiliary/old_markup.hpp"
//#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

/**
 * @todo This function enables the wml markup for all items, but the interface
 * is a bit hacky. Especially the fiddling in the internals of the listbox is
 * ugly. There needs to be a clean interface to set whether a widget has a
 * markup and what kind of markup. These fixes will be post 1.6.
 */
void tinput_message_::pre_show(CVideo& video, twindow& window)
{
	timage_message_::pre_show(video, window);
	// Find the input box related fields.
	tlabel& caption = find_widget<tlabel>(&window, "input_caption", false);
	ttext_box& input = find_widget<ttext_box>(&window, "input", true);

	caption.set_label(input_caption_);
	caption.set_use_markup(true);
	input.set_value(*input_text_);
	input.set_maximum_length(input_maximum_lenght_);
	window.keyboard_capture(&input);
	window.set_click_dismiss(false);
	window.set_escape_disabled(true);
	// click_dismiss has been disabled due to the input.
}

void tinput_message_::post_show(twindow& window)
{
	*input_text_ =
			find_widget<ttext_box>(&window, "input", true).get_value();
}

REGISTER_DIALOG(input_message_left)

REGISTER_DIALOG(input_message_right)

int show_input_message(const bool left_side
		, CVideo& video
		, const std::string& title
		, const std::string& message
		, const std::string& portrait
		, const bool mirror
		, const std::string& input_caption
		, std::string* input_text
		, const unsigned maximum_length)
{
	std::auto_ptr<tinput_message_> dlg;
	if(left_side) {
		dlg.reset(new tinput_message_left(
				title, message, portrait, mirror, input_caption, input_text, maximum_length));
	} else {
		dlg.reset(new tinput_message_right(
				title, message, portrait, mirror, input_caption, input_text, maximum_length));
	}
	assert(dlg.get());

	dlg->show(video);
	return dlg->get_retval();
}

} // namespace gui2

