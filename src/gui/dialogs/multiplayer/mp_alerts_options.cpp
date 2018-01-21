/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
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

#include "gui/dialogs/multiplayer/mp_alerts_options.hpp"

#include "desktop/notifications.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

#include "mp_ui_alerts.hpp"
#include "preferences/general.hpp"

#include "utils/functional.hpp"

#include "gettext.hpp"

namespace gui2
{
namespace dialogs
{
/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_mp_alerts_options
 *
 * == Lobby sounds options ==
 *
 * A Preferences subdialog permitting to configure the sounds and notifications
 * generated in response to various mp lobby / game events.
 *
 * @begin{table}{dialog_widgets}
 *
 * _label & & label & m &
 *        Item name. $
 *
 * _sound & & toggle_button & m &
 *        Toggles whether to play the item sound. $
 *
 * _notif & & toggle_button & m &
 *        Toggles whether to give a notification. $
 *
 * _lobby & & toggle_button & m &
 *        Toggles whether to take actions for this item when in the lobby. $
 *
 * @end{table}
 */

static toggle_button * setup_pref_toggle_button(const std::string & id, bool def, window & window)
{
	toggle_button * b = find_widget<toggle_button>(&window, id, false, true);
	b->set_value(preferences::get(id, def));

	//ensure we have yes / no for the toggle button, so that the preference matches the toggle button for sure.
	if (preferences::get(id).empty()) {
		preferences::set(id, def);
	}

	connect_signal_mouse_left_click(*b, std::bind([b, id]() { preferences::set(id, b->get_value_bool()); }));

	return b;
}

static void setup_item(const std::string & item, window & window)
{
	// Set up the sound checkbox
	setup_pref_toggle_button(item+"_sound", mp_ui_alerts::get_def_pref_sound(item), window);

	// Set up the notification checkbox
	toggle_button * notif = setup_pref_toggle_button(item+"_notif", mp_ui_alerts::get_def_pref_notif(item), window);

	// Check if desktop notifications are available
	if (!desktop::notifications::available()) {
		notif->set_value(false);
		notif->set_active(false);
		preferences::set(item+"_notif", false);
	} else {
		notif->set_active(true);
	}

	// Set up the in_lobby checkbox
	setup_pref_toggle_button(item+"_lobby", mp_ui_alerts::get_def_pref_lobby(item), window);
}

static void set_pref_and_button(const std::string & id, bool value, window & window)
{
	preferences::set(id,value);
	toggle_button * button = find_widget<toggle_button>(&window, id, false, true);
	button->set_value(value);
}

static void revert_to_default_pref_values(window & window)
{
	for (const std::string & i : mp_ui_alerts::items) {
		set_pref_and_button(i+"_sound", mp_ui_alerts::get_def_pref_sound(i), window);
		set_pref_and_button(i+"_notif", mp_ui_alerts::get_def_pref_notif(i), window);
		set_pref_and_button(i+"_lobby", mp_ui_alerts::get_def_pref_lobby(i), window);
	}
}

REGISTER_DIALOG(mp_alerts_options)

mp_alerts_options::mp_alerts_options()
{
}

void mp_alerts_options::pre_show(window& window)
{
	for (const std::string & i : mp_ui_alerts::items) {
		setup_item(i, window);
	}

	if (!desktop::notifications::available()) {
		label * nlabel = find_widget<label>(&window, "notification_label", false, true);
		nlabel->set_tooltip(_("This build of wesnoth does not include support for desktop notifications, contact your package manager"));
	}

	toggle_button * in_lobby;
	in_lobby = find_widget<toggle_button>(&window,"ready_for_start_lobby", false, true);
	in_lobby->set_visible(widget::visibility::invisible);

	in_lobby = find_widget<toggle_button>(&window,"game_has_begun_lobby", false, true);
	in_lobby->set_visible(widget::visibility::invisible);

	in_lobby = find_widget<toggle_button>(&window,"turn_changed_sound", false, true); // If we get a sound for this then don't remove this button
	in_lobby->set_visible(widget::visibility::invisible);

	in_lobby = find_widget<toggle_button>(&window,"turn_changed_lobby", false, true);
	in_lobby->set_visible(widget::visibility::invisible);

	button * defaults;
	defaults = find_widget<button>(&window,"revert_to_defaults", false, true);
	connect_signal_mouse_left_click(*defaults, std::bind(&revert_to_default_pref_values, std::ref(window)));
}

void mp_alerts_options::post_show(window& /*window*/)
{
}

} // namespace dialogs
} // namespace gui2
