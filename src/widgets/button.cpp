/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "../global.hpp"

#include "button.hpp"
#include "../font.hpp"
#include "../image.hpp"
#include "../log.hpp"
#include "../util.hpp"
#include "../video.hpp"
#include "../wml_separators.hpp"
#include "serialization/string_utils.hpp"

namespace gui {

const int font_size = font::SIZE_SMALL;
const int horizontal_padding = font::SIZE_SMALL;
const int checkbox_horizontal_padding = font::SIZE_SMALL / 2;
const int vertical_padding = font::SIZE_SMALL / 2;

button::button(CVideo& video, const std::string& label, button::TYPE type,
               std::string button_image_name, SPACE_CONSUMPTION spacing)
	: widget(video), label_(label),
	  image_(NULL), pressedImage_(NULL), activeImage_(NULL), pressedActiveImage_(NULL),
	  button_(true), state_(NORMAL), type_(type), enabled_(true), pressed_(false),
	  spacing_(spacing), base_height_(0), base_width_(0)
{
	if(button_image_name.empty() && type == TYPE_PRESS) {
		button_image_name = "button";
	} else if(button_image_name.empty() && type == TYPE_CHECK) {
		button_image_name = "checkbox";
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

	if (type == TYPE_CHECK) {
		pressed_active_image.assign(image::get_image("buttons/" + button_image_name + "-active-pressed.png",
		                                             image::UNSCALED));
		if (pressed_active_image.null())
			pressed_active_image.assign(pressed_image);
	}

	if (button_image.null())
		throw error();

	base_height_ = button_image->h;
	base_width_ = button_image->w;

	set_label(label);

	if(type == TYPE_PRESS) {
		image_.assign(scale_surface(button_image,location().w,location().h));
		pressedImage_.assign(scale_surface(pressed_image,location().w,location().h));
		activeImage_.assign(scale_surface(active_image,location().w,location().h));
	} else {
		image_.assign(scale_surface(button_image,button_image->w,button_image->h));
		pressedImage_.assign(scale_surface(pressed_image,button_image->w,button_image->h));
		activeImage_.assign(scale_surface(active_image,button_image->w,button_image->h));
		if (type == TYPE_CHECK)
			pressedActiveImage_.assign(scale_surface(pressed_active_image, button_image->w, button_image->h));
	}
}

void button::calculate_size()
{
	SDL_Rect const &loc = location();
	bool change_size = loc.h == 0 || loc.w == 0;

	if (!change_size) {
		unsigned w = loc.w - (type_ == TYPE_PRESS ? horizontal_padding :
		                                            checkbox_horizontal_padding + base_width_);
		label_ = font::make_text_ellipsis(label_, font_size, w);
	}

	textRect_ = font::draw_text(NULL, screen_area(), font_size,
	                            font::BUTTON_COLOUR, label_, 0, 0);

	if (!change_size)
		return;

#ifdef USE_TINY_GUI
	set_height(textRect_.h+vertical_padding);
#else
	set_height(maximum(textRect_.h+vertical_padding,base_height_));
#endif
	if(type_ == TYPE_PRESS) {
#ifdef USE_TINY_GUI
		set_width(textRect_.w + horizontal_padding);
#else
		if(spacing_ == MINIMUM_SPACE) {
			set_width(textRect_.w + horizontal_padding);
		} else {
			set_width(maximum(textRect_.w+horizontal_padding,base_width_));
		}
#endif
	} else {
		set_width(checkbox_horizontal_padding + textRect_.w + base_width_);
	}
}

void button::set_check(bool check)
{
	if (type_ != TYPE_CHECK)
		return;
	STATE new_state = check ? PRESSED : NORMAL;
	if (state_ != new_state) {
		state_ = new_state;
		set_dirty();
	}
}

bool button::checked() const
{
	return state_ == PRESSED || state_ == PRESSED_ACTIVE;
}

void button::enable(bool new_val)
{
	if (enabled_ != new_val) {
		enabled_ = new_val;
		state_ = NORMAL;
		pressed_ = false;
		set_dirty();
	}
}

bool button::enabled() const
{
	return enabled_;
}

void button::draw_contents()
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
		if (type_ == TYPE_PRESS)
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

	if (type_ != TYPE_CHECK)
		textx = loc.x + image->w / 2 - textRect_.w / 2 + offset;
	else
		textx = loc.x + image_w + checkbox_horizontal_padding / 2;

	SDL_Color button_colour = font::BUTTON_COLOUR;

	if (!enabled_) {
		image = greyscale_image(image);
		button_colour = font::STONED_COLOUR;
	}

	video().blit_surface(loc.x, loc.y, image);
	font::draw_text(&video(), clipArea, font_size, button_colour, label_, textx, texty);

	update_rect(loc);
}

bool button::hit(int x, int y) const
{
	return point_in_rect(x,y,location());
}

namespace {
	bool not_image(const std::string& str) { return !str.empty() && str[0] != IMAGE_PREFIX; }
}

void button::set_label(const std::string& val)
{
	label_ = val;

	//if we have a list of items, use the first one that isn't an image
	if (std::find(label_.begin(), label_.end(), COLUMN_SEPARATOR) != label_.end()) {
		const std::vector<std::string>& items = utils::split(label_, COLUMN_SEPARATOR);
		const std::vector<std::string>::const_iterator i = std::find_if(items.begin(),items.end(),not_image);
		if(i != items.end()) {
			label_ = *i;
		}
	}

	calculate_size();

	set_dirty(true);
}

void button::mouse_motion(SDL_MouseMotionEvent const &event)
{
	if (hit(event.x, event.y)) {
		// the cursor is over the widget
		if (state_ == NORMAL)
			state_ = ACTIVE;
		else if (state_ == PRESSED && type_ == TYPE_CHECK)
			state_ = PRESSED_ACTIVE;
	} else {
		// the cursor is not over the widget
		if (state_ == PRESSED_ACTIVE)
			state_ = PRESSED;
		else if (type_ != TYPE_CHECK || state_ != PRESSED)
			state_ = NORMAL;
	}
}

void button::mouse_down(SDL_MouseButtonEvent const &event)
{
	if (hit(event.x, event.y) && event.button == SDL_BUTTON_LEFT && type_ != TYPE_CHECK)
		state_ = PRESSED;
}

void button::mouse_up(SDL_MouseButtonEvent const &event)
{
	if (!(hit(event.x, event.y) && event.button == SDL_BUTTON_LEFT))
		return;
	// the user has stopped pressing the mouse left button while on the widget
	switch (type_) {
	case TYPE_CHECK:
		state_ = state_ == ACTIVE ? PRESSED_ACTIVE : ACTIVE;
		pressed_ = true;
		break;
	case TYPE_PRESS:
		if (state_ == PRESSED) {
			state_ = ACTIVE;
			pressed_ = true;
		}
		break;
	case TYPE_TURBO:
		state_ = ACTIVE;
		break;
	}
}

void button::handle_event(const SDL_Event& event)
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

bool button::pressed()
{
	if (type_ != TYPE_TURBO) {
		const bool res = pressed_;
		pressed_ = false;
		return res;
	} else
		return state_ == PRESSED || state_ == PRESSED_ACTIVE;
}

}
