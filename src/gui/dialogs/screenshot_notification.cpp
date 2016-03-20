/*
   Copyright (C) 2013 - 2016 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/screenshot_notification.hpp"

#include "desktop/clipboard.hpp"
#include "desktop/open.hpp"
#include "filesystem.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

#include <boost/bind.hpp>

#include "gettext.hpp"

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_screenshot_notification
 *
 * == Screenshot notification ==
 *
 * Notification dialog used after saving a game or map screenshot to display
 * information about it for the user.
 *
 * @begin{table}{dialog_widgets}
 *
 * path & & text_box & m &
 *         Read-only textbox containing the screenshot path. $
 *
 * filesize & & label & o &
 *         Optional label to display the file size. $
 *
 * copy & & button & m &
 *         Button to copy the path to clipboard. $
 *
 * open & & button & m &
 *         Button to open the screnshot using the default application. $
 *
 * browse_dir & & button & m &
 *         Button to browse the screenshots directory in the file manager. $
 *
 * @end{table}
 */

REGISTER_DIALOG(screenshot_notification)

tscreenshot_notification::tscreenshot_notification(const std::string& path)
	: path_(path), screenshots_dir_path_(filesystem::get_screenshot_dir())
{
	const int filesize = filesystem::file_size(path);

	const std::string sizetext
			= filesize >= 0
			? utils::si_string(filesize, true, _("unit_byte^B"))
			: _("file_size^Unknown");

	register_label("filesize",
				   false,
				   sizetext,
				   false);
}

void tscreenshot_notification::pre_show(twindow& window)
{
	ttext_box& path_box = find_widget<ttext_box>(&window, "path", false);
	path_box.set_value(filesystem::base_name(path_));
	path_box.set_active(false);

	tbutton& copy_b = find_widget<tbutton>(&window, "copy", false);
	connect_signal_mouse_left_click(
			copy_b, boost::bind(&desktop::clipboard::copy_to_clipboard, boost::ref(path_), false));

	if (!desktop::clipboard::available()) {
		copy_b.set_active(false);
		copy_b.set_tooltip(_("Clipboard support not found, contact your packager"));
	}

	tbutton& open_b = find_widget<tbutton>(&window, "open", false);
	connect_signal_mouse_left_click(
			open_b, boost::bind(&desktop::open_object, boost::ref(path_)));

	tbutton& bdir_b = find_widget<tbutton>(&window, "browse_dir", false);
	connect_signal_mouse_left_click(
			bdir_b,
			boost::bind(&desktop::open_object,
						boost::ref(screenshots_dir_path_)));
}
}
