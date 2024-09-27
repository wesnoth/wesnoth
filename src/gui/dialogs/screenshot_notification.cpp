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

#include "gui/dialogs/screenshot_notification.hpp"

#include "desktop/clipboard.hpp"
#include "desktop/open.hpp"
#include "filesystem.hpp"
#include "gettext.hpp"
#include "gui/core/event/dispatcher.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"
#include "picture.hpp"

#include <boost/filesystem/path.hpp>

#include <functional>
#include <stdexcept>
#include <utility>

namespace gui2::dialogs
{

REGISTER_DIALOG(screenshot_notification)

screenshot_notification::screenshot_notification(const std::string& path, surface screenshot)
	: modal_dialog(window_id())
	, path_(path)
	, screenshots_dir_path_(filesystem::get_screenshot_dir())
	, screenshot_(std::move(screenshot))
{
}

void screenshot_notification::pre_show()
{
	set_enter_disabled(true);

	text_box& path_box = find_widget<text_box>("path");
	path_box.set_value(filesystem::base_name(path_));
	path_box.set_selection(0, path_box.text().find_last_of('.')); // TODO: do this cleaner!
	keyboard_capture(&path_box);
	connect_signal_pre_key_press(path_box, std::bind(&screenshot_notification::keypress_callback, this,
		std::placeholders::_3, std::placeholders::_5));

	find_widget<label>("filesize").set_label(font::unicode_em_dash);

	button& copy_b = find_widget<button>("copy");
	connect_signal_mouse_left_click(
		copy_b, std::bind(&desktop::clipboard::copy_to_clipboard, std::ref(path_)));
	copy_b.set_active(false);

	button& open_b = find_widget<button>("open");
	connect_signal_mouse_left_click(
		open_b, std::bind(&desktop::open_object, std::ref(path_)));
	open_b.set_active(false);

	button& bdir_b = find_widget<button>("browse_dir");
	connect_signal_mouse_left_click(
		bdir_b,
		std::bind(&desktop::open_object,
			std::ref(screenshots_dir_path_)));

	button& save_b = find_widget<button>("save");
	connect_signal_mouse_left_click(save_b, std::bind(&screenshot_notification::save_screenshot, this));
}

void screenshot_notification::save_screenshot()
{
	text_box& path_box = find_widget<text_box>("path");
	std::string filename = path_box.get_value();
	boost::filesystem::path path(screenshots_dir_path_);
	path /= filename;

	path_ = path.string();

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
		find_widget<button>("open").set_active(true);
		find_widget<button>("save").set_active(false);
		find_widget<button>("copy").set_active(true);

		const int filesize = filesystem::file_size(path_);
		const std::string sizetext = utils::si_string(filesize, true, _("unit_byte^B"));
		find_widget<label>("filesize").set_label(sizetext);
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
