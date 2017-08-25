/*
   Copyright (C) 2014 - 2017 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/game_cache_options.hpp"

#include "desktop/clipboard.hpp"
#include "config_cache.hpp"
#include "cursor.hpp"
#include "desktop/open.hpp"
#include "filesystem.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

#include "utils/functional.hpp"

#include "gettext.hpp"
#include "video.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_game_cache_options
 *
 * == Game cache options ==
 *
 * A Preferences subdialog including a report on the location and size of the
 * game's WML cache, buttons to copy its path to clipboard or browse to it,
 * and the possibility of clearing stale files from the cache or purging it
 * entirely.
 *
 * @begin{table}{dialog_widgets}
 *
 * path & & text_box & m &
 *        Cache dir path. $
 *
 * copy & & button & m &
 *        Copies the cache path to clipboard. $
 *
 * browse & & button & m &
 *        Browses to the cache path using the platform's file management
 *        application. $
 *
 * size & & label & m &
 *        Current total size of the cache dir's contents. $
 *
 * clean & & button & m &
 *        Cleans the cache, erasing stale files not used by the Wesnoth
 *        version presently running the dialog. $
 *
 * purge & & button & m &
 *        Purges the cache in its entirety. $
 *
 * @end{table}
 */

REGISTER_DIALOG(game_cache_options)

game_cache_options::game_cache_options()
	: cache_path_(filesystem::get_cache_dir())
	, clean_button_(nullptr)
	, purge_button_(nullptr)
	, size_label_(nullptr)
{
}

void game_cache_options::pre_show(window& window)
{
	clean_button_ = find_widget<button>(&window, "clean", false, true);
	purge_button_ = find_widget<button>(&window, "purge", false, true);
	size_label_ = find_widget<label>(&window, "size", false, true);

	update_cache_size_display();

	text_box_base& path_box = find_widget<text_box_base>(&window, "path", false);
	path_box.set_value(cache_path_);
	path_box.set_active(false);

	button& copy = find_widget<button>(&window, "copy", false);
	connect_signal_mouse_left_click(copy,
									std::bind(&game_cache_options::copy_to_clipboard_callback,
												this));
	if (!desktop::clipboard::available()) {
		copy.set_active(false);
		copy.set_tooltip(_("Clipboard support not found, contact your packager"));
	}

	button& browse = find_widget<button>(&window, "browse", false);
	connect_signal_mouse_left_click(browse,
									std::bind(&game_cache_options::browse_cache_callback,
												this));

	connect_signal_mouse_left_click(*clean_button_,
									std::bind(&game_cache_options::clean_cache_callback,
												this,
												std::ref(window.video())));

	connect_signal_mouse_left_click(*purge_button_,
									std::bind(&game_cache_options::purge_cache_callback,
												this,
												std::ref(window.video())));
}

void game_cache_options::post_show(window& /*window*/)
{
	size_label_ = nullptr;
}

void game_cache_options::update_cache_size_display()
{
	if(!size_label_) {
		return;
	}

	const cursor::setter cs(cursor::WAIT);
	const int size = filesystem::dir_size(cache_path_);

	if(size < 0) {
		size_label_->set_label(_("dir_size^Unknown"));
	} else {
		size_label_->set_label(utils::si_string(size, true, _("unit_byte^B")));
	}

	if(size == 0) {
		clean_button_->set_active(false);
		purge_button_->set_active(false);
	}
}

void game_cache_options::copy_to_clipboard_callback()
{
	desktop::clipboard::copy_to_clipboard(cache_path_, false);
}

void game_cache_options::browse_cache_callback()
{
	desktop::open_object(cache_path_);
}

void game_cache_options::clean_cache_callback(CVideo& video)
{
	if(clean_cache()) {
		show_message(video,
					 _("Cache Cleaned"),
					 _("The game data cache has been cleaned."));
	} else {
		show_error_message(video,
						   _("The game data cache could not be completely cleaned."));
	}

	update_cache_size_display();
}

bool game_cache_options::clean_cache()
{
	const cursor::setter cs(cursor::WAIT);
	return game_config::config_cache::instance().clean_cache();
}

void game_cache_options::purge_cache_callback(CVideo& video)
{
	if(purge_cache()) {
		show_message(video,
					 _("Cache Purged"),
					 _("The game data cache has been purged."));
	} else {
		show_error_message(video,
						   _("The game data cache could not be purged."));
	}

	update_cache_size_display();
}

bool game_cache_options::purge_cache()
{
	const cursor::setter cs(cursor::WAIT);
	return game_config::config_cache::instance().purge_cache();
}

} // namespace dialogs
} // namespace gui2
