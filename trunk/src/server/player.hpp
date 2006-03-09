/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
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
	player(const std::string& n, config& cfg,size_t max_messages=4,size_t time_period=10);

	void mark_available(bool val,std::string location);

	const std::string& name() const;

	config* config_address();

	bool silenced() const;
	bool is_message_flooding();

private:
	std::string name_;
	config& cfg_;

	time_t flood_start_;
	int messages_since_flood_start_;
	size_t MaxMessages;
	size_t TimePeriod;
};

#endif
