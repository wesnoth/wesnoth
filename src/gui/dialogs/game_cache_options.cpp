/*
   Copyright (C) 2014 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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
#include "desktop/open.hpp"
#include "filesystem.hpp"
#include "gui/auxiliary/find_widget.tpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

#include <boost/bind.hpp>

#include "gettext.hpp"

namespace gui2
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

tgame_cache_options::tgame_cache_options()
	: cache_path_(filesystem::get_cache_dir())
	, size_label_(NULL)
{
}

void tgame_cache_options::pre_show(CVideo& video, twindow& window)
{
	size_label_ = &find_widget<tlabel>(&window, "size", false);
	update_cache_size_display();

	ttext_& path_box = find_widget<ttext_>(&window, "path", false);
	path_box.set_value(cache_path_);
	path_box.set_active(false);

	tbutton& copy = find_widget<tbutton>(&window, "copy", false);
	connect_signal_mouse_left_click(copy,
									boost::bind(&tgame_cache_options::copy_to_clipboard_callback,
												this));
	if (!desktop::clipboard::available()) {
		copy.set_active(false);
	}

	tbutton& browse = find_widget<tbutton>(&window, "browse", false);
	connect_signal_mouse_left_click(browse,
									boost::bind(&tgame_cache_options::browse_cache_callback,
												this));

	tbutton& clean = find_widget<tbutton>(&window, "clean", false);
	connect_signal_mouse_left_click(clean,
									boost::bind(&tgame_cache_options::clean_cache_callback,
												this,
												boost::ref(video)));

	tbutton& purge = find_widget<tbutton>(&window, "purge", false);
	connect_signal_mouse_left_click(purge,
									boost::bind(&tgame_cache_options::purge_cache_callback,
												this,
												boost::ref(video)));
}

void tgame_cache_options::post_show(twindow& /*window*/)
{
	size_label_ = NULL;
}

void tgame_cache_options::update_cache_size_display()
{
	if(!size_label_) {
		return;
	}

	size_label_->set_label(utils::si_string(filesystem::dir_size(cache_path_),
											true,
											_("unit_byte^B")));
}

void tgame_cache_options::copy_to_clipboard_callback()
{
	desktop::clipboard::copy_to_clipboard(cache_path_, false);
}

void tgame_cache_options::browse_cache_callback()
{
	desktop::open_object(cache_path_);
}

void tgame_cache_options::clean_cache_callback(CVideo& video)
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

bool tgame_cache_options::clean_cache()
{
	return game_config::config_cache::instance().clean_cache();
}

void tgame_cache_options::purge_cache_callback(CVideo& video)
{
	if(show_message(video,
					 _("Purge Cache"),
					 _("Are you sure you want to purge the game data cache? "
					   "All files in the cache directory will be deleted, and "
					   "the cache will be regenerated next time it is "
					   "required."),
					 gui2::tmessage::yes_no_buttons) != gui2::twindow::OK)
	{
		return;
	}

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

bool tgame_cache_options::purge_cache()
{
	return game_config::config_cache::instance().purge_cache();
}

} // end namespace gui2
