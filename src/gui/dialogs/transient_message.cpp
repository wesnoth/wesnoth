/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/transient_message.hpp"

#include "gettext.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"

namespace gui2 {

REGISTER_WINDOW(transient_message)

void ttransient_message::pre_show(CVideo& /*video*/, twindow& window)
{
	tlabel& title = find_widget<tlabel>(&window, "title", false);
	title.set_label(title_);
	title.set_use_markup(title_use_markup_);

	tlabel& message = find_widget<tlabel>(&window, "message", false);
	message.set_label(message_);
	message.set_use_markup(message_use_markup_);
	message.set_can_wrap(true);

	timage& image = find_widget<timage>(&window, "image", false);

	if(!image_.empty()) {
		image.set_image(image_);;
	}
}

void show_transient_message(CVideo& video, const std::string& title,
	const std::string& message, const std::string& image,
	bool message_use_markup, bool title_use_markup)
{
	ttransient_message dlg(title, title_use_markup,
			message, message_use_markup, image);
	dlg.show(video);
}

void show_transient_error_message(CVideo& video
		, const std::string& message
		, const std::string& image
		, bool message_use_markup)
{
	LOG_STREAM(err, lg::general) << message << '\n';
	show_transient_message(video, _("Error"), message, image, message_use_markup);
}

} // namespace gui2

