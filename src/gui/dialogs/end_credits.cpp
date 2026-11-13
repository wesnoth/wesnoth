/*
	Copyright (C) 2016 - 2025
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

#include "gui/dialogs/end_credits.hpp"

#include "about.hpp"
#include "game_config.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"
#include "serialization/markup.hpp"

#include <SDL3/SDL_timer.h>

#include <functional>
#include <sstream>

namespace gui2::dialogs
{

REGISTER_DIALOG(end_credits)

end_credits::end_credits(const std::string& campaign)
	: modal_dialog(window_id())
	, focus_on_(campaign)
	, backgrounds_()
	, text_widget_(nullptr)
	, scroll_speed_(100)
	, last_scroll_(std::numeric_limits<uint64_t>::max())
{
}

void end_credits::pre_show()
{
	last_scroll_ = SDL_GetTicks();

	connect_signal_pre_key_press(*this, std::bind(&end_credits::key_press_callback, this, std::placeholders::_5));

	std::stringstream ss;
	std::stringstream focus_ss;

	for(const about::credits_group& group : about::get_credits_data()) {
		std::stringstream& group_stream = (group.id == focus_on_) ? focus_ss : ss;
		group_stream << "\n";

		if(!group.header.empty()) {
			group_stream << markup::span_size("xx-large", group.header) << "\n";
		}

		for(const about::credits_group::about_group& about : group.sections) {
			group_stream << "\n" << markup::span_size("x-large", about.title) << "\n";

			for(const auto& entry : about.names) {
				group_stream << entry.first << "\n";
			}
		}
	}

	// If a section is focused, move it to the top
	if(!focus_ss.str().empty()) {
		focus_ss << ss.rdbuf();
	}

	// Get the appropriate background images
	backgrounds_ = about::get_background_images(focus_on_);

	if(backgrounds_.empty()) {
		backgrounds_.push_back(game_config::images::game_title_background);
	}

	// TODO: implement showing all available images as the credits scroll
	get_canvas(0).set_variable("background_image", wfl::variant(backgrounds_[0]));

	text_widget_ = find_widget<scroll_label>("text", false, true);

	text_widget_->set_use_markup(true);
	text_widget_->set_link_aware(false);

	// Text shapes only render their visible part, so the whole text can go in
	// one label without exceeding the maximum surface or texture size.
	text_widget_->set_label(focus_ss.str().empty() ? ss.str() : focus_ss.str());

	// HACK: always hide the scrollbar, even if it's needed.
	// This should probably be implemented as a scrollbar mode.
	// Also, for some reason hiding the whole grid doesn't work, and the elements need to be hidden manually
	if(grid* v_grid = dynamic_cast<grid*>(text_widget_->find("_vertical_scrollbar_grid", false))) {
		v_grid->find_widget<scrollbar_base>("_vertical_scrollbar").set_visible(widget::visibility::hidden);

		// TODO: enable again if e24336afeb7 is reverted.
		//v_grid.find_widget<repeating_button>("_half_page_up").set_visible(widget::visibility::hidden);
		//v_grid.find_widget<repeating_button>("_half_page_down").set_visible(widget::visibility::hidden);
	}
}

void end_credits::update()
{
	const uint64_t now = SDL_GetTicks();
	if(last_scroll_ > now) {
		return;
	}

	// Calculate how far the text should have scrolled by now, in whole pixels.
	// The division by 1000 is to convert milliseconds to seconds.
	const uint64_t needed_dist = (now - last_scroll_) * scroll_speed_ / 1000;
	if(needed_dist == 0) {
		// Wait until enough time has passed to scroll at least one pixel, otherwise
		// high frame rates or slow speeds would never scroll at all.
		return;
	}

	text_widget_->set_vertical_scrollbar_item_position(
		text_widget_->get_vertical_scrollbar_item_position() + static_cast<unsigned>(needed_dist));

	// Only consume the time that was used, carrying the remainder over to the next frame.
	last_scroll_ += needed_dist * 1000 / scroll_speed_;
}

void end_credits::key_press_callback(const SDL_Keycode key)
{
	if(key == SDLK_UP && scroll_speed_ < 400) {
		scroll_speed_ <<= 1;
	}

	if(key == SDLK_DOWN && scroll_speed_ > 50) {
		scroll_speed_ >>= 1;
	}
}

} // namespace dialogs
