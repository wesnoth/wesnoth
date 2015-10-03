/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Mark de Wever <koraq@xs4all.nl>
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

#include "foreach.hpp"
#include "gettext.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"

namespace gui2 {

/**
 * Helper to implement private functions without modifing the header.
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
	static void
	init_button(twindow& window, tmessage::tbutton_status& button_status,
			const std::string& id)
	{
		button_status.button = find_widget<tbutton>(
				&window, id, false, true);
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
	tmessage_implementation::
			init_button(window, buttons_[left_1], "left_side");
	tmessage_implementation::
			init_button(window, buttons_[cancel], "cancel");
	tmessage_implementation::
			init_button(window, buttons_[ok] ,"ok");
	tmessage_implementation::
			init_button(window, buttons_[right_1], "right_side");

	// ***** ***** ***** ***** Set up the widgets ***** ***** ***** *****
	if(!title_.empty()) {
		find_widget<tlabel>(&window, "title", false).set_label(title_);
	}

	if(!image_.empty()) {
		find_widget<timage>(&window, "image", false).set_label(image_);
	}

	tcontrol& label = find_widget<tcontrol>(&window, "label", false);
	label.set_label(message_);

	// The label might not always be a scroll_label but the capturing
	// shouldn't hurt.
	window.keyboard_capture(&label);

	// Override the user value, to make sure it's set properly.
	window.set_click_dismiss(auto_close_);
}

void tmessage::post_show(twindow& /*window*/)
{
	BOOST_FOREACH(tbutton_status& button_status, buttons_) {
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
		const twidget::tvisible visible)
{
	buttons_[button].visible = visible;
	if(buttons_[button].button) {
		buttons_[button].button->set_visible(visible);
	}
}

void tmessage::set_button_retval(const tbutton_id button,
		const int retval)
{
	buttons_[button].retval = retval;
	if(buttons_[button].button) {
		buttons_[button].button->set_retval(retval);
	}
}

tmessage::tbutton_status::tbutton_status()
	: button(NULL)
	, caption()
	, visible(twidget::INVISIBLE)
	, retval(twindow::NONE)
{
}

twindow* tmessage::build_window(CVideo& video)
{
	return build(video, get_id(MESSAGE));
}

void show_message(CVideo& video, const std::string& title,
	const std::string& message, const std::string& button_caption,
	const bool auto_close)
{
	tmessage dlg(title, message, auto_close);
	dlg.set_button_caption(tmessage::ok, button_caption);
	dlg.show(video);
}

int show_message(CVideo& video, const std::string& title,
	const std::string& message, const tmessage::tbutton_style button_style,
	bool /*message_use_markup*/,
	bool /*message_title_mode*/)
{
	/** @todo implement the markup mode. */
	tmessage dlg(title, message, button_style == tmessage::auto_close);

	switch(button_style) {
		case tmessage::auto_close :
			break;
		case tmessage::ok_button :
			dlg.set_button_visible(tmessage::ok, twidget::VISIBLE);
			dlg.set_button_caption(tmessage::ok, _("OK"));
			break;
		case tmessage::close_button :
			dlg.set_button_visible(tmessage::ok, twidget::VISIBLE);
			break;
		case tmessage::ok_cancel_buttons :
			dlg.set_button_visible(tmessage::ok, twidget::VISIBLE);
			dlg.set_button_caption(tmessage::ok, _("OK"));
			/* FALL DOWN */
		case tmessage::cancel_button :
			dlg.set_button_visible(tmessage::cancel, twidget::VISIBLE);
			break;
		case tmessage::yes_no_buttons :
			dlg.set_button_visible(tmessage::ok, twidget::VISIBLE);
			dlg.set_button_caption(tmessage::ok, _("Yes"));
			dlg.set_button_visible(tmessage::cancel, twidget::VISIBLE);
			dlg.set_button_caption(tmessage::cancel, _("No"));
			break;
	}

	dlg.show(video);
	return dlg.get_retval();
}

void show_error_message(CVideo& video, const std::string& message,
	bool message_use_markup)
{
	LOG_STREAM(err, lg::general) << message << '\n';
	show_message(video, _("Error"), message,
			tmessage::ok_button, message_use_markup);
}

} // namespace gui2

