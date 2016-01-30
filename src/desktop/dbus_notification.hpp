/*
   Copyright (C) 2014 - 2016 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef DBUS_NOTIFICATION_HPP_
#define DBUS_NOTIFICATION_HPP_

#include <string>

namespace dbus {
	void send_notification(const std::string& owner, const std::string& message, bool with_history);
}

#endif
