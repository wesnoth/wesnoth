/* $Id$ */
/*
   Copyright (C) 2009 by Mark de Wever <koraq@xs4all.nl>
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
	tlabel* title =
		dynamic_cast<tlabel*>(window.find_widget("title", false));
	VALIDATE(title, missing_widget("title"));

	title->set_label(title_);
	title->set_markup_mode(title_markup_mode_);

	tlabel* message =
		dynamic_cast<tlabel*>(window.find_widget("message", false));
	VALIDATE(message, missing_widget("message"));

	message->set_label(message_);
	message->set_markup_mode(message_markup_mode_);
	message->set_can_wrap(true);
}

void show_transient_message(CVideo& video, const std::string& title,
	const std::string& message,
	const tcontrol::tmarkup_mode message_markup_mode,
	const tcontrol::tmarkup_mode title_markup_mode)
{
	ttransient_message dlg(title, title_markup_mode,
			message, message_markup_mode);
	dlg.show(video);
}

} // namespace gui2

