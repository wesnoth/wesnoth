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

#include "../global.hpp"

#include "combo.hpp"
#include "button.hpp"
#include "../display.hpp"
#include "../show_dialog.hpp"
#include "../video.hpp"

namespace gui {

const int font_size = font::SIZE_SMALL;
const int horizontal_padding = 10;
const int vertical_padding = 10;

combo::combo(display& disp, const std::vector<std::string>& items)
	: button(disp.video(), items.empty() ? "" : items[0]),
	  items_(items), selected_(0), oldSelected_(0), disp_(&disp)
{
}

int combo::selected() const
{
	return selected_;
}

bool combo::changed()
{
	if (oldSelected_ != selected_) {
		oldSelected_ = selected_;
		return true;
	} else
		return false;
}

void combo::set_items(const std::vector<std::string>& items)
{
	items_ = items;
	selected_ = -1;
}

size_t combo::items_size() const
{
	return items_.size();
}

void combo::set_selected_internal(int val)
{
	const size_t index = size_t(val);
	if (val == selected_ || index >= items_.size())
		return;
	set_label(items_[index]);
	oldSelected_ = selected_;
	selected_ = val;
}

void combo::set_selected(int val)
{
	set_selected_internal(val);
	oldSelected_ = selected_;
}

void combo::process_event()
{
	if (!pressed())
		return;
	SDL_Rect const &loc = location();
	set_selected_internal(gui::show_dialog(*disp_, NULL, "", "", gui::MESSAGE, &items_,
	                                       NULL, "", NULL, -1, NULL, NULL, loc.x, loc.y + loc.h));
}

}
