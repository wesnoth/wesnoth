/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "server/player.hpp"
#include "lexical_cast.hpp"
#include "serialization/string_utils.hpp"

wesnothd::player::player(const std::string& n, simple_wml::node& cfg,
                         bool registered, const size_t max_messages,
                         const size_t time_period, const bool sp,
                         const bool moderator)
  : name_(n)
  , cfg_(cfg)
  , selective_ping_(sp)
  , registered_(registered)
  , flood_start_(0)
  , messages_since_flood_start_(0)
  , MaxMessages(max_messages)
  , TimePeriod(time_period)
  , status_(LOBBY)
  , moderator_(moderator)
{
	cfg_.set_attr_dup("name", n.c_str());
	cfg_.set_attr("registered", registered ? "yes" : "no");
	mark_available();
}


void wesnothd::player::set_status(wesnothd::player::STATUS status)
{
	status_ = status;
	switch (status)
	{
		case wesnothd::player::LOBBY:
			cfg_.set_attr("status", "lobby");
			break;
		case wesnothd::player::PLAYING:
			cfg_.set_attr("status", "playing");
			break;
		case wesnothd::player::OBSERVING:
			cfg_.set_attr("status", "observing");
			break;
		default:
			cfg_.set_attr("status", "unknown");
	}
}

// keep 'available' and game name ('location') for backward compatibility
void wesnothd::player::mark_available(const int game_id,
                                      const std::string& location)
{
	if (game_id == 0) {
		cfg_.set_attr("available", "yes");
		set_status(LOBBY);
	} else {
		cfg_.set_attr("available", "no");
	}
	cfg_.set_attr_dup("game_id", lexical_cast<std::string>(game_id).c_str());
	cfg_.set_attr_dup("location", location.c_str());
}

void wesnothd::player::mark_registered(bool registered)
{
    cfg_.set_attr("registered", registered ? "yes" : "no");
    registered_ = registered;
}

bool wesnothd::player::is_message_flooding()
{
	const time_t now = time(nullptr);
	if (flood_start_ == 0) {
		flood_start_ = now;
		return false;
	}

	++messages_since_flood_start_;

	if (now - flood_start_ > TimePeriod) {
		messages_since_flood_start_ = 0;
		flood_start_ = now;
	} else if (messages_since_flood_start_ >= MaxMessages) {
		return true;
	}
	return false;
}
