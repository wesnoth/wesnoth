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
#include "../image.hpp"
#include "../video.hpp"

#include <algorithm>
#include <iostream>

namespace {
	const std::string slider_image = "buttons/slider.png";
	const std::string selected_image = "buttons/slider-selected.png";
}

namespace gui {

slider::slider(display& disp, SDL_Rect& rect, double value)
: disp_(&disp),
 buffer_(NULL), area_(rect), value_(value), drawn_(false),
 highlight_(false), clicked_(true), dragging_(false)
{
	background_changed();
}

slider::slider(const slider& o) :
   disp_(o.disp_),
   buffer_(NULL),
   area_(o.area_), value_(o.value_), drawn_(o.drawn_), highlight_(o.highlight_),
   clicked_(o.clicked_), dragging_(o.dragging_)
{
	background_changed();
}

slider& slider::operator=(const slider& o)
{
	disp_ = o.disp_;
	buffer_.assign(NULL);
	area_ = o.area_;
	value_ = o.value_;
	drawn_ = o.drawn_;
	highlight_ = o.highlight_;
	clicked_ = o.clicked_;
	dragging_ = o.dragging_;
	background_changed();

	return *this;
}

int slider::height(display& disp)
{
	const scoped_sdl_surface image(image::get_image(slider_image,image::UNSCALED));
	if(image != NULL)
		return image->h;
	else
		return 0;
}

double slider::normalize(int value, int min_value, int max_value)
{
	return (double(value) - double(min_value))/double(max_value - min_value);
}

int slider::denormalize(double value, int min_value, int max_value)
{
	return min_value + int(value*double(max_value - min_value));
}

void slider::draw()
{
	drawn_ = true;

	const scoped_sdl_surface image(image::get_image(highlight_ ? selected_image : slider_image,image::UNSCALED));
	if(image == NULL || buffer_ == NULL)
		return;

	const int hpadding = image->w/2;
	if(hpadding*2 >= area_.w)
		return;

	SDL_Surface* const screen = disp_->video().getSurface();

	SDL_BlitSurface(buffer_,NULL,screen,&area_);

	surface_lock screen_lock(screen);
	display::Pixel* const pixels = screen_lock.pixels();
	display::Pixel* const line_dest = pixels + screen->w*(area_.y+area_.h/3) +
	                                           area_.x + hpadding;
	std::fill(line_dest,line_dest+area_.w-hpadding*2,0xFFFF);

	SDL_Rect slider = slider_area();
	disp_->blit_surface(slider.x,slider.y,image);

	update_rect(area_);
}

void slider::set_value(double value)
{
	value_ = value;
}

double slider::process(int mousex, int mousey, bool button)
{
	const scoped_sdl_surface img(image::get_image(slider_image,image::UNSCALED));
	if(img == NULL)
		return 0.0;

	bool should_draw = !drawn_;

	const SDL_Rect& hit_area = slider_area();
	const bool on = mousex > hit_area.x && mousex <= hit_area.x+hit_area.w &&
	                mousey > hit_area.y && mousey <= hit_area.y+hit_area.h;

	if(on != highlight_) {
		highlight_ = on;
		should_draw = true;
	}

	const bool new_click = button && !clicked_;
	if(new_click && on) {
		dragging_ = true;
	}

	if(!button) {
		dragging_ = false;
	}

	clicked_ = button;

	double new_value = value_;

	if(dragging_) {
		new_value = double(mousex - (area_.x + img->w/2))/
		            double(area_.w - img->w);
		if(new_value < 0.0)
			new_value = 0.0;

		if(new_value > 1.0)
			new_value = 1.0;
	}

	if(should_draw || new_value != value_)
		draw();

	if(new_value != value_) {
		value_ = new_value;
		return value_;
	} else {
		return -1.0;
	}
}

SDL_Rect slider::slider_area() const
{
	static const SDL_Rect default_value = {0,0,0,0};
	const scoped_sdl_surface img(image::get_image(slider_image,image::UNSCALED));
	if(img == NULL)
		return default_value;

	const int hpadding = img->w/2;
	if(hpadding*2 >= area_.w)
		return default_value;

	const int position = int(value_*double(area_.w - hpadding*2));
	const int xpos = area_.x + position;
	SDL_Rect res = {xpos,area_.y,img->w,img->h};
	return res;
}

void slider::background_changed()
{
	const scoped_sdl_surface img(image::get_image(slider_image,image::UNSCALED));
	if(img != NULL) {
		area_.h = img->h;
		buffer_.assign(get_surface_portion(disp_->video().getSurface(),area_));
	}
}

}
