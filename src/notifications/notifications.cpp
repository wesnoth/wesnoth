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

#include "log.hpp"
#include "game_config.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"

#include <boost/cstdint.hpp>
#include <cstdlib>
#include <list>

#include "sdl/compat.hpp"

#if SDL_VERSION_ATLEAST(2,0,0)
#include "video.hpp"
#else
#include "SDL_active.h"
#endif

#ifdef HAVE_LIBDBUS
#include <dbus/dbus.h>
#endif

#ifdef HAVE_GROWL
#include <Growl/GrowlApplicationBridge-Carbon.h>
#include <Carbon/Carbon.h>
Growl_Delegate growl_obj;
#endif

#ifdef _WIN32
#include "windows_tray_notification.hpp"
#endif

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)
#define LOG_DP LOG_STREAM(info, log_display)

using boost::uint32_t;

namespace notifications
{
#if !(defined(HAVE_LIBDBUS) || defined(HAVE_GROWL) || defined(_WIN32))
void send_notification(const std::string& /*owner*/, const std::string& /*message*/)
{}
#else

#ifdef HAVE_LIBDBUS
/** Use KDE 4 notifications. */
static bool kde_style = false;

struct wnotify
{
	wnotify()
		: id()
		, owner()
		, message()
	{
	}

	uint32_t id;
	std::string owner;
	std::string message;
};

static std::list<wnotify> notifications;

static DBusHandlerResult filter_dbus_signal(DBusConnection *, DBusMessage *buf, void *)
{
	if (!dbus_message_is_signal(buf, "org.freedesktop.Notifications", "NotificationClosed")) {
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	uint32_t id;
	dbus_message_get_args(buf, NULL,
		DBUS_TYPE_UINT32, &id,
		DBUS_TYPE_INVALID);

	std::list<wnotify>::iterator i = notifications.begin(),
		i_end = notifications.end();
	while (i != i_end && i->id != id) ++i;
	if (i != i_end)
		notifications.erase(i);

	return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusConnection *get_dbus_connection()
{
	if (preferences::get("disable_notifications", false)) {
		return NULL;
	}

	static bool initted = false;
	static DBusConnection *connection = NULL;

	if (!initted)
	{
		initted = true;
		if (getenv("KDE_SESSION_VERSION")) {
			// This variable is defined for KDE 4 only.
			kde_style = true;
		}
		DBusError err;
		dbus_error_init(&err);
		connection = dbus_bus_get(DBUS_BUS_SESSION, &err);
		if (!connection) {
			ERR_DP << "Failed to open DBus session: " << err.message << '\n';
			dbus_error_free(&err);
			return NULL;
		}
		dbus_connection_add_filter(connection, filter_dbus_signal, NULL, NULL);
	}
	if (connection) {
		dbus_connection_read_write(connection, 0);
		while (dbus_connection_dispatch(connection) == DBUS_DISPATCH_DATA_REMAINS) {}
	}
	return connection;
}

static uint32_t send_dbus_notification(DBusConnection *connection, uint32_t replaces_id,
	const std::string &owner, const std::string &message)
{
	DBusMessage *buf = dbus_message_new_method_call(
		kde_style ? "org.kde.VisualNotifications" : "org.freedesktop.Notifications",
		kde_style ? "/VisualNotifications" : "/org/freedesktop/Notifications",
		kde_style ? "org.kde.VisualNotifications" : "org.freedesktop.Notifications",
		"Notify");
	const char *app_name = "Battle for Wesnoth";
	dbus_message_append_args(buf,
		DBUS_TYPE_STRING, &app_name,
		DBUS_TYPE_UINT32, &replaces_id,
		DBUS_TYPE_INVALID);
	if (kde_style) {
		const char *event_id = "";
		dbus_message_append_args(buf,
			DBUS_TYPE_STRING, &event_id,
			DBUS_TYPE_INVALID);
	}
	std::string app_icon_ = game_config::path + "/images/wesnoth-icon-small.png";
	const char *app_icon = app_icon_.c_str();
	const char *summary = owner.c_str();
	const char *body = message.c_str();
	const char **actions = NULL;
	dbus_message_append_args(buf,
		DBUS_TYPE_STRING, &app_icon,
		DBUS_TYPE_STRING, &summary,
		DBUS_TYPE_STRING, &body,
		DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, &actions, 0,
		DBUS_TYPE_INVALID);
	DBusMessageIter iter, hints;
	dbus_message_iter_init_append(buf, &iter);
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &hints);
	dbus_message_iter_close_container(&iter, &hints);
	int expire_timeout = kde_style ? 5000 : -1;
	dbus_message_append_args(buf,
		DBUS_TYPE_INT32, &expire_timeout,
		DBUS_TYPE_INVALID);
	DBusError err;
	dbus_error_init(&err);
	DBusMessage *ret = dbus_connection_send_with_reply_and_block(connection, buf, 1000, &err);
	dbus_message_unref(buf);
	if (!ret) {
		ERR_DP << "Failed to send visual notification: " << err.message << '\n';
		dbus_error_free(&err);
		if (kde_style) {
			ERR_DP << " Retrying with the freedesktop protocol." << std::endl;
			kde_style = false;
			return send_dbus_notification(connection, replaces_id, owner, message);
		}
		return 0;
	}
	uint32_t id;
	dbus_message_get_args(ret, NULL,
		DBUS_TYPE_UINT32, &id,
		DBUS_TYPE_INVALID);
	dbus_message_unref(ret);
	// TODO: remove once closing signals for KDE are handled in filter_dbus_signal.
	if (kde_style) return 0;
	return id;
}
#endif // end #if LIB_DBUS

void send_notification(const std::string& owner, const std::string& message)
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
	DBusConnection *connection = get_dbus_connection();
	if (!connection) return;

	std::list<wnotify>::iterator i = notifications.begin(),
		i_end = notifications.end();
	while (i != i_end && i->owner != owner) ++i;

	if (i != i_end) {
		i->message = message + "\n" + i->message;
		int endl_pos = -1;
		for (int ctr = 0; ctr < 5; ctr++)
			endl_pos = i->message.find('\n', endl_pos+1);

		i->message = i->message.substr(0,endl_pos);

		send_dbus_notification(connection, i->id, owner, i->message);
		return;
	} else {
		uint32_t id = send_dbus_notification(connection, 0, owner, message);
		if (!id) return;
		wnotify visual;
		visual.id = id;
		visual.owner = owner;
		visual.message = message;
		notifications.push_back(visual);
	}
#endif

#ifdef HAVE_GROWL
	CFStringRef app_name = CFStringCreateWithCString(NULL, "Wesnoth", kCFStringEncodingUTF8);
	CFStringRef cf_owner = CFStringCreateWithCString(NULL, owner.c_str(), kCFStringEncodingUTF8);
	CFStringRef cf_message = CFStringCreateWithCString(NULL, message.c_str(), kCFStringEncodingUTF8);
	//Should be changed as soon as there are more than 2 types of notifications
	CFStringRef cf_note_name = CFStringCreateWithCString(NULL, owner == _("Turn changed") ? _("Turn changed") : _("Chat message"), kCFStringEncodingUTF8);

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

	if (owner == _("Turn changed")) {
		notification_title = owner;
		notification_message = message;
	} else {
		notification_title = _("Chat message");
		notification_message = owner + ": " + message;
	}

	windows_tray_notification::show(notification_title, notification_message);
#endif
}
#endif //end #else !defined(NO_NOTIFICATIONS)

} //end namespace notifications
