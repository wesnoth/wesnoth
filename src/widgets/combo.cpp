/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "combo.hpp"
#include "button.hpp"
#include "../show_dialog.hpp"
#include "../video.hpp"

namespace gui {

const int font_size = 12;
const int horizontal_padding = 10;
const int vertical_padding = 10;

combo::combo(display& disp, const std::vector<std::string>& items) :
		items_(items), selected_(0), display_(&disp),
		button_(gui::button(disp, items[0]))
{
}


int combo::height() const
{
	return button_.height();
}

int combo::width() const
{
	return button_.width();
}

int combo::selected() const
{
	return selected_;
}

void combo::set_xy(int x, int y)
{
	button_.set_xy(x,y);
}

void combo::set_selected(int val)
{
	selected_ = val;
	const std::string& label = items_[selected_];
	if(selected_ >= 0)
		button_.set_label(label);
}

void combo::process(int x, int y, bool button)
{
	if(button_.process(x,y,button))
	{
		selected_ = gui::show_dialog(*display_,NULL,"","",
				gui::MESSAGE,&items_);
		const std::string& label = items_[selected_];
		if(selected_ >= 0)
			button_.set_label(label);
	}
}

}
