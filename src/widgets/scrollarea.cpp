/* $Id$*/
/*
   Copyright (C) 2004 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "../global.hpp"

#include "scrollarea.hpp"
#include "../display.hpp"
#include "../image.hpp"
#include "../video.hpp"

#include <algorithm>
#include <iostream>

namespace gui {

scrollarea::scrollarea(display &d)
	: widget(d), scrollbar_(d),
	  old_position_(0), recursive_(false), shown_scrollbar_(false)
{
}

bool scrollarea::has_scrollbar() const
{
	return scrollbar_.grip_height_ < scrollbar_.full_height_ && scrollbar_.has_valid_size();
}

void scrollarea::update_location(SDL_Rect const &rect)
{
	SDL_Rect r = rect;
	shown_scrollbar_ = has_scrollbar();
	if (shown_scrollbar_) {
		int w = r.w - scrollbar_.width();
		r.x += w;
		r.w -= w;
		scrollbar_.set_location(r);
		r.x -= w;
		r.w = w;
	}
	if (!hidden())
		scrollbar_.hide(!shown_scrollbar_);
	set_inner_location(r);
}

void scrollarea::test_scrollbar()
{
	if (recursive_)
		return;
	recursive_ = true;
	if (shown_scrollbar_ != has_scrollbar()) {
		bg_restore();
		bg_cancel();
		update_location(location());
	}
	recursive_ = false;
}

void scrollarea::hide(bool value)
{
	widget::hide(value);
	if (has_scrollbar())
		scrollbar_.hide(value);
}

unsigned scrollarea::get_position() const
{
	return scrollbar_.get_position();
}

unsigned scrollarea::get_max_position() const
{
	return scrollbar_.get_max_position();
}

void scrollarea::set_position(unsigned pos)
{
	scrollbar_.set_position(pos);
}

void scrollarea::adjust_position(unsigned pos)
{
	scrollbar_.adjust_position(pos);
}

void scrollarea::move_position(int dep)
{
	scrollbar_.move_position(dep);
}

void scrollarea::set_shown_size(unsigned h)
{
	scrollbar_.set_shown_size(h);
	test_scrollbar();
}

void scrollarea::set_full_size(unsigned h)
{
	scrollbar_.set_full_size(h);
	test_scrollbar();
}

void scrollarea::set_scroll_rate(unsigned r)
{
	scrollbar_.set_scroll_rate(r);
}

void scrollarea::process_event()
{
	int grip_position = scrollbar_.get_position();
	if (grip_position == old_position_)
		return;
	old_position_ = grip_position;
	scroll(grip_position);
}

SDL_Rect scrollarea::inner_location() const
{
	SDL_Rect r = location();
	if (has_scrollbar())
		r.w -= scrollbar_.width();
	return r;
}

unsigned scrollarea::scrollbar_width() const
{
	return scrollbar_.width();
}

void scrollarea::handle_event(const SDL_Event& event)
{
	if (hidden() || event.type != SDL_MOUSEBUTTONDOWN)
		return;

	SDL_MouseButtonEvent const &e = event.button;
	if (point_in_rect(e.x, e.y, inner_location()))
		if (e.button == SDL_BUTTON_WHEELDOWN)
			scrollbar_.move_position(scrollbar_.scroll_rate_);
		else if (e.button == SDL_BUTTON_WHEELUP)
			scrollbar_.move_position(-scrollbar_.scroll_rate_);
}

}
