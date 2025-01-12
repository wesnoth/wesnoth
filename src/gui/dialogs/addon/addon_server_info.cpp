/*
	Copyright (C) 2024 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/addon/addon_server_info.hpp"

#include "gettext.hpp"
#include "gui/dialogs/addon/addon_auth.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/prompt.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/button.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(addon_server_info)

addon_server_info::addon_server_info(addons_client& client, const std::string& addon, bool& needs_refresh)
	: modal_dialog(window_id())
	, client_(client)
	, addon_(addon)
	, needs_refresh_(needs_refresh)
{
	connect_signal_mouse_left_click(
		find_widget<button>("downloads_by_version"),
		std::bind(&addon_server_info::downloads_by_version, this));

	connect_signal_mouse_left_click(
		find_widget<button>("addon_count_by_forum_auth"),
		std::bind(&addon_server_info::addon_count_by_forum_auth, this));

	connect_signal_mouse_left_click(
		find_widget<button>("admin_delete_addon"),
		std::bind(&addon_server_info::admin_delete_addon, this));

	connect_signal_mouse_left_click(
		find_widget<button>("admin_hide_addon"),
		std::bind(&addon_server_info::admin_hide_addon, this));

	connect_signal_mouse_left_click(
		find_widget<button>("admin_unhide_addon"),
		std::bind(&addon_server_info::admin_unhide_addon, this));

	connect_signal_mouse_left_click(
		find_widget<button>("admin_list_hidden"),
		std::bind(&addon_server_info::admin_list_hidden, this));
}

void addon_server_info::pre_show()
{

}

void addon_server_info::post_show()
{

}

void addon_server_info::downloads_by_version()
{
	config downloads = client_.get_addon_downloads_by_version(addon_);
	PLAIN_LOG << downloads.debug();
}

void addon_server_info::addon_count_by_forum_auth()
{
	config forum_auths = client_.get_forum_auth_usage();
	PLAIN_LOG << forum_auths.debug();
}

void addon_server_info::admin_delete_addon()
{
	config admins = client_.get_addon_admins();

	std::set<std::string> admin_set;
	for(const auto& admin : admins.child_range("admin")) {
		admin_set.emplace(admin["username"]);
	}

	std::string msg;
	if(!client_.delete_remote_addon(addon_, msg, admin_set)) {
		gui2::show_error_message(_("The server responded with an error:") + "\n" + client_.get_last_server_error());
	} else {
		gui2::show_transient_message(_("Response"), msg);
		needs_refresh_ = true;
	}
}

void addon_server_info::admin_hide_addon()
{
	if(!addon_.empty()) {
		config admins = client_.get_addon_admins();

		std::set<std::string> admin_set;
		for(const auto& admin : admins.child_range("admin")) {
			admin_set.emplace(admin["username"]);
		}

		config cfg;
		cfg["primary_authors"] = utils::join(admin_set);
		if(!gui2::dialogs::addon_auth::execute(cfg)) {
			gui2::show_error_message(_("Password not provided"));
			return;
		}

		if(!client_.hide_addon(addon_, cfg["uploader"].str(), cfg["passphrase"].str())) {
			gui2::show_error_message(_("The server responded with an error:") + "\n" + client_.get_last_server_error());
		} else {
			needs_refresh_ = true;
		}
	}
}

void addon_server_info::admin_unhide_addon()
{
	std::string addon;
	gui2::dialogs::prompt::execute(addon);

	if(!addon.empty()) {
		config admins = client_.get_addon_admins();

		std::set<std::string> admin_set;
		for(const auto& admin : admins.child_range("admin")) {
			admin_set.emplace(admin["username"]);
		}

		config cfg;
		cfg["primary_authors"] = utils::join(admin_set);
		if(!gui2::dialogs::addon_auth::execute(cfg)) {
			gui2::show_error_message(_("Password not provided"));
			return;
		}

		if(!client_.unhide_addon(addon, cfg["uploader"].str(), cfg["passphrase"].str())) {
			gui2::show_error_message(_("The server responded with an error:") + "\n" + client_.get_last_server_error());
		} else {
			needs_refresh_ = true;
		}
	}
}

void addon_server_info::admin_list_hidden()
{
	config admins = client_.get_addon_admins();

	std::set<std::string> admin_set;
	for(const auto& admin : admins.child_range("admin")) {
		admin_set.emplace(admin["username"]);
	}

	config cfg;
	cfg["primary_authors"] = utils::join(admin_set);
	if(!gui2::dialogs::addon_auth::execute(cfg)) {
		gui2::show_error_message(_("Password not provided"));
		return;
	}

	PLAIN_LOG << client_.get_hidden_addons(cfg["uploader"].str(), cfg["passphrase"].str());
}

} // namespace dialogs
