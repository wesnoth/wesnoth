/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/message.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(message)

/**
 * Helper to implement private functions without modifying the header.
 *
 * The class is a helper to avoid recompilation and only has static
 * functions.
 */
struct message_implementation
{
	/**
	 * Initialiazes a button.
	 *
	 * @param window              The window that contains the button.
	 * @param button_status       The button status to modify.
	 * @param id                  The id of the button.
	 */
	static void init_button(window& window,
							message::button_status& button_status,
							const std::string& id)
	{
		button_status.ptr = find_widget<button>(&window, id, false, true);
		button_status.ptr->set_visible(button_status.visible);

		if(!button_status.caption.empty()) {
			button_status.ptr->set_label(button_status.caption);
		}

		if(button_status.retval != retval::NONE) {
			button_status.ptr->set_retval(button_status.retval);
		}
	}
};

void message::pre_show(window& window)
{
	set_restore(true);

	// ***** Validate the required buttons ***** ***** ***** *****
	message_implementation::init_button(window, buttons_[left_1], "left_side");
	message_implementation::init_button(window, buttons_[cancel], "cancel");
	message_implementation::init_button(window, buttons_[ok], "ok");
	message_implementation::init_button(
			window, buttons_[right_1], "right_side");

	// ***** ***** ***** ***** Set up the widgets ***** ***** ***** *****
	styled_widget& title_widget = find_widget<label>(&window, "title", false);
	if(!title_.empty()) {
		title_widget.set_label(title_);
		title_widget.set_use_markup(title_use_markup_);
	} else {
		title_widget.set_visible(widget::visibility::invisible);
	}

	styled_widget& img_widget = find_widget<image>(&window, "image", false);
	if(!image_.empty()) {
		img_widget.set_label(image_);
	} else {
		img_widget.set_visible(widget::visibility::invisible);
	}

	styled_widget& label = find_widget<styled_widget>(&window, "label", false);
	label.set_label(message_);
	label.set_use_markup(message_use_markup_);

	// The label might not always be a scroll_label but the capturing
	// shouldn't hurt.
	window.keyboard_capture(&label);

	// Override the user value, to make sure it's set properly.
	window.set_click_dismiss(auto_close_);
}

void message::post_show(window& /*window*/)
{
	for(auto & button_status : buttons_)
	{
		button_status.ptr = nullptr;
	}
}

void message::set_button_caption(const button_id button,
								  const std::string& caption)
{
	buttons_[button].caption = caption;
	if(buttons_[button].ptr) {
		buttons_[button].ptr->set_label(caption);
	}
}

void message::set_button_visible(const button_id button,
								  const widget::visibility visible)
{
	buttons_[button].visible = visible;
	if(buttons_[button].ptr) {
		buttons_[button].ptr->set_visible(visible);
	}
}

void message::set_button_retval(const button_id button, const int retval)
{
	buttons_[button].retval = retval;
	if(buttons_[button].ptr) {
		buttons_[button].ptr->set_retval(retval);
	}
}

message::button_status::button_status()
	: ptr(nullptr)
	, caption()
	, visible(widget::visibility::invisible)
	, retval(retval::NONE)
{
}

} // namespace dialogs

using namespace dialogs;

void show_message(const std::string& title,
				  const std::string& msg,
				  const std::string& button_caption,
				  const bool auto_close,
				  const bool message_use_markup,
				  const bool title_use_markup)
{
	message dlg(title, msg, auto_close, message_use_markup, title_use_markup);
	dlg.set_button_caption(message::ok, button_caption);
	dlg.show();
}

int show_message(const std::string& title,
				 const std::string& msg,
				 const message::button_style button_style,
				 bool message_use_markup,
				 bool title_use_markup)
{
	message dlg(title,
				 msg,
				 button_style == message::auto_close,
				 message_use_markup,
				 title_use_markup);

	switch(button_style) {
		case message::auto_close:
			break;
		case message::ok_button:
			dlg.set_button_visible(message::ok, widget::visibility::visible);
			dlg.set_button_caption(message::ok, _("OK"));
			break;
		case message::close_button:
			dlg.set_button_visible(message::ok, widget::visibility::visible);
			break;
		case message::ok_cancel_buttons:
			dlg.set_button_visible(message::ok, widget::visibility::visible);
			dlg.set_button_caption(message::ok, _("OK"));
			FALLTHROUGH;
		case message::cancel_button:
			dlg.set_button_visible(message::cancel, widget::visibility::visible);
			break;
		case message::yes_no_buttons:
			dlg.set_button_visible(message::ok, widget::visibility::visible);
			dlg.set_button_caption(message::ok, _("Yes"));
			dlg.set_button_visible(message::cancel,  widget::visibility::visible);
			dlg.set_button_caption(message::cancel, _("No"));
			break;
	}

	dlg.show();
	return dlg.get_retval();
}

void show_error_message(const std::string& msg,
						bool message_use_markup)
{
	LOG_STREAM(err, lg::general()) << msg << '\n';
	(void) show_message(
				 _("Error"),
				 msg,
				 message::ok_button,
				 message_use_markup);
}
} // namespace gui2
