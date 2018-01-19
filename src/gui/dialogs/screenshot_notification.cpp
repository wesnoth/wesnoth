/*
   Copyright (C) 2013 - 2018 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "utils/functional.hpp"

#include "gettext.hpp"

namespace gui2
{
namespace dialogs
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

screenshot_notification::screenshot_notification(const std::string& path)
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

void screenshot_notification::pre_show(window& window)
{
	text_box& path_box = find_widget<text_box>(&window, "path", false);
	path_box.set_value(filesystem::base_name(path_));
	path_box.set_active(false);

	button& copy_b = find_widget<button>(&window, "copy", false);
	connect_signal_mouse_left_click(
			copy_b, std::bind(&desktop::clipboard::copy_to_clipboard, std::ref(path_), false));

	if (!desktop::clipboard::available()) {
		copy_b.set_active(false);
		copy_b.set_tooltip(_("Clipboard support not found, contact your packager"));
	}

	button& open_b = find_widget<button>(&window, "open", false);
	connect_signal_mouse_left_click(
			open_b, bind_void(&desktop::open_object, std::ref(path_)));

	button& bdir_b = find_widget<button>(&window, "browse_dir", false);
	connect_signal_mouse_left_click(
			bdir_b,
			bind_void(&desktop::open_object,
						std::ref(screenshots_dir_path_)));
}
} // namespace dialogs
} // namespace gui2
