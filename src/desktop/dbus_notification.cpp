/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "dbus_notification.hpp"
#include "global.hpp"

#include "game_config.hpp"
#include "log.hpp"

#ifndef HAVE_LIBDBUS
#error "The HAVE_LIBDBUS symbol is not defined, you do not have lib dbus available, you should not be trying to compile dbus_notification.cpp"
#endif

#include <dbus/dbus.h>

#include <boost/cstdint.hpp>
#include <list>
#include <stdlib.h>

using boost::uint32_t;

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)
#define LOG_DP LOG_STREAM(info, log_display)

namespace { // anonymous namespace

/** Use KDE 4 notifications. */
bool kde_style = false;

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

std::list<wnotify> notifications;

DBusHandlerResult filter_dbus_signal(DBusConnection *, DBusMessage *buf, void *)
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

DBusConnection *get_dbus_connection()
{
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

uint32_t send_dbus_notification(DBusConnection *connection, uint32_t replaces_id,
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

} // end anonymous namespace

namespace dbus {

void send_notification(const std::string & owner, const std::string & message)
{
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
}

} // end namespace dbus

