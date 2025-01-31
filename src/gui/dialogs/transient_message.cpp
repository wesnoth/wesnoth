/*
	Copyright (C) 2009 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "gui/widgets/window.hpp"
#include "log.hpp"

#include "gettext.hpp"

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(transient_message)

transient_message::transient_message(const std::string& title,
									   const bool title_use_markup,
									   const std::string& message,
									   const bool message_use_markup,
									   const std::string& image)
	: modal_dialog(window_id())
	, hide_title_(title.empty())
	, hide_image_(image.empty())
{
	register_label("title", true, title, title_use_markup);
	register_label("message", true, message, message_use_markup);
	register_image("image", true, image);
}

void transient_message::pre_show()
{
	if(hide_title_) {
		widget& title = find_widget<widget>("title");
		title.set_visible(widget::visibility::invisible);
	}

	if(hide_image_) {
		widget& image = find_widget<widget>("image");
		image.set_visible(widget::visibility::invisible);
	}
}
} // namespace dialogs

void show_transient_message(const std::string& title,
							const std::string& message,
							const std::string& image,
							const bool message_use_markup,
							const bool title_use_markup)
{
	dialogs::transient_message dlg(
			title, title_use_markup, message, message_use_markup, image);

	dlg.show();
}

void show_transient_error_message(const std::string& message,
								  const std::string& image,
								  const bool message_use_markup)
{
	LOG_STREAM(err, lg::general()) << message;
	show_transient_message(_("Error"), message, image, message_use_markup);
}

} // namespace gui2
