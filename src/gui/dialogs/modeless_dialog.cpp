/*
   Copyright (C) 2011 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/modeless_dialog.hpp"

#include "gui/widgets/window.hpp"
#include "video.hpp"

namespace gui2
{
namespace dialogs
{

modeless_dialog::modeless_dialog() : window_(nullptr)
{
}

modeless_dialog::~modeless_dialog()
{
	hide();
}

void modeless_dialog::show(CVideo& video,
				  const bool allow_interaction,
				  const unsigned /*auto_close_time*/)
{
	if(video.faked()) {
		return;
	}

	hide();

	window_ = build_window(video);

	post_build(*window_);

	pre_show(*window_);

	if(allow_interaction) {
		open_window_stack.push_back(window_);
		window_->show_non_modal();
	} else {
		window_->show_tooltip(/*auto_close_time*/);
	}
}

void modeless_dialog::hide()
{
	if(window_) {
		// Possible TODO: Only run through this loop if the window's show_mode_ == modal
		// (For some reason, non-modal windows still have show_mode_ = modal.)
		// Don't bother if show_mode_ == tooltip, because in that case we didn't add it anyway.
		for(auto iter = open_window_stack.rbegin(); iter != open_window_stack.rend(); iter++) {
			if(*iter == window_) {
				open_window_stack.erase(std::next(iter).base());
				break;
			}
		}
		delete window_;
		window_ = nullptr;
	}
}

window* modeless_dialog::build_window(CVideo& video) const
{
	return build(video, window_id());
}

void modeless_dialog::post_build(window& /*window*/)
{
	/* DO NOTHING */
}

void modeless_dialog::pre_show(window& /*window*/)
{
	/* DO NOTHING */
}

} // namespace dialogs
} // namespace gui2
