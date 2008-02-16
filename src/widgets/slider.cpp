/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "widgets/slider.hpp"
#include "game_config.hpp"
#include "font.hpp"
#include "image.hpp"
#include "sound.hpp"
#include "video.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace {
	const std::string slider_image = "buttons/slider.png";
	const std::string selected_image = "buttons/slider-selected.png";
}

namespace gui {

slider::slider(CVideo &video)
	: widget(video), image_(image::get_image(slider_image)),
	  highlightedImage_(image::get_image(selected_image)),
	  min_(-100000), max_(100000), value_(0),
	  increment_(1), value_change_(false), state_(NORMAL)
{
}

void slider::enable(bool new_val)
{
	if(new_val != enabled())
	{
		state_ = NORMAL;
		widget::enable(new_val);
	}
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
	if (value_ < min_) {
		value_ = min_;
		value_change_ = true;
	}
	set_dirty(true);
}

void slider::set_max(int value)
{
	max_ = value;
	if (value_ > max_) {
		value_ = max_;
		value_change_ = true;
	}
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
	value_change_ = true;
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

bool slider::value_change()
{
	if (value_change_) {
		value_change_ = false;
		return true;
	}
	return false;
}

SDL_Rect slider::slider_area() const
{
	static const SDL_Rect default_value = {0,0,0,0};
	SDL_Rect const &loc = location();
	if (image_.null() || image_->w >= loc.w)
		return default_value;

	int xpos = loc.x + (value_ - min_) *
		static_cast<int>(loc.w - image_->w) / (max_ - min_);
	SDL_Rect res = { xpos, loc.y, image_->w, image_->h };
	return res;
}

void slider::draw_contents()
{
	surface image(state_ != NORMAL ? highlightedImage_ : image_);
	if (image == NULL)
		return;
	SDL_Color line_colour = font::NORMAL_COLOUR;
	if (!enabled()) {
		image = greyscale_image(image);
		line_colour = font::DISABLED_COLOUR;
	}

	SDL_Rect const &loc = location();
	if (image->w >= loc.w)
		return;

	surface const screen = video().getSurface();

	SDL_Rect line_rect = { loc.x + image->w / 2, loc.y + loc.h / 2, loc.w - image->w, 1 };
	SDL_FillRect(screen, &line_rect, SDL_MapRGB(screen->format,
		line_colour.r, line_colour.g, line_colour.b));

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

	set_value(tmp * (max_ - min_) / static_cast<int>(loc.w - image_->w) + min_);
}

void slider::mouse_motion(const SDL_MouseMotionEvent& event)
{
	if (state_ == NORMAL || state_ == ACTIVE) {
		bool on = point_in_rect(event.x, event.y, slider_area());
		state_ = on ? ACTIVE : NORMAL;
	} else if (state_ == CLICKED || state_ == DRAGGED) {
		state_ = DRAGGED;
		bool prev_change = value_change_;
		value_change_ = false;
		set_slider_position(event.x);
		if(value_change_) {
			sound::play_UI_sound(game_config::sounds::slider_adjust);
		} else {
			value_change_ = prev_change;
		}
	}
}

void slider::mouse_down(const SDL_MouseButtonEvent& event)
{
	if (event.button != SDL_BUTTON_LEFT || !point_in_rect(event.x, event.y, location()))
		return;

	state_ = CLICKED;
	bool prev_change = value_change_;
	value_change_ = false;
	set_focus(true);
	set_slider_position(event.x);
	if(value_change_) {
		sound::play_UI_sound(game_config::sounds::slider_adjust);
	} else {
		sound::play_UI_sound(game_config::sounds::button_press);
		value_change_ = prev_change;
	}
}

bool slider::requires_event_focus(const SDL_Event* event) const
{
	if(!focus_ || !enabled() || hidden()) {
		return false;
	}
	if(event == NULL) {
		//when event is not specified, signal that focus may be desired later
		return true;
	}

	if(event->type == SDL_KEYDOWN) {
		SDLKey key = event->key.keysym.sym;
		switch(key) {
		case SDLK_LEFT:
		case SDLK_RIGHT:
			return true;
		default:
			break;
		}
	}
	//mouse events are processed regardless of focus
	return false;
}

void slider::handle_event(const SDL_Event& event)
{
	if (!enabled() || hidden())
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
	case SDL_KEYDOWN:
		if(focus(&event)) {
			const SDL_keysym& key = reinterpret_cast<const SDL_KeyboardEvent&>(event).keysym;
			const int c = key.sym;
			if(c == SDLK_LEFT) {
				sound::play_UI_sound(game_config::sounds::slider_adjust);
				set_value(value_ - increment_);
			} else if(c == SDLK_RIGHT) {
				sound::play_UI_sound(game_config::sounds::slider_adjust);
				set_value(value_ + increment_);
			}
		}
		break;
	default:
		return;
	}
	if (start_state != state_)
		set_dirty(true);
}

template<typename T>
list_slider<T>::list_slider(CVideo &video) :
	slider(video)
{
	set_min(0);
	set_increment(1);
	slider::set_value(0);
}

template<typename T>
list_slider<T>::list_slider(CVideo &video, const std::vector<T> &items) :
	slider(video),
	items_(items)
{
	set_min(0);
	set_increment(1);
	if(items.size() > 0)
	{
		set_max(items.size() - 1);
	}
	slider::set_value(0);
}

template<typename T>
const T& list_slider<T>::item_selected() const
{
	return items_[value()];
}

template<typename T>
bool list_slider<T>::select_item(const T& item)
{
	for(unsigned i = 0, nb = items_.size(); i < nb; ++i)
	{
		if(item == items_[i])
		{
			slider::set_value(i);
			return true;
		}
	}
	return false;
}

template<typename T>
void list_slider<T>::set_items(const std::vector<T> &items)
{
	items_ = items;
	if(items.size() > 0)
	{
		set_max(items.size() - 1);
	}
	slider::set_value(0);
}

// Force compilation of the following template instantiations
template class list_slider< double >;
template class list_slider< int >;
template class list_slider< std::string >;

} //end namespace gui
