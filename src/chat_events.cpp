/*
	Copyright (C) 2017 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "chat_events.hpp"

#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "chat_command_handler.hpp"
#include "preferences/preferences.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

namespace events {

chat_handler::chat_handler()
{
}

chat_handler::~chat_handler()
{
}

/**
* Change the log level of a log domain.
*
* @param data string of the form: "@<level@> @<domain@>"
*/
void chat_handler::change_logging(const std::string& data) {
	const std::string::const_iterator j =
		std::find(data.begin(), data.end(), ' ');
	if (j == data.end()) return;
	const std::string level(data.begin(), j);
	const std::string domain(j + 1, data.end());
	lg::severity severity;
	if (level == "error") severity = lg::err().get_severity();
	else if (level == "warning") severity = lg::warn().get_severity();
	else if (level == "info") severity = lg::info().get_severity();
	else if (level == "debug") severity = lg::debug().get_severity();
	else {
		utils::string_map symbols;
		symbols["level"] = level;
		const std::string& msg =
			VGETTEXT("Unknown debug level: '$level'.", symbols);
		ERR_NG << msg;
		add_chat_message(std::chrono::system_clock::now(), _("error"), 0, msg);
		return;
	}
	if (!lg::set_log_domain_severity(domain, severity)) {
		utils::string_map symbols;
		symbols["domain"] = domain;
		const std::string& msg =
			VGETTEXT("Unknown debug domain: '$domain'.", symbols);
		ERR_NG << msg;
		add_chat_message(std::chrono::system_clock::now(), _("error"), 0, msg);
		return;
	}
	else {
		utils::string_map symbols;
		symbols["level"] = level;
		symbols["domain"] = domain;
		const std::string& msg =
			VGETTEXT("Switched domain: '$domain' to level: '$level'.", symbols);
		LOG_NG << msg;
		add_chat_message(std::chrono::system_clock::now(), "log", 0, msg);
	}
}

void chat_handler::send_command(const std::string& cmd, const std::string& args /* = "" */) {
	config data;
	if (cmd == "muteall") {
		data.add_child(cmd);
	}
	else if (cmd == "query") {
		data.add_child(cmd)["type"] = args;
	}
	else if (cmd == "ban" || cmd == "unban" || cmd == "kick"
		|| cmd == "mute" || cmd == "unmute") {
		data.add_child(cmd)["username"] = args;
	}
	else if (cmd == "ping") {
		// Not using serialize_timestamp here since we need the steady clock
		auto now = std::chrono::steady_clock::now();
		data.add_child("ping")["requested_at"] = now.time_since_epoch();
	}
	else if (cmd == "report") {
		data.add_child("query")["type"] = "report " + args;
	}
	else if (cmd == "roll") {
		data.add_child("query")["type"] = "roll " + args;
	}
	send_to_server(data);
}

bool chat_handler::do_speak(const std::string& message, bool allies_only)
{
	if (message.empty() || message == "/") {
		return false;
	}
	bool is_command = (message[0] == '/');
	bool quoted_command = (is_command && message[1] == ' ');

	if (!is_command) {
		send_chat_message(message, allies_only);
		return true;
	}
	else if (quoted_command) {
		send_chat_message(std::string(message.begin() + 2, message.end()), allies_only);
		return true;
	}
	std::string cmd(message.begin() + 1, message.end());
	chat_command_handler cch(*this, allies_only);
	return cch.dispatch(cmd);
}

void chat_handler::user_relation_changed(const std::string& /*name*/)
{
}

void chat_handler::send_whisper(const std::string& receiver, const std::string& message)
{
	config cwhisper, data;
	cwhisper["receiver"] = receiver;
	cwhisper["message"] = message;
	cwhisper["sender"] = prefs::get().login();
	data.add_child("whisper", std::move(cwhisper));
	send_to_server(data);
}

void chat_handler::add_whisper_sent(const std::string& receiver, const std::string& message)
{
	utils::string_map symbols;
	symbols["receiver"] = receiver;
	add_chat_message(std::chrono::system_clock::now(), VGETTEXT("whisper to $receiver", symbols), 0, message);
}

void chat_handler::add_whisper_received(const std::string& sender, const std::string& message)
{
	utils::string_map symbols;
	symbols["sender"] = sender;
	add_chat_message(std::chrono::system_clock::now(), VGETTEXT("whisper: $sender", symbols), 0, message);
}

void chat_handler::send_chat_room_message(const std::string& room,
	const std::string& message)
{
	config cmsg, data;
	cmsg["room"] = room;
	cmsg["message"] = message;
	cmsg["sender"] = prefs::get().login();
	data.add_child("message", std::move(cmsg));
	send_to_server(data);
}

void chat_handler::add_chat_room_message_sent(const std::string &room, const std::string &message)
{
	add_chat_room_message_received(room, prefs::get().login(), message);
}

void chat_handler::add_chat_room_message_received(const std::string &room,
	const std::string &speaker, const std::string &message)
{
	add_chat_message(std::chrono::system_clock::now(), room + ": " + speaker, 0, message, events::chat_handler::MESSAGE_PRIVATE);
}

}
