/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef NOTIFICATIONS_NOTIFICATIONS_HPP_INCLUDED
#define NOTIFICATIONS_NOTIFICATIONS_HPP_INCLUDED

#include <string>

namespace notifications
{
	enum type {CHAT, TURN_CHANGED, OTHER};

	void send(const std::string& owner, const std::string& message, type t);

	bool available();
}

#endif
