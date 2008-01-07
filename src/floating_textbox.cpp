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

#include "floating_textbox.hpp"

#include "font.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "log.hpp"
#include "team.hpp"

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
		case gui::TEXTBOX_MESSAGE:
		{
			std::string text = box_->text();
			std::string semiword;
			bool beginning;

			const size_t last_space = text.rfind(" ");

			//if last character is a space return
			if(last_space == text.size() -1) {
				return;
			}

			if(last_space == std::string::npos) {
				beginning = true;
				semiword = text;
			}else{
				beginning = false;
				semiword.assign(text,last_space+1,text.size());
			}

			std::set<std::string>		matches;
			std::string best_match = semiword;

			for(size_t n = 0; n != teams.size(); ++n) {
				if(teams[n].is_empty()) {
					continue;
				}
				const std::string& name = teams[n].current_player();
				if( name.size() >= semiword.size() &&
						std::equal(semiword.begin(),semiword.end(),name.begin(),chars_equal_insensitive)) {
					if(matches.empty()) {
						best_match = name;
					} else {
						// Testing if this name already isn't in set
						//   Because some players can control more sides
						if (matches.count(name) >= 1){
							continue;
						}
						int j= 0;;
						while(best_match[j] == name[j]) j++;
						best_match.erase(best_match.begin()+j,best_match.end());
					}
					matches.insert(name);
				}
			}

			// Searching in observers list
			const std::set<std::string>& observers = gui.observers();
			for(std::set<std::string>::const_iterator i = observers.begin(); i != observers.end(); ++i) {
				if( i->size() >= semiword.size() &&
						std::equal(semiword.begin(),semiword.end(),i->begin(),chars_equal_insensitive)) {
					if(matches.empty()) {
						best_match = *i;
					} else {
						int j = 0;
						while(toupper(best_match[j]) == toupper((*i)[j])) j++;
						best_match.erase(best_match.begin()+j,best_match.end());
					}
					matches.insert(*i);
				}
			}

			if(!matches.empty()) {
				std::string add = beginning ? ": " : " ";
				text.replace(last_space+1, semiword.size(), best_match);
				if(matches.size() == 1) {
					text.append(add);
				} else {
					std::string completion_list;
					std::set<std::string>::iterator it;
					for(it =matches.begin();it!=matches.end();it++) {
						completion_list += " ";
						completion_list += *it;
					}
					gui.add_chat_message(time(NULL), "", 0, completion_list,
							game_display::MESSAGE_PRIVATE, false);
				}
				box_->set_text(text);
			}
			break;
		}
		default:
			LOG_STREAM(err, display) << "unknown textbox mode\n";
		}
	}
}
