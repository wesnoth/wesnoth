/* $Id$*/
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "scrollbar.hpp"
#include "../image.hpp"
#include "../video.hpp"

#include <algorithm>
#include <iostream>

namespace {
	const std::string scrollbar_top = "buttons/scrolltop.png";
	const std::string scrollbar_bottom = "buttons/scrollbottom.png";
	const std::string scrollbar_mid = "buttons/scrollmid.png";
	const std::string groove_top = "buttons/scrollgroove-top.png";
	const std::string groove_mid = "buttons/scrollgroove-mid.png";
	const std::string groove_bottom = "buttons/scrollgroove-bottom.png";
	const int sb_width_pad = 2;
}

namespace gui {

scrollbar::scrollbar(display& d)
	: widget(d), highlight_(false),
	  clicked_(false), dragging_(false), grip_position_(0), grip_height_(0),
	  enabled_(false), width_(0), minimum_grip_height_(0)
{
	static const scoped_sdl_surface img(image::get_image(scrollbar_mid, 
										image::UNSCALED));
	
	if (img != NULL) {
		width_ = img->w + sb_width_pad*2;
		// this is a bit rough maybe
		minimum_grip_height_ = 2 * img->h;
		grip_height_ = minimum_grip_height_;
	}
	set_dirty(true);
}

void scrollbar::enable(bool en)
{
	enabled_ = en;
}

bool scrollbar::enabled() const 
{
	return enabled_;
}
		
int scrollbar::get_width() const
{
	if (enabled()) 
		return get_max_width();
	else 
		return 0;
}

int scrollbar::get_max_width() const 
{
	return width_;
}
		

int scrollbar::get_grip_height() const 
{
	return grip_height_;
}

bool scrollbar::set_grip_height(int h) 
{
	if (h < minimum_grip_height_ || h > location().h) {
		return false;
	}

	grip_height_ = h;
	return true;
}

int scrollbar::get_minimum_grip_height() const 
{
	return minimum_grip_height_;
}
	

SDL_Rect scrollbar::scroll_grip_area() const
{
	SDL_Rect res = {location().x, location().y+grip_position_, 
					width_, grip_height_};
	return res;

}

void scrollbar::redraw()
{
	draw();
}

// I'm sure this code is inefficient, but I'm not sure of how to do 
// it more efficiently using scoped_resource 
void scrollbar::draw()
{
	if (!enabled() || !dirty())
		return;
	
	const scoped_sdl_surface mid_img(image::get_image(scrollbar_mid, 
											image::UNSCALED));
	const scoped_sdl_surface bottom_img(image::get_image(scrollbar_bottom,
										image::UNSCALED));
	const scoped_sdl_surface top_img(image::get_image(scrollbar_top,
									 image::UNSCALED));

	const scoped_sdl_surface top_grv(image::get_image(groove_top,
											image::UNSCALED));
	const scoped_sdl_surface mid_grv(image::get_image(groove_mid,
											image::UNSCALED));
	const scoped_sdl_surface bottom_grv(image::get_image(groove_bottom,
											image::UNSCALED));

	if (mid_img == NULL || bottom_img == NULL || top_img == NULL
	 || top_grv == NULL || bottom_grv == NULL || mid_grv == NULL){
		std::cerr << "Failure to load scrollbar image.\n";
		return;
	}

	int mid_height = grip_height_ - top_img->h - bottom_img->h;
	if (mid_height <= 0) {
		// for now, minimum size of the middle piece is 1. This should
		// never really be encountered, and if it is, it's an symptom
		// of a larger problem, I think.
		mid_height = 1;
	}
	const scoped_sdl_surface mid_scaled(scale_surface_blended(mid_img, 
										mid_img->w, mid_height));

	const scoped_sdl_surface highlighted(scale_surface_blended(
						brighten_image(mid_img, 1.5), 
						mid_img->w, mid_height));


	int groove_height = location().h - top_grv->h - bottom_grv->h;
	if (groove_height <= 0) {
		groove_height = 1;
	}
	const scoped_sdl_surface groove_scaled(scale_surface_blended(mid_grv,
											 mid_grv->w, groove_height));

	if (mid_scaled == NULL || groove_scaled == NULL) {
		std::cerr << "Failure during scrollbar image scale.\n";
		return;
	}

	if (grip_height_ >= location().h)
		return;

	SDL_Surface* const screen = disp().video().getSurface();

	bg_restore();

	int xpos = location().x + sb_width_pad;

	// draw scrollbar "groove"
	disp().blit_surface(xpos, location().y, top_grv);
	disp().blit_surface(xpos, location().y + top_grv->h, groove_scaled);
	disp().blit_surface(xpos, location().y + top_grv->h + groove_height,
						bottom_grv);

	// draw scrollbar "grip"
	SDL_Rect scrollbar = scroll_grip_area();
	xpos = scrollbar.x + sb_width_pad;
	disp().blit_surface(xpos, scrollbar.y, top_img); 
	disp().blit_surface(xpos, scrollbar.y + top_img->h,
						highlight_ ? highlighted : mid_scaled);
	disp().blit_surface(xpos, scrollbar.y + top_img->h + mid_height,
						bottom_img);

	set_dirty(false);
	update_rect(location());
}	

bool scrollbar::set_grip_position(int pos) 
{
	if (pos < 0)
		pos = 0;
	if (pos >= location().h - grip_height_) 
		pos = location().h - grip_height_;

	grip_position_ = pos;
	return true;
}

int scrollbar::get_grip_position() const
{
	return grip_position_;
}

void scrollbar::process()
{
	if (!enabled()) 
		return; 

	int mousex, mousey;
	const int mouse_flags = SDL_GetMouseState(&mousex, &mousey);
	const bool button = mouse_flags & SDL_BUTTON_LMASK;
	static int mousey_on_grip = 0;

	SDL_Rect rect = {location().x, location().y, width_, location().h};
	set_location(rect);

	const SDL_Rect& hit_area = scroll_grip_area();
	const bool xrange = mousex > hit_area.x && mousex <= hit_area.x+hit_area.w;
	const bool yrange = mousey > hit_area.y && mousey <= hit_area.y+hit_area.h;

	const bool on = xrange && yrange;

	bool start_dragging = (button && !clicked_ && on);

	if (start_dragging) {
		dragging_ = true;
		mousey_on_grip = mousey - grip_position_;
	}

	if (!button) 
		dragging_ = false;
	
	if (highlight_ != on) {
		highlight_ = on;
		set_dirty(true);
	}

	int new_position = grip_position_;

	if (dragging_) {
		highlight_ = true;
		new_position = mousey - mousey_on_grip;
	}
	else if (button && xrange) {
		int button_position = mousey - location().y;
		if (button_position != grip_position_) 
			new_position = button_position;
	}

	if (new_position < 0)
		new_position = 0;
	if (new_position > location().h - grip_height_)
		new_position = location().h - grip_height_;

	if (new_position != grip_position_) {
		grip_position_ = new_position;
		set_dirty(true);
	}

	clicked_ = button;
	draw();
}
	
}
