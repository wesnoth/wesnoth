/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "desktop/notifications.hpp"
#include "global.hpp"

#include "game_preferences.hpp"
#include "gettext.hpp"

#include "video.hpp" //CVideo::get_singleton().window_state()

#ifdef HAVE_LIBDBUS
#include "desktop/dbus_notification.hpp"
#endif

#ifdef __APPLE__
#include "desktop/apple_notification.hpp"
#endif

#ifdef _WIN32
#include "desktop/windows_tray_notification.hpp"
#endif

namespace desktop {

namespace notifications {

#if !(defined(HAVE_LIBDBUS) || defined(__APPLE__) || defined(_WIN32))

bool available() { return false; }

void send(const std::string& /*owner*/, const std::string& /*message*/, type /*t*/)
{}

#else

bool available() { return true; }

void send(const std::string& owner, const std::string& message, type t)
{
	Uint8 app_state = CVideo::get_singleton().window_state();

	// Do not show notifications when the window is visible...
	if ((app_state & SDL_APPACTIVE) != 0)
	{
		// ... and it has a focus.
		if ((app_state & (SDL_APPMOUSEFOCUS | SDL_APPINPUTFOCUS)) != 0) {
			return;
		}
	}

#ifdef HAVE_LIBDBUS
	dbus::send_notification(owner, message, t == CHAT);
#endif

#ifdef __APPLE__
	apple_notifications::send_notification(owner, message, t);
#endif

#ifdef _WIN32
	std::string notification_title;
	std::string notification_message;

	switch (t) {
		case CHAT:
			notification_title = _("Chat message");
			notification_message = owner + ": " + message;
			break;
		case TURN_CHANGED:
		case OTHER:
			notification_title = owner;
			notification_message = message;
			break;
	}

	windows_tray_notification::show(notification_title, notification_message);
#endif
}
#endif //end #else (defined(HAVE_LIBDBUS) || defined(HAVE_GROWL) || defined(_WIN32))

} //end namespace notifications

} //end namespace desktop
