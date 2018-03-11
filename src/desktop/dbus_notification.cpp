/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "desktop/dbus_notification.hpp"

#include "filesystem.hpp"
#include "game_config.hpp"
#include "log.hpp"

#ifndef HAVE_LIBDBUS
#error "The HAVE_LIBDBUS symbol is not defined, you do not have lib dbus available, you should not be trying to compile dbus_notification.cpp"
#endif

#include <dbus/dbus.h>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <cstdlib>
#include <string>

#pragma GCC diagnostic ignored "-Wold-style-cast"

static lg::log_domain log_desktop("desktop");
#define ERR_DU LOG_STREAM(err, log_desktop)
#define LOG_DU LOG_STREAM(info, log_desktop)
#define DBG_DU LOG_STREAM(info, log_desktop)

namespace { // anonymous namespace

/** Use KDE 4 notifications. */
bool kde_style = false;

struct wnotify
{
	wnotify(uint32_t id_arg, std::string owner_arg, std::string message_arg)
		: id(id_arg)
		, owner(owner_arg)
		, message(message_arg)
	{
	}

	uint32_t id;
	std::string owner;
	mutable std::string message;
};

struct by_id {};
struct by_owner {};

using boost::multi_index::hashed_unique;
using boost::multi_index::indexed_by;
using boost::multi_index::tag;
using boost::multi_index::member;

typedef boost::multi_index_container<
	wnotify,
	indexed_by<
		//hashed by ids
		hashed_unique<tag<by_id>, member<wnotify,const uint32_t,&wnotify::id>>,
		//hashed by owners
		hashed_unique<tag<by_owner>, member<wnotify,const std::string,&wnotify::owner>>
	>
> wnotify_set;

typedef wnotify_set::index<by_owner>::type wnotify_by_owner;
typedef wnotify_by_owner::iterator wnotify_owner_it;

wnotify_set notifications; //!< Holds all the notifications transaction records

DBusHandlerResult filter_dbus_signal(DBusConnection *, DBusMessage *buf, void *)
{
	if (!dbus_message_is_signal(buf, "org.freedesktop.Notifications", "NotificationClosed")) {
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	uint32_t id;
	dbus_message_get_args(buf, nullptr,
		DBUS_TYPE_UINT32, &id,
		DBUS_TYPE_INVALID);

	size_t num_erased = notifications.get<by_id>().erase(id);
	LOG_DU << "Erased " << num_erased << " notifications records matching id=" << id;

	return DBUS_HANDLER_RESULT_HANDLED;
}

DBusConnection *get_dbus_connection()
{
	static bool initted = false;
	static DBusConnection *connection = nullptr;

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
			ERR_DU << "Failed to open DBus session: " << err.message << '\n';
			dbus_error_free(&err);
			return nullptr;
		}
		dbus_connection_add_filter(connection, filter_dbus_signal, nullptr, nullptr);
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

	std::string app_icon_ = filesystem::normalize_path(game_config::path + "/" + game_config::images::app_icon);
	if (!filesystem::file_exists(app_icon_)) {
		ERR_DU << "Error: Could not find notification icon.\n"
			<< "raw path =\'" << game_config::path << "\' / \'" << game_config::images::app_icon << "\'\n"
			<< "normalized path =\'" << app_icon_ << "\'\n";
	} else {
		DBG_DU << "app_icon_=\'" << app_icon_ << "\'\n";
	}

	const char *app_icon = app_icon_.c_str();
	const char *summary = owner.c_str();
	const char *body = message.c_str();
	const char **actions = nullptr;
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
		ERR_DU << "Failed to send visual notification: " << err.message << '\n';
		dbus_error_free(&err);
		if (kde_style) {
			ERR_DU << " Retrying with the freedesktop protocol." << std::endl;
			kde_style = false;
			return send_dbus_notification(connection, replaces_id, owner, message);
		}
		return 0;
	}
	uint32_t id;
	dbus_message_get_args(ret, nullptr,
		DBUS_TYPE_UINT32, &id,
		DBUS_TYPE_INVALID);
	dbus_message_unref(ret);
	// TODO: remove once closing signals for KDE are handled in filter_dbus_signal.
	if (kde_style) return 0;
	return id;
}

} // end anonymous namespace

namespace dbus {

const size_t MAX_MSG_LINES = 5;

void send_notification(const std::string & owner, const std::string & message, bool with_history)
{
	DBusConnection *connection = get_dbus_connection();
	if (!connection) return;

	wnotify_by_owner & noticias = notifications.get<by_owner>();

	wnotify_owner_it i = noticias.find(owner);

	if (i != noticias.end()) {
		if (with_history) {
			i->message = message + "\n" + i->message;

			size_t endl_pos = i->message.find('\n');
			size_t ctr = 1;

			while (ctr < MAX_MSG_LINES && endl_pos != std::string::npos) {
				endl_pos = i->message.find('\n', endl_pos+1);
				ctr++;
			}

			i->message = i->message.substr(0,endl_pos);
		} else {
			i->message = message;
		}

		send_dbus_notification(connection, i->id, owner, i->message);
		return;
	} else {
		uint32_t id = send_dbus_notification(connection, 0, owner, message);
		if (!id) return;
		wnotify visual(id,owner,message);
		std::pair<wnotify_owner_it, bool> result = noticias.insert(visual);
		if (!result.second) {
			ERR_DU << "Failed to insert a dbus notification message:\n"
				<< "New Item:\n" << "\tid=" << id << "\n\towner=" << owner << "\n\tmessage=" << message << "\n"
				<< "Old Item:\n" << "\tid=" << result.first->id << "\n\towner=" << result.first->owner << "\n\tmessage=" << result.first->message << "\n";
		}
	}
}

} // end namespace dbus
