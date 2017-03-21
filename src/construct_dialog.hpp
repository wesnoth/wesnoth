/*
   Copyright (C) 2006 - 2017 by Patrick Parker <patrick_x99@hotmail.com>
   wesnoth widget Copyright (C) 2003-5 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef CONSTRUCT_DIALOG_H_INCLUDED
#define CONSTRUCT_DIALOG_H_INCLUDED

#include "show_dialog.hpp"

#include "widgets/label.hpp"
#include "widgets/textbox.hpp"
#include "key.hpp"

namespace gui {

struct dialog_process_info
{
public:
	dialog_process_info() :
		key(),
		left_button(true),
		right_button(true),
		key_down(true),
		first_time(true),
		double_clicked(false),
		new_left_button(false),
		new_right_button(false),
		new_key_down(false),
		selection(-1),
		clear_buttons_(false)
	{}

	void clear_buttons() {
		clear_buttons_ = true;
	}

	void cycle() {
		if(clear_buttons_) {
			left_button = true;
			right_button = true;
			key_down = true;
			clear_buttons_ = false;
		} else {
			left_button = new_left_button;
			right_button = new_right_button;
			key_down = new_key_down;
		}
	}
	CKey key;
	bool left_button, right_button, key_down, first_time, double_clicked;
	bool new_left_button, new_right_button, new_key_down;
	int selection;
private:
	bool clear_buttons_;
};

class dialog_button : public button {
public:
	dialog_button(CVideo& video, const std::string& label, TYPE type=TYPE_PRESS,
		int simple_result=CONTINUE_DIALOG, dialog_button_action *handler=nullptr)
		: button(video,label,type,"",DEFAULT_SPACE,false), simple_result_(simple_result),
		handler_(handler)
	{}
	bool is_option() const {
		return (type_ == TYPE_CHECK);
	}
	virtual int action(dialog_process_info &info);
protected:
	const int simple_result_;
private:
	dialog_button_action *handler_;
};

} //end namespace gui
#endif
