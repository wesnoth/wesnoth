/*
   Copyright (C) 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "config.hpp"
#include "game_config.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/timer.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/repeating_button.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include "utils/functional.hpp"

#include <sstream>

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(end_credits)

end_credits::end_credits(const std::string& campaign)
	: focus_on_(campaign)
	, backgrounds_()
	, timer_id_()
	, text_widget_(nullptr)
	, scroll_speed_(100)
{
}

end_credits::~end_credits()
{
	if(timer_id_ != 0) {
		remove_timer(timer_id_);
		timer_id_ = 0;
	}
}

static void parse_about_tags(const config& cfg, std::stringstream& ss)
{
	for(const auto& about : cfg.child_range("about")) {
		if(!about.has_child("entry")) {
			continue;
		}

		ss << "\n" << "<span size='x-large'>" << about["title"] << "</span>" << "\n";

		for(const auto& entry : about.child_range("entry")) {
			ss << entry["name"] << "\n";
		}
	}
}

void end_credits::pre_show(window& window)
{
	timer_id_ = add_timer(10, std::bind(&end_credits::timer_callback, this), true);

	// Delay a little before beginning the scrolling
	last_scroll_ = SDL_GetTicks() + 3000;

	connect_signal_pre_key_press(window, std::bind(&end_credits::key_press_callback, this, _5));

	std::stringstream ss;
	std::stringstream focus_ss;

	const config& credits_config = about::get_about_config();

	// First, parse all the toplevel [about] tags
	parse_about_tags(credits_config, ss);

	// Next, parse all the grouped [about] tags (usually by campaign)
	for(const auto& group : credits_config.child_range("credits_group")) {
		std::stringstream& group_stream = (group["id"] == focus_on_) ? focus_ss : ss;

		group_stream << "\n" << "<span size='xx-large'>" << group["title"] << "</span>" << "\n";

		parse_about_tags(group, group_stream);
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
	window.get_canvas()[0].set_variable("background_image", variant(backgrounds_[0]));

	text_widget_ = find_widget<scroll_label>(&window, "text", false, true);

	text_widget_->set_use_markup(true);
	text_widget_->set_label((focus_ss.str().empty() ? ss : focus_ss).str());

	// HACK: always hide the scrollbar, even if it's needed.
	// This should probably be implemented as a scrollbar mode.
	// Also, for some reason hiding the whole grid doesn't work, and the elements need to be hidden manually
	if(grid* v_grid = dynamic_cast<grid*>(text_widget_->find("_vertical_scrollbar_grid", false))) {
		find_widget<scrollbar_base>(v_grid, "_vertical_scrollbar", false).set_visible(widget::visibility::hidden);

		// TODO: enable again if e24336afeb7 is reverted.
		//find_widget<repeating_button>(v_grid, "_half_page_up", false).set_visible(widget::visibility::hidden);
		//find_widget<repeating_button>(v_grid, "_half_page_down", false).set_visible(widget::visibility::hidden);
	}
}

void end_credits::timer_callback()
{
	uint32_t now = SDL_GetTicks();
	if(last_scroll_ > now) {
		return;
	}

	uint32_t missed_time = now - last_scroll_;

	unsigned int cur_pos = text_widget_->get_vertical_scrollbar_item_position();

	// Calculate how far the text should have scrolled by now
	// The division by 1000 is to convert milliseconds to seconds.
	unsigned int needed_dist = missed_time * scroll_speed_ / 1000;

	text_widget_->set_vertical_scrollbar_item_position(cur_pos + needed_dist);

	last_scroll_ = now;

	if(text_widget_->vertical_scrollbar_at_end()) {
		remove_timer(timer_id_);
	}
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
} // namespace gui2
