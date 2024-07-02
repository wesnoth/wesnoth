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
namespace
{
bool lobby_pref(const std::string& id)
{
	return prefs::get().mp_alert_option(id, "lobby", get_def_pref_lobby(id));
}

bool sound_pref(const std::string& id)
{
	return prefs::get().mp_alert_option(id, "sound", get_def_pref_sound(id));
}

bool notif_pref(const std::string& id)
{
	return prefs::get().mp_alert_option(id, "notif", get_def_pref_notif(id));
}
} // end anonymous namespace

const std::vector<std::string> items{
	pref_constants::player_joins,
	pref_constants::player_leaves,
	pref_constants::private_message,
	pref_constants::friend_message,
	pref_constants::public_message,
	pref_constants::server_message,
	pref_constants::ready_for_start,
	pref_constants::game_has_begun,
	pref_constants::turn_changed,
	pref_constants::game_created,
};

void game_created(const std::string& scenario, const std::string& name)
{
	if(!lobby_pref(pref_constants::game_created)) {
		return;
	}

	if(sound_pref(pref_constants::game_created)) {
		sound::play_UI_sound(game_config::sounds::game_created);
	}

	if(notif_pref(pref_constants::game_created)) {
		const std::string message = VGETTEXT("A game ($name|, $scenario|) has been created", {{"name", name}, {"scenario", scenario}});
		desktop::notifications::send(_("Wesnoth"), message, desktop::notifications::OTHER);
	}
}

void player_joins(bool is_lobby)
{
	if(is_lobby && !lobby_pref(pref_constants::player_joins)) {
		return;
	}

	if(sound_pref(pref_constants::player_joins)) {
		sound::play_UI_sound(game_config::sounds::player_joins);
	}

	if(notif_pref(pref_constants::player_joins)) {
		desktop::notifications::send(_("Wesnoth"), _("A player has joined"), desktop::notifications::OTHER);
	}
}

void player_leaves(bool is_lobby)
{
	if(is_lobby && !lobby_pref(pref_constants::player_leaves)) {
		return;
	}

	if(sound_pref(pref_constants::player_leaves)) {
		sound::play_UI_sound(game_config::sounds::player_leaves);
	}

	if(notif_pref(pref_constants::player_leaves)) {
		desktop::notifications::send(_("Wesnoth"), _("A player has left"), desktop::notifications::OTHER);
	}
}

void public_message(bool is_lobby, const std::string& sender, const std::string& message)
{
	if(is_lobby && !lobby_pref(pref_constants::public_message)) {
		return;
	}

	if(sound_pref(pref_constants::public_message)) {
		sound::play_UI_sound(game_config::sounds::public_message);
	}

	if(notif_pref(pref_constants::public_message)) {
		desktop::notifications::send(sender, message, desktop::notifications::CHAT);
	}
}

void friend_message(bool is_lobby, const std::string& sender, const std::string& message)
{
	if(is_lobby && !lobby_pref(pref_constants::friend_message)) {
		return;
	}

	if(sound_pref(pref_constants::friend_message)) {
		sound::play_UI_sound(game_config::sounds::friend_message);
	}

	if(notif_pref(pref_constants::friend_message)) {
		desktop::notifications::send(sender, message, desktop::notifications::CHAT);
	}
}

void private_message(bool is_lobby, const std::string& sender, const std::string& message)
{
	if(is_lobby && !lobby_pref(pref_constants::private_message)) {
		return;
	}

	if(sound_pref(pref_constants::private_message)) {
		sound::play_UI_sound(game_config::sounds::private_message);
	}

	if(notif_pref(pref_constants::private_message)) {
		desktop::notifications::send(sender, message, desktop::notifications::CHAT);
	}
}

void server_message(bool is_lobby, const std::string& sender, const std::string& message)
{
	if(is_lobby && !lobby_pref(pref_constants::server_message)) {
		return;
	}

	if(sound_pref(pref_constants::server_message)) {
		sound::play_UI_sound(game_config::sounds::server_message);
	}

	if(notif_pref(pref_constants::server_message)) {
		desktop::notifications::send(sender, message, desktop::notifications::CHAT);
	}
}

void ready_for_start()
{
	if(sound_pref(pref_constants::ready_for_start)) {
		if(prefs::get().ui_sound_on()) {
			// this is play_bell instead of play_UI_sound to economize on sound channels. UI only has two
			// sounds, and turn bell has a dedicated channel.
			sound::play_bell(game_config::sounds::ready_for_start);
		}
	}

	if(notif_pref(pref_constants::ready_for_start)) {
		desktop::notifications::send(_("Wesnoth"), _("Ready to start!"), desktop::notifications::OTHER);
	}
}

void game_has_begun()
{
	if(sound_pref(pref_constants::game_has_begun)) {
		sound::play_UI_sound(game_config::sounds::game_has_begun);
	}

	if(notif_pref(pref_constants::game_has_begun)) {
		desktop::notifications::send(_("Wesnoth"), _("Game has begun!"), desktop::notifications::OTHER);
	}
}

void turn_changed(const std::string& player_name)
{
	if(notif_pref(pref_constants::turn_changed)) {
		utils::string_map player;
		player["name"] = player_name;
		desktop::notifications::send(_("Turn changed"), VGETTEXT("$name has taken control", player), desktop::notifications::TURN_CHANGED);
	}
}

bool get_def_pref_sound(const std::string& id)
{
	return (id != pref_constants::public_message && id != pref_constants::friend_message);
}

bool get_def_pref_notif(const std::string& id)
{
	return (desktop::notifications::available() && (
		id == pref_constants::private_message ||
		id == pref_constants::ready_for_start ||
		id == pref_constants::game_has_begun ||
		id == pref_constants::turn_changed ||
		id == pref_constants::game_created
	));
}

bool get_def_pref_lobby(const std::string& id)
{
	return (id == pref_constants::private_message || id == pref_constants::server_message || id == pref_constants::game_created);
}

} // end namespace mp_ui_alerts
