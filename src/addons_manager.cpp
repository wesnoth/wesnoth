/* $Id$ */
/*
   Copyright (C) 2008 by Ignacio Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#if 0

#include "global.hpp"
#include "log.hpp"

#include "config.hpp"
#include "dialogs.hpp"
#include "publish_campaign.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"
#include "wml_separators.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <sstream>

void addons_manager(game_display &disp, const config& campaigns_cfg)
{
	std::vector<std::string> campaigns, versions, uploads, options;

	std::string sep(1, COLUMN_SEPARATOR);

	std::stringstream heading;
	heading << HEADING_PREFIX << sep << _("Name") << sep << _("Version") << sep
			<< _("Author") << sep << _("Downloads") << sep << _("Size");

	const config::child_list& cmps = campaigns_cfg->get_children("campaign");
	const std::vector<std::string>& publish_options = available_campaigns();

	std::vector<std::string> delete_options;

	std::vector<int> sizes;

	for(config::child_list::const_iterator i = cmps.begin(); i != cmps.end(); ++i) {
		const std::string& name = (**i)["name"];
		campaigns.push_back(name);
		versions.push_back((**i)["version"]);
		uploads.push_back((**i)["uploads"]);

		if(std::count(publish_options.begin(),publish_options.end(),name) != 0) {
			delete_options.push_back(name);
		}

		std::string title = (**i)["title"];
		if(title == "") {
			title = name;
			std::replace(title.begin(),title.end(),'_',' ');
		}

		std::string version   = (**i)["version"],
					author    = (**i)["author"];

		utils::truncate_as_wstring(title, 20);
		utils::truncate_as_wstring(version, 12);
		utils::truncate_as_wstring(author, 16);

		//add negative sizes to reverse the sort order
		sizes.push_back(-atoi((**i)["size"].c_str()));

		std::string icon = (**i)["icon"];
		if(icon.find("units/") != std::string::npos
		&& icon.find_first_of('~') == std::string::npos) {
			//a hack to prevent magenta icons, because they look awful
			icon.append("~RC(magenta>red)");
		}
		options.push_back(IMAGE_PREFIX + icon + COLUMN_SEPARATOR +
							title + COLUMN_SEPARATOR +
							version + COLUMN_SEPARATOR +
							author + COLUMN_SEPARATOR +
							(**i)["downloads"].str() + COLUMN_SEPARATOR +
							format_file_size((**i)["size"]));
	}
	options.push_back(heading.str());

	for(std::vector<std::string>::const_iterator j = publish_options.begin(); j != publish_options.end(); ++j) {
		options.push_back(sep + _("Publish add-on: ") + *j);
	}

	for(std::vector<std::string>::const_iterator d = delete_options.begin(); d != delete_options.end(); ++d) {
		options.push_back(sep + _("Delete add-on: ") + *d);
	}

	if(campaigns.empty() && publish_options.empty()) {
		gui::show_error_message(disp(), _("There are no add-ons available for download from this server."));
		return;
	}

	gui::menu::basic_sorter sorter;
	sorter.set_alpha_sort(1).set_alpha_sort(2).set_alpha_sort(3).set_numeric_sort(4).set_position_sort(5,sizes);

	gui::dialog addon_dialog(disp(), _("Get Add-ons"),
						_("Choose the add-on to download."),
						gui::OK_CANCEL);
	gui::menu::imgsel_style addon_style(gui::menu::bluebg_style);

	//make sure the icon isn't too big
	addon_style.scale_images(font::relative_size(72), font::relative_size(72));
	gui::menu *addon_menu = new gui::menu(disp().video(), options, false, -1,
		gui::dialog::max_menu_width, &sorter, &addon_style, false);
	addon_dialog.set_menu(addon_menu);
	const int index = addon_dialog.show();
	if(index < 0) {
		return;
	}

	if(index >= int(campaigns.size() + publish_options.size())) {
		delete_campaign(delete_options[index - int(campaigns.size() + publish_options.size())],sock);
		return;
	}

	if(index >= int(campaigns.size())) {
		upload_campaign(publish_options[index - int(campaigns.size())],sock);
		return;
	}

	// Get all dependencies of the campaign selected for download.
	const config *selected_campaign = campaigns_cfg->find_child("campaign", "name", campaigns[index]);
	std::vector<std::string> dependencies = utils::split((*selected_campaign)["dependencies"]);
	if (!dependencies.empty()) {
		// Get all dependencies which are not already installed.
		// TODO: Somehow determine if the version is outdated.
		const std::vector<std::string>& installed = installed_campaigns();
		std::vector<std::string>::iterator i;
		std::string missing = "";
		for (i = dependencies.begin(); i != dependencies.end(); i++) {
			if (std::find(installed.begin(), installed.end(), *i) == installed.end()) {
				missing += "\n" + *i;
			}
		}
		// If there are any, display a message.
		// TODO: Somehow offer to automatically download
		// the missing dependencies.
		if (!missing.empty()) {
			if (gui::dialog(disp(),
							_("Dependencies"),
							std::string(_("This add-on requires the following additional dependencies:")) +
						"\n" + missing +
						"\n" + _("Do you still want to download it?"), gui::OK_CANCEL).show())
				return;
		}
	}

	config request;
	request.add_child("request_campaign")["name"] = campaigns[index];
	// @todo Should be enabled once the campaign server can be recompiled.
	network::send_data(request, sock, false);

	res = dialogs::network_receive_dialog(disp(),_("Downloading add-on..."),cfg,sock);
	if(!res) {
		return;
	}

	if(cfg.child("error") != NULL) {
		gui::show_error_message(disp(), (*cfg.child("error"))["message"]);
		return;
	}

	if(!check_names_legal(cfg)) {
		gui::show_error_message(disp(), "The add-on has an invalid file or directory name and can not be installed.");
		return;
	}

	//remove any existing versions of the just downloaded campaign
	//assuming it consists of a dir and a cfg file
	remove_campaign(campaigns[index]);

	//add revision info to the addon
			config *maindir = cfg.find_child("dir", "name", campaigns[index]);
			if (maindir) {
				config f;
				f["name"] = "info.cfg";
				std::string s;
				s += "[info]\n";
				s += "version=\"" + versions[index] + "\"\n";
				s += "uploads=\"" + uploads[index] + "\"\n";
				s += "[/info]\n";
				f["contents"] = s;
				maindir->add_child("file", f);
			}

	//put a break at line below to see that it really works.
	unarchive_campaign(cfg);

	//force a reload of configuration information
	const bool old_cache = use_caching_;
	use_caching_ = false;
	old_defines_map_.clear();
	refresh_game_cfg();
	use_caching_ = old_cache;
	::init_textdomains(game_config_);
	paths_manager_.set_paths(game_config_);

	clear_binary_paths_cache();

	std::string warning = "";
	std::vector<config *> scripts = find_scripts(cfg, ".unchecked");
	if (!scripts.empty()) {
		warning += "\nUnchecked script files found:";
		std::vector<config *>::iterator i;
		for (i = scripts.begin(); i != scripts.end(); ++i) {
			warning += "\n" + (**i)["name"];
		}
	}

	/* GCC-3.3 needs a temp var otherwise compilation fails */
	gui::message_dialog dlg(disp(),_("Add-on Installed"),_("The add-on has been installed."));
	dlg.show();
}
#endif
