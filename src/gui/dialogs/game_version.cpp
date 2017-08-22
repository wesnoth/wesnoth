/*
   Copyright (C) 2013 - 2017 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/game_version.hpp"

#include "build_info.hpp"
#include "desktop/clipboard.hpp"
#include "desktop/open.hpp"
#include "desktop/version.hpp"
#include "filesystem.hpp"
#include "formula/string_utils.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/styled_widget.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/selectable_item.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/text_box_base.hpp"
#include "gui/widgets/window.hpp"
#ifdef _WIN32
#include "log_windows.hpp"
#endif
#include "serialization/string_utils.hpp"

#include "gettext.hpp"

#include "utils/functional.hpp"

namespace
{

const std::string text_feature_on =  "<span color='#0f0'>&#9679;</span>";
const std::string text_feature_off = "<span color='#f00'>&#9679;</span>";

} // end anonymous namespace

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_game_version
 *
 * == Game paths ==
 *
 * Dialog displaying the various paths used by the game to locate
 * resource and configuration files.
 *
 * There are several item types used to build widget ids in this dialog.
 * All references to TYPE below refer to the following suffixes:
 * datadir, config, userdata, saves, addons, cache.
 *
 * @begin{table}{dialog_widgets}
 *
 * path_TYPE & & text_box & m &
 *        Textbox containing the filesystem path for the given item. $
 *
 * copy_TYPE & & button & m &
 *        Copies the given item's path to clipboard. $
 *
 * browse_TYPE & & button & m &
 *        Launches the default file browser on the given item's path. $
 *
 * @end{table}
 */

REGISTER_DIALOG(game_version)

game_version::game_version()
	: path_wid_stem_("path_")
	, copy_wid_stem_("copy_")
	, browse_wid_stem_("browse_")
	, path_map_()
#ifdef _WIN32
	, log_path_(lg::log_file_path())
#endif
	, deps_()
	, opts_(game_config::optional_features_table())
	, report_()
{
	// NOTE: these path_map_ entries are referenced by the GUI2 WML
	// definition of this dialog using preprocessor macros.
	path_map_["datadir"] = game_config::path;
	path_map_["config"] = filesystem::get_user_config_dir();
	path_map_["userdata"] = filesystem::get_user_data_dir();
	path_map_["saves"] = filesystem::get_saves_dir();
	path_map_["addons"] = filesystem::get_addons_dir();
	path_map_["cache"] = filesystem::get_cache_dir();

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
	string_map i18n_syms;

	//
	// General information.
	//

	styled_widget& version_label = find_widget<styled_widget>(&window, "version", false);
	i18n_syms["version"] = game_config::revision;
	version_label.set_label(VGETTEXT("Version $version", i18n_syms));

	styled_widget& os_label = find_widget<styled_widget>(&window, "os", false);
	i18n_syms["os"] = desktop::os_version();
	os_label.set_label(VGETTEXT("Running on $os", i18n_syms));

	button& copy_all = find_widget<button>(&window, "copy_all", false);
	connect_signal_mouse_left_click(
			copy_all,
			std::bind(&game_version::report_copy_callback, this));

	//
	// Game paths tab.
	//

	for(const auto & path_ent : path_map_)
	{
		const std::string& path_id = path_ent.first;
		const std::string& path_path = path_ent.second;

		text_box_base& path_w
				= find_widget<text_box_base>(&window, path_wid_stem_ + path_id, false);
		button& copy_w = find_widget<button>(
				&window, copy_wid_stem_ + path_id, false);
		button& browse_w = find_widget<button>(
				&window, browse_wid_stem_ + path_id, false);

		path_w.set_value(path_path);
		path_w.set_active(false);

		connect_signal_mouse_left_click(
				copy_w,
				std::bind(&game_version::copy_to_clipboard_callback,
							this,
							path_path));
		connect_signal_mouse_left_click(
				browse_w,
				std::bind(&game_version::browse_directory_callback,
							this,
							path_path));

		if(!desktop::open_object_is_supported()) {
			// No point in displaying these on platforms that can't do
			// open_object().
			browse_w.set_visible(widget::visibility::invisible);
		}

		if(!desktop::clipboard::available()) {
			copy_w.set_active(false);
			copy_w.set_tooltip(_("Clipboard support not found, contact your packager"));
		}
	}

#ifndef _WIN32
	grid& w32_options_grid
			= find_widget<grid>(&window, "win32_paths", false);
	w32_options_grid.set_visible(widget::visibility::invisible);
#else
	button& stderr_button
			= find_widget<button>(&window, "open_stderr", false);
	connect_signal_mouse_left_click(
			stderr_button,
			std::bind(&game_version::browse_directory_callback,
						this,
						log_path_));
	stderr_button.set_active(!log_path_.empty());
#endif

	//
	// Build info tab.
	//

	std::map<std::string, string_map> list_data;

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
			list_data["dep_rt_version"]["label"] = _("version^N/A");
		}

		deps_listbox.add_row(list_data);
	}

	deps_listbox.select_row(0);
	list_data.clear();

	//
	// Features tab.
	//

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
	// Set-up page stack and auxiliary controls last.
	//

	stacked_widget& pager
			= find_widget<stacked_widget>(&window, "tabs_container", false);
	pager.select_layer(0);

	listbox& tab_bar
			= find_widget<listbox>(&window, "tab_bar", false);

	window.keyboard_capture(&tab_bar);

	const unsigned tab_count = tab_bar.get_item_count();
	VALIDATE(tab_count == pager.get_layer_count(), "Tab bar and container size mismatch");

	connect_signal_notify_modified(tab_bar,
		std::bind(&game_version::tab_switch_callback, this, std::ref(window)));
}

void game_version::tab_switch_callback(window& window)
{
	stacked_widget& pager
			= find_widget<stacked_widget>(&window, "tabs_container", false);
	listbox& tab_bar
			= find_widget<listbox>(&window, "tab_bar", false);

	pager.select_layer(std::max<int>(0, tab_bar.get_selected_row()));
}

void game_version::browse_directory_callback(const std::string& path)
{
	desktop::open_object(path);
}

void game_version::copy_to_clipboard_callback(const std::string& path)
{
	desktop::clipboard::copy_to_clipboard(path, false);
}

void game_version::report_copy_callback()
{
	desktop::clipboard::copy_to_clipboard(report_, false);
}

void game_version::generate_plain_text_report()
{
	report_ = game_config::full_build_report();
}

} // namespace dialogs
} // namespace gui2
