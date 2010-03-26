/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Thomas Baumhauer <thomas.baumhauer@NOSPAMgmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/mp_cmd_wrapper.hpp"

#include "gui/widgets/button.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"

#include "game_preferences.hpp"

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_mp_cmd_wrapper
 *
 * == Multiplayer command wrapper ==
 *
 * This shows a dialog that provides a graphical front-end
 * to various commands in the multiplayer lobby
 *
 * @start_table = container
 *     message (text_box)         Text to send as a private message.
 *     reason (text_box)          The reason for a ban.
 *     time (text_box)            The time the ban lasts.
 *     [send_message] (button)    Execute /msg.
 *     [add_friend] (button)      Execute /friend.
 *     [add_ignore] (button)      Execute /ignore.
 *     [remove] (button)          Execute /remove.
 *     [status] (button)          Execute /query status.
 *     [kick] (button)            Execute /query kick.
 *     [ban] (button)             Execute /query kban.
 * @end_table
 */

REGISTER_WINDOW(mp_cmd_wrapper)

tmp_cmd_wrapper::tmp_cmd_wrapper(const t_string& user) :
		message_(), reason_(), time_(), user_(user) { }

void tmp_cmd_wrapper::pre_show(CVideo& /*video*/, twindow& window)
{
	ttext_box* message =
		dynamic_cast<ttext_box*>(window.find("message", false));
	if(message) {
		/**
		 * @todo For some reason the text wrapping fails on Windows and Mac,
		 * this causes an exception to be thrown, which brings the user back
		 * to the main menu. So avoid that problem by imposing a maximum
		 * length (the number of letters W that fit).
		 */
#if defined(_WIN32) || defined(__APPLE__)
		message->set_maximum_length(18);
#endif
		window.keyboard_capture(message);
	}

	message = dynamic_cast<ttext_box*>(window.find("reason", false));
	if(message) message->set_active(preferences::is_authenticated());

	message = dynamic_cast<ttext_box*>(window.find("time", false));
	if(message) message->set_active(preferences::is_authenticated());

	tlabel* label =
		dynamic_cast<tlabel*>(window.find("user_label", false));
	if(label) label->set_label(user_);


	tbutton* b = dynamic_cast<tbutton*>(window.find("add_friend", false));
	if(b) b->set_retval(1);

	b = dynamic_cast<tbutton*>(window.find("add_ignore", false));
	if(b) b->set_retval(2);

	b = dynamic_cast<tbutton*>(window.find("remove", false));
	if(b) b->set_retval(3);

	b = dynamic_cast<tbutton*>(window.find("status", false));
	if(b) {
		b->set_retval(4);
		b->set_active(preferences::is_authenticated());
	}

	b = dynamic_cast<tbutton*>(window.find("kick", false));
	if(b) {
		b->set_retval(5);
		b->set_active(preferences::is_authenticated());
	}

	b = dynamic_cast<tbutton*>(window.find("ban", false));
	if(b) {
		b->set_retval(6);
		b->set_active(preferences::is_authenticated());
	}

}

void tmp_cmd_wrapper::post_show(twindow& window)
{
	ttext_box* message =
		dynamic_cast<ttext_box*>(window.find("message", false));
	message_ = message ? message_ = message->get_value() : "";

	ttext_box* reason =
		dynamic_cast<ttext_box*>(window.find("reason", false));
	reason_ = reason ? reason_ = reason->get_value() : "";

	ttext_box* time =
		dynamic_cast<ttext_box*>(window.find("time", false));
	time_ = time ? time_ = time->get_value() : "";

}

} // namespace gui2

