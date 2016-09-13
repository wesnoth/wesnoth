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

#include "game_config.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/timer.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/repeating_button.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "formatter.hpp"
#include "marked-up_text.hpp"

#include "utils/functional.hpp"

namespace gui2
{

REGISTER_DIALOG(end_credits)

tend_credits::tend_credits(const std::vector<std::string>& text, const std::vector<std::string>& backgrounds)
	: text_(text)
	, backgrounds_(backgrounds)
	, timer_id_()
	, text_widget_(nullptr)
	, scroll_speed_(100)
{
	if(backgrounds_.empty()) {
		backgrounds_.push_back(game_config::images::game_title_background);
	}
}

tend_credits::~tend_credits()
{
	if(timer_id_ != 0) {
		remove_timer(timer_id_);
		timer_id_ = 0;
	}
}

void tend_credits::pre_show(twindow& window)
{
	// Delay a little before beginning the scrolling
	add_timer(1000, [this](size_t) {
		timer_id_ = add_timer(50, std::bind(&tend_credits::timer_callback, this), true);
		last_scroll_ = SDL_GetTicks();
	});

	connect_signal_pre_key_press(window, std::bind(&tend_credits::key_press_callback, this, _3, _4, _5));

	// TODO: apparently, multiple images are supported... need to implement along with scrolling
	window.canvas()[0].set_variable("background_image", variant(backgrounds_[0]));

	std::stringstream str;

	// BIG FAT TODO: get rid of this hacky string crap once we drop the GUI1 version
	for(const auto& line : text_) {
		if(line[0] == '-') {
			str << font::escape_text(line.substr(1)) << "\n";
		} else if(line[0] == '+') {
			str << "<span size='x-large'>" << font::escape_text(line.substr(1)) << "</span>" << "\n";
		}
	}

	text_widget_ = find_widget<tscroll_label>(&window, "text", false, true);

	text_widget_->set_use_markup(true);
	text_widget_->set_label(str.str());

	// HACK: always hide the scrollbar, even if it's needed.
	// This should probably be implemented as a scrollbar mode.
	// Also, for some reason hiding the whole grid doesn't work, and the elements need to be hidden manually
	if(tgrid* v_grid = dynamic_cast<tgrid*>(text_widget_->find("_vertical_scrollbar_grid", false))) {
		find_widget<tscrollbar_>(v_grid, "_vertical_scrollbar", false).set_visible(twidget::tvisible::hidden);
		find_widget<trepeating_button>(v_grid, "_half_page_up", false).set_visible(twidget::tvisible::hidden);
		find_widget<trepeating_button>(v_grid, "_half_page_down", false).set_visible(twidget::tvisible::hidden);
	}
}

void tend_credits::timer_callback()
{
	uint32_t now = SDL_GetTicks();
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

void tend_credits::key_press_callback(bool&, bool&, const SDLKey key)
{
	if(key == SDLK_UP && scroll_speed_ < 200) {
		scroll_speed_ <<= 1;
	}

	if(key == SDLK_DOWN && scroll_speed_ > 50) {
		scroll_speed_ >>= 1;
	}
}

} // namespace gui2
