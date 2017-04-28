/*
   Copyright (C) 2009 - 2017 by Thomas Baumhauer
   <thomas.baumhauer@NOSPAMgmail.com>
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

#include "gui/dialogs/multiplayer/mp_cmd_wrapper.hpp"

#include "gui/auxiliary/field.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include "game_preferences.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_mp_cmd_wrapper
 *
 * == Multiplayer command wrapper ==
 *
 * This shows a dialog that provides a graphical front-end
 * to various commands in the multiplayer lobby
 *
 * @begin{table}{dialog_widgets}
 *
 * message & & text_box & o &
 *         Text to send as a private message. $
 *
 * reason & & text_box & o &
 *         The reason for a ban. $
 *
 * time & & text_box & o &
 *         The time the ban lasts. $
 *
 * user_label & & label & o &
 *         The label to show which user has been selected. $
 *
 * send_message & & button & m &
 *         Execute /msg. $
 *
 * add_friend & & button & m &
 *         Execute /friend. $
 *
 * add_ignore & & button & m &
 *         Execute /ignore. $
 *
 * remove & & button & m &
 *         Execute /remove. $
 *
 * status & & button & m &
 *         Execute /query status. $
 *
 * kick & & button & m &
 *         Execute /query kick. $
 *
 * ban & & button & m &
 *         Execute /query kban. $
 *
 * mod_options & & grid & m &
 *         Grid containing the status/kick/ban options. This grid and its
 *         children are hidden when these options are unavailable. $
 *
 * @end{table}
 */

REGISTER_DIALOG(mp_cmd_wrapper)

mp_cmd_wrapper::mp_cmd_wrapper(const t_string& user)
	: message_(), reason_(), time_()
{
	register_text("message", false, message_, true);
	register_text("reason", false, reason_);
	register_text("time", false, time_);
	register_label("user_label", false, user);

	set_always_save_fields(true);
}

void mp_cmd_wrapper::pre_show(window& window)
{
#if defined(_WIN32) || defined(__APPLE__)
	text_box* message
			= find_widget<text_box>(&window, "message", false, false);
	if(message) {
		/**
		 * @todo For some reason the text wrapping fails on Windows and Mac,
		 * this causes an exception to be thrown, which brings the user back
		 * to the main menu. So avoid that problem by imposing a maximum
		 * length (the number of letters W that fit).
		 */
		message->set_maximum_length(18);
	}
#endif

	const bool authenticated = preferences::is_authenticated();

	if(grid* g = find_widget<grid>(&window, "mod_options", false, false)) {
		g->set_active(authenticated);
		g->set_visible(authenticated ? widget::visibility::visible : widget::visibility::invisible);
	}

	/**
	 * @todo Not really happy with the retval code in general. Need to give it
	 * some more thought. Therefore separated the set_retval from the
	 * set_active code.
	 */
	if(button* b = find_widget<button>(&window, "add_friend", false, false)) {
		b->set_retval(1);
	}

	if(button* b = find_widget<button>(&window, "add_ignore", false, false)) {
		b->set_retval(2);
	}

	if(button* b = find_widget<button>(&window, "remove", false, false)) {
		b->set_retval(3);
	}

	if(button* b = find_widget<button>(&window, "status", false, false)) {
		b->set_retval(4);
	}

	if(button* b = find_widget<button>(&window, "kick", false, false)) {
		b->set_retval(5);
	}

	if(button* b = find_widget<button>(&window, "ban", false, false)) {
		b->set_retval(6);
	}
}

} // namespace dialogs
} // namespace gui2
