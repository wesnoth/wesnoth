/*
   Copyright (C) 2013 - 2018 by Iris Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/screenshot_notification.hpp"

#include "desktop/clipboard.hpp"
#include "desktop/open.hpp"
#include "display.hpp"
#include "filesystem.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/event/dispatcher.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"
#include "picture.hpp"

#include "utils/functional.hpp"

#include "gettext.hpp"

#include <boost/filesystem.hpp>
#include <stdexcept>

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

screenshot_notification::screenshot_notification(const std::string& path, surface screenshot)
	: path_(path)
	, screenshots_dir_path_(filesystem::get_screenshot_dir())
	, screenshot_(screenshot)
{

}

void screenshot_notification::pre_show(window& window)
{
	window.set_enter_disabled(true);

	text_box& path_box = find_widget<text_box>(&window, "path", false);
	path_box.set_value(filesystem::base_name(path_));
	window.keyboard_capture(&path_box);
	connect_signal_pre_key_press(path_box, std::bind(&screenshot_notification::keypress_callback, this,
		std::placeholders::_3, std::placeholders::_5));

	find_widget<label>(&window, "filesize", false).set_label(font::unicode_em_dash);

	button& copy_b = find_widget<button>(&window, "copy", false);
	connect_signal_mouse_left_click(
		copy_b, std::bind(&desktop::clipboard::copy_to_clipboard, std::ref(path_), false));
	copy_b.set_active(false);

	if (!desktop::clipboard::available()) {
		copy_b.set_tooltip(_("Clipboard support not found, contact your packager"));
	}

	button& open_b = find_widget<button>(&window, "open", false);
	connect_signal_mouse_left_click(
		open_b, std::bind(&desktop::open_object, std::ref(path_)));
	open_b.set_active(false);

	button& bdir_b = find_widget<button>(&window, "browse_dir", false);
	connect_signal_mouse_left_click(
		bdir_b,
		std::bind(&desktop::open_object,
			std::ref(screenshots_dir_path_)));

	button& save_b = find_widget<button>(&window, "save", false);
	connect_signal_mouse_left_click(save_b, std::bind(&screenshot_notification::save_screenshot, this));
}

void screenshot_notification::save_screenshot()
{
	window& window = *get_window();
	text_box& path_box = find_widget<text_box>(&window, "path", false);
	std::string filename = path_box.get_value();
	boost::filesystem::path path(screenshots_dir_path_);
	path /= filename;

	image::save_result res = image::save_image(screenshot_, path.string());
	if(res == image::save_result::unsupported_format) {
		gui2::show_error_message(_("Unsupported image format.\n\n"
			"Try to save the screenshot as PNG instead."));
	} else if(res == image::save_result::save_failed) {
		gui2::show_error_message(
			translation::dsgettext("wesnoth", "Screenshot creation failed.\n\n"
			"Make sure there is enough space on the drive holding Wesnoth’s player resource files and that file permissions are set up correctly."));
	} else if(res != image::save_result::success) {
		throw std::logic_error("Unexpected error while trying to save a screenshot");
	} else {
		path_box.set_active(false);
		find_widget<button>(&window, "open", false).set_active(true);
		find_widget<button>(&window, "save", false).set_active(false);

		if(desktop::clipboard::available()) {
			find_widget<button>(&window, "copy", false).set_active(true);
		}

		const int filesize = filesystem::file_size(path.string());
		const std::string sizetext = utils::si_string(filesize, true, _("unit_byte^B"));
		find_widget<label>(&window, "filesize", false).set_label(sizetext);
	}
}

void screenshot_notification::keypress_callback(bool& handled, SDL_Keycode key)
{
	if(key == SDLK_RETURN || key == SDLK_KP_ENTER) {
		save_screenshot();
		handled = true;
	}
}

} // namespace dialogs
} // namespace gui2
