/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
                 2008 - 2015 by Iris Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "addon/manager_ui.hpp"

#include "addon/client.hpp"
#include "addon/info.hpp"
#include "addon/manager.hpp"
#include "filesystem.hpp"
#include "formula/string_utils.hpp"
#include "preferences/game.hpp"
#include "gettext.hpp"
#include "gui/dialogs/addon/manager.hpp"
#include "gui/dialogs/addon/uninstall_list.hpp"
#include "gui/dialogs/addon/connect.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "wml_exception.hpp"

static lg::log_domain log_config("config");
static lg::log_domain log_network("network");
static lg::log_domain log_filesystem("filesystem");
static lg::log_domain log_addons_client("addons-client");

#define ERR_CFG LOG_STREAM(err,   log_config)

#define ERR_NET LOG_STREAM(err,   log_network)

#define ERR_FS  LOG_STREAM(err,   log_filesystem)

#define LOG_AC  LOG_STREAM(info,  log_addons_client)


namespace {

bool get_addons_list(addons_client& client, addons_list& list)
{
	list.clear();

	config cfg;
	client.request_addons_list(cfg);

	if(!cfg) {
		return false;
	}

	read_addons_list(cfg, list);

	return true;
}

bool addons_manager_ui(const std::string& remote_address)
{
	bool need_wml_cache_refresh = false;

	preferences::set_campaign_server(remote_address);

	try {
		addons_client client(remote_address);
		client.connect();

		gui2::dialogs::addon_manager dlg(client);
		dlg.show();

		need_wml_cache_refresh = dlg.get_need_wml_cache_refresh();
	} catch(const config::error& e) {
		ERR_CFG << "config::error thrown during transaction with add-on server; \""<< e.message << "\"" << std::endl;
		gui2::show_error_message(_("Network communication error."));
	} catch(const network_asio::error& e) {
		ERR_NET << "network_asio::error thrown during transaction with add-on server; \""<< e.what() << "\"" << std::endl;
		gui2::show_error_message(_("Remote host disconnected."));
	} catch(const filesystem::io_exception& e) {
		ERR_FS << "filesystem::io_exception thrown while installing an addon; \"" << e.what() << "\"" << std::endl;
		gui2::show_error_message(_("A problem occurred when trying to create the files necessary to install this add-on."));
	} catch(const invalid_pbl_exception& e) {
		ERR_CFG << "could not read .pbl file " << e.path << ": " << e.message << std::endl;

		utils::string_map symbols;
		symbols["path"] = e.path;
		symbols["msg"] = e.message;

		gui2::show_error_message(
			vgettext("A local file with add-on publishing information could not be read.\n\nFile: $path\nError message: $msg", symbols));
	} catch(wml_exception& e) {
		e.show();
	} catch(const addons_client::user_exit&) {
		LOG_AC << "initial connection canceled by user\n";
	} catch(const addons_client::user_disconnect&) {
		LOG_AC << "attempt to reconnect canceled by user\n";
	} catch(const addons_client::invalid_server_address&) {
		gui2::show_error_message(_("The add-ons server address specified is not valid."));
	}

	return need_wml_cache_refresh;
}

bool uninstall_local_addons()
{
	const std::string list_lead = "\n\n";

	const std::vector<std::string>& addons = installed_addons();

	if(addons.empty()) {
		gui2::show_error_message(_("You have no add-ons installed."));
		return false;
	}

	std::map<std::string, std::string> addon_titles_map;

	for(const std::string& id : addons) {
		std::string title;

		if(have_addon_install_info(id)) {
			// _info.cfg may have the add-on's title starting with 1.11.7,
			// if the add-on was downloading using the revised _info.cfg writer.
			config cfg;
			get_addon_install_info(id, cfg);

			const config& info_cfg = cfg.child("info");

			if(info_cfg) {
				title = info_cfg["title"].str();
			}
		}

		if(title.empty()) {
			// Transform the id into a title as a last resort.
			title = make_addon_title(id);
		}

		addon_titles_map[id] = title;
	}

	int res;

	std::vector<std::string> remove_ids;
	std::set<std::string> remove_names;

	do {
		gui2::dialogs::addon_uninstall_list dlg(addon_titles_map);
		dlg.show();

		remove_ids = dlg.selected_addons();
		if(remove_ids.empty()) {
			return false;
		}

		remove_names.clear();

		for(const std::string& id : remove_ids) {
			remove_names.insert(addon_titles_map[id]);
		}

		const std::string confirm_message = _n(
			"Are you sure you want to remove the following installed add-on?",
			"Are you sure you want to remove the following installed add-ons?",
			remove_ids.size()) + list_lead + utils::bullet_list(remove_names);

		res = gui2::show_message(
				_("Confirm")
				, confirm_message
				, gui2::dialogs::message::yes_no_buttons);
	} while (res != gui2::window::OK);

	std::set<std::string> failed_names, skipped_names, succeeded_names;

	for(const std::string& id : remove_ids) {
		const std::string& name = addon_titles_map[id];

		if(have_addon_pbl_info(id) || have_addon_in_vcs_tree(id)) {
			skipped_names.insert(name);
		} else if(remove_local_addon(id)) {
			succeeded_names.insert(name);
		} else {
			failed_names.insert(name);
		}
	}

	if(!skipped_names.empty()) {
		const std::string dlg_msg = _n(
			"The following add-on appears to have publishing or version control information stored locally, and will not be removed:",
			"The following add-ons appear to have publishing or version control information stored locally, and will not be removed:",
			skipped_names.size());

		gui2::show_error_message(
			dlg_msg + list_lead + utils::bullet_list(skipped_names));
	}

	if(!failed_names.empty()) {
		gui2::show_error_message(_n(
			"The following add-on could not be deleted properly:",
			"The following add-ons could not be deleted properly:",
			failed_names.size()) + list_lead + utils::bullet_list(failed_names));
	}

	if(!succeeded_names.empty()) {
		const std::string dlg_title =
			_n("Add-on Deleted", "Add-ons Deleted", succeeded_names.size());
		const std::string dlg_msg = _n(
			"The following add-on was successfully deleted:",
			"The following add-ons were successfully deleted:",
			succeeded_names.size());

		gui2::show_transient_message(
			dlg_title,
			dlg_msg + list_lead + utils::bullet_list(succeeded_names), "", false, false);

		return true;
	}

	return false;
}

} // end anonymous namespace

bool manage_addons()
{
	static const int addon_download  = 0;
	// NOTE: the following two values are also known by WML, so don't change them.
	static const int addon_uninstall = 2;

	std::string host_name = preferences::campaign_server();
	const bool have_addons = !installed_addons().empty();

	gui2::dialogs::addon_connect addon_dlg(host_name, have_addons);
	addon_dlg.show();
	int res = addon_dlg.get_retval();

	if(res == gui2::window::OK) {
		res = addon_download;
	}

	switch(res) {
		case addon_download:
			return addons_manager_ui(host_name);
		case addon_uninstall:
			return uninstall_local_addons();
		default:
			return false;
	}
}

bool ad_hoc_addon_fetch_session(const std::vector<std::string>& addon_ids)
{
	std::string remote_address = preferences::campaign_server();

	// These exception handlers copied from addon_manager_ui fcn above.
	try {

		addons_client client(remote_address);
		client.connect();

		addons_list addons;

		if(!get_addons_list(client, addons)) {
			gui2::show_error_message(_("An error occurred while downloading the add-ons list from the server."));
			return false;
		}

		bool return_value = true;
		for(const std::string & addon_id : addon_ids) {
			addons_list::const_iterator it = addons.find(addon_id);
			if(it != addons.end()) {
				const addon_info& addon = it->second;
				addons_client::install_result res = client.install_addon_with_checks(addons, addon);
				return_value = return_value && (res.outcome == addons_client::install_outcome::success);
			} else {
				utils::string_map symbols;
				symbols["addon_id"] = addon_id;
				gui2::show_error_message(vgettext("Could not find an add-on matching id $addon_id on the add-on server.", symbols));
				return_value = false;
			}
		}

		return return_value;

	} catch(const config::error& e) {
		ERR_CFG << "config::error thrown during transaction with add-on server; \""<< e.message << "\"" << std::endl;
		gui2::show_error_message(_("Network communication error."));
	} catch(const network_asio::error& e) {
		ERR_NET << "network_asio::error thrown during transaction with add-on server; \""<< e.what() << "\"" << std::endl;
		gui2::show_error_message(_("Remote host disconnected."));
	} catch(const filesystem::io_exception& e) {
		ERR_FS << "io_exception thrown while installing an addon; \"" << e.what() << "\"" << std::endl;
		gui2::show_error_message(_("A problem occurred when trying to create the files necessary to install this add-on."));
	} catch(const invalid_pbl_exception& e) {
		ERR_CFG << "could not read .pbl file " << e.path << ": " << e.message << std::endl;

		utils::string_map symbols;
		symbols["path"] = e.path;
		symbols["msg"] = e.message;

		gui2::show_error_message(
			vgettext("A local file with add-on publishing information could not be read.\n\nFile: $path\nError message: $msg", symbols));
	} catch(wml_exception& e) {
		e.show();
	} catch(const addons_client::user_exit&) {
		LOG_AC << "initial connection canceled by user\n";
	} catch(const addons_client::invalid_server_address&) {
		gui2::show_error_message(_("The add-ons server address specified is not valid."));
	}

	return false;
}
