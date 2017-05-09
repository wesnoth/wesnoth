/*
   Copyright (C) 2006 - 2017 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

class config;
#include <ctime>
#include <string>

namespace events {

class chat_handler
{
public:
	chat_handler();
	virtual ~chat_handler();

	enum MESSAGE_TYPE { MESSAGE_PUBLIC, MESSAGE_PRIVATE };

	void send_command(const std::string& cmd, const std::string& args="");

	virtual void send_to_server(const config& cfg) = 0;
protected:
	void do_speak(const std::string& message, bool allies_only=false);

	//called from do_speak
	virtual void add_chat_message(const time_t& time,
			const std::string& speaker, int side, const std::string& message,
			MESSAGE_TYPE type=MESSAGE_PRIVATE) = 0;
	virtual void send_chat_message(const std::string& message, bool allies_only=false) = 0;

	//Why are these virtual?
	virtual void send_whisper(const std::string& receiver, const std::string& message);

	virtual void add_whisper_sent(const std::string& receiver, const std::string& message);

	virtual void add_whisper_received(const std::string& sender, const std::string& message);

	virtual void send_chat_room_message(const std::string& room, const std::string& message);

	virtual void add_chat_room_message_sent(const std::string& room, const std::string& message);

	virtual void add_chat_room_message_received(const std::string& room,
		const std::string& speaker, const std::string& message);

	/**
	 * Called when a processed command results in a relation (friend/ignore) change
	 * for a user whose name is passed as the 'name' arg
	 */
	virtual void user_relation_changed(const std::string& name);

	void change_logging(const std::string& data);

	friend class chat_command_handler;
};

}
