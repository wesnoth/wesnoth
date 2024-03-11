/*
	Copyright (C) 2017 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "chat_command_handler.hpp"

#include "chat_events.hpp"
#include "game_initialization/multiplayer.hpp"
#include "game_version.hpp"
#include "gui/dialogs/multiplayer/mp_report.hpp"
#include "gui/dialogs/preferences_dialog.hpp"
#include "map_command_handler.hpp"
#include "preferences/game.hpp"

namespace events {

bool chat_command_handler::is_enabled(const map_command_handler<chat_command_handler>::command& c) const
{
	return !(c.has_flag('A') && !mp::logged_in_as_moderator());
}

void chat_command_handler::print(const std::string& title, const std::string& message)
{
	chat_handler_.add_chat_message(std::time(nullptr), title, 0, message);
}

void chat_command_handler::do_emote()
{
	chat_handler_.send_chat_message("/me " + get_data(), allies_only_);
}

void chat_command_handler::do_network_send()
{
	chat_handler_.send_command(get_cmd(), get_data());
}

void chat_command_handler::do_network_send(const std::string& data)
{
	chat_handler_.send_command(get_cmd(), data);
}

void chat_command_handler::do_network_send_req_arg()
{
	if (get_data(1).empty()) return command_failed_need_arg(1);
	do_network_send();
}

void chat_command_handler::do_whisper()
{
	if (get_data(1).empty()) return command_failed_need_arg(1);
	if (get_data(2).empty()) return command_failed_need_arg(2);
	chat_handler_.send_whisper(get_arg(1), get_data(2));
	chat_handler_.add_whisper_sent(get_arg(1), get_data(2));
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

		if (preferences::add_acquaintance(get_arg(1), "ignore", get_data(2)).first) {
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

		if (preferences::add_acquaintance(get_arg(1), "friend", get_data(2)).first) {
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
	gui2::dialogs::preferences_dialog::display(preferences::VIEW_FRIENDS);
}

void chat_command_handler::do_version() {
	print(_("version"), game_config::revision);
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

void chat_command_handler::do_clear_messages() {
	chat_handler_.clear_messages();
}

void chat_command_handler::do_mp_report() {
	std::string report_text;
	gui2::dialogs::mp_report::execute(report_text);

	if(!report_text.empty()) {
		do_network_send(report_text);
	}
}

}
