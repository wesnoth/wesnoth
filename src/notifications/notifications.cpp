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

#include "notifications.hpp"
#include "global.hpp"

#include "game_preferences.hpp"
#include "gettext.hpp"

#include "sdl/compat.hpp"

#if SDL_VERSION_ATLEAST(2,0,0)
#include "video.hpp"
#else
#include "SDL_active.h"
#endif

#ifdef HAVE_LIBDBUS
#include "dbus_notification.hpp"
#endif

#ifdef HAVE_GROWL
#include <Growl/GrowlApplicationBridge-Carbon.h>
#include <Carbon/Carbon.h>
Growl_Delegate growl_obj;
#endif

#ifdef _WIN32
#include "windows_tray_notification.hpp"
#endif

namespace notifications
{
#if !(defined(HAVE_LIBDBUS) || defined(HAVE_GROWL) || defined(_WIN32))
void send_notification(const std::string& /*owner*/, const std::string& /*message*/, type /*t*/)
{}
#else

void send_notification(const std::string& owner, const std::string& message, type t)
{
	if (preferences::get("disable_notifications", false)) { return; }

	Uint8 app_state = SDL_GetAppState();

	// Do not show notifications when the window is visible...
	if ((app_state & SDL_APPACTIVE) != 0)
	{
		// ... and it has a focus.
		if ((app_state & (SDL_APPMOUSEFOCUS | SDL_APPINPUTFOCUS)) != 0) {
			return;
		}
	}

#ifdef HAVE_LIBDBUS
	(void)t;
	dbus::send_notification(owner, message);
#endif

#ifdef HAVE_GROWL
	std::string note_name = "";
	switch (t) {
		case CHAT:
			note_name = _("Chat Message");
			break;
		case TURN_CHANGED:
			note_name = _("Turn Changed");
			break;
		case OTHER:
			note_name = _("Wesnoth");
			break;
	}

	CFStringRef app_name = CFStringCreateWithCString(NULL, "Wesnoth", kCFStringEncodingUTF8);
	CFStringRef cf_owner = CFStringCreateWithCString(NULL, owner.c_str(), kCFStringEncodingUTF8);
	CFStringRef cf_message = CFStringCreateWithCString(NULL, message.c_str(), kCFStringEncodingUTF8);
	CFStringRef cf_note_name = CFStringCreateWithCString(NULL, note_name.c_str(), kCFStringEncodingUTF8);

	growl_obj.applicationName = app_name;
	growl_obj.registrationDictionary = NULL;
	growl_obj.applicationIconData = NULL;
	growl_obj.growlIsReady = NULL;
	growl_obj.growlNotificationWasClicked = NULL;
	growl_obj.growlNotificationTimedOut = NULL;

	Growl_SetDelegate(&growl_obj);
	Growl_NotifyWithTitleDescriptionNameIconPriorityStickyClickContext(cf_owner, cf_message, cf_note_name, NULL, NULL, NULL, NULL);

	CFRelease(app_name);
	CFRelease(cf_owner);
	CFRelease(cf_message);
	CFRelease(cf_note_name);
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
#endif //end #else !defined(NO_NOTIFICATIONS)

} //end namespace notifications
