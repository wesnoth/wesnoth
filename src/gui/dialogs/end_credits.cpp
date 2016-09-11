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
#include "gui/widgets/settings.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/window.hpp"
#include "formatter.hpp"
#include "marked-up_text.hpp"

namespace gui2
{

REGISTER_DIALOG(end_credits)

tend_credits::tend_credits(const std::vector<std::string>& text, const std::vector<std::string>& backgrounds)
	: text_(text)
	, backgrounds_(backgrounds)
	, timer_id_()
	, text_widget_(nullptr)
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
	timer_id_ = add_timer(50, std::bind(&tend_credits::timer_callback, this, std::ref(window)), true);

	// TODO: apparently, multiple images are supported... need to implement along with scrolling
	window.canvas()[0].set_variable("background_image", variant(backgrounds_[0]));

	std::stringstream str;

	// BIG FAT TODO: get rid of this hacky string crap once we drop the GUI1 version
	for(const auto& line : text_) {
		if(line[0] == '-') {
			str << font::escape_text(line.substr(1)) << "\n";
		} else if(line[0] == '+') {
			str << "<big>" << font::escape_text(line.substr(1)) << "</big>" << "\n";
		}
	}

	text_widget_ = find_widget<tscroll_label>(&window, "text", false, true);

	text_widget_->set_use_markup(true);
	text_widget_->set_label(str.str());
}

void tend_credits::timer_callback(twindow&)
{
	text_widget_->scroll_vertical_scrollbar(tscrollbar_::ITEM_FORWARD);
}

}
