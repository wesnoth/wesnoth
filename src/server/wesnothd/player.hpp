/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "server/common/simple_wml.hpp"

#include <chrono>
#include <set>

namespace wesnothd {

class player
{
public:
	enum STATUS {
		LOBBY,
		PLAYING,
		OBSERVING
	};

	player(const std::string& n, simple_wml::node& cfg, long id, bool registered, const std::string& version, const std::string& source,
	       unsigned long long login_id, const std::size_t max_messages=4, const std::chrono::seconds& time_period=std::chrono::seconds{10},
	       const bool moderator=false);

	void set_status(STATUS status);

	// mark a player as member of the game 'game_id' or as located in the lobby
	void mark_available(const int game_id=0, const std::string& location="");

	//Mark a player as registered if he has authorized
	void mark_registered(bool registered =true);
	bool registered() const {return registered_;}


	const std::string& name() const { return name_; }
	const std::string& version() const { return version_; }
	const std::string& source() const { return source_; }
	const simple_wml::node* config_address() const { return &cfg_; }

	bool is_message_flooding();

	void set_moderator(bool moderator) { moderator_ = moderator; }
	bool is_moderator() const { return moderator_; }

	unsigned long long get_login_id() const { return login_id_; };

private:
	const std::string name_;
	std::string version_;
	std::string source_;
	simple_wml::node& cfg_;

	bool registered_;

	std::chrono::steady_clock::time_point flood_start_;
	unsigned int messages_since_flood_start_;
	const std::size_t MaxMessages;
	const std::chrono::seconds TimePeriod;
	STATUS status_;
	bool moderator_;
	unsigned long long login_id_;
};

} //namespace wesnothd
