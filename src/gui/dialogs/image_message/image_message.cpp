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

#include "gui/dialogs/image_message/image_message.hpp"

#include "foreach.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/auxiliary/old_markup.hpp"
//#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
//#ifdef GUI2_EXPERIMENTAL_LISTBOX
//#include "gui/widgets/list.hpp"
//#else
//#include "gui/widgets/listbox.hpp"
//#endif
#include "gui/widgets/settings.hpp"
//#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

/**
 * @todo This function enables the wml markup for all items, but the interface
 * is a bit hacky. Especially the fiddling in the internals of the listbox is
 * ugly. There needs to be a clean interface to set whether a widget has a
 * markup and what kind of markup. These fixes will be post 1.6.
 */
void timage_message_::pre_show(CVideo& /*video*/, twindow& window)
{
	window.canvas(1).set_variable("portrait_image", variant(portrait_));
	window.canvas(1).set_variable("portrait_mirror", variant(mirror_));

	// Set the markup
	tlabel& title = find_widget<tlabel>(&window, "title", false);
	title.set_label(title_);
	title.set_use_markup(true);
	title.set_can_wrap(true);

	tcontrol& message = find_widget<tcontrol>(&window, "message", false);
	message.set_label(message_);
	message.set_use_markup(true);
	// The message label might not always be a scroll_label but the capturing
	// shouldn't hurt.
	window.keyboard_capture(&message);

	window.set_click_dismiss(true);
}

REGISTER_DIALOG(image_message_left)

REGISTER_DIALOG(image_message_right)

int show_image_message(const bool left_side
		, CVideo& video
		, const std::string& title
		, const std::string& message
		, const std::string& portrait
		, const bool mirror)
{
	std::auto_ptr<timage_message_> dlg;
	if(left_side) {
		dlg.reset(new timage_message_left(title, message, portrait, mirror));
	} else {
		dlg.reset(new timage_message_right(title, message, portrait, mirror));
	}
	assert(dlg.get());

	dlg->show(video);
	return dlg->get_retval();
}

} // namespace gui2

