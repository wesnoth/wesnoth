/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
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

#include "global.hpp"
#include "preferences_display.hpp"

#include "display.hpp"
#include "filechooser.hpp"
#include "filesystem.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/preferences_dialog.hpp"
#include "gui/dialogs/theme_list.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"

#include <boost/foreach.hpp>
#include <boost/math/common_factor_rt.hpp>

namespace preferences {

void show_preferences_dialog(CVideo& video, const config& game_cfg, const DIALOG_OPEN_TO initial_view)
{
	gui2::tpreferences dlg(video, game_cfg);

	switch (initial_view) {
		case VIEW_DEFAULT:
			// Default value (0,0) already set in tpreferences
			break;
		case VIEW_FRIENDS: {
			dlg.set_selected_index(std::make_pair(4, 1));
			break;
		}
	}

	dlg.show(video);
}

void set_scroll_to_action(bool ison)
{
	_set_scroll_to_action(ison);
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

void set_ellipses(bool ison)
{
	_set_ellipses(ison);
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

bool show_theme_dialog(CVideo& video)
{
	std::vector<theme_info> themes = theme::get_known_themes();

	if (themes.empty()) {
		gui2::show_transient_message(video, "",
			_("No known themes. Try changing from within an existing game."));

		return false;
	}

	gui2::ttheme_list dlg(themes);

	for (size_t k = 0; k < themes.size(); ++k) {
		if(themes[k].id == preferences::theme()) {
			dlg.set_selected_index(static_cast<int>(k));
		}
	}

	dlg.show(video);
	const int action = dlg.selected_index();

	if (action >= 0) {
		// FIXME: it would be preferable for the new theme to take effect
		//        immediately.
		preferences::set_theme(themes[action].id);

		return true;
	}

	return false;
}

std::string show_wesnothd_server_search(CVideo& video)
{
	// Showing file_chooser so user can search the wesnothd
	std::string old_path = preferences::get_mp_server_program_name();
	size_t offset = old_path.rfind("/");
	if (offset != std::string::npos)
	{
		old_path = old_path.substr(0, offset);
	}
	else
	{
		old_path.clear();
	}
#ifndef _WIN32

#ifndef WESNOTH_PREFIX
#define WESNOTH_PREFIX "/usr"
#endif
	const std::string filename = "wesnothd";
	std::string path = WESNOTH_PREFIX + std::string("/bin");
	if (!filesystem::is_directory(path))
		path = filesystem::get_cwd();

#else
	const std::string filename = "wesnothd.exe";
	std::string path = filesystem::get_cwd();
#endif
	if (!old_path.empty()
			&& filesystem::is_directory(old_path))
	{
		path = old_path;
	}

	utils::string_map symbols;

	symbols["filename"] = filename;

	const std::string title = utils::interpolate_variables_into_string(
			  _("Find $filename server binary")
			, &symbols);

	int res = dialogs::show_file_chooser_dialog(video, path, title, false, filename);
	if (res == 0)
		return path;
	else
		return "";
}

} // end namespace preferences

