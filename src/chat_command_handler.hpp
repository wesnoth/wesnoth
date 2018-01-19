/*
   Copyright (C) 2017-2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "map_command_handler.hpp"
#include "gettext.hpp"

namespace events {
	class chat_handler;
//command handler for chat /commands
class chat_command_handler : public map_command_handler<chat_command_handler>
{
public:
	typedef map_command_handler<chat_command_handler> map;
	chat_command_handler(chat_handler& chathandler, bool allies_only)
		: map(), chat_handler_(chathandler), allies_only_(allies_only)
	{
	}

protected:
	void do_emote();
	void do_network_send();
	void do_network_send_req_arg();
	void do_room_query();
	void do_room_query_noarg();
	void do_gen_room_query();
	void do_whisper();
	void do_chanmsg();
	void do_log();
	void do_ignore();
	void do_friend();
	void do_remove();
	void do_display();
	void do_version();

	/** Ask the server to register the currently used nick. */
	void do_register();

	/** Ask the server do drop the currently used (and registered) nick. */
	void do_drop();

	/** Update details for the currently used username. */
	void do_set();

	/** Request information about a user from the server. */
	void do_info();

	/**
	* Request a list of details that can be set for a username
	* as these might vary depending on the configuration of the server.
	*/
	void do_details();

	std::string get_flags_description() const {
		return _("(A) — admin command");
	}

	std::string get_command_flags_description(
		const map_command_handler<chat_command_handler>::command& c) const
	{
		if (c.has_flag('A')) {
			return std::string(" ") + _("(admin only)");
		}
		else {
			return "";
		}
	}

	bool is_enabled(const map_command_handler<chat_command_handler>::command& c) const;

	void print(const std::string& title, const std::string& message);

	void init_map()
	{
		set_cmd_prefix("/");
		register_command("query", &chat_command_handler::do_network_send,
			_("Send a query to the server. Without arguments the server"
				" should tell you the available commands."));
		register_alias("query", "q");
		register_command("ban", &chat_command_handler::do_network_send_req_arg,
			_("Ban and kick a player or observer. If he is not in the"
				" game but on the server he will only be banned."), _("<nickname>"));
		register_command("unban", &chat_command_handler::do_network_send_req_arg,
			_("Unban a user. He does not have to be in the game but on"
				" the server."), _("<nickname>"));
		register_command("kick", &chat_command_handler::do_network_send_req_arg,
			_("Kick a player or observer."), _("<nickname>"));
		register_command("mute", &chat_command_handler::do_network_send,
			_("Mute an observer. Without an argument displays the mute status."), _("<nickname>"));
		register_command("unmute", &chat_command_handler::do_network_send,
			_("Unmute an observer. Without an argument unmutes everyone."), _("<nickname>"));
		register_command("muteall", &chat_command_handler::do_network_send,
			_("Mute/Unmute all observers. (toggles)"), "");
		register_command("ping", &chat_command_handler::do_network_send,
			"");
		register_command("green", &chat_command_handler::do_network_send_req_arg,
			"", "", "A");
		register_command("red", &chat_command_handler::do_network_send_req_arg,
			"", "", "A");
		register_command("yellow", &chat_command_handler::do_network_send_req_arg,
			"", "", "A");
		register_command("report", &chat_command_handler::do_network_send_req_arg,
			_("Report abuse, rule violations, etc. to the server moderators. "
				"Make sure to mention relevant nicknames, etc."), "");
		register_alias("report", "adminmsg");  // deprecated
		register_command("emote", &chat_command_handler::do_emote,
			_("Send an emotion or personal action in chat."), _("<message>"));
		register_alias("emote", "me");
		register_command("whisper", &chat_command_handler::do_whisper,
			_("Sends a private message. "
				"You cannot send private messages to players in a running game you observe or play in."),
			_("<nickname> <message>"));
		register_alias("whisper", "msg");
		register_alias("whisper", "m");
		register_command("log", &chat_command_handler::do_log,
			_("Change the log level of a log domain."), _("<level> <domain>"));
		register_command("ignore", &chat_command_handler::do_ignore,
			_("Add a nickname to your ignores list."), _("<nickname>"));
		register_command("friend", &chat_command_handler::do_friend,
			_("Add a nickname to your friends list."), _("<nickname>"));
		register_command("remove", &chat_command_handler::do_remove,
			_("Remove a nickname from your ignores or friends list."), _("<nickname>"));
		register_command("version", &chat_command_handler::do_version,
			_("Display version information."));
		register_command("register", &chat_command_handler::do_register,
			_("Register your nickname"), _("<password> <email (optional)>"));
		register_command("drop", &chat_command_handler::do_drop,
			_("Drop your nickname."));
		register_command("set", &chat_command_handler::do_set,
			_("Update details for your nickname. For possible details see '/details'."),
			_("<detail> <value>"));
		register_command("info", &chat_command_handler::do_info,
			_("Request information about a nickname."), _("<nickname>"));
		register_command("details", &chat_command_handler::do_details,
			_("Request a list of details you can set for your registered nickname."));
		register_command("join", &chat_command_handler::do_network_send_req_arg,
			_("Join a room."), _("<room>"));
		register_alias("join", "j");
		register_command("part", &chat_command_handler::do_network_send_req_arg,
			_("Part a room."), _("<room>"));
		register_command("names", &chat_command_handler::do_room_query,
			_("List room members."), _("<room>"));
		register_command("rooms", &chat_command_handler::do_room_query_noarg,
			_("List available rooms."));
		register_command("room", &chat_command_handler::do_chanmsg,
			_("Room message."), _("<room> <msg>"));
		register_command("room_query", &chat_command_handler::do_gen_room_query,
			_("Room query."), _("<room> <type> [value]"));
		register_alias("room_query", "rq");
	}
private:
	chat_handler& chat_handler_;
	bool allies_only_;
};

}
