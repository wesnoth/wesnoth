/*
   Copyright (C) 2004 - 2017 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "widgets/scrollarea.hpp"
#include "sdl/rect.hpp"
#include <video.hpp>


namespace gui {

scrollarea::scrollarea(CVideo &video, const bool auto_join)
	: widget(video, auto_join), scrollbar_(video),
	  old_position_(0), recursive_(false), shown_scrollbar_(false),
	  shown_size_(0), full_size_(0), swipe_dy_(0)
{
	scrollbar_.hide(true);
}

bool scrollarea::has_scrollbar() const
{
	return shown_size_ < full_size_ && scrollbar_.is_valid_height(location().h);
}

sdl_handler_vector scrollarea::handler_members()
{
	sdl_handler_vector h;
	h.push_back(&scrollbar_);
	return h;
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
	if (shown_scrollbar_)
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
	shown_size_ = h;
	test_scrollbar();
}

void scrollarea::set_full_size(unsigned h)
{
	scrollbar_.set_full_size(h);
	full_size_ = h;
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
	if (shown_scrollbar_)
		r.w -= scrollbar_.width();
	return r;
}

unsigned scrollarea::scrollbar_width() const
{
	return scrollbar_.width();
}

void scrollarea::handle_event(const SDL_Event& event)
{
	gui::widget::handle_event(event);

	if (mouse_locked() || hidden())
		return;

	if (event.type == SDL_MOUSEWHEEL) {
		const SDL_MouseWheelEvent &ev = event.wheel;
		int x, y;
		SDL_GetMouseState(&x, &y);
		if (sdl::point_in_rect(x, y, inner_location())) {
			if (ev.y > 0) {
				scrollbar_.scroll_up();
			} else if (ev.y < 0) {
				scrollbar_.scroll_down();
			}
		}
	}
	
	if (event.type == SDL_FINGERUP) {
		swipe_dy_ = 0;
	}
	
	if (event.type == SDL_FINGERDOWN || event.type == SDL_FINGERMOTION) {
		SDL_Rect r = screen_area();
		auto tx = (int) (event.tfinger.x * r.w);
		auto ty = (int) (event.tfinger.y * r.h);
		auto dy = (int) (event.tfinger.dy * r.h);
		
		if (event.type == SDL_FINGERDOWN) {
			swipe_dy_ = 0;
			swipe_origin_.x = tx;
			swipe_origin_.y = ty;
		}

		if (event.type == SDL_FINGERMOTION) {
			
			swipe_dy_ += dy;
			if (scrollbar_.get_max_position() == 0) {
				return;
			}

			int scrollbar_step = scrollbar_.height() / scrollbar_.get_max_position();
			if (scrollbar_step <= 0) {
				return;
			}
			
			if (sdl::point_in_rect(swipe_origin_.x, swipe_origin_.y, inner_location())
				&& abs(swipe_dy_) >= scrollbar_step)
			{
				unsigned int pos = (unsigned int) std::max(
						(int) scrollbar_.get_position() - swipe_dy_ / scrollbar_step,
						0);
				scrollbar_.set_position(pos);
				swipe_dy_ %= scrollbar_step;
			}
		}
	}
	
}

} // end namespace gui

