/*
   Copyright (C) 2006 - 2016 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"
#include "floating_textbox.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"
#include "gui/dialogs/dialog.hpp"

#include "display_chat_manager.hpp"
#include "floating_label.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "log.hpp"
#include "resources.hpp"

#include <ctime>

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

namespace gui2 {

	REGISTER_DIALOG(floating_textbox);

	tfloating_textbox::tfloating_textbox(game_display& gui, MODE mode, const std::string& label, const std::string& check_label, bool checked) :
//		ttext_box(),
	//	gui_(gui),
		check_(nullptr),
		mode_(mode),
		label_string_(label),
		check_label_(check_label),
		initially_checked_(checked),
		active_(false)
	{/*
		set_definition("transparent");
		CVideo& video = CVideo::get_singleton();
		if(!video.faked()) {
			connect_signal<event::DRAW>(std::bind(&tfloating_textbox::draw, this, std::ref(video)));
		}*/
	}
	/*
	tfloating_textbox::~tfloating_textbox()
	{
		if(check_ && mode_ == MESSAGE) {
			preferences::set_message_private(check_->get_value_bool());
		}
		font::remove_floating_label(label_);
		gui_.invalidate_all();
	}
	
	void tfloating_textbox::draw(CVideo& video)
	{
		surface& frame_buffer = video.getSurface();
		draw_background(frame_buffer, 0, 0);
		draw_children(frame_buffer, 0, 0);
		draw_foreground(frame_buffer, 0, 0);
	}
	*/
	void tfloating_textbox::pre_show(twindow& window)
	{
		active_ = true;
		
		find_widget<tcontrol>(&window, "label", false).set_label(label_string_);
		box_ = find_widget<ttext_box>(&window, "box", false, true);

		check_ = find_widget<ttoggle_button>(&window, "mode_toggle", false, true);
		if(check_label_.empty()) {
			check_->set_visible(twindow::tvisible::invisible);
		} else {
			check_->set_label(check_label_);
			check_->set_value_bool(initially_checked_);
		}

		/*
		const SDL_Rect& area = gui_.map_outside_area();
		const int border_size = 10;
		const int check_height = check_ ? check_->get_best_size().y : 0;
		const int ypos = area.y+area.h-30 - check_height;

		const SDL_Rect& label_area = font::get_floating_label_rect(label_);
		const int textbox_width = area.w - label_area.w - border_size*3;

		if(textbox_width <= 0) {
			font::remove_floating_label(label_);
			return;
		}

		tpoint origin(area.x + label_area.w + border_size * 2, ypos);
		tpoint size(textbox_width, get_best_size().y);
		set_origin(origin);
		place(origin, size);

		if(check_) {
			tpoint origin(get_x(), get_y() + get_height());
			tpoint size = check_->get_best_size();
			check_->set_origin(origin);
			check_->place(origin, size);
		}*/
	}
	/*
	void tfloating_textbox::show(MODE mode, const std::string& label, const std::string& check_label, bool checked)
	{
		label_string_ = label;
		mode_ = mode;

//		set_label(label);
		set_font_size(font::SIZE_PLUS);
		connect();
//		box_.reset(new gui::textbox(gui.video(),100,"",true,256,font::SIZE_PLUS,0.8,0.6));

		if(!check_label.empty()) {
			check_.reset(new ttoggle_button);
			check_->set_definition("default");
			check_->set_label(check_label);
			check_->set_value_bool(checked);
			check_->set_parent(this);
			check_->connect();
		}

		update_location();
		fire(event::DRAW, *this);
	}
	*/
	void tfloating_textbox::tab(const std::set<std::string>& dictionary)
	{
		std::string text = box_->get_value();
		std::vector<std::string> matches(dictionary.begin(), dictionary.end());
		const bool line_start = utils::word_completion(text, matches);

		if (matches.empty()) return;
		if (matches.size() == 1 && mode_ == MESSAGE) {
			text.append(line_start ? ": " : " ");
		} else if (matches.size() > 1) {
			std::string completion_list = utils::join(matches, " ");
			resources::screen->get_chat_manager().add_chat_message(time(nullptr), "", 0, completion_list,
					events::chat_handler::MESSAGE_PRIVATE, false);
		}
		box_->set_value(text);
	}

	bool tfloating_textbox::checked() const
	{
		return check_ ? check_->get_value_bool() : false;
	}

	std::string tfloating_textbox::get_value() const
	{
		return box_->get_value();
	}
}
