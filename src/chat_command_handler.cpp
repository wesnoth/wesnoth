/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/preferences_dialog.hpp"
#include "map_command_handler.hpp"
#include "chat_command_handler.hpp"
#include "chat_events.hpp"
#include "game_preferences.hpp"
#include "preferences_display.hpp"
#include "video.hpp"
#include "game_config_manager.hpp"

namespace events {

bool chat_command_handler::is_enabled(const map_command_handler<chat_command_handler>::command& c) const
{
	return !(c.has_flag('A') && !preferences::is_authenticated());
}

void chat_command_handler::print(const std::string& title, const std::string& message)
{
	chat_handler_.add_chat_message(time(nullptr), title, 0, message);
}


void chat_command_handler::do_emote()
{
	chat_handler_.send_chat_message("/me " + get_data(), allies_only_);
}

void chat_command_handler::do_network_send()
{
	chat_handler_.send_command(get_cmd(), get_data());
}

void chat_command_handler::do_network_send_req_arg()
{
	if (get_data(1).empty()) return command_failed_need_arg(1);
	do_network_send();
}

void chat_command_handler::do_room_query_noarg()
{
	config data;
	config& q = data.add_child("room_query");
	q.add_child(get_cmd());
	chat_handler_.send_to_server(data);
}

void chat_command_handler::do_room_query()
{
	if (get_data(1).empty()) return command_failed_need_arg(1);
	config data;
	config& q = data.add_child("room_query");
	q["room"] = get_arg(1);
	q.add_child(get_cmd());
	chat_handler_.send_to_server(data);
}

void chat_command_handler::do_gen_room_query()
{
	if (get_data(1).empty()) return command_failed_need_arg(1);
	config data;
	config& q = data.add_child("room_query");
	q["room"] = get_arg(1);
	config& c = q.add_child(get_arg(2));
	c["value"] = get_data(3);
	chat_handler_.send_to_server(data);
}

void chat_command_handler::do_whisper()
{
	if (get_data(1).empty()) return command_failed_need_arg(1);
	if (get_data(2).empty()) return command_failed_need_arg(2);
	chat_handler_.send_whisper(get_arg(1), get_data(2));
	chat_handler_.add_whisper_sent(get_arg(1), get_data(2));
}

void chat_command_handler::do_chanmsg()
{
	if (get_data(1).empty()) return command_failed_need_arg(1);
	if (get_data(2).empty()) return command_failed_need_arg(2);
	chat_handler_.send_chat_room_message(get_arg(1), get_data(2));
	chat_handler_.add_chat_room_message_sent(get_arg(1), get_data(2));
}

void chat_command_handler::do_log()
{
	chat_handler_.change_logging(get_data());
}

void chat_command_handler::do_ignore()
{
	if (get_arg(1).empty()) {
		do_display();
	}
	else {
		utils::string_map symbols;
		symbols["nick"] = get_arg(1);

		if (preferences::add_acquaintance(get_arg(1), "ignore", get_data(2))) {
			print(_("ignores list"), VGETTEXT("Added to ignore list: $nick", symbols));
			chat_handler_.user_relation_changed(get_arg(1));
		}
		else {
			command_failed(VGETTEXT("Invalid username: $nick", symbols));
		}
	}
}

void chat_command_handler::do_friend()
{
	if (get_arg(1).empty()) {
		do_display();
	}
	else {
		utils::string_map symbols;
		symbols["nick"] = get_arg(1);

		if (preferences::add_acquaintance(get_arg(1), "friend", get_data(2))) {
			print(_("friends list"), VGETTEXT("Added to friends list: $nick", symbols));
			chat_handler_.user_relation_changed(get_arg(1));
		}
		else {
			command_failed(VGETTEXT("Invalid username: $nick", symbols));
		}
	}
}

void chat_command_handler::do_remove()
{
	for (int i = 1;!get_arg(i).empty();i++) {
		preferences::remove_acquaintance(get_arg(i));
		chat_handler_.user_relation_changed(get_arg(i));
		utils::string_map symbols;
		symbols["nick"] = get_arg(i);
		print(_("friends and ignores list"), VGETTEXT("Removed from list: $nick", symbols));
	}
}

void chat_command_handler::do_display()
{
	// TODO: add video and game config argument to chat_command_handler?
	gui2::dialogs::preferences_dialog::display(CVideo::get_singleton(), game_config_manager::get()->game_config(),
		preferences::VIEW_FRIENDS);
}

void chat_command_handler::do_version() {
	print(_("version"), game_config::revision);
}

void chat_command_handler::do_register() {
	config data;
	config& nickserv = data.add_child("nickserv");

	if (get_data(1).empty()) return command_failed_need_arg(1);

	config &reg = nickserv.add_child("register");
	reg["password"] = get_arg(1);
	if (!get_data(2).empty()) {
		reg["mail"] = get_arg(2);
	}
	std::string msg;
	if (get_data(2).empty()) {
		msg = _("registering with password *** and no email address");
	}
	else {
		utils::string_map symbols;
		symbols["email"] = get_data(2);
		msg = VGETTEXT("registering with password *** and "
			"email address $email", symbols);
	}
	print(_("nick registration"), msg);

	chat_handler_.send_to_server(data);
}

void chat_command_handler::do_drop() {
	config data;
	config& nickserv = data.add_child("nickserv");

	nickserv.add_child("drop");

	print(_("nick registration"), _("dropping your username"));

	chat_handler_.send_to_server(data);
}

void chat_command_handler::do_set() {
	config data;
	config& nickserv = data.add_child("nickserv");

	if (get_data(1).empty()) return command_failed_need_arg(1);
	if (get_data(2).empty()) return command_failed_need_arg(2);

	config &set = nickserv.add_child("set");
	set["detail"] = get_arg(1);
	set["value"] = get_data(2);
	utils::string_map symbols;
	symbols["var"] = get_arg(1);
	symbols["value"] = get_arg(2);
	print(_("nick registration"), VGETTEXT("setting $var to $value", symbols));

	chat_handler_.send_to_server(data);
}

void chat_command_handler::do_info() {
	if (get_data(1).empty()) return command_failed_need_arg(1);

	config data;
	config& nickserv = data.add_child("nickserv");

	nickserv.add_child("info")["name"] = get_data(1);
	utils::string_map symbols;
	symbols["nick"] = get_arg(1);
	print(_("nick registration"), VGETTEXT("requesting information for user $nick", symbols));

	chat_handler_.send_to_server(data);
}

void chat_command_handler::do_details() {

	config data;
	config& nickserv = data.add_child("nickserv");
	nickserv.add_child("details");

	chat_handler_.send_to_server(data);
}

}
