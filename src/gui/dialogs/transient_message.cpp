/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/transient_message.hpp"

#include "gui/widgets/label.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

twindow* ttransient_message::build_window(CVideo& video)
{
	return build(video, get_id(TRANSIENT_MESSAGE));
}

void ttransient_message::pre_show(CVideo& /*video*/, twindow& window)
{
	tlabel& title = find_widget<tlabel>(&window, "title", false);
	title.set_label(title_);
	title.set_use_markup(title_use_markup_);

	tlabel& message = find_widget<tlabel>(&window, "message", false);
	message.set_label(message_);
	message.set_use_markup(message_use_markup_);
	message.set_can_wrap(true);
}

void show_transient_message(CVideo& video, const std::string& title,
	const std::string& message, bool message_use_markup, bool title_use_markup)
{
	ttransient_message dlg(title, title_use_markup,
			message, message_use_markup);
	dlg.show(video);
}

} // namespace gui2

