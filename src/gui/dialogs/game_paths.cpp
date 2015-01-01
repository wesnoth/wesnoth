/*
   Copyright (C) 2013 - 2015 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/game_paths.hpp"

#include "clipboard.hpp"
#include "desktop_util.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "gui/auxiliary/find_widget.tpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/control.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text.hpp"
#include "gui/widgets/window.hpp"
#include "serialization/string_utils.hpp"

#include <boost/bind.hpp>

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_game_paths
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

REGISTER_DIALOG(game_paths)

tgame_paths::tgame_paths()
	: path_wid_stem_("path_")
	, copy_wid_stem_("copy_")
	, browse_wid_stem_("browse_")
	, path_map_()
{
	// NOTE: these path_map_ entries are referenced by the GUI2 WML
	// definition of this dialog using preprocessor macros.
	path_map_["datadir"] = game_config::path;
	path_map_["config"] = filesystem::get_user_config_dir();
	path_map_["userdata"] = filesystem::get_user_data_dir();
	path_map_["saves"] = filesystem::get_saves_dir();
	path_map_["addons"] = filesystem::get_addons_dir();
	path_map_["cache"] = filesystem::get_cache_dir();
}

void tgame_paths::pre_show(CVideo& /*video*/, twindow& window)
{
	FOREACH(const AUTO & path_ent, path_map_)
	{
		const std::string& path_id = path_ent.first;
		const std::string& path_path = path_ent.second;

		ttext_& path_w
				= find_widget<ttext_>(&window, path_wid_stem_ + path_id, false);
		tbutton& copy_w = find_widget<tbutton>(
				&window, copy_wid_stem_ + path_id, false);
		tbutton& browse_w = find_widget<tbutton>(
				&window, browse_wid_stem_ + path_id, false);

		path_w.set_value(path_path);
		path_w.set_active(false);

		connect_signal_mouse_left_click(
				copy_w,
				boost::bind(&tgame_paths::copy_to_clipboard_callback,
							this,
							path_path));
		connect_signal_mouse_left_click(
				browse_w,
				boost::bind(&tgame_paths::browse_directory_callback,
							this,
							path_path));

		if(!desktop::open_object_is_supported()) {
			// No point in displaying these on platforms that can't do
			// open_object().
			browse_w.set_visible(tcontrol::tvisible::invisible);
		}
	}
}

void tgame_paths::browse_directory_callback(const std::string& path)
{
	desktop::open_object(path);
}

void tgame_paths::copy_to_clipboard_callback(const std::string& path)
{
	copy_to_clipboard(path, false);
}
}
