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

#include "floating_textbox.hpp"

#include "gui/core/event/handler.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/dialogs/modal_dialog.hpp"

#include "log.hpp"

#include <ctime>

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

namespace gui2::dialogs {

REGISTER_DIALOG(floating_textbox);

floating_textbox::floating_textbox(MODE mode, const std::string& label, const std::string& check_label, bool checked)
	: modeless_dialog(window_id())
	, check_(nullptr)
	, mode_(mode)
	, label_string_(label)
	, check_label_(check_label)
	, initially_checked_(checked)
	, active_(false)
{
	active_ = true;
	set_properties(mode_, label_string_, check_label_, initially_checked_);

	box_->connect_signal<event::SDL_KEY_DOWN>(std::bind(
		&floating_textbox::key_down, this, std::placeholders::_2, std::placeholders::_3, std::placeholders::_5, std::placeholders::_6));
}

void floating_textbox::pre_show()
{
#if defined(__ANDROID__) || defined(__IPHONEOS__)
	// Show onscreen keyboard
	SDL_StartTextInput(video::get_window());
#endif
}

void floating_textbox::set_properties(MODE mode, const std::string& label_str, const std::string& check_label, bool checked)
{
	mode_ = mode;
	find_widget<label>("label").set_label(label_str);
	box_ = find_widget<text_box>("box", false, true);
	keyboard_capture(box_);

	check_ = find_widget<toggle_button>("mode_toggle", false, true);
	if(check_label.empty()) {
		check_->set_visible(visibility::invisible);
	} else {
		check_->set_label(check_label);
		check_->set_value_bool(checked);
	}
}



std::string floating_textbox::tab(const std::set<std::string>& dictionary)
{
	if(active() == false) {
		return "";
	}

	std::string text = box_->text();
	std::vector<std::string> matches(dictionary.begin(), dictionary.end());
	const bool line_start = utils::word_completion(text, matches);

	if(matches.empty()) {
		return "";
	}

	std::string completion_list;
	if(matches.size() == 1 && mode_ == MESSAGE) {
		text.append(line_start ? ": " : " ");
	} else if(matches.size() > 1) {
		completion_list = utils::join(matches, " ");
	}
	box_->set_value(text);
	return completion_list;
}

void floating_textbox::memorize_command(const std::string& command)
{
	if(command.empty()) {
		return;
	}

	auto prev = std::find(command_history_.begin(), command_history_.end(), command);

	if(prev != command_history_.end()) {
		command_history_.erase(prev);
	}
	command_history_.emplace_back(command);
}

bool floating_textbox::checked() const
{
	return check_ ? check_->get_value_bool() : false;
}

std::string floating_textbox::get_value() const
{
	return box_->get_value();
}

void floating_textbox::set_value(const std::string& text)
{
	box_->set_value(text);
}

void floating_textbox::history_update(bool up)
{
	const std::string str = get_value();

	auto prev = std::find(command_history_.begin(), command_history_.end(), str);

	if (prev != command_history_.end())
	{
		if(up) {
			if(prev != command_history_.begin()) {
				set_value(*--prev);
			}
		} else {
			if(++prev != command_history_.end()) {
				set_value(*prev);
			} else {
				set_value("");
			}
		}
	} else if (up) {
		if(command_history_.size() > 0) {
			set_value(*--prev);
		}

		if(!str.empty()) {
			memorize_command(str);
		}
	}
}

void floating_textbox::key_down(const event::ui_event /*event*/,
								bool& handled,
								const SDL_Keycode key,
								SDL_Keymod /*modifier*/)
{
	switch(key) {
		case SDLK_ESCAPE:
			handled = true;
			hide();
			break;

		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			handled = true;
			do_enter_(get_value());
			hide();
			break;
	}
}

}
