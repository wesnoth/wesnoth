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

slider::slider(display& d, const SDL_Rect& rect)
	: widget(d, rect), min_(-100000), max_(100000), increment_(1), 
	  value_(0), highlight_(false), clicked_(true), dragging_(false)
{
	set_dirty(true);
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
	if (value == value_)
		return;

	value_ = value;
	if (value_ > max_)
		value_ = max_;
	if (value_ < min_)
		value_ = min_;

	if (increment_ > 1) {
		int hi = increment_ / 2;
		value_ = ((value_ + hi) / increment_) * increment_;
	}

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
	const surface img(image::get_image(slider_image,image::UNSCALED));
	if(img == NULL)
		return default_value;

	if(img->w >= location().w)
		return default_value;

	double tmp = (value_ - min_ + 0.0) / (max_ - min_ + 0.0);
	int tmp2 = (int)(tmp * (location().w - img->w));
	const int xpos = location().x + tmp2;
	SDL_Rect res = {xpos,location().y,img->w,img->h};
	return res;
}

void slider::draw()
{
	if(!dirty() || hidden()) {
		return;
	}

	const surface image(image::get_image(highlight_ ? selected_image : slider_image,image::UNSCALED));
	if(image == NULL || dirty() == false)
		return;

	if(image->w >= location().w)
		return;

	surface const screen = disp().video().getSurface();

	bg_restore();

	SDL_Rect line_rect = {location().x, location().y + location().h/2,
			      location().w, 1};
	SDL_FillRect(screen,&line_rect,SDL_MapRGB(screen->format,255,255,255));

	SDL_Rect slider = slider_area();
	disp().blit_surface(slider.x,slider.y,image);

	set_dirty(false);
	update_rect(location());
}

void slider::process()
{
	if(hidden()) {
		return;
	}

	int mousex, mousey;
	const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
	const bool button = mouse_flags&SDL_BUTTON_LMASK;

	const surface img(image::get_image(slider_image,image::UNSCALED));
	if(img == NULL)
		return;

	SDL_Rect rect = {location().x, location().y, location().w, img->h};
	set_location(rect);

	const SDL_Rect& hit_area = slider_area();
	const bool on = mousex > hit_area.x && mousex <= hit_area.x+hit_area.w &&
	                mousey > hit_area.y && mousey <= hit_area.y+hit_area.h;

	if(on != highlight_) {
		highlight_ = on;
		set_dirty(true);
	}

	const bool new_click = button && !clicked_;
	if(new_click && on) {
		dragging_ = true;
	}

	if(!button) {
		dragging_ = false;
	}

	clicked_ = button;

	int new_value = value_;

	if(dragging_ || new_click && point_in_rect(mousex,mousey,rect)) {
		int tmp = mousex - location().x;
		if (tmp < 0)
			tmp = 0;
		if (tmp > location().w - img->w)
			tmp = location().w - img->w;

		double tmp2 = (tmp + 0.0) / (location().w - img->w + 0.0);
		new_value = (int)(tmp2 * (max_ - min_ + 0.0)) + min_;
	}

	set_value(new_value);

#if 0
	if(new_value != value_) {
		value_ = new_value;
		set_dirty(true);
	}
#endif

	draw();
}

}
