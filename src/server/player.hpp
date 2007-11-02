/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PLAYER_HPP_INCLUDED
#define PLAYER_HPP_INCLUDED

#include <ctime>

#include "../config.hpp"

#include <string>

class player
{
public:
	player(const std::string& n, config& cfg, const size_t max_messages=4, const size_t time_period=10);

	// mark a player as member of the game 'game_id' or as located in the lobby
	void mark_available(std::string game_id="", std::string location="");


	const std::string& name() const { return name_; }

	config* config_address() const { return &cfg_; }

	bool silenced() const { return messages_since_flood_start_ > MaxMessages; }
	bool is_message_flooding();

private:
	const std::string name_;
	config& cfg_;

	time_t flood_start_;
	unsigned int messages_since_flood_start_;
	const size_t MaxMessages;
	const time_t TimePeriod;
};

#endif
