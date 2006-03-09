/* $Id: button.cpp 7396 2005-07-02 21:37:20Z ott $ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "widgets/image_button.hpp"
#include "font.hpp"
#include "image.hpp"
#include "log.hpp"
#include "util.hpp"
#include "video.hpp"
#include "wml_separators.hpp"
#include "serialization/string_utils.hpp"

namespace gui {

const int font_size = font::SIZE_SMALL;
const int horizontal_padding = font::SIZE_SMALL;
const int checkbox_horizontal_padding = font::SIZE_SMALL / 2;
const int vertical_padding = font::SIZE_SMALL / 2;

image_button::image_button(CVideo& video, std::string button_image_name, SPACE_CONSUMPTION spacing)
	: widget(video), image_(NULL), pressedImage_(NULL), activeImage_(NULL), pressedActiveImage_(NULL),
	  button_(true), state_(NORMAL), enabled_(true), pressed_(false),
	  spacing_(spacing), base_height_(0), base_width_(0)
{
	if(button_image_name.empty()) {
		button_image_name = "button";
	}

	const std::string button_image_file = "buttons/" + button_image_name + ".png";
	surface button_image(image::get_image(button_image_file,image::UNSCALED));
	surface pressed_image(image::get_image("buttons/" + button_image_name + "-pressed.png", image::UNSCALED));
	surface active_image(image::get_image("buttons/" + button_image_name + "-active.png", image::UNSCALED));
	surface pressed_active_image;

	if (pressed_image.null())
		pressed_image.assign(button_image);

	if (active_image.null())
		active_image.assign(button_image);

	if (button_image.null())
		throw error();

	base_height_ = button_image->h;
	base_width_ = button_image->w;

	image_.assign(scale_surface(button_image,location().w,location().h));
	pressedImage_.assign(scale_surface(pressed_image,location().w,location().h));
	activeImage_.assign(scale_surface(active_image,location().w,location().h));
}

void image_button::calculate_size()
{
	SDL_Rect const &loc = location();
	bool change_size = loc.h == 0 || loc.w == 0;

	if (!change_size) {
		unsigned w = loc.w - horizontal_padding;
	}

	if (!change_size)
		return;

#ifdef USE_TINY_GUI
	set_height(textRect_.h+vertical_padding);
#else
	set_height(maximum(textRect_.h+vertical_padding,base_height_));
#endif
#ifdef USE_TINY_GUI
	set_width(textRect_.w + horizontal_padding);
#else
	if(spacing_ == MINIMUM_SPACE) {
		set_width(textRect_.w + horizontal_padding);
	} else {
		set_width(maximum(textRect_.w+horizontal_padding,base_width_));
	}
#endif
}

void image_button::enable(bool new_val)
{
	if (enabled_ != new_val) {
		enabled_ = new_val;
		state_ = NORMAL;
		pressed_ = false;
		set_dirty();
	}
}

bool image_button::enabled() const
{
	return enabled_;
}

void image_button::draw_contents()
{
	surface image = image_;
	const int image_w = image_->w;

	int offset = 0;
	switch(state_) {
	case ACTIVE:
		image = activeImage_;
		break;
	case PRESSED:
		image = pressedImage_;
		offset = 1;
		break;
	case PRESSED_ACTIVE:
		image = pressedActiveImage_;
		break;
	default:
		break;
	}

	SDL_Rect const &clipArea = screen_area();
	SDL_Rect const &loc = location();
	const int texty = loc.y + loc.h / 2 - textRect_.h / 2 + offset;
	int textx;

	textx = loc.x + image_w + checkbox_horizontal_padding / 2;

	SDL_Color button_colour = font::BUTTON_COLOUR;

	if (!enabled_) {
		image = greyscale_image(image);
		button_colour = font::STONED_COLOUR;
	}

	video().blit_surface(loc.x, loc.y, image);

	update_rect(loc);
}

bool image_button::hit(int x, int y) const
{
	return point_in_rect(x,y,location());
}

namespace {
	bool not_image(const std::string& str) { return !str.empty() && str[0] != IMAGE_PREFIX; }
}

void image_button::mouse_motion(SDL_MouseMotionEvent const &event)
{
	if (hit(event.x, event.y)) {
		// the cursor is over the widget
		if (state_ == NORMAL)
			state_ = ACTIVE;
	} else {
		// the cursor is not over the widget
		if (state_ != PRESSED)
			state_ = NORMAL;
	}
}

void image_button::mouse_down(SDL_MouseButtonEvent const &event)
{
	if (hit(event.x, event.y) && event.button == SDL_BUTTON_LEFT)
		state_ = PRESSED;
}

void image_button::mouse_up(SDL_MouseButtonEvent const &event)
{
	if (!(hit(event.x, event.y) && event.button == SDL_BUTTON_LEFT))
		return;
	// the user has stopped pressing the mouse left button while on the widget
	if (state_ == PRESSED) {
		state_ = ACTIVE;
		pressed_ = true;
	}
}

void image_button::handle_event(const SDL_Event& event)
{
	if (hidden() || !enabled_)
		return;

	STATE start_state = state_;

	switch(event.type) {
	case SDL_MOUSEBUTTONDOWN:
		mouse_down(event.button);
		break;
	case SDL_MOUSEBUTTONUP:
		mouse_up(event.button);
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

bool image_button::pressed()
{
	return state_ == PRESSED || state_ == PRESSED_ACTIVE;
}

}
