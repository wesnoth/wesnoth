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

#pragma once

#include "chat_events.hpp"

#include <cstdint>
#include <ctime>
#include <set>
#include <string>
#include <vector>

class display;

class display_chat_manager {
public:
	display_chat_manager(display & disp) : my_disp_(disp) {}

	void add_observer(const std::string& name) { observers_.insert(name); }
	void remove_observer(const std::string& name) { observers_.erase(name); }
	const std::set<std::string>& observers() const { return observers_; }

	void add_whisperer(const std::string& nick) { whisperers_.insert(nick); }
	void remove_whisperer(const std::string& nick) { whisperers_.erase(nick); }
	const std::set<std::string>& whisperers() const { return whisperers_; }

	void add_chat_message(const time_t& time, const std::string& speaker,
		int side, const std::string& msg, events::chat_handler::MESSAGE_TYPE type, bool bell);
	void clear_chat_messages() { prune_chat_messages(true); }

	friend class game_display; //needed because it calls prune_chat_message
private:
	std::set<std::string> observers_;
	std::set<std::string> whisperers_; //nicks who whisper you for tab-completition purpose

	struct chat_message
	{
		chat_message(int speaker, int h);

		int speaker_handle;
		int handle;
		uint32_t created_at;
	};

	void prune_chat_messages(bool remove_all=false);

	std::vector<chat_message> chat_messages_;

	display & my_disp_;
};
