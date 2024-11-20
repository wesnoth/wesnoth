/*
	Copyright (C) 2021 - 2024
	by Iris Morelle <shadowm@wesnoth.org>
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

#include "gui/dialogs/file_progress.hpp"

#include "draw_manager.hpp"
#include "events.hpp"
#include "gui/widgets/button.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/progress_bar.hpp"
#include "gui/widgets/window.hpp"


namespace gui2::dialogs {

REGISTER_WINDOW(file_progress)

const std::string& file_progress::window_id() const
{
	static std::string wid = "file_progress";
	return wid;
}

file_progress::file_progress(const std::string& title, const std::string& message)
	: modeless_dialog(file_progress::window_id())
	, title_(title)
	, message_(message)
	, update_time_()
{
	find_widget<label>("title").set_label(title_);
	auto& label_widget = find_widget<label>("message");

	label_widget.set_use_markup(true);
	label_widget.set_label(message_);

	find_widget<button>("placeholder").set_active(false);

	update_time_ = clock::now();
}

void file_progress::update_progress(unsigned value)
{
	auto now = clock::now();
	auto elapsed = now - update_time_;

	// Update at most once per vsync.
	if(elapsed < draw_manager::get_frame_length()) {
		return;
	}

	update_time_ = now;

	find_widget<progress_bar>("progress").set_percentage(value);

	queue_redraw();
	events::draw();
}

}
