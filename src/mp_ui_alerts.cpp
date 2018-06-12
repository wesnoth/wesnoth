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

/**
 * This namespace provides handlers which play the sounds / notificaitons
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

namespace mp_ui_alerts {

namespace {

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

} // end anonymous namespace

// Note: This list must agree with data/gui/.../lobby_sound_options.cfg
const std::vector<std::string> items {"player_joins", "player_leaves", "private_message", "friend_message", "public_message", 
										"server_message", "ready_for_start", "game_has_begun", "turn_changed", "game_created"};

void game_created(bool is_lobby)
{
	std::string id = "game_created";

	sound::play_UI_sound(game_config::sounds::game_created);
	desktop::notifications::send(_("Wesnoth"), _("A game has been created"), desktop::notifications::OTHER);

	/*
	@lilinitsy: I don't know what to do with all that preference namespace
		stuff, so I'm just leaving that in for someone else to do.
		For now, it'll default to playing a sound & notifications.

	if(is_lobby && !lobby_pref(id))
	{
		return;
	} 

	if(sound_pref(id))
	{
		sound::play_UI_sound(game_config::sounds::game_created);
	}

	if(notif_pref(id))
	{
		desktop::notifications::send(_("Wesnoth"), _("A game has been created"), desktop::notifications::OTHER);
	}
	*/
}


void player_joins(bool is_lobby)
{
	std::string id = "player_enters";
	if (is_lobby && !lobby_pref(id)) {
		return ;
	}
	if (sound_pref(id)) {
		sound::play_UI_sound(game_config::sounds::player_joins);
	}
	if (notif_pref(id)) {
		desktop::notifications::send(_("Wesnoth"), _("A player has joined"), desktop::notifications::OTHER);
	}
}

void player_leaves(bool is_lobby)
{
	std::string id = "player_leaves";
	if (is_lobby && !lobby_pref(id)) {
		return ;
	}
	if (sound_pref(id)) {
		sound::play_UI_sound(game_config::sounds::player_leaves);
	}
	if (notif_pref(id)) {
		desktop::notifications::send(_("Wesnoth"), _("A player has left"), desktop::notifications::OTHER);
	}
}

void public_message(bool is_lobby, const std::string & sender, const std::string & message)
{
	std::string id = "public_message";
	if (is_lobby && !lobby_pref(id)) {
		return ;
	}
	if (sound_pref(id)) {
		sound::play_UI_sound(game_config::sounds::public_message);
	}
	if (notif_pref(id)) {
		desktop::notifications::send(sender, message, desktop::notifications::CHAT);
	}
}

void friend_message(bool is_lobby, const std::string & sender, const std::string & message)
{
	std::string id = "friend_message";
	if (is_lobby && !lobby_pref(id)) {
		return ;
	}
	if (sound_pref(id)) {
		sound::play_UI_sound(game_config::sounds::friend_message);
	}
	if (notif_pref(id)) {
		desktop::notifications::send(sender, message, desktop::notifications::CHAT);
	}
}

void private_message(bool is_lobby, const std::string & sender, const std::string & message)
{
	std::string id = "private_message";
	if (is_lobby && !lobby_pref(id)) {
		return ;
	}
	if (sound_pref(id)) {
		sound::play_UI_sound(game_config::sounds::private_message);
	}
	if (notif_pref(id)) {
		desktop::notifications::send(sender, message, desktop::notifications::CHAT);
	}
}

void server_message(bool is_lobby, const std::string & sender, const std::string & message)
{
	std::string id = "server_message";
	if (is_lobby && !lobby_pref(id)) {
		return ;
	}
	if (sound_pref(id)) {
		sound::play_UI_sound(game_config::sounds::server_message);
	}
	if (notif_pref(id)) {
		desktop::notifications::send(sender, message, desktop::notifications::CHAT);
	}
}

void ready_for_start()
{
	std::string id = "ready_for_start";
	if (sound_pref(id)) {
		if (preferences::UI_sound_on()) {
			sound::play_bell(game_config::sounds::ready_for_start); //this is play_bell instead of play_UI_sound to economize on sound channels. UI only has two sounds, and turn bell has a dedicated channel.
		}
	}
	if (notif_pref(id)) {
		desktop::notifications::send(_("Wesnoth"), _("Ready to start!"), desktop::notifications::OTHER);
	}
}

void game_has_begun()
{
	std::string id = "game_has_begun";
	if (sound_pref(id)) {
		sound::play_UI_sound(game_config::sounds::game_has_begun);
	}
	if (notif_pref(id)) {
		desktop::notifications::send(_("Wesnoth"), _ ("Game has begun!"), desktop::notifications::OTHER);
	}
}

void turn_changed(const std::string & player_name)
{
	std::string id = "turn_changed";
	if (notif_pref(id)) {
		utils::string_map player;
		player["name"] = player_name;
		desktop::notifications::send(_("Turn changed"), VGETTEXT("$name has taken control", player), desktop::notifications::TURN_CHANGED);
	}
}

bool get_def_pref_sound(const std::string & id) {
	return (id != "public_message" && id != "friend_message" && id != "game_created");
}

bool get_def_pref_notif(const std::string & id) {
	return (desktop::notifications::available() && (id == "private_message" || id == "ready_for_start" || id == "game_has_begun" || id == "turn_changed" || id == "game_created"));
}

bool get_def_pref_lobby(const std::string & id) {
	return (id == "private_message" || id == "server_message" || id == "game_created");
}


} // end namespace mp_ui_alerts
