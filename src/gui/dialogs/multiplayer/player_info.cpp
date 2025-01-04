/*
	Copyright (C) 2009 - 2024
	by Tomasz Sniatowski <kailoran@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "gui/dialogs/multiplayer/player_info.hpp"

#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

#include "preferences/preferences.hpp"
#include "game_initialization/multiplayer.hpp"
#include "gettext.hpp"

#include <functional>

namespace gui2::dialogs
{

REGISTER_DIALOG(lobby_player_info)

lobby_player_info::lobby_player_info(events::chat_handler& chat,
									   const mp::user_info& info,
									   const mp::lobby_info& li)
	: modal_dialog(window_id())
	, chat_(chat)
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

void lobby_player_info::pre_show()
{
	relation_ = find_widget<label>("relation_info", false, true);

	button& whisper = find_widget<button>("start_whisper");
	if(info_.get_relation() != mp::user_info::user_relation::ME) {
		connect_signal_mouse_left_click(whisper,
			std::bind(&lobby_player_info::start_whisper_button_callback, this));
	} else {
		whisper.set_active(false);
	}

	add_to_friends_ = find_widget<button>("add_to_friends", false, true);
	connect_signal_mouse_left_click(
			*add_to_friends_,
			std::bind(&lobby_player_info::add_to_friends_button_callback, this));

	add_to_ignores_ = find_widget<button>("add_to_ignores", false, true);
	connect_signal_mouse_left_click(
			*add_to_ignores_,
			std::bind(&lobby_player_info::add_to_ignores_button_callback, this));

	remove_from_list_
			= find_widget<button>("remove_from_list", false, true);
	connect_signal_mouse_left_click(
			*remove_from_list_,
			std::bind(&lobby_player_info::remove_from_list_button_callback, this));

	connect_signal_mouse_left_click(
			find_widget<button>("check_status"),
			std::bind(&lobby_player_info::check_status_button_callback, this));

	connect_signal_mouse_left_click(
			find_widget<button>("kick"),
			std::bind(&lobby_player_info::kick_button_callback, this));

	connect_signal_mouse_left_click(
			find_widget<button>("kick_ban"),
			std::bind(&lobby_player_info::kick_ban_button_callback, this));

	connect_signal_mouse_left_click(
			find_widget<button>("stopgame"),
			std::bind(&lobby_player_info::stopgame_button_callback, this));

	find_widget<label>("player_name").set_label(info_.name);

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

	time_ = find_widget<text_box>("time", false, true);
	reason_ = find_widget<text_box>("reason", false, true);
	add_to_tab_order(reason_);
	add_to_tab_order(time_);

	find_widget<label>("location_info").set_label(loc.str());

	update_relation();

	if(!mp::logged_in_as_moderator()) {
		widget* aw = find("admin", false);
		aw->set_visible(widget::visibility::invisible);
	}
}

void lobby_player_info::post_show()
{
}

void lobby_player_info::update_relation()
{
	add_to_friends_->set_active(false);
	add_to_ignores_->set_active(false);
	remove_from_list_->set_active(false);
	switch(info_.get_relation()) {
		case mp::user_info::user_relation::FRIEND:
			relation_->set_label(_("On friends list"));
			add_to_ignores_->set_active(true);
			remove_from_list_->set_active(true);
			break;
		case mp::user_info::user_relation::IGNORED:
			relation_->set_label(_("On ignores list"));
			add_to_friends_->set_active(true);
			remove_from_list_->set_active(true);
			break;
		case mp::user_info::user_relation::NEUTRAL:
			relation_->set_label(_("Neither a friend nor ignored"));
			add_to_friends_->set_active(true);
			add_to_ignores_->set_active(true);
			break;
		case mp::user_info::user_relation::ME:
			relation_->set_label(_("You"));
			break;
		default:
			relation_->set_label(_("Error"));
	}
}

void lobby_player_info::add_to_friends_button_callback()
{
	prefs::get().add_acquaintance(info_.name, "friend", "");
	update_relation();
}

void lobby_player_info::add_to_ignores_button_callback()
{
	prefs::get().add_acquaintance(info_.name, "ignore", "");
	update_relation();
}

void lobby_player_info::remove_from_list_button_callback()
{
	prefs::get().remove_acquaintance(info_.name);
	update_relation();
}

void lobby_player_info::start_whisper_button_callback()
{
	result_open_whisper_ = true;
	close();
}

void lobby_player_info::check_status_button_callback()
{
	chat_.send_command("query", "status " + info_.name);
	close();
}

void lobby_player_info::kick_button_callback()
{
	do_kick_ban(false);
	close();
}

void lobby_player_info::kick_ban_button_callback()
{
	do_kick_ban(true);
	close();
}

void lobby_player_info::stopgame_button_callback()
{
	do_stopgame();
	close();
}

void lobby_player_info::do_stopgame()
{
	std::stringstream ss;
	ss << "stopgame " << info_.name;
	if(!reason_->get_value().empty()) {
		ss << " " << reason_->get_value();
	}

	chat_.send_command("query", ss.str());
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
