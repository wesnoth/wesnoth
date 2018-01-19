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

#pragma once

#include <string>

namespace desktop {

namespace notifications
{
	enum type {CHAT, TURN_CHANGED, OTHER};

/**
 * Displays a desktop notification @a message, from @a owner, of type @a t.
 *
 * If it is an appropriate time to send a desktop notification (i.e. the window
 * does not have focus and the feature is not disabled by the preferences),
 * and wesnoth was compiled with support for this feature, a notification will
 * be issued. If there is no support for notifications, this fcn is a no-op.
 *
 * @note Currently we have support for dbus (linux), windows tray notifications,
 * and growl (Apple). To enable one of these, the corresponding compilation unit
 * dbus_notification.cpp, growl_notification.cpp, windows_tray_notification.cpp,
 * must be compiled, and the corresponding C++ symbol HAVE_LIBDBUS, HAVE_GROWL,
 * _WIN32 must be defined for that compilation unit _and for this one_.
 */
	void send(const std::string& owner, const std::string& message, type t);

/** Returns whether we were compiled with support for desktop notifications. */
	bool available();
}

}
