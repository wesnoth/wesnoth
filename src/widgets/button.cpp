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

#include "button.hpp"
#include "../display.hpp"
#include "../game.hpp"
#include "../font.hpp"
#include "../image.hpp"
#include "../log.hpp"
#include "../util.hpp"
#include "../video.hpp"

namespace gui {

const int font_size = font::SIZE_SMALL;
const int horizontal_padding = font::SIZE_SMALL;
const int checkbox_horizontal_padding = font::SIZE_SMALL / 2;
const int vertical_padding = font::SIZE_SMALL / 2;

button::button(display& disp, const std::string& label, button::TYPE type,
               std::string button_image_name, SPACE_CONSUMPTION spacing) :
                          widget(disp), label_(label), display_(&disp),
						  image_(NULL), pressedImage_(NULL), activeImage_(NULL), pressedActiveImage_(NULL),
                          button_(true), state_(NORMAL), type_(type), enabled_(true), pressed_(false)
{
	set_label(label);

	if(button_image_name.empty() && type == TYPE_PRESS) {
		button_image_name = "button";
	} else if(button_image_name.empty() && type == TYPE_CHECK) {
		button_image_name = "checkbox";
	}

	const std::string button_image_file = "buttons/" + button_image_name + ".png";
	surface button_image(image::get_image(button_image_file,image::UNSCALED));
	surface pressed_image(image::get_image("buttons/" + button_image_name + "-pressed.png", image::UNSCALED));
	surface active_image(image::get_image("buttons/" + button_image_name + "-active.png", image::UNSCALED));
	surface pressed_active_image(image::get_image("buttons/" + button_image_name + "-active-pressed.png", image::UNSCALED));

	if(pressed_image == NULL) {
		pressed_image.assign(image::get_image(button_image_file,image::UNSCALED));
	}

	if(active_image == NULL) {
		active_image.assign(image::get_image(button_image_file,image::UNSCALED));
	}

	if(pressed_active_image == NULL) {
		pressed_active_image.assign(image::get_image(button_image_file,image::UNSCALED));
	}

	if(button_image == NULL) {
		std::cerr << "could not find button image: '" << button_image_file << "'\n";
		throw error();
	}

	textRect_.x = 0;
	textRect_.y = 0;
	textRect_.w = disp.x();
	textRect_.h = disp.y();

	textRect_ = font::draw_text(NULL,textRect_,font_size,
	                            font::BUTTON_COLOUR,label_,0,0);

	set_height(maximum(textRect_.h+vertical_padding,button_image->h));

	if(type == TYPE_PRESS) {
		if(spacing == MINIMUM_SPACE) {
			set_width(textRect_.w + horizontal_padding);
		} else {
			set_width(maximum(textRect_.w+horizontal_padding,button_image->w));
		}

		image_.assign(scale_surface(button_image,location().w,location().h));
		pressedImage_.assign(scale_surface(pressed_image,location().w,location().h));
		activeImage_.assign(scale_surface(active_image,location().w,location().h));
		pressedActiveImage_.assign(scale_surface(pressed_active_image,location().w,location().h));

	} else {
		set_width(checkbox_horizontal_padding + textRect_.w + button_image->w);
		image_.assign(scale_surface(button_image,button_image->w,button_image->h));
		pressedImage_.assign(scale_surface(pressed_image,button_image->w,button_image->h));
		activeImage_.assign(scale_surface(active_image,button_image->w,button_image->h));
		pressedActiveImage_.assign(scale_surface(pressed_active_image,button_image->w,button_image->h));
	}
}

void button::set_check(bool check)
{
	if(type_ == TYPE_CHECK)
		state_ = check ? PRESSED : NORMAL;
}

bool button::checked() const
{
	return state_ == PRESSED || state_ == PRESSED_ACTIVE;
}

void button::enable(bool new_val)
{
	if(enabled_ != new_val) {
		enabled_ = new_val;
		set_dirty(true);
	}
}

bool button::enabled() const
{
	return enabled_;
}

void button::draw()
{
	if(location().x == 0 && location().y == 0 || hidden() || !dirty()) {
		return;
	}

	if(type_ == TYPE_CHECK) {
		bg_restore();
	}

	surface image = image_;
	const int image_w = image_->w;
	
	int offset = 0;
	switch(state_) {
		case ACTIVE: image = activeImage_;
		             break;
		case PRESSED: image = pressedImage_;
			          if(type_ == TYPE_PRESS) { offset = 1; }
			          break;
		case PRESSED_ACTIVE: image = pressedActiveImage_;
		                     break;
		case NORMAL:
		default: break;
	}

	const SDL_Rect clipArea = display_->screen_area();
	const int texty = location().y + location().h/2 - textRect_.h/2 + offset;
	int textx;

	if(type_ == TYPE_PRESS) {
		textx = location().x + image->w/2 - textRect_.w/2 + offset;
	} else {
		textx = location().x + image_w + checkbox_horizontal_padding/2;
	}

	surface greyed_image(NULL);
	if(!enabled_) {
		greyed_image.assign(greyscale_image(image));
		image = greyed_image;
	}

	display_->blit_surface(location().x,location().y,image);
	font::draw_text(display_,clipArea,font_size,
					font::BUTTON_COLOUR,label_,textx,texty);

	update_rect(location());

	set_dirty(false);
}

bool button::hit(int x, int y) const
{
	return point_in_rect(x,y,location());
}

namespace {
	bool not_image(const std::string& str) { return str != "" && str[0] != '&'; }
}

void button::set_label(const std::string& val)
{
	label_ = val;

	//if we have a list of items, use the first one that isn't an image
	if(std::find(label_.begin(),label_.end(),',') != label_.end()) {
		const std::vector<std::string>& items = config::split(label_);
		const std::vector<std::string>::const_iterator i = std::find_if(items.begin(),items.end(),not_image);
		if(i != items.end()) {
			label_ = *i;
		}
	}

	textRect_ = display_->screen_area();
	textRect_ = font::draw_text(NULL,textRect_,font_size,
	                            font::BUTTON_COLOUR,label_,0,0);

	set_dirty(true);
}

void button::mouse_motion(const SDL_MouseMotionEvent& event)
{
	const bool is_hit = hit(event.x,event.y);

	if(state_ == NORMAL && is_hit) {
		state_ = ACTIVE;
	} else if(state_ == PRESSED && is_hit && type_ == TYPE_CHECK) {
		state_ = PRESSED_ACTIVE;
	} else if(state_ == ACTIVE && !is_hit) {
		state_ = NORMAL;
	} else if(state_ == PRESSED_ACTIVE && !is_hit) {
		state_ = PRESSED;
	} else if(state_ == PRESSED && !is_hit && type_ == TYPE_PRESS) {
		state_ = NORMAL;
	}
}

void button::mouse_down(const SDL_MouseButtonEvent& event)
{
	const bool is_hit = hit(event.x,event.y);
	if(is_hit && type_ == TYPE_PRESS) {
		state_ = PRESSED;
	}
}

void button::mouse_up(const SDL_MouseButtonEvent& event)
{
	const bool is_hit = hit(event.x,event.y);
	if(is_hit && type_ == TYPE_CHECK) {
		if(state_ == ACTIVE) {
			state_ = PRESSED_ACTIVE;
		} else {
			state_ = ACTIVE;
		}

		pressed_ = true;
	} else if(is_hit && type_ == TYPE_PRESS && state_ == PRESSED) {
		state_ = ACTIVE;
		pressed_ = true;
	}
}

void button::handle_event(const SDL_Event& event)
{
	if(hidden() || !enabled_) {
		return;
	}

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
	}

	if(start_state != state_) {
		set_dirty(true);
	}
}

bool button::process(int mousex, int mousey, bool button)
{
	return pressed();
}

bool button::pressed()
{
	const bool res = pressed_;
	pressed_ = false;
	return res;
}

}
