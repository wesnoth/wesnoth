/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
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

#include "gui/dialogs/multiplayer/mp_alerts_options.hpp"

#include "desktop/notifications.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

#include "mp_ui_alerts.hpp"
#include "preferences/preferences.hpp"

#include <functional>

#include "gettext.hpp"

namespace gui2::dialogs
{

/**
 * Sets up the toggle buttons for a set of three MP lobby alerts
 *
 * @param pref_item_sound prefs_list constant for the sound preference
 * @param pref_item_notif prefs_list constant for the notification preference
 * @param pref_item_lobby prefs_list constant for the lobby preference
 */
#define SETUP_ITEMS(pref_item_sound, pref_item_notif, pref_item_lobby)                                                             \
{                                                                                                                                  \
toggle_button* sound = find_widget<toggle_button>(prefs_list::pref_item_sound, false, true);                              \
sound->set_value(prefs::get().pref_item_sound());                                                                                  \
connect_signal_mouse_left_click(*sound, [sound](auto&&...) { prefs::get().set_##pref_item_sound(sound->get_value_bool()); });    \
\
toggle_button* notif = find_widget<toggle_button>(prefs_list::pref_item_notif, false, true);                              \
\
if (!desktop::notifications::available()) {                                                                                        \
	notif->set_value(false);                                                                                                       \
	notif->set_active(false);                                                                                                      \
	prefs::get().set_##pref_item_notif(false);                                                                                     \
} else {                                                                                                                           \
	notif->set_active(true);                                                                                                       \
	notif->set_value(prefs::get().pref_item_notif());                                                                              \
	connect_signal_mouse_left_click(*notif, [notif](auto&&...) { prefs::get().set_##pref_item_notif(notif->get_value_bool()); });\
}                                                                                                                                  \
\
toggle_button* lobby = find_widget<toggle_button>(prefs_list::pref_item_lobby, false, true);                              \
lobby->set_value(prefs::get().pref_item_lobby());                                                                                  \
connect_signal_mouse_left_click(*lobby, [lobby](auto&&...) { prefs::get().set_##pref_item_lobby(lobby->get_value_bool()); });    \
}

/**
 * Set the defaults on the UI after clearing the preferences.
 *
 * @param pref_item_sound prefs_list constant for the sound preference
 * @param pref_item_notif prefs_list constant for the notification preference
 * @param pref_item_lobby prefs_list constant for the lobby preference
 */
#define RESET_DEFAULT(pref_item_sound, pref_item_notif, pref_item_lobby)                                                                 \
window.find_widget<toggle_button>(prefs_list::pref_item_sound).set_value(prefs::get().pref_item_sound());  \
window.find_widget<toggle_button>(prefs_list::pref_item_notif).set_value(prefs::get().pref_item_notif());  \
window.find_widget<toggle_button>(prefs_list::pref_item_lobby).set_value(prefs::get().pref_item_lobby());


static void revert_to_default_pref_values(window& window)
{
	// clear existing preferences for MP alerts
	prefs::get().clear_mp_alert_prefs();

	// each preference knows its own default, so after clearing you get the default by just using the getter
	RESET_DEFAULT(player_joins_sound, player_joins_notif, player_joins_lobby)
	RESET_DEFAULT(player_leaves_sound, player_leaves_notif, player_leaves_lobby)
	RESET_DEFAULT(private_message_sound, private_message_notif, private_message_lobby)
	RESET_DEFAULT(friend_message_sound, friend_message_notif, friend_message_lobby)
	RESET_DEFAULT(public_message_sound, public_message_notif, public_message_lobby)
	RESET_DEFAULT(server_message_sound, server_message_notif, server_message_lobby)
	RESET_DEFAULT(ready_for_start_sound, ready_for_start_notif, ready_for_start_lobby)
	RESET_DEFAULT(game_has_begun_sound, game_has_begun_notif, game_has_begun_lobby)
	RESET_DEFAULT(turn_changed_notif, turn_changed_notif, turn_changed_lobby)
	RESET_DEFAULT(game_created_sound, game_created_notif, game_created_lobby)
}

REGISTER_DIALOG(mp_alerts_options)

mp_alerts_options::mp_alerts_options()
	: modal_dialog(window_id())
{
}

void mp_alerts_options::pre_show()
{
	SETUP_ITEMS(player_joins_sound, player_joins_notif, player_joins_lobby)
	SETUP_ITEMS(player_leaves_sound, player_leaves_notif, player_leaves_lobby)
	SETUP_ITEMS(private_message_sound, private_message_notif, private_message_lobby)
	SETUP_ITEMS(friend_message_sound, friend_message_notif, friend_message_lobby)
	SETUP_ITEMS(public_message_sound, public_message_notif, public_message_lobby)
	SETUP_ITEMS(server_message_sound, server_message_notif, server_message_lobby)
	SETUP_ITEMS(ready_for_start_sound, ready_for_start_notif, ready_for_start_lobby)
	SETUP_ITEMS(game_has_begun_sound, game_has_begun_notif, game_has_begun_lobby)
	SETUP_ITEMS(turn_changed_notif, turn_changed_notif, turn_changed_lobby)
	SETUP_ITEMS(game_created_sound, game_created_notif, game_created_lobby)

	if (!desktop::notifications::available()) {
		label* nlabel = find_widget<label>("notification_label", false, true);
		nlabel->set_tooltip(_("This build of wesnoth does not include support for desktop notifications, contact your package manager"));
	}

	toggle_button* in_lobby;
	in_lobby = find_widget<toggle_button>(prefs_list::ready_for_start_lobby, false, true);
	in_lobby->set_visible(widget::visibility::invisible);

	in_lobby = find_widget<toggle_button>(prefs_list::game_has_begun_lobby, false, true);
	in_lobby->set_visible(widget::visibility::invisible);

	in_lobby = find_widget<toggle_button>(prefs_list::turn_changed_sound, false, true); // If we get a sound for this then don't remove this button
	in_lobby->set_visible(widget::visibility::invisible);

	in_lobby = find_widget<toggle_button>(prefs_list::turn_changed_lobby, false, true);
	in_lobby->set_visible(widget::visibility::invisible);

	button& defaults = find_widget<button>("revert_to_defaults");
	connect_signal_mouse_left_click(defaults, [this](auto&&...) { revert_to_default_pref_values(*this); });
}

void mp_alerts_options::post_show()
{
}

} // namespace dialogs
