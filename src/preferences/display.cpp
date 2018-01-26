/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

/**
 *  @file
 *  Manage display-related preferences, e.g. screen-size, etc.
 */

#include "preferences/display.hpp"

#include "display.hpp"
#include "filesystem.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "preferences/game.hpp"
#include "gettext.hpp"
#include "gui/dialogs/file_dialog.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/preferences_dialog.hpp"
#include "gui/dialogs/theme_list.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "play_controller.hpp"
#include "game_data.hpp"
#include "game_display.hpp"
#include "game_config_manager.hpp"
#include "resources.hpp"

#include <boost/math/common_factor_rt.hpp>

namespace preferences {

void set_preference_display_settings()
{
	set_grid(grid());
	set_turbo(turbo());
	set_turbo_speed(turbo_speed());
	set_color_cursors(preferences::get("color_cursors", true));
}

void set_turbo(bool ison)
{
	_set_turbo(ison);

	if(display::get_singleton()) {
		display::get_singleton()->set_turbo(ison);
	}
}

void set_turbo_speed(double speed)
{
	save_turbo_speed(speed);

	if(display::get_singleton()) {
		display::get_singleton()->set_turbo_speed(speed);
	}
}

void set_grid(bool ison)
{
	_set_grid(ison);

	if(display::get_singleton()) {
		display::get_singleton()->set_grid(ison);
	}
}

void set_color_cursors(bool value)
{
	_set_color_cursors(value);

	cursor::set();
}

void set_idle_anim(bool ison) {
	_set_idle_anim(ison);

	if(display::get_singleton()) {
		display::get_singleton()->set_idle_anim(ison);
	}
}

void set_idle_anim_rate(int rate) {
	_set_idle_anim_rate(rate);

	if(display::get_singleton()) {
		display::get_singleton()->set_idle_anim_rate(rate);
	}
}

bool show_theme_dialog()
{
	std::vector<theme_info> themes = theme::get_known_themes();

	if (themes.empty()) {
		gui2::show_transient_message("",
			_("No known themes. Try changing from within an existing game."));

		return false;
	}

	gui2::dialogs::theme_list dlg(themes);

	for (size_t k = 0; k < themes.size(); ++k) {
		if(themes[k].id == preferences::theme()) {
			dlg.set_selected_index(static_cast<int>(k));
		}
	}

	dlg.show();
	const int action = dlg.selected_index();

	if (action >= 0) {
		preferences::set_theme(themes[action].id);
		if(game_display::get_singleton() && resources::controller && resources::gamedata && resources::gamedata->get_theme().empty()) {
			game_display::get_singleton()->set_theme(resources::controller->get_theme(game_config_manager::get()->game_config(), themes[action].id));
		}

		return true;
	}

	return false;
}

void show_wesnothd_server_search()
{
#ifndef _WIN32
	const std::string filename = "wesnothd";
#else // _WIN32
	const std::string filename = "wesnothd.exe";
#endif

	const std::string& old_path = filesystem::directory_name(preferences::get_mp_server_program_name());
	std::string path =
		!old_path.empty() && filesystem::is_directory(old_path)
		? old_path : filesystem::get_exe_dir();

	const std::string msg = vgettext(
			  "The <b>$filename</b> server application provides multiplayer server functionality and is required for hosting local network games. It will normally be found in the same folder as the game executable.", {{"filename", filename}});

	gui2::dialogs::file_dialog dlg;

	dlg.set_title(_("Find Server Application"))
	   .set_message(msg)
	   .set_ok_label(_("Select"))
	   .set_read_only(true)
	   .set_filename(filename)
	   .set_path(path);

	if(dlg.show()) {
		path = dlg.path();
		preferences::set_mp_server_program_name(path);
	}
}

} // end namespace preferences

