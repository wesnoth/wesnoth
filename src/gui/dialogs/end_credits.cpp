/*
	Copyright (C) 2016 - 2022
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
#include "config.hpp"
#include "game_config.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"

#include <functional>

#include <sstream>

namespace gui2::dialogs
{

REGISTER_DIALOG(end_credits)

end_credits::end_credits(const std::string& campaign)
	: modal_dialog(window_id())
	, focus_on_(campaign)
	, backgrounds_()
	, scroll_speed_(100)
	, last_scroll_(std::numeric_limits<uint32_t>::max())
{
}

void end_credits::pre_show(window& window)
{
	// Delay a little before beginning the scrolling
	last_scroll_ = SDL_GetTicks() + 3000;

	connect_signal_pre_key_press(window, std::bind(&end_credits::key_press_callback, this, std::placeholders::_5));

	listbox& list = find_widget<listbox>(&window, "listbox", false);
	window.keyboard_capture(&list);

	for(const auto& group : about::get_credits_data()) {
		if(!group.header.empty()) {
			std::map<std::string, utils::string_map> data;
			utils::string_map item;

			item["label"] = group.header;
			data.emplace("group_header", item);

			item["label"] = "";
			data.emplace("section_title", item);

			item["label"] = "";
			data.emplace("contributor_name", item);

			list.add_row(data);
		}
		for(const auto& section : group.sections) {
			std::map<std::string, utils::string_map> data;
			utils::string_map item;

			item["label"] = "";
			data.emplace("group_header", item);

			if(!section.title.empty()) {
				item["label"] = section.title;
			} else {
				item["label"] = "";
			}
			data.emplace("section_title", item);

			std::stringstream ss;
			for(const auto& name : section.names) {
				ss << name.first << "\n";
			}

			item["label"] = ss.str();
			data.emplace("contributor_name", item);

			// TODO: should there be calls to set_use_markup(true) and set_link_aware(false)?

			list.add_row(data);
		}

		// TODO: should focus_on_ be implemented? It should check for matching groups here,
		// however since commit c3e578ba80c the "outro" dialog has been used instead of
		// "end_credits" for end-of-campaign credits.
	}

	for(unsigned i = 0; i < list.get_item_count(); ++i) {
		list.select_row(i, false);
		list.set_row_active(i, false);
	}

	// Get the appropriate background images
	backgrounds_ = about::get_background_images(focus_on_);

	if(backgrounds_.empty()) {
		backgrounds_.push_back(game_config::images::game_title_background);
	}

	// TODO: implement showing all available images as the credits scroll
	window.get_canvas(0).set_variable("background_image", wfl::variant(backgrounds_[0]));
}

void end_credits::update()
{
	uint32_t now = SDL_GetTicks();
	if(last_scroll_ > now) {
		return;
	}

	uint32_t missed_time = now - last_scroll_;

	auto scroll_widget = find_widget<scrollbar_container>(get_window(), "listbox", false, true);
	unsigned int cur_pos = scroll_widget->get_vertical_scrollbar_item_position();

	// Calculate how far the text should have scrolled by now
	// The division by 1000 is to convert milliseconds to seconds.
	unsigned int needed_dist = missed_time * scroll_speed_ / 1000;

	scroll_widget->set_vertical_scrollbar_item_position(cur_pos + needed_dist);

	last_scroll_ = now;
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
