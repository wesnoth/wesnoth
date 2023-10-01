/*
	Copyright (C) 2016 - 2023
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "end_credits.hpp"
#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/end_credits.hpp"

#include "about.hpp"
#include "config.hpp"
#include "game_config.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/repeating_button.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"

#include <functional>

#include <sstream>
//#include <iostream>
namespace gui2::dialogs
{

REGISTER_DIALOG(end_credits)

end_credits::end_credits(const std::string& campaign)
	: modal_dialog(window_id())
	, focus_on_(campaign)
	, backgrounds_()
	, text_widget_(nullptr)
	, scroll_speed_(100)
	, last_scroll_(std::numeric_limits<uint32_t>::max())
{
}

void end_credits::pre_show(window& window)
{
	// Delay a little before beginning the scrolling
	last_scroll_ = SDL_GetTicks() + 3000;
	connect_signal_pre_key_press(window, std::bind(&end_credits::key_press_callback, this, std::placeholders::_5));

	std::stringstream ss;
	std::stringstream focus_ss;
	
	for(const about::credits_group& group : about::get_credits_data()) {
		std::stringstream& group_stream = (group.id == focus_on_) ? focus_ss : ss;
		group_stream << "\n";

		if(!group.header.empty()) {
			group_stream << "<span size='xx-large'>" << group.header << "</span>" << "\n";
		}

		for(const about::credits_group::about_group& about : group.sections) {
			group_stream << "\n" << "<span size='x-large'>" << about.title << "</span>" << "\n";

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
	window.get_canvas(0).set_variable("background_image", wfl::variant(backgrounds_[0]));

	text_widget_ = find_widget<scroll_label>(&window, "text", false, true);
	text_widget_->set_use_markup(true);
	text_widget_->set_link_aware(false);
	
	content_ = focus_ss.str().empty() ? ss.str() : focus_ss.str();

	//TODO: Fix this better ? 
	//Don't really know how to deal with this the best way but 
	//lambda fucntion to parse the markup language ensure when storing the 
	//substrings it doesn't cut in between spans and thus making markup invalid
	//when combinding multiple substrings for the sliding window
	//IrregularBismuth 2023-09-30
	auto populateChunks = [this](const size_t& maxChunkSize) -> void {
		size_t startPos = 0;
		while (startPos < content_.size()) {
			size_t endPos = startPos + maxChunkSize;
			if (endPos >= content_.size()) {
				endPos = content_.size();
			} else {
				// Look for the closest newline or closing tag before endPos
				size_t newlinePos = content_.rfind('\n', endPos);
				size_t tagClosePos = content_.rfind("</span>", endPos);

				if (newlinePos != std::string::npos || tagClosePos != std::string::npos) {
					endPos = std::max(newlinePos, tagClosePos) + 1;  // +1 to include newline or closing tag
				} else {
					// If neither is found, search forward for the next newline or closing tag
					newlinePos = content_.find('\n', endPos);
					tagClosePos = content_.find("</span>", endPos);
					endPos = (newlinePos == std::string::npos) ? tagClosePos + 1 : std::min(newlinePos, tagClosePos) + 1;
				}

				// Ensure the chunk doesn't end right after an opening tag
				size_t tagOpenPos = content_.rfind("<span", endPos);
				if (tagOpenPos != std::string::npos && tagOpenPos > newlinePos) {
					tagClosePos = content_.find("</span>", tagOpenPos);
					if (tagClosePos != std::string::npos) {
						endPos = tagClosePos + 7;  // +7 to include the closing tag
					}
				}
			}
		contentSubstrings_.push_back(content_.substr(startPos, endPos - startPos));
		//	std::cout << contentSubstrings_.size() << " size of vector" << std::endl;
			startPos = endPos;
		}
	};

	static constexpr size_t maxChunkSize = 1280;
	populateChunks(maxChunkSize);

	
	slidingContent_.clear();
	for(size_t i=0; i<=slidingSize_; ++i){
		slidingContent_+=contentSubstrings_.at(i);
	}	
	//concat substring strings
	text_widget_->set_label(slidingContent_);

	//text_widget_->set_vertical_scrollbar_item_position(-500);
	// HACK: always hide the scrollbar, even if it's needed.
	// This should probably be implemented as a scrollbar mode.
	// Also, for some reason hiding the whole grid doesn't work, and the elements need to be hidden manually
	//
	
	if(grid* v_grid = dynamic_cast<grid*>(text_widget_->find("_vertical_scrollbar_grid", false))) {
		find_widget<scrollbar_base>(v_grid, "_vertical_scrollbar", false).set_visible(widget::visibility::hidden);

		// TODO: enable again if e24336afeb7 is reverted.
		//find_widget<repeating_button>(v_grid, "_half_page_up", false).set_visible(widget::visibility::hidden);
		//find_widget<repeating_button>(v_grid, "_half_page_down", false).set_visible(widget::visibility::hidden);
	}
}

void end_credits::update()
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

	//###### --- ######
	//this might be in need of modification 
	//this doesn't allow for scrolling up again after been scrolled down 
	//  only the content in the current sliding window can be scrolled up
	// TODO : lock scroll bar content
	if(!(cur_pos > text_widget_->get_height()+screenSpace_ - magicNumber_)){	
		text_widget_->set_vertical_scrollbar_item_position(cur_pos + needed_dist);
	}
	else {

		if(firstIdx_ < contentSubstrings_.size()-slidingSize_){
			++firstIdx_;
			lastIdx_ = firstIdx + slidingSize_;	
			//std::cout<< "first Idx "<< firstIdx << " "<< std::endl; 
			//std::cout<<"last idx "<< lastIdx << " "<<std::endl;
			slidingContent_="";
			if(lastIdx <= contentSubstrings_.size()){
				for(size_t i=firstIdx_; i< lastIdx_; ++i)
				{
					slidingContent_+=contentSubstrings_[i];
				}
			}
			text_widget_->set_label(slidingContent_); //updates the sliding window 
			cur_pos-=(text_widget_->get_height()+screenSpace_ - magicNumber_);
		}
	}

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
//end dialogs
}


