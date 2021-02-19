/*
   Copyright (C) 2021 by Iris Morelle <shadowm@wesnoth.org>
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

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/progress_bar.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

namespace gui2::dialogs {

REGISTER_WINDOW(file_progress)

const std::string& file_progress::window_id() const
{
	static std::string wid = "file_progress";
	return wid;
}

file_progress::file_progress(const std::string& title, const std::string& message)
	: title_(title)
	, message_(message)
	, update_time_()
{
}

void file_progress::pre_show(window& window)
{
	find_widget<label>(&window, "title", false).set_label(title_);
	auto& message = find_widget<label>(&window, "message", false);

	message.set_use_markup(true);
	message.set_label(message_);

	find_widget<button>(&window, "placeholder", false).set_active(false);

	update_time_ = clock::now();
}

void file_progress::update_progress(unsigned value)
{
	auto* window = get_window();

	if(!window) {
		return;
	}

	using std::chrono::duration_cast;
	using std::chrono::milliseconds;
	using namespace std::chrono_literals;

	auto now = clock::now();

	if(duration_cast<milliseconds>(now - update_time_) < 120ms) {
		return;
	}

	find_widget<progress_bar>(window, "progress", false).set_percentage(value);
	force_redraw();
	update_time_ = now;
}

}
