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

#include "../global.hpp"

#include "player.hpp"

player::player(const std::string& n, config& cfg) : name_(n), cfg_(cfg), flood_start_(0), messages_since_flood_start_(0)
{
	cfg_["name"] = n;
	mark_available(true,"");
}

void player::mark_available(bool val,std::string location )
{
	cfg_.values["available"] = (val ? "yes" : "no");
	cfg_.values["location"] = location;
}

const std::string& player::name() const
{
	return name_;
}

config* player::config_address()
{
	return &cfg_;
}

namespace
{
	const size_t MaxMessages = 4;
	const size_t TimePeriod = 5;
}


bool player::silenced() const
{
	return messages_since_flood_start_ > MaxMessages;
}

bool player::is_message_flooding()
{
	const time_t now = time(NULL);
	if(flood_start_ == 0) {
		flood_start_ = now;
		return false;
	}

	++messages_since_flood_start_;

	if(now - flood_start_ > TimePeriod) {
		messages_since_flood_start_ = 0;
		flood_start_ = now;
	} else if(messages_since_flood_start_ == MaxMessages) {
		return true;
	}

	return false;
}
