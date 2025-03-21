/*
	Copyright (C) 2014 - 2025
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

/**
 * This namespace provides handlers which play the sounds / notifications
 * for various mp server events, depending on the preference configuration.
 */

#include "mp_ui_alerts.hpp"

#include "desktop/notifications.hpp"
#include "formula/string_utils.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "preferences/preferences.hpp"
#include "sound.hpp"

#include <string>
#include <vector>

namespace mp::ui_alerts
{
void game_created(const std::string& scenario, const std::string& name)
{
	if(!prefs::get().game_created_lobby()) {
		return;
	}

	if(prefs::get().game_created_sound()) {
		sound::play_UI_sound(game_config::sounds::game_created);
	}

	if(prefs::get().game_created_notif()) {
		const std::string message = VGETTEXT("A game ($name|, $scenario|) has been created", {{"name", name}, {"scenario", scenario}});
		desktop::notifications::send(_("Wesnoth"), message, desktop::notifications::OTHER);
	}
}

void player_joins(bool is_lobby)
{
	if(is_lobby && !prefs::get().player_joins_lobby()) {
		return;
	}

	if(prefs::get().player_joins_sound()) {
		sound::play_UI_sound(game_config::sounds::player_joins);
	}

	if(prefs::get().player_joins_notif()) {
		desktop::notifications::send(_("Wesnoth"), _("A player has joined"), desktop::notifications::OTHER);
	}
}

void player_leaves(bool is_lobby)
{
	if(is_lobby && !prefs::get().player_leaves_lobby()) {
		return;
	}

	if(prefs::get().player_leaves_sound()) {
		sound::play_UI_sound(game_config::sounds::player_leaves);
	}

	if(prefs::get().player_leaves_notif()) {
		desktop::notifications::send(_("Wesnoth"), _("A player has left"), desktop::notifications::OTHER);
	}
}

void public_message(bool is_lobby, const std::string& sender, const std::string& message)
{
	if(is_lobby && !prefs::get().public_message_lobby()) {
		return;
	}

	if(prefs::get().public_message_sound()) {
		sound::play_UI_sound(game_config::sounds::public_message);
	}

	if(prefs::get().public_message_notif()) {
		desktop::notifications::send(sender, message, desktop::notifications::CHAT);
	}
}

void friend_message(bool is_lobby, const std::string& sender, const std::string& message)
{
	if(is_lobby && !prefs::get().friend_message_lobby()) {
		return;
	}

	if(prefs::get().friend_message_sound()) {
		sound::play_UI_sound(game_config::sounds::friend_message);
	}

	if(prefs::get().friend_message_notif()) {
		desktop::notifications::send(sender, message, desktop::notifications::CHAT);
	}
}

void private_message(bool is_lobby, const std::string& sender, const std::string& message)
{
	if(is_lobby && !prefs::get().private_message_lobby()) {
		return;
	}

	if(prefs::get().private_message_sound()) {
		sound::play_UI_sound(game_config::sounds::private_message);
	}

	if(prefs::get().private_message_notif()) {
		desktop::notifications::send(sender, message, desktop::notifications::CHAT);
	}
}

void server_message(bool is_lobby, const std::string& sender, const std::string& message)
{
	if(is_lobby && !prefs::get().server_message_lobby()) {
		return;
	}

	if(prefs::get().server_message_sound()) {
		sound::play_UI_sound(game_config::sounds::server_message);
	}

	if(prefs::get().server_message_notif()) {
		desktop::notifications::send(sender, message, desktop::notifications::CHAT);
	}
}

void ready_for_start()
{
	if(prefs::get().ready_for_start_sound()) {
		if(prefs::get().ui_sound_on()) {
			// this is play_bell instead of play_UI_sound to economize on sound channels. UI only has two
			// sounds, and turn bell has a dedicated channel.
			sound::play_bell(game_config::sounds::ready_for_start);
		}
	}

	if(prefs::get().ready_for_start_notif()) {
		desktop::notifications::send(_("Wesnoth"), _("Ready to start!"), desktop::notifications::OTHER);
	}
}

void game_has_begun()
{
	if(prefs::get().game_has_begun_sound()) {
		sound::play_UI_sound(game_config::sounds::game_has_begun);
	}

	if(prefs::get().game_has_begun_notif()) {
		desktop::notifications::send(_("Wesnoth"), _("Game has begun!"), desktop::notifications::OTHER);
	}
}

void turn_changed(const std::string& player_name)
{
	if(prefs::get().turn_changed_notif()) {
		utils::string_map player;
		player["name"] = player_name;
		desktop::notifications::send(_("Turn changed"), VGETTEXT("$name has taken control", player), desktop::notifications::TURN_CHANGED);
	}
}

} // end namespace mp_ui_alerts
