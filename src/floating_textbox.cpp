/* $Id$ */
/*
   Copyright (C) 2006 - 2007 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "floating_textbox.hpp"
#include "font.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "log.hpp"
#include "team.hpp"

#include <ctime>

namespace gui{
	floating_textbox::floating_textbox() : 
		box_(NULL), 
		check_(NULL), 
		mode_(TEXTBOX_NONE), 
		label_string_(),
		label_(0)
	{}

	void floating_textbox::close(game_display& gui)
	{
		if(!active()) {
			return;
		}
		if(check_ != NULL) {
			if(mode_ == TEXTBOX_MESSAGE) {
				preferences::set_message_private(check_->checked());
			}
		}
		box_.assign(NULL);
		check_.assign(NULL);
		font::remove_floating_label(label_);
		mode_ = TEXTBOX_NONE;
		gui.invalidate_all();
	}

	void floating_textbox::update_location(game_display& gui)
	{
		if (box_ == NULL)
			return;

		const SDL_Rect& area = gui.map_outside_area();

		const int border_size = 10;

		const int ypos = area.y+area.h-30 - (check_ != NULL ? check_->height() + border_size : 0);

		if (label_ != 0)
			font::remove_floating_label(label_);

		label_ = font::add_floating_label(label_string_,font::SIZE_NORMAL,
				font::YELLOW_COLOUR,area.x+border_size,ypos,0,0,-1, area,font::LEFT_ALIGN);

		if (label_ == 0)
			return;

		const SDL_Rect& label_area = font::get_floating_label_rect(label_);
		const int textbox_width = area.w - label_area.w - border_size*3;

		if(textbox_width <= 0) {
			font::remove_floating_label(label_);
			return;
		}

		if(box_ != NULL) {
			box_->set_volatile(true);
			const SDL_Rect rect = {
				area.x + label_area.w + border_size*2, ypos,
				textbox_width, box_->height()
			};
			box_->set_location(rect);
		}

		if(check_ != NULL) {
			check_->set_volatile(true);
			check_->set_location(box_->location().x,box_->location().y + box_->location().h + border_size);
		}
	}

	void floating_textbox::show(gui::TEXTBOX_MODE mode, const std::string& label,
		const std::string& check_label, bool checked, game_display& gui)
	{
		close(gui);

		label_string_ = label;
		mode_ = mode;

		if(check_label != "") {
			check_.assign(new gui::button(gui.video(),check_label,gui::button::TYPE_CHECK));
			check_->set_check(checked);
		}


		box_.assign(new gui::textbox(gui.video(),100,"",true,256,0.8,0.6));

		update_location(gui);
	}

	void floating_textbox::tab(std::vector<team>& teams, const unit_map& /*units*/, game_display& gui)
	{
		if(active() == false) {
			return;
		}

		switch(mode_) {
		case gui::TEXTBOX_SEARCH:
		case gui::TEXTBOX_COMMAND:
		case gui::TEXTBOX_MESSAGE:
		{
			std::string text = box_->text();
			std::vector<std::string> matches;
			// Add players
			for(size_t n = 0; n != teams.size(); ++n) {
				if(teams[n].is_empty()) continue;
				matches.push_back(teams[n].current_player());
			}
			// Add observers
			const std::set<std::string>& observers = gui.observers();
			for(std::set<std::string>::const_iterator i = observers.begin();
					i != observers.end(); ++i)
			{
					matches.push_back(*i);
			}
			// Remove duplicates.
			std::sort<std::vector<std::string>::iterator>
					(matches.begin(), matches.end());
			matches.erase(std::unique(matches.begin(), matches.end()), matches.end());
			// Exclude own nick from tab-completion.
			if (mode_ == gui::TEXTBOX_MESSAGE) {
				matches.erase(std::remove(matches.begin(), matches.end(),
						preferences::login()), matches.end());
			}
			const bool line_start = utils::word_completion(text, matches);

			if (matches.empty()) return;
			if (matches.size() == 1 && mode_ == gui::TEXTBOX_MESSAGE) {
				text.append(line_start ? ": " : " ");
			} else {
				std::string completion_list = utils::join(matches, ' ');
				gui.add_chat_message(time(NULL), "", 0, completion_list,
						game_display::MESSAGE_PRIVATE, false);
			}
			box_->set_text(text);
			break;
		}
		default:
			LOG_STREAM(err, display) << "unknown textbox mode\n";
		}
	}
}
