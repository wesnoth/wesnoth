/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/message.hpp"

#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

void tmessage::pre_show(CVideo& /*video*/, twindow& window)
{
	if(!title_.empty()) {
		tlabel* title =
			dynamic_cast<tlabel*>(window.find_widget("title", false));
		VALIDATE(title, missing_widget("title"));

		title->set_label(title_);
	}

	if(!image_.empty()) {
		timage* image =
			dynamic_cast<timage*>(window.find_widget("image", false));
		VALIDATE(image, missing_widget("image"));

		image->set_label(image_);
	}

	tcontrol* label =
		dynamic_cast<tcontrol*>(window.find_widget("label", false));
	VALIDATE(label, missing_widget("label"));

	label->set_label(message_);
	// The label might not always be a scroll_label but the capturing
	// shouldn't hurt.
	window.keyboard_capture(label);

	if(auto_close_) {
		/*
		 * Hide the buttton and do the layout, if window.does_easy_close() is
		 * false the scroll_label has a scrollbar so we need to show the
		 * button. When the button is hidden the text for the label is bigger
		 * and thus not need a scrollbar. Also when the button is visible
		 * easy_close will always return false.
		 */
		tbutton* button =
			dynamic_cast<tbutton*>(window.find_widget("ok", false));
		VALIDATE(button, missing_widget("ok"));
		button->set_visible(twidget::INVISIBLE);
		window.layout();

		if(! window.does_easy_close()) {
			button->set_visible(twidget::VISIBLE);
		}
	}
}

twindow* tmessage::build_window(CVideo& video)
{
	return build(video, get_id(MESSAGE));
}

/** @todo the caption is ignored. */
void show_message(CVideo& video, const std::string& title,
	const std::string& message, const std::string& /*button_caption*/,
	const bool auto_close)
{
	tmessage(title, message, auto_close).show(video);
}

} // namespace gui2

