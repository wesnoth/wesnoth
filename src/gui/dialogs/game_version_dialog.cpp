/*
	Copyright (C) 2013 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/game_version_dialog.hpp"

#include "build_info.hpp"
#include "desktop/clipboard.hpp"
#include "desktop/open.hpp"
#include "desktop/version.hpp"
#include "filesystem.hpp"
#include "formula/string_utils.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/migrate_version_selection.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/tab_container.hpp"
#include "gui/widgets/text_box_base.hpp"
#include "gui/widgets/window.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/end_credits.hpp"
#include "gettext.hpp"
#include "help/help.hpp"

#include <functional>

namespace
{

const std::string text_feature_on =  "<span color='#0f0'>&#9679;</span>";
const std::string text_feature_off = "<span color='#f00'>&#9679;</span>";

} // end anonymous namespace

namespace gui2::dialogs
{

REGISTER_DIALOG(game_version)

game_version::game_version(unsigned start_page)
	: modal_dialog(window_id())
	, path_wid_stem_("path_")
	, copy_wid_stem_("copy_")
	, browse_wid_stem_("browse_")
	, path_map_()
	, log_path_(lg::get_log_file_path())
	, deps_()
	, opts_(game_config::optional_features_table())
	, report_()
	, start_page_(start_page)
{
	// NOTE: these path_map_ entries are referenced by the GUI2 WML
	// definition of this dialog using preprocessor macros.
	path_map_["datadir"] = game_config::path;
	path_map_["userdata"] = filesystem::get_user_data_dir();
	path_map_["saves"] = filesystem::get_saves_dir();
	path_map_["addons"] = filesystem::get_addons_dir();
	path_map_["cache"] = filesystem::get_cache_dir();
	// path to logs directory
	path_map_["logs"] = filesystem::get_logs_dir();
	path_map_["screenshots"] = filesystem::get_screenshot_dir();

	for(unsigned k = 0; k < game_config::LIB_COUNT; ++k) {
		const game_config::LIBRARY_ID lib = game_config::LIBRARY_ID(k);

		deplist_entry e;
		e[0] = game_config::library_name(lib);
		if(e[0].empty()) {
			continue;
		}
		e[1] = game_config::library_build_version(lib);
		e[2] = game_config::library_runtime_version(lib);
		deps_.push_back(e);
	}

	generate_plain_text_report();
}

void game_version::pre_show(window& window)
{
	utils::string_map i18n_syms;

	tab_container& tabs = find_widget<tab_container>(&window, "tabs", false);

	//
	// General information.
	//
	tabs.select_tab(0);

	styled_widget& version_label = find_widget<styled_widget>(&window, "version", false);
	styled_widget& os_label = find_widget<styled_widget>(&window, "os", false);
	styled_widget& arch_label = find_widget<styled_widget>(&window, "arch", false);

	version_label.set_label(game_config::revision);
	os_label.set_label("<i>"+desktop::os_version()+"</i>");
	arch_label.set_label(game_config::build_arch());

	button& copy_all = find_widget<button>(&window, "copy_all", false);
	connect_signal_mouse_left_click(copy_all, std::bind(&game_version::report_copy_callback, this));

	// Bottom row buttons
	button& credits_button = find_widget<button>(&window, "credits", false);
	connect_signal_mouse_left_click(credits_button, std::bind(&game_version::show_credits_dialog, this));

	button& license_button = find_widget<button>(&window, "license", false);
	connect_signal_mouse_left_click(license_button, std::bind(&game_version::show_license, this));

	button& issue_button = find_widget<button>(&window, "issue", false);
	connect_signal_mouse_left_click(issue_button, std::bind(&game_version::report_issue, this));

	connect_signal_mouse_left_click(find_widget<button>(&window, "run_migrator", false), std::bind(&game_version::run_migrator, this));

	//
	// Game paths tab.
	//
	tabs.select_tab(1);

	for(const auto & path_ent : path_map_)
	{
		const std::string& path_id = path_ent.first;
		const std::string& path_path = filesystem::normalize_path(path_ent.second, true);

		text_box_base& path_w = find_widget<text_box_base>(&window, path_wid_stem_ + path_id, false);
		button& copy_w = find_widget<button>(&window, copy_wid_stem_ + path_id, false);
		button& browse_w = find_widget<button>(&window, browse_wid_stem_ + path_id, false);

		path_w.set_value(path_path);

		connect_signal_mouse_left_click(
				copy_w,
				std::bind(&game_version::copy_to_clipboard_callback, this, path_path, copy_wid_stem_ + path_id));
		connect_signal_mouse_left_click(
				browse_w,
				std::bind(&game_version::browse_directory_callback, this, path_path));

		if(!desktop::open_object_is_supported()) {
			// No point in displaying these on platforms that can't do
			// open_object().
			browse_w.set_visible(widget::visibility::invisible);
		}
	}

	button& stderr_button = find_widget<button>(&window, "open_stderr", false);
	connect_signal_mouse_left_click(stderr_button, std::bind(&game_version::browse_directory_callback, this, log_path_));
	stderr_button.set_active(!log_path_.empty() && filesystem::file_exists(log_path_));

	//
	// Build info tab.
	//
	tabs.select_tab(2);

	widget_data list_data;

	listbox& deps_listbox
			= find_widget<listbox>(&window, "deps_listbox", false);

	for(const auto & dep : deps_)
	{
		list_data["dep_name"]["label"] = dep[0];

		list_data["dep_build_version"]["label"] = dep[1];

		// The build version is always known, but runtime version isn't, esp.
		// for header-only libraries like Boost for which the concept does not
		// apply.
		if(!dep[2].empty()) {
			list_data["dep_rt_version"]["label"] = dep[2];
		} else {
			list_data["dep_rt_version"]["label"] = font::unicode_em_dash;
		}

		deps_listbox.add_row(list_data);
	}

	deps_listbox.select_row(0);
	list_data.clear();

	//
	// Features tab.
	//
	tabs.select_tab(3);

	listbox& opts_listbox
			= find_widget<listbox>(&window, "opts_listbox", false);

	for(const auto & opt : opts_)
	{
		list_data["opt_name"]["label"] = opt.name;

		if(opt.enabled) {
			list_data["opt_status"]["label"] = text_feature_on;
		} else {
			list_data["opt_status"]["label"] = text_feature_off;
		}
		list_data["opt_status"]["use_markup"] = "true";

		opts_listbox.add_row(list_data);
	}

	opts_listbox.select_row(0);
	list_data.clear();

	//
	// Community tab
	//
	tabs.select_tab(4);

	connect_signal_mouse_left_click(find_widget<button>(&window, "forums", false), std::bind(&desktop::open_object, "https://forums.wesnoth.org/"));
	connect_signal_mouse_left_click(find_widget<button>(&window, "discord", false), std::bind(&desktop::open_object, "https://discord.gg/battleforwesnoth"));
	connect_signal_mouse_left_click(find_widget<button>(&window, "irc", false), std::bind(&desktop::open_object, "https://web.libera.chat/#wesnoth"));
	connect_signal_mouse_left_click(find_widget<button>(&window, "steam", false), std::bind(&desktop::open_object, "https://steamcommunity.com/app/599390/discussions/"));
	connect_signal_mouse_left_click(find_widget<button>(&window, "reddit", false), std::bind(&desktop::open_object, "https://www.reddit.com/r/wesnoth/"));
	connect_signal_mouse_left_click(find_widget<button>(&window, "donate", false), std::bind(&desktop::open_object, "https://www.spi-inc.org/projects/wesnoth/"));

	//
	// Set-up page stack and auxiliary controls last.
	//

	tabs.select_tab(start_page_);
}

void game_version::browse_directory_callback(const std::string& path)
{
	desktop::open_object(path);
}

void game_version::run_migrator()
{
	migrate_version_selection::execute();
}

void game_version::copy_to_clipboard_callback(const std::string& path, const std::string btn_id)
{
	desktop::clipboard::copy_to_clipboard(path);

	button& copy_w = find_widget<button>(get_window(), btn_id, false);
	copy_w.set_success(true);
}

void game_version::report_copy_callback()
{
	desktop::clipboard::copy_to_clipboard(report_);

	button& copy_all = find_widget<button>(get_window(), "copy_all", false);
	copy_all.set_success(true);
}

void game_version::generate_plain_text_report()
{
	report_ = game_config::full_build_report();
}

void game_version::show_credits_dialog() {
	gui2::dialogs::end_credits::display();
}

void game_version::show_license() {
	help::show_help("license");
}

void game_version::report_issue() {
	if (!desktop::open_object_is_supported()) {
		show_message("", _("Opening links is not supported, contact your packager"), dialogs::message::auto_close);
		return;
	} else {
		desktop::open_object("https://bugs.wesnoth.org");
	}
}

} // namespace dialogs
