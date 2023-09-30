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
#include <iostream>
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
	std::cout<<" hello testing testing testing "<<std::endl;
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


	std::cout<<"very long text of "<<focus_ss.str().size()<<"   fdasfdaf da errror probably efter detta ";

	std::cout<<std::endl<<" annars detta "<< ss.str().size()<<std::endl;
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
	
	std::cout<< "window height set to x";
	auto calcMax=[]() -> size_t {
		const size_t pixelChar = 5;
		const size_t maxHeight = 6400;
		const size_t maxChunkSize = maxHeight /pixelChar;
		return maxChunkSize;
	};


	auto populateChunks = [this](size_t maxChunkSize) -> void {
		size_t startPos = 0;
		while (startPos < content_.size()) {
			size_t endPos = startPos + maxChunkSize;
			if (endPos >= content_.size()) {
				endPos = content_.size();
			} else {
				// Look for the closest newline or closing tag before endPos
				size_t newlinePos = content_.rfind('\n', endPos);
				size_t tagClosePos = content_.rfind("</span>", endPos);
				endPos = std::max(newlinePos, tagClosePos);

				// If neither newline nor closing tag is found, fallback to the original logic
				if (endPos == std::string::npos) {
					size_t tagPos = content_.rfind('<', endPos);
					if (tagPos != std::string::npos && tagPos >= startPos) {
						size_t tagEndPos = content_.find('>', tagPos);
						if (tagEndPos != std::string::npos && tagEndPos > endPos) {
							endPos = tagPos;
						}
					}
				}
			}
			contentSubstrings_.push_back(content_.substr(startPos, endPos - startPos));
			std::cout << contentSubstrings_.size() << " size of vector" << std::endl;

			startPos = endPos;
		}
	};



	size_t maxChunkSize = calcMax();
	populateChunks(maxChunkSize);
	
	

	//std::cout<<std::endl<<contentSubstrings_.at(0)<<std::endl; //contentSubstrings_.size()-1) <<std::endl;
	//std::cout<<"print 2 "<<std::endl;
	//std::cout<<std::endl<<contentSubstrings_.at(1)<<std::endl; //contentSubstrings_.size()-1) <<std::endl;
	

/*
	for(size_t i=0; i< contentSubstrings_.size(); ++i)
	{
	}
	


	const unsigned int maxChunkSize = 8000;
	size_t pos = 0;
	while (pos < content_.size()) {
		size_t endPos = pos + maxChunkSize;
		if (endPos >= content_.size()) {
			endPos = content_.size();
		} else {
			size_t nextClosePos = content_.find("</span>", endPos);
			if (nextClosePos != std::string::npos) {
				endPos = nextClosePos + 7; // 7 is the length of "</span>"
			} else {
				endPos = content_.size();
			}
		}
		// Push the valid chunk to contentChunks
		contentSubstrings_.push_back(content_.substr(pos, endPos - pos));
		pos = endPos;
	}*/
	const size_t max_per_view=3;
	slidingContent_.clear();
	for(size_t i=0; i<max_per_view; ++i){
		if(i==0){ firstIdx = 0; }
		if(i==2){ lastIdx = 2;   };
		slidingContent_+=contentSubstrings_.at(i);
	
	}

	
	//concat 3 substring
	text_widget_->set_label(slidingContent_);

	//text_widget_->set_vertical_scrollbar_item_position(-500);
	// HACK: always hide the scrollbar, even if it's needed.
	// This should probably be implemented as a scrollbar mode.
	// Also, for some reason hiding the whole grid doesn't work, and the elements need to be hidden manually
	//
	
	if(grid* v_grid = dynamic_cast<grid*>(text_widget_->find("_vertical_scrollbar_grid", false))) {
		//find_widget<scrollbar_base>(v_grid, "_vertical_scrollbar", false).set_visible(widget::visibility::hidden);

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
	



	/*size_t scaled_cur_pos = static_cast<size_t>(cur_pos) * 15;

	size_t maxChunkSize = 4000;
	size_t newChunkIndex = scaled_cur_pos / maxChunkSize;
		
	if(newChunkIndex != currentChunkIdx_ && newChunkIndex < contentSubstrings_.size()){
			currentChunkIdx_ = newChunkIndex;
			std::string newChunk = contentSubstrings_[currentChunkIdx_];
			text_widget_->set_label(newChunk);
	}*/

/*
	auto calculateChunkIdx = [this](unsigned int curpos) -> size_t {
		const unsigned int chunkHeight=3200;
		size_t chunkIdx= curpos/chunkHeight;
		return std::min(chunkIdx,contentSubstrings_.size() -1);
	};

	size_t newChunkIdx=calculateChunkIdx(cur_pos);
	if(newChunkIdx != currentChunkIdx_)
	{
		currentChunkIdx_ = newChunkIdx;
		std::string newChunk = getChunkByIndex(content_,9500,currentChunkIdx_);
		text_widget_->set_use_markup(true);
		text_widget_->set_label(contentSubstrings_[currentChunkIdx_]);

	}
*/	

	// Calculate how far the text should have scrolled by now
	// The division by 1000 is to convert milliseconds to seconds.
	unsigned int needed_dist = missed_time * scroll_speed_ / 1000;

//	std::cout<< " cur_pos + needed_dist "<< cur_pos + needed_dist << " chunkIndex " << "1" <<std::endl;	
	
	

/*	if(grid* v_grid = dynamic_cast<grid*>(text_widget_->find("_vertical_scrollbar_grid", false))) {
	
		std::cout<< " max höjd?? " <<find_widget<scrollbar_base>(v_grid, "_vertical_scrollbar", false).get_height()<<" \n"<<std::endl;
		std::cout<< " pos y  " <<find_widget<scrollbar_base>(v_grid, "_vertical_scrollbar", false).get_item_position();

	}


// logic needed
	

	auto mod=[this](unsigned int cur_pos) -> void{
		if(cur_pos == 900)
		{	
			slidingContent_.erase(0,contentSubstrings_[0].length());
			slidingContent_+=(contentSubstrings_[1]);
			text_widget_->set_label(slidingContent_);
		}
	};

	mod(cur_pos);
*/
	
	
//	std::cout<<" height :  "<<text_widget_->get_height()<<" \n";

//	std::cout<<" item: y mod : " << (cur_pos % text_widget_->get_height())/1240 << std::endl;
	if(!(cur_pos > 1240+screenSpace)){	
	text_widget_->set_vertical_scrollbar_item_position(cur_pos + needed_dist);
	}
	else
	{
		++firstIdx;
		lastIdx = firstIdx + slidingSize;	
		std::cout<< "first Idx "<< firstIdx << " "<< std::endl; 
		std::cout<<"last idx "<< lastIdx << " "<<std::endl;
		slidingContent_="";
		if(lastIdx < contentSubstrings_.size()){
		for(size_t i=firstIdx; i< lastIdx; ++i)
		{
			slidingContent_+=contentSubstrings_[i];
		}
		}
		text_widget_->set_label(slidingContent_);
		cur_pos=0;
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


