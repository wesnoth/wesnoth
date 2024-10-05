/*
	Copyright (C) 2008 - 2024
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

#include "gui/dialogs/migrate_version_selection.hpp"

#include "addon/manager_ui.hpp"
#include "filesystem.hpp"
#include "game_version.hpp"
#include "gettext.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/window.hpp"
#include "preferences/preferences.hpp"
#include "serialization/parser.hpp"

#include <boost/algorithm/string.hpp>

static lg::log_domain log_version_migration{"gui/dialogs/migrate_version_selection"};
#define ERR_LOG_VERSION_MIGRATION LOG_STREAM(err, log_version_migration)
#define WRN_LOG_VERSION_MIGRATION LOG_STREAM(warn, log_version_migration)
#define LOG_LOG_VERSION_MIGRATION LOG_STREAM(info, log_version_migration)
#define DBG_LOG_VERSION_MIGRATION LOG_STREAM(debug, log_version_migration)

namespace gui2::dialogs
{
REGISTER_DIALOG(migrate_version_selection)

void migrate_version_selection::execute()
{
	migrate_version_selection mig = migrate_version_selection();
	if(mig.versions_.size() > 0) {
		mig.show();
	} else {
		gui2::show_message(_("No Other Version Found"), _("This would import settings from a previous version of Wesnoth, but no other version was found on this device"), gui2::dialogs::message::button_style::auto_close);
	}
}

migrate_version_selection::migrate_version_selection()
	: modal_dialog(window_id())
{
	version_info current_version = game_config::wesnoth_version;
	std::string current_version_str = filesystem::get_version_path_suffix();

	for(unsigned int i = 1; i < current_version.minor_version(); i++) {
		std::string previous_version_str = std::to_string(current_version.major_version()) + "."
			+ std::to_string(current_version.minor_version() - i);
		std::string previous_addons_dir
			= boost::replace_all_copy(filesystem::get_addons_dir(), current_version_str, previous_version_str);

		if(previous_addons_dir != filesystem::get_addons_dir() && filesystem::file_exists(previous_addons_dir)) {
			versions_.push_back(previous_version_str);
		}
	}
}

void migrate_version_selection::pre_show()
{
	listbox& version_list = find_widget<listbox>("versions_listbox");

	for(const auto& version : versions_) {
		widget_data data;
		widget_item item_label;

		item_label["label"] = version;
		data["version_label"] = item_label;

		version_list.add_row(data);
	}
}

void migrate_version_selection::post_show()
{
	if(get_retval() == gui2::OK) {
		std::string current_version_str = filesystem::get_version_path_suffix();
		listbox& version_list = find_widget<listbox>("versions_listbox");
		int selected_row = version_list.get_selected_row();
		std::string selected = versions_.at(selected_row);

		std::string migrate_addons_dir
			= boost::replace_all_copy(filesystem::get_addons_dir(), current_version_str, selected);
		std::string migrate_synced_prefs_file
			= boost::replace_all_copy(filesystem::get_synced_prefs_file(), current_version_str, selected);
		std::string migrate_unsynced_prefs_file
			= boost::replace_all_copy(filesystem::get_unsynced_prefs_file(), current_version_str, selected);
		std::string migrate_credentials_file
			= boost::replace_all_copy(filesystem::get_credentials_file(), current_version_str, selected);

		// given self-compilation and linux distros being able to do whatever they want plus command line options to
		// alter locations make sure the directories/files are actually different before doing anything with them
		if(migrate_addons_dir != filesystem::get_addons_dir()) {
			std::vector<std::string> old_addons;
			std::vector<std::string> current_addons;
			std::vector<std::string> migrate_addons;

			filesystem::get_files_in_dir(migrate_addons_dir, nullptr, &old_addons);
			filesystem::get_files_in_dir(filesystem::get_addons_dir(), nullptr, &current_addons);

			std::set_difference(old_addons.begin(), old_addons.end(), current_addons.begin(), current_addons.end(), std::back_inserter(migrate_addons));

			if(migrate_addons.size() > 0) {
				ad_hoc_addon_fetch_session(migrate_addons);
			}
		}

#if !defined(_WIN32) && !defined(__APPLE__)
		bool already_migrated = false;
		std::string linux_old_config_dir = old_config_dir();
		std::string old_migrate_prefs_file = linux_old_config_dir + "/preferences";
		std::string old_migrate_credentials_file = linux_old_config_dir + "/credentials-aes";

		if(filesystem::file_exists(old_migrate_prefs_file)) {
			already_migrated = true;
			prefs::get().migrate_preferences(old_migrate_prefs_file);
		}
		if(filesystem::file_exists(old_migrate_credentials_file)) {
			already_migrated = true;
			migrate_credentials(old_migrate_credentials_file);
		}

		if(!already_migrated)
#endif
		{
			prefs::get().migrate_preferences(migrate_unsynced_prefs_file);
			prefs::get().migrate_preferences(migrate_synced_prefs_file);
			migrate_credentials(migrate_credentials_file);
		}

		// reload preferences and credentials
		// otherwise the copied files won't be used and also will get overwritten/deleted when Wesnoth closes

		prefs::get().reload_preferences();
	}
}

/**
 * Prior to 1.19 linux installs would usually store the credentials and preferences file under XDG_CONFIG_HOME with no version separation.
 * That special handling has been removed, but still needs to be accounted for when migrating
 */
std::string migrate_version_selection::old_config_dir()
{
	char const* xdg_config = getenv("XDG_CONFIG_HOME");
	std::string old_config_dir;

	if(!xdg_config || xdg_config[0] == '\0') {
		xdg_config = getenv("HOME");
		if(!xdg_config) {
			old_config_dir = filesystem::get_user_data_dir();
			return old_config_dir;
		}

		old_config_dir = xdg_config;
		old_config_dir += "/.config";
	} else {
		old_config_dir = xdg_config;
	}

	old_config_dir += "/wesnoth";
	return old_config_dir;
}

void migrate_version_selection::migrate_credentials(const std::string& migrate_credentials_file)
{
	// don't touch the credentials file on migrator re-run if it already exists
	if(migrate_credentials_file != filesystem::get_credentials_file() && filesystem::file_exists(migrate_credentials_file) && !filesystem::file_exists(filesystem::get_credentials_file())) {
		filesystem::copy_file(migrate_credentials_file, filesystem::get_credentials_file());
	}
}

} // namespace gui2::dialogs
