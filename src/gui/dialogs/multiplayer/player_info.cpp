/*
   Copyright (C) 2009 - 2018 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/multiplayer/player_info.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

#include "preferences/game.hpp"
#include "gettext.hpp"

#include "utils/functional.hpp"

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(lobby_player_info)

lobby_player_info::lobby_player_info(events::chat_handler& chat,
									   mp::user_info& info,
									   const mp::lobby_info& li)
	: chat_(chat)
	, info_(info)
	, reason_(nullptr)
	, time_(nullptr)
	, relation_(nullptr)
	, add_to_friends_(nullptr)
	, add_to_ignores_(nullptr)
	, remove_from_list_(nullptr)
	, result_open_whisper_(false)
	, lobby_info_(li)
{
}

lobby_player_info::~lobby_player_info()
{
}

void lobby_player_info::pre_show(window& window)
{
	relation_ = find_widget<label>(&window, "relation_info", false, true);
	connect_signal_mouse_left_click(
			find_widget<button>(&window, "start_whisper", false),
			std::bind(&lobby_player_info::start_whisper_button_callback,
						this,
						std::ref(window)));

	add_to_friends_ = find_widget<button>(&window, "add_to_friends", false, true);
	connect_signal_mouse_left_click(
			*add_to_friends_,
			std::bind(&lobby_player_info::add_to_friends_button_callback, this));

	add_to_ignores_ = find_widget<button>(&window, "add_to_ignores", false, true);
	connect_signal_mouse_left_click(
			*add_to_ignores_,
			std::bind(&lobby_player_info::add_to_ignores_button_callback, this));

	remove_from_list_
			= find_widget<button>(&window, "remove_from_list", false, true);
	connect_signal_mouse_left_click(
			*remove_from_list_,
			std::bind(&lobby_player_info::remove_from_list_button_callback, this));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "check_status", false),
			std::bind(&lobby_player_info::check_status_button_callback,
						this,
						std::ref(window)));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "kick", false),
			std::bind(&lobby_player_info::kick_button_callback,
						this,
						std::ref(window)));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "kick_ban", false),
			std::bind(&lobby_player_info::kick_ban_button_callback,
						this,
						std::ref(window)));

	find_widget<label>(&window, "player_name", false).set_label(info_.name);

	std::stringstream loc;
	const mp::game_info* game = lobby_info_.get_game_by_id(info_.game_id);
	if(game != nullptr) {
		loc << _("In game:") << " " << game->name << " ";
		if(info_.observing) {
			loc << _("(observing)");
		} else {
			loc << _("(playing)");
		}
	} else {
		loc << _("In lobby");
	}

	time_ = find_widget<text_box>(&window, "time", false, true);
	reason_ = find_widget<text_box>(&window, "reason", false, true);
	window.add_to_tab_order(reason_);
	window.add_to_tab_order(time_);

	find_widget<label>(&window, "location_info", false).set_label(loc.str());

	update_relation();

	if(!preferences::is_authenticated()) {
		widget* aw = window.find("admin", false);
		aw->set_visible(widget::visibility::invisible);
	}
}

void lobby_player_info::post_show(window& /*window*/)
{
}

void lobby_player_info::update_relation()
{
	add_to_friends_->set_active(false);
	add_to_ignores_->set_active(false);
	remove_from_list_->set_active(false);
	switch(info_.relation) {
		case mp::user_info::FRIEND:
			relation_->set_label(_("On friends list"));
			add_to_ignores_->set_active(true);
			remove_from_list_->set_active(true);
			break;
		case mp::user_info::IGNORED:
			relation_->set_label(_("On ignores list"));
			add_to_friends_->set_active(true);
			remove_from_list_->set_active(true);
			break;
		case mp::user_info::NEUTRAL:
			relation_->set_label(_("Neither a friend nor ignored"));
			add_to_friends_->set_active(true);
			add_to_ignores_->set_active(true);
			break;
		case mp::user_info::ME:
			relation_->set_label(_("You"));
			break;
		default:
			relation_->set_label(_("Error"));
	}
}

void lobby_player_info::add_to_friends_button_callback()
{
	preferences::add_acquaintance(info_.name, "friend", "");
	info_.relation = mp::user_info::FRIEND;
	update_relation();
}

void lobby_player_info::add_to_ignores_button_callback()
{
	preferences::add_acquaintance(info_.name, "ignore", "");
	info_.relation = mp::user_info::IGNORED;
	update_relation();
}

void lobby_player_info::remove_from_list_button_callback()
{
	preferences::remove_acquaintance(info_.name);
	info_.relation = mp::user_info::NEUTRAL;
	update_relation();
}

void lobby_player_info::start_whisper_button_callback(window& w)
{
	result_open_whisper_ = true;
	w.close();
}

void lobby_player_info::check_status_button_callback(window& w)
{
	chat_.send_command("query", "status " + info_.name);
	w.close();
}

void lobby_player_info::kick_button_callback(window& w)
{
	do_kick_ban(false);
	w.close();
}

void lobby_player_info::kick_ban_button_callback(window& w)
{
	do_kick_ban(true);
	w.close();
}

void lobby_player_info::do_kick_ban(bool ban)
{
	std::stringstream ss;
	ss << (ban ? "kban " : "kick ") << info_.name;
	if(ban && !time_->get_value().empty()) {
		ss << " " << time_->get_value();
	}
	if(!reason_->get_value().empty()) {
		ss << " " << reason_->get_value();
	}

	chat_.send_command("query", ss.str());
}

} // namespace dialogs
} // namespace gui2
