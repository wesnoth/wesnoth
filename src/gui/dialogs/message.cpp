/*
   Copyright (C) 2008 - 2014 by Mark de Wever <koraq@xs4all.nl>
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
#include "gui/auxiliary/find_widget.tpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "utils/foreach.tpp"

namespace gui2
{

REGISTER_DIALOG(message)

/**
 * Helper to implement private functions without modifying the header.
 *
 * The class is a helper to avoid recompilation and only has static
 * functions.
 */
struct tmessage_implementation
{
	/**
	 * Initialiazes a button.
	 *
	 * @param window              The window that contains the button.
	 * @param button_status       The button status to modify.
	 * @param id                  The id of the button.
	 */
	static void init_button(twindow& window,
							tmessage::tbutton_status& button_status,
							const std::string& id)
	{
		button_status.button = find_widget<tbutton>(&window, id, false, true);
		button_status.button->set_visible(button_status.visible);

		if(!button_status.caption.empty()) {
			button_status.button->set_label(button_status.caption);
		}

		if(button_status.retval != twindow::NONE) {
			button_status.button->set_retval(button_status.retval);
		}
	}
};

void tmessage::pre_show(CVideo& /*video*/, twindow& window)
{
	// ***** Validate the required buttons ***** ***** ***** *****
	tmessage_implementation::init_button(window, buttons_[left_1], "left_side");
	tmessage_implementation::init_button(window, buttons_[cancel], "cancel");
	tmessage_implementation::init_button(window, buttons_[ok], "ok");
	tmessage_implementation::init_button(
			window, buttons_[right_1], "right_side");

	// ***** ***** ***** ***** Set up the widgets ***** ***** ***** *****
	tcontrol& title_widget = find_widget<tlabel>(&window, "title", false);
	if(!title_.empty()) {
		title_widget.set_label(title_);
	} else {
		title_widget.set_visible(twidget::tvisible::invisible);
	}

	tcontrol& img_widget = find_widget<timage>(&window, "image", false);
	if(!image_.empty()) {
		img_widget.set_label(image_);
	} else {
		img_widget.set_visible(twidget::tvisible::invisible);
	}

	tcontrol& label = find_widget<tcontrol>(&window, "label", false);
	label.set_label(message_);
	label.set_use_markup(message_use_markup_);

	// The label might not always be a scroll_label but the capturing
	// shouldn't hurt.
	window.keyboard_capture(&label);

	// Override the user value, to make sure it's set properly.
	window.set_click_dismiss(auto_close_);
}

void tmessage::post_show(twindow& /*window*/)
{
	FOREACH(AUTO & button_status, buttons_)
	{
		button_status.button = NULL;
	}
}

void tmessage::set_button_caption(const tbutton_id button,
								  const std::string& caption)
{
	buttons_[button].caption = caption;
	if(buttons_[button].button) {
		buttons_[button].button->set_label(caption);
	}
}

void tmessage::set_button_visible(const tbutton_id button,
								  const twidget::tvisible::scoped_enum visible)
{
	buttons_[button].visible = visible;
	if(buttons_[button].button) {
		buttons_[button].button->set_visible(visible);
	}
}

void tmessage::set_button_retval(const tbutton_id button, const int retval)
{
	buttons_[button].retval = retval;
	if(buttons_[button].button) {
		buttons_[button].button->set_retval(retval);
	}
}

tmessage::tbutton_status::tbutton_status()
	: button(NULL)
	, caption()
	, visible(twidget::tvisible::invisible)
	, retval(twindow::NONE)
{
}

void show_message(CVideo& video,
				  const std::string& title,
				  const std::string& message,
				  const std::string& button_caption,
				  const bool auto_close,
				  const bool message_use_markup)
{
	tmessage dlg(title, message, auto_close, message_use_markup);
	dlg.set_button_caption(tmessage::ok, button_caption);
	dlg.show(video);
}

int show_message(CVideo& video,
				 const std::string& title,
				 const std::string& message,
				 const tmessage::tbutton_style button_style,
				 bool message_use_markup,
				 bool /*message_title_mode*/)
{
	tmessage dlg(title,
				 message,
				 button_style == tmessage::auto_close,
				 message_use_markup);

	switch(button_style) {
		case tmessage::auto_close:
			break;
		case tmessage::ok_button:
			dlg.set_button_visible(tmessage::ok, twidget::tvisible::visible);
			dlg.set_button_caption(tmessage::ok, _("OK"));
			break;
		case tmessage::close_button:
			dlg.set_button_visible(tmessage::ok, twidget::tvisible::visible);
			break;
		case tmessage::ok_cancel_buttons:
			dlg.set_button_visible(tmessage::ok, twidget::tvisible::visible);
			dlg.set_button_caption(tmessage::ok, _("OK"));
		/* FALL DOWN */
		case tmessage::cancel_button:
			dlg.set_button_visible(tmessage::cancel,
								   twidget::tvisible::visible);
			break;
		case tmessage::yes_no_buttons:
			dlg.set_button_visible(tmessage::ok, twidget::tvisible::visible);
			dlg.set_button_caption(tmessage::ok, _("Yes"));
			dlg.set_button_visible(tmessage::cancel,
								   twidget::tvisible::visible);
			dlg.set_button_caption(tmessage::cancel, _("No"));
			break;
	}

	dlg.show(video);
	return dlg.get_retval();
}

void show_error_message(CVideo& video,
						const std::string& message,
						bool message_use_markup)
{
	LOG_STREAM(err, lg::general) << message << '\n';
	show_message(video,
				 _("Error"),
				 message,
				 tmessage::ok_button,
				 message_use_markup);
}

} // namespace gui2
