/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "global.hpp"

#include "construct_dialog.hpp"
#include "widgets/combo.hpp"

class CVideo;

namespace gui {

const std::string combo::empty_combo_label = "";

const int combo::font_size = font::SIZE_SMALL;
const int combo::horizontal_padding = 10;
const int combo::vertical_padding = 10;

combo::combo(CVideo& video, const std::vector<std::string>& items)
	: button(video, items.empty() ? empty_combo_label : items[0]),
	  items_(items), selected_(0), oldSelected_(0), video_(&video)
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

void combo::make_drop_down_menu()
{
	SDL_Rect const &loc = location();
	set_selected_internal(gui::show_dialog(*video_, nullptr, "", "", gui::MESSAGE, &items_,
	                                       nullptr, "", nullptr, -1, nullptr, loc.x, loc.y + loc.h,
	                                       &gui::dialog::hotkeys_style));
}

void combo::process_event()
{
	if (!pressed())
		return;
	make_drop_down_menu();
}

}
