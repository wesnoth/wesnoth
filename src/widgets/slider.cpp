/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
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

#include "widgets/slider.hpp"
#include "game_config.hpp"
#include "font.hpp"
#include "image.hpp"
#include "sdl/rect.hpp"
#include "sound.hpp"
#include "video.hpp"


namespace {
	const std::string slider_image   = ".png";
	const std::string disabled_image = ".png~GS()";
	const std::string pressed_image  = "-pressed.png";
	const std::string active_image   = "-active.png";
}

namespace gui {

slider::slider(CVideo &video, const std::string& image, bool black)
	: widget(video), image_(image::get_image(image + slider_image)),
	  pressedImage_(image::get_image(image + pressed_image)),
	  activeImage_(image::get_image(image + active_image)),
	  disabledImage_(image::get_image(image + disabled_image)),
	  line_color_(black ? font::BLACK_COLOR : font::NORMAL_COLOR),
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

	int xpos = loc.x + (value_ - min_) * (loc.w - image_->w) / (max_ - min_);
	return sdl::create_rect(xpos, loc.y, image_->w, image_->h);
}

void slider::draw_contents()
{
	surface image;

	switch (state_) {
		case NORMAL:
			image.assign(image_);
			break;
		case ACTIVE:
			image.assign(activeImage_);
			break;
		default:
			image.assign(pressedImage_);
			break;
	}

	assert(image != NULL);

	SDL_Color line_color = line_color_;
	if (!enabled()) {
		image.assign(disabledImage_);
		line_color = font::DISABLED_COLOR;
	}

	SDL_Rect const &loc = location();
	if (image->w >= loc.w)
		return;

	surface& screen = video().getSurface();

	SDL_Rect line_rect = sdl::create_rect(loc.x + image->w / 2
			, loc.y + loc.h / 2
			, loc.w - image->w
			, 1);

	sdl::fill_rect(screen, &line_rect, SDL_MapRGB(screen->format,
		line_color.r, line_color.g, line_color.b));

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

	set_value(tmp * (max_ - min_) / (loc.w - image_->w) + min_);
}

void slider::mouse_motion(const SDL_MouseMotionEvent& event)
{
	if (state_ == NORMAL || state_ == ACTIVE) {
		bool on = sdl::point_in_rect(event.x, event.y, location());
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
	bool prev_change = value_change_;

	if (!sdl::point_in_rect(event.x, event.y, location()))
		return;

#if !SDL_VERSION_ATLEAST(2,0,0)
	if (event.button == SDL_BUTTON_WHEELUP || event.button == SDL_BUTTON_WHEELRIGHT) {
		value_change_ = false;
		set_focus(true);
		set_value(value_ + increment_);
		if(value_change_) {
			sound::play_UI_sound(game_config::sounds::slider_adjust);
		} else {
			value_change_ = prev_change;
		}
	}
	if (event.button == SDL_BUTTON_WHEELDOWN || event.button == SDL_BUTTON_WHEELLEFT) {
		value_change_ = false;
		set_focus(true);
		set_value(value_ - increment_);
		if(value_change_) {
			sound::play_UI_sound(game_config::sounds::slider_adjust);
		} else {
			value_change_ = prev_change;
		}
	}
#endif

	if (event.button != SDL_BUTTON_LEFT)
		return;

	state_ = CLICKED;
	set_focus(true);
	if (sdl::point_in_rect(event.x, event.y, slider_area())) {
		sound::play_UI_sound(game_config::sounds::button_press);
	} else {
		value_change_ = false;
		set_slider_position(event.x);
		if(value_change_) {
			sound::play_UI_sound(game_config::sounds::slider_adjust);
		} else {
			value_change_ = prev_change;
		}
	}
}

#if SDL_VERSION_ATLEAST(2,0,0)
void slider::mouse_wheel(const SDL_MouseWheelEvent& event) {
	bool prev_change = value_change_;
	int x, y;
	SDL_GetMouseState(&x, &y);

	if (!sdl::point_in_rect(x, y, location()))
		return;

	if (event.y > 0 || event.x > 0) {
		value_change_ = false;
		set_focus(true);
		set_value(value_ + increment_);
		if(value_change_) {
			sound::play_UI_sound(game_config::sounds::slider_adjust);
		} else {
			value_change_ = prev_change;
		}
	}
	if (event.y < 0 || event.x < 0) {
		value_change_ = false;
		set_focus(true);
		set_value(value_ - increment_);
		if(value_change_) {
			sound::play_UI_sound(game_config::sounds::slider_adjust);
		} else {
			value_change_ = prev_change;
		}
	}
}
#endif

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
		if (!mouse_locked()) {
			bool on = sdl::point_in_rect(event.button.x, event.button.y, slider_area());
			state_ = on ? ACTIVE : NORMAL;
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (!mouse_locked())
			mouse_down(event.button);
		break;
	case SDL_MOUSEMOTION:
		if (!mouse_locked())
			mouse_motion(event.motion);
		break;
	case SDL_KEYDOWN:
		if(focus(&event) && allow_key_events()) { //allow_key_events is used by zoom_sliders to disable left-right key press, which is buggy for them
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
#if SDL_VERSION_ATLEAST(2,0,0)
	case SDL_MOUSEWHEEL:
		if (!mouse_locked())
			mouse_wheel(event.wheel);
		break;
#endif
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

/***
*
* Zoom Slider
*
***/

zoom_slider::zoom_slider(CVideo &video, const std::string& image, bool black)
	: slider(video, image, black)
{
}

} //end namespace gui
