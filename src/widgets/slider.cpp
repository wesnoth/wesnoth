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

#include "slider.hpp"
#include "../display.hpp"
#include "../image.hpp"
#include "../video.hpp"

#include <algorithm>
#include <iostream>

namespace {
	const std::string slider_image = "buttons/slider.png";
	const std::string selected_image = "buttons/slider-selected.png";
}

namespace gui {

slider::slider(display& d)
	: widget(d), image_(image::get_image(slider_image, image::UNSCALED)),
	  highlightedImage_(image::get_image(selected_image, image::UNSCALED)),
	  min_(-100000), max_(100000), value_(0), 
	  increment_(1), state_(NORMAL)
{
}

void slider::set_location(SDL_Rect const &rect)
{
	SDL_Rect r = rect;
	r.h = image_->h;
	widget::set_location(r);
}

void slider::set_min(int value)
{
	min_ = value;
	if (value_ < min_)
		value_ = min_;
	set_dirty(true);
}

void slider::set_max(int value)
{
	max_ = value;
	if (value_ > max_)
		value_ = max_;
	set_dirty(true);
}

void slider::set_value(int value)
{
	if (value > max_)
		value = max_;
	if (value < min_)
		value = min_;

	if (increment_ > 1) {
		int hi = increment_ / 2;
		value = ((value + hi) / increment_) * increment_;
	}

	if (value == value_)
		return;

	value_ = value;
	set_dirty(true);
}

void slider::set_increment(int increment)
{
	increment_ = increment;
}

int slider::value() const
{
	return value_;
}

int slider::min_value() const
{
	return min_;
}

int slider::max_value() const
{
	return max_;
}

SDL_Rect slider::slider_area() const
{
	static const SDL_Rect default_value = {0,0,0,0};
	SDL_Rect const &loc = location();
	if (image_.null() || image_->w >= loc.w)
		return default_value;

	int xpos = loc.x + (value_ - min_) * (int)(loc.w - image_->w) / (max_ - min_);
	SDL_Rect res = { xpos, loc.y, image_->w, image_->h };
	return res;
}

void slider::draw_contents()
{
	const surface image(state_ != NORMAL ? highlightedImage_ : image_);
	if (image == NULL)
		return;

	SDL_Rect const &loc = location();
	if (image->w >= loc.w)
		return;

	surface const screen = video().getSurface();

	SDL_Rect line_rect = { loc.x + image->w / 2, loc.y + loc.h / 2, loc.w - image->w, 1 };
	SDL_FillRect(screen, &line_rect, SDL_MapRGB(screen->format, 255, 255, 255));

	SDL_Rect const &slider = slider_area();
	video().blit_surface(slider.x, slider.y, image);
}

void slider::set_slider_position(int x)
{
	SDL_Rect const &loc = location();
	int tmp = x - loc.x - image_->w / 2;
	if (tmp < 0)
		tmp = 0;
	if (tmp > loc.w - image_->w)
		tmp = loc.w - image_->w;

	set_value(tmp * (max_ - min_) / (int)(loc.w - image_->w) + min_);
}

void slider::mouse_motion(const SDL_MouseMotionEvent& event)
{
	if (state_ == NORMAL || state_ == ACTIVE) {
		bool on = point_in_rect(event.x, event.y, slider_area());
		state_ = on ? ACTIVE : NORMAL;
	} else if (state_ == CLICKED || state_ == DRAGGED) {
		state_ = DRAGGED;
		set_slider_position(event.x);
	}
}

void slider::mouse_down(const SDL_MouseButtonEvent& event)
{
	if (event.button != SDL_BUTTON_LEFT || !point_in_rect(event.x, event.y, location()))
		return;

	state_ = CLICKED;
	set_slider_position(event.x);
}

void slider::handle_event(const SDL_Event& event)
{
	if (hidden())
		return;

	STATE start_state = state_;

	switch(event.type) {
	case SDL_MOUSEBUTTONUP:
		state_ = NORMAL;
		break;
	case SDL_MOUSEBUTTONDOWN:
		mouse_down(event.button);
		break;
	case SDL_MOUSEMOTION:
		mouse_motion(event.motion);
		break;
	default:
		return;
	}

	if (start_state != state_)
		set_dirty(true);
}

}
