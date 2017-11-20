/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "help/help_browser.hpp"
#include <iostream>                     // for operator<<, basic_ostream, etc
#include <SDL_mouse.h>                  // for SDL_GetMouseState, etc
#include "cursor.hpp"                   // for set, CURSOR_TYPE::HYPERLINK, etc
#include "font/constants.hpp"           // for relative_size
#include "gettext.hpp"                  // for _
#include "gui/dialogs/transient_message.hpp"
#include "help/help_text_area.hpp"      // for help_text_area
#include "help/help_impl.hpp"                // for find_topic, hidden_symbol, etc
#include "key.hpp"                      // for CKey
#include "log.hpp"                      // for log_scope
#include "sdl/rect.hpp"

class CVideo;
struct SDL_Rect;

namespace help {

help_browser::help_browser(CVideo& video, const section &toplevel) :
	gui::widget(video),
	menu_(video,
	toplevel),
	text_area_(video, toplevel), toplevel_(toplevel),
	ref_cursor_(false),
	back_topics_(),
	forward_topics_(),
	back_button_(video, "", gui::button::TYPE_PRESS, "button_normal/button_small_H22", gui::button::DEFAULT_SPACE, true, "icons/arrows/long_arrow_ornate_left"),
	forward_button_(video, "", gui::button::TYPE_PRESS, "button_normal/button_small_H22", gui::button::DEFAULT_SPACE, true, "icons/arrows/long_arrow_ornate_right"),
	shown_topic_(nullptr)
{
	// Hide the buttons at first since we do not have any forward or
	// back topics at this point. They will be unhidden when history
	// appears.
	back_button_.hide(true);
	forward_button_.hide(true);
	// Set sizes to some default values.
	set_measurements(font::relative_size(400), font::relative_size(500));
}

void help_browser::adjust_layout()
{
	const int menu_buttons_padding = font::relative_size(10);
	const int menu_y = location().y;
	const int menu_x = location().x;
	const int menu_w = 250;
	const int menu_h = height();

	const int menu_text_area_padding = font::relative_size(10);
	const int text_area_y = location().y;
	const int text_area_x = menu_x + menu_w + menu_text_area_padding;
	const int text_area_w = width() - menu_w - menu_text_area_padding;
	const int text_area_h = height();

	const int button_border_padding = 0;
	const int button_button_padding = font::relative_size(10);
	const int back_button_x = location().x + button_border_padding;
	const int back_button_y = menu_y + menu_h + menu_buttons_padding;
	const int forward_button_x = back_button_x + back_button_.width() + button_button_padding;
	const int forward_button_y = back_button_y;

	menu_.set_width(menu_w);
	menu_.set_location(menu_x, menu_y);
	menu_.set_max_height(menu_h);
	menu_.set_max_width(menu_w);

	text_area_.set_location(text_area_x, text_area_y);
	text_area_.set_width(text_area_w);
	text_area_.set_height(text_area_h);

	back_button_.set_location(back_button_x, back_button_y);
	forward_button_.set_location(forward_button_x, forward_button_y);

	set_dirty(true);
}

void help_browser::update_location(SDL_Rect const &)
{
	adjust_layout();
}

void help_browser::process_event()
{
	CKey key;
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);

	/// Fake focus functionality for the menu, only process it if it has focus.
	if (sdl::point_in_rect(mousex, mousey, menu_.location())) {
		menu_.process();
		const topic *chosen_topic = menu_.chosen_topic();
		if (chosen_topic != nullptr && chosen_topic != shown_topic_) {
			/// A new topic has been chosen in the menu, display it.
			show_topic(*chosen_topic);
		}
	}
	if (back_button_.pressed()) {
		move_in_history(back_topics_, forward_topics_);
	}
	if (forward_button_.pressed()) {
		move_in_history(forward_topics_, back_topics_);
	}
	back_button_.hide(back_topics_.empty());
	forward_button_.hide(forward_topics_.empty());
}

void help_browser::move_in_history(std::deque<const topic *> &from,
		std::deque<const topic *> &to)
{
	if (!from.empty()) {
		const topic *to_show = from.back();
		from.pop_back();
		if (shown_topic_ != nullptr) {
			if (to.size() > max_history) {
				to.pop_front();
			}
			to.push_back(shown_topic_);
		}
		show_topic(*to_show, false);
	}
}


void help_browser::handle_event(const SDL_Event &event)
{
	gui::widget::handle_event(event);

	SDL_MouseButtonEvent mouse_event = event.button;
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		if (mouse_event.button == SDL_BUTTON_LEFT) {
			// Did the user click a cross-reference?
			const int mousex = mouse_event.x;
			const int mousey = mouse_event.y;
			const std::string ref = text_area_.ref_at(mousex, mousey);
			if (!ref.empty()) {
				const topic *t = find_topic(toplevel_, ref);
				if (t == nullptr) {
					std::stringstream msg;
					msg << _("Reference to unknown topic: ") << "'" << ref << "'.";
					gui2::show_transient_message("", msg.str());
					update_cursor();
				}
				else {
					show_topic(*t);
					update_cursor();
				}
			}
		}
	}
	else if (event.type == SDL_MOUSEMOTION) {
		update_cursor();
	}
}

void help_browser::update_cursor()
{
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	const std::string ref = text_area_.ref_at(mousex, mousey);
	if (!ref.empty() && !ref_cursor_) {
		cursor::set(cursor::HYPERLINK);
		ref_cursor_ = true;
	}
	else if (ref.empty() && ref_cursor_) {
		cursor::set(cursor::NORMAL);
		ref_cursor_ = false;
	}
}

void help_browser::show_topic(const std::string &topic_id)
{
	const topic *t = find_topic(toplevel_, topic_id);

	if (t != nullptr) {
		show_topic(*t);
	} else if (topic_id.find(unit_prefix)==0 || topic_id.find(hidden_symbol() + unit_prefix)==0) {
		show_topic(unknown_unit_topic);
	} else {
		std::cerr << "Help browser tried to show topic with id '" << topic_id
				  << "' but that topic could not be found." << std::endl;
	}
}

void help_browser::show_topic(const topic &t, bool save_in_history)
{
	log_scope("show_topic");

	if (save_in_history) {
		forward_topics_.clear();
		if (shown_topic_ != nullptr) {
			if (back_topics_.size() > max_history) {
				back_topics_.pop_front();
			}
			back_topics_.push_back(shown_topic_);
		}
	}

	shown_topic_ = &t;
	text_area_.show_topic(t);
	menu_.select_topic(t);
	update_cursor();
}


} // end namespace help
