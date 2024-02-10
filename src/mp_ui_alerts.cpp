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
#include "preferences/general.hpp"
#include "sound.hpp"

#include <string>
#include <vector>

namespace mp::ui_alerts
{
namespace
{
bool lobby_pref(const std::string& id)
{
	return preferences::get(id + "_lobby", get_def_pref_lobby(id));
}

bool sound_pref(const std::string& id)
{
	return preferences::get(id + "_sound", get_def_pref_sound(id));
}

bool notif_pref(const std::string& id)
{
	return preferences::get(id + "_notif", get_def_pref_notif(id));
}

const std::string _player_joins = "player_joins";
const std::string _player_leaves = "player_leaves";
const std::string _private_message = "private_message";
const std::string _friend_message = "friend_message";
const std::string _public_message = "public_message";
const std::string _server_message = "server_message";
const std::string _ready_for_start = "ready_for_start";
const std::string _game_has_begun = "game_has_begun";
const std::string _turn_changed = "turn_changed";
const std::string _game_created = "game_created";
} // end anonymous namespace

const std::vector<std::string> items{
	_player_joins,
	_player_leaves,
	_private_message,
	_friend_message,
	_public_message,
	_server_message,
	_ready_for_start,
	_game_has_begun,
	_turn_changed,
	_game_created,
};

void game_created(const std::string& scenario, const std::string& name)
{
	if(!lobby_pref(_game_created)) {
		return;
	}

	if(sound_pref(_game_created)) {
		sound::play_UI_sound(game_config::sounds::game_created);
	}

	if(notif_pref(_game_created)) {
		const std::string message = VGETTEXT("A game ($name|, $scenario|) has been created", {{"name", name}, {"scenario", scenario}});
		desktop::notifications::send(_("Wesnoth"), message, desktop::notifications::OTHER);
	}
}

void player_joins(bool is_lobby)
{
	if(is_lobby && !lobby_pref(_player_joins)) {
		return;
	}

	if(sound_pref(_player_joins)) {
		sound::play_UI_sound(game_config::sounds::player_joins);
	}

	if(notif_pref(_player_joins)) {
		desktop::notifications::send(_("Wesnoth"), _("A player has joined"), desktop::notifications::OTHER);
	}
}

void player_leaves(bool is_lobby)
{
	if(is_lobby && !lobby_pref(_player_leaves)) {
		return;
	}

	if(sound_pref(_player_leaves)) {
		sound::play_UI_sound(game_config::sounds::player_leaves);
	}

	if(notif_pref(_player_leaves)) {
		desktop::notifications::send(_("Wesnoth"), _("A player has left"), desktop::notifications::OTHER);
	}
}

void public_message(bool is_lobby, const std::string& sender, const std::string& message)
{
	if(is_lobby && !lobby_pref(_public_message)) {
		return;
	}

	if(sound_pref(_public_message)) {
		sound::play_UI_sound(game_config::sounds::public_message);
	}

	if(notif_pref(_public_message)) {
		desktop::notifications::send(sender, message, desktop::notifications::CHAT);
	}
}

void friend_message(bool is_lobby, const std::string& sender, const std::string& message)
{
	if(is_lobby && !lobby_pref(_friend_message)) {
		return;
	}

	if(sound_pref(_friend_message)) {
		sound::play_UI_sound(game_config::sounds::friend_message);
	}

	if(notif_pref(_friend_message)) {
		desktop::notifications::send(sender, message, desktop::notifications::CHAT);
	}
}

void private_message(bool is_lobby, const std::string& sender, const std::string& message)
{
	if(is_lobby && !lobby_pref(_private_message)) {
		return;
	}

	if(sound_pref(_private_message)) {
		sound::play_UI_sound(game_config::sounds::private_message);
	}

	if(notif_pref(_private_message)) {
		desktop::notifications::send(sender, message, desktop::notifications::CHAT);
	}
}

void server_message(bool is_lobby, const std::string& sender, const std::string& message)
{
	if(is_lobby && !lobby_pref(_server_message)) {
		return;
	}

	if(sound_pref(_server_message)) {
		sound::play_UI_sound(game_config::sounds::server_message);
	}

	if(notif_pref(_server_message)) {
		desktop::notifications::send(sender, message, desktop::notifications::CHAT);
	}
}

void ready_for_start()
{
	if(sound_pref(_ready_for_start)) {
		if(preferences::UI_sound_on()) {
			// this is play_bell instead of play_UI_sound to economize on sound channels. UI only has two
			// sounds, and turn bell has a dedicated channel.
			sound::play_bell(game_config::sounds::ready_for_start);
		}
	}

	if(notif_pref(_ready_for_start)) {
		desktop::notifications::send(_("Wesnoth"), _("Ready to start!"), desktop::notifications::OTHER);
	}
}

void game_has_begun()
{
	if(sound_pref(_game_has_begun)) {
		sound::play_UI_sound(game_config::sounds::game_has_begun);
	}

	if(notif_pref(_game_has_begun)) {
		desktop::notifications::send(_("Wesnoth"), _("Game has begun!"), desktop::notifications::OTHER);
	}
}

void turn_changed(const std::string& player_name)
{
	if(notif_pref(_turn_changed)) {
		utils::string_map player;
		player["name"] = player_name;
		desktop::notifications::send(_("Turn changed"), VGETTEXT("$name has taken control", player), desktop::notifications::TURN_CHANGED);
	}
}

bool get_def_pref_sound(const std::string& id)
{
	return (id != _public_message && id != _friend_message);
}

bool get_def_pref_notif(const std::string& id)
{
	return (desktop::notifications::available() && (
		id == _private_message ||
		id == _ready_for_start ||
		id == _game_has_begun ||
		id == _turn_changed ||
		id == _game_created
	));
}

bool get_def_pref_lobby(const std::string& id)
{
	return (id == _private_message || id == _server_message || id == _game_created);
}

} // end namespace mp_ui_alerts
