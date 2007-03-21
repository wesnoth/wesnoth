/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "display.hpp"
#include "halo.hpp"
#include "image.hpp"
#include "preferences.hpp"
#include "sdl_utils.hpp"
#include "util.hpp"
#include "video.hpp"
#include "wassert.hpp"
#include "serialization/string_utils.hpp"

#include <algorithm>
#include <map>

namespace halo
{

namespace {
display* disp = NULL;

class effect
{
public:
	effect(int xpos, int ypos, const animated<std::string>::anim_description& img,
			const gamemap::location& loc, ORIENTATION, bool infinite);

	void set_location(int x, int y);

	void render();
	void unrender();

	bool expired() const;
private:

	const std::string& current_image();

	animated<std::string> images_;

	ORIENTATION orientation_;

	int x_, y_;
	surface surf_, buffer_;
	SDL_Rect rect_;
	gamemap::location loc_;
};

std::map<int,effect> haloes;
int halo_id = 1;

bool hide_halo = false;


effect::effect(int xpos, int ypos, const animated<std::string>::anim_description& img,
	const gamemap::location& loc, ORIENTATION orientation, bool infinite) :
		images_(img), orientation_(orientation), x_(xpos), y_(ypos),
		surf_(NULL), buffer_(NULL), rect_(empty_rect), loc_(loc)
{
	wassert(disp != NULL);
	// std::cerr << "Constructing halo sequence from image " << img << "\n";

	set_location(xpos,ypos);

	images_.start_animation(0,infinite);

	if(!images_.animation_finished()) {
		images_.update_last_draw_time();
	}
}

void effect::set_location(int x, int y)
{
	const gamemap::location zero_loc(0,0);
	x_ = x - disp->get_location_x(zero_loc);
	y_ = y - disp->get_location_y(zero_loc);
}

const std::string& effect::current_image()
{
	static const std::string r = "";

	const std::string& res = images_.get_current_frame();

	return res;
}

void effect::render()
{
	if(disp == NULL) {
		return;
	}
	
	if(loc_.x != -1 && loc_.y != -1 && disp->shrouded(loc_.x, loc_.y)) {
		return;
	}

	images_.update_last_draw_time();
	surf_.assign(image::get_image(current_image(),image::SCALED_TO_ZOOM));
	if(surf_ == NULL) {
		return;
	}
	if(orientation_ == HREVERSE || orientation_ == HVREVERSE) {
		surf_.assign(image::reverse_image(surf_));
	}
	if(orientation_ == VREVERSE || orientation_ == HVREVERSE) {
		surf_.assign(flop_surface(surf_));
	}

	const gamemap::location zero_loc(0,0);
	const int screenx = disp->get_location_x(zero_loc);
	const int screeny = disp->get_location_y(zero_loc);

	const int xpos = x_ + screenx - surf_->w/2;
	const int ypos = y_ + screeny - surf_->h/2;

	SDL_Rect rect = {xpos,ypos,surf_->w,surf_->h};
	rect_ = rect;
	SDL_Rect clip_rect = disp->map_area();
	if(rects_overlap(rect,clip_rect) == false) {
		buffer_.assign(NULL);
		return;
	}

	surface const screen = disp->video().getSurface();

	const clip_rect_setter clip_setter(screen,clip_rect);
	if(buffer_ == NULL || buffer_->w != rect.w || buffer_->h != rect.h) {
		SDL_Rect rect = rect_;
		buffer_.assign(get_surface_portion(screen,rect));
	} else {
		SDL_Rect rect = rect_;
		SDL_BlitSurface(screen,&rect,buffer_,NULL);
	}

	SDL_BlitSurface(surf_,NULL,screen,&rect);

	update_rect(rect_);
}

void effect::unrender()
{
	if(buffer_ == NULL) {
		return;
	}

	surface const screen = disp->video().getSurface();

	SDL_Rect clip_rect = disp->map_area();
	const clip_rect_setter clip_setter(screen,clip_rect);
	SDL_Rect rect = rect_;
	SDL_BlitSurface(buffer_,NULL,screen,&rect);
	update_rect(rect_);
}

bool effect::expired() const
{
	return images_.animation_finished();
}

}

manager::manager(display& screen) : old(disp)
{
	disp = &screen;
}

manager::~manager()
{
	haloes.clear();
	disp = old;
}

halo_hider::halo_hider() : old(hide_halo)
{
	render();
	hide_halo = true;
}

halo_hider::~halo_hider()
{
	hide_halo = old;
	unrender();
}

int add(int x, int y, const std::string& image, const gamemap::location& loc,
		ORIENTATION orientation,  bool infinite)
{
	const int id = halo_id++;
	animated<std::string>::anim_description image_vector;
	std::vector<std::string> items = utils::split(image);
	std::vector<std::string>::const_iterator itor = items.begin();
	for(; itor != items.end(); ++itor) {
		const std::vector<std::string>& items = utils::split(*itor, ':');
		std::string str;
		int time;

		if(items.size() > 1) {
			str = items.front();
			time = atoi(items.back().c_str());
		} else {
			str = *itor;
			time = 100;
		}
		image_vector.push_back(animated<std::string>::frame_description(time,std::string(str)));

	}
	haloes.insert(std::pair<int,effect>(id,effect(x,y,image_vector,loc,orientation,infinite)));
	return id;
}

void set_location(int handle, int x, int y)
{
	const std::map<int,effect>::iterator itor = haloes.find(handle);
	if(itor != haloes.end()) {
		itor->second.set_location(x,y);
	}
}

void remove(int handle)
{
	haloes.erase(handle);
}

void render()
{
	if(hide_halo || preferences::show_haloes() == false) {
		return;
	}

	for(std::map<int,effect>::iterator i = haloes.begin(); i != haloes.end(); ) {
		if(i->second.expired()) {
			haloes.erase(i++);
		} else {
			i->second.render();
			++i;
		}
	}
}

void unrender()
{
	if(hide_halo || preferences::show_haloes() == false) {
		return;
	}

	for(std::map<int,effect>::reverse_iterator i = haloes.rbegin(); i != haloes.rend(); ++i) {
		i->second.unrender();
	}
}

}
