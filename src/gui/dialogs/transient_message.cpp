/* $Id$ */
/*
   Copyright (C) 2009 - 2012 by Mark de Wever <koraq@xs4all.nl>
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
#include "gui/widgets/settings.hpp"
#include "log.hpp"

namespace gui2 {

REGISTER_DIALOG(transient_message)

ttransient_message::ttransient_message(const std::string& title
		, const bool title_use_markup
		, const std::string& message
		, const bool message_use_markup
		, const std::string& image)
{
	register_label("title", true, title, title_use_markup);
	register_label("message", true, message, message_use_markup);
	register_image("image", true, image);
}

void show_transient_message(CVideo& video
		, const std::string& title
		, const std::string& message
		, const std::string& image
		, const bool message_use_markup
		, const bool title_use_markup)
{
	ttransient_message dlg(title
			, title_use_markup
			, message
			, message_use_markup
			, image);

	dlg.show(video);
}

void show_transient_error_message(CVideo& video
		, const std::string& message
		, const std::string& image
		, const bool message_use_markup)
{
	LOG_STREAM(err, lg::general) << message << '\n';
	show_transient_message(video
			, _("Error")
			, message
			, image
			, message_use_markup);
}

} // namespace gui2

