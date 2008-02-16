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

#include "widgets/button.hpp"
#include "game_config.hpp"
#include "font.hpp"
#include "marked-up_text.hpp"
#include "image.hpp"
#include "log.hpp"
#include "sound.hpp"
#include "util.hpp"
#include "video.hpp"
#include "wml_separators.hpp"
#include "serialization/string_utils.hpp"

#define ERR_DP LOG_STREAM(err, display)

namespace gui {

const int font_size = font::SIZE_SMALL;
const int horizontal_padding = font::SIZE_SMALL;
const int checkbox_horizontal_padding = font::SIZE_SMALL / 2;
const int vertical_padding = font::SIZE_SMALL / 2;

button::button(CVideo& video, const std::string& label, button::TYPE type,
               std::string button_image_name, SPACE_CONSUMPTION spacing, const bool auto_join)
	: widget(video, auto_join), type_(type), label_(label),
	  image_(NULL), pressedImage_(NULL), activeImage_(NULL), pressedActiveImage_(NULL),
	  button_(true), state_(NORMAL), pressed_(false),
	  spacing_(spacing), base_height_(0), base_width_(0)
{
	if(button_image_name.empty() && type == TYPE_PRESS) {
		button_image_name = "button";
	} else if(button_image_name.empty() && type == TYPE_CHECK) {
		button_image_name = "checkbox";
	}

	const std::string button_image_file = "buttons/" + button_image_name + ".png";
	surface button_image(image::get_image(button_image_file));
	surface pressed_image(image::get_image("buttons/" + button_image_name + "-pressed.png"));
	surface active_image(image::get_image("buttons/" + button_image_name + "-active.png"));
	surface pressed_active_image;

	if (pressed_image.null())
		pressed_image.assign(button_image);

	if (active_image.null())
		active_image.assign(button_image);

	if (type == TYPE_CHECK) {
		pressed_active_image.assign(image::get_image("buttons/" + button_image_name + "-active-pressed.png"));
		if (pressed_active_image.null())
			pressed_active_image.assign(pressed_image);
	}

	if (button_image.null()) {
		ERR_DP << "error initializing button!\n";
		throw error();
	}

	base_height_ = button_image->h;
	base_width_ = button_image->w;

	if (type_ != TYPE_IMAGE){
		set_label(label);
	}

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

	if (type_ == TYPE_IMAGE){
		calculate_size();
	}
}

void button::calculate_size()
{
	if (type_ == TYPE_IMAGE){
		SDL_Rect loc_image = location();
		loc_image.h = image_->h;
		loc_image.w = image_->w;
		set_location(loc_image);
		return;
	}
	SDL_Rect const &loc = location();
	bool change_size = loc.h == 0 || loc.w == 0;

	if (!change_size) {
		unsigned w = loc.w - (type_ == TYPE_PRESS ? horizontal_padding : checkbox_horizontal_padding + base_width_);
		if (type_ != TYPE_IMAGE){
			label_ = font::make_text_ellipsis(label_, font_size, w, false);
		}
	}

	if (type_ != TYPE_IMAGE){
		textRect_ = font::draw_text(NULL, screen_area(), font_size,
		                            font::BUTTON_COLOUR, label_, 0, 0);
	}

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
		if(label_.empty()) {
			set_width(base_width_);
		} else {
			set_width(checkbox_horizontal_padding + textRect_.w + base_width_);
		}
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
	if(new_val != enabled())
	{
		pressed_ = false;
		// check buttons should keep their state
		if(type_ != TYPE_CHECK) {
			state_ = NORMAL;
		}
		widget::enable(new_val);
	}
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

	SDL_Rect const &loc = location();
	SDL_Rect clipArea = loc;
	const int texty = loc.y + loc.h / 2 - textRect_.h / 2 + offset;
	int textx;

	if (type_ != TYPE_CHECK)
		textx = loc.x + image->w / 2 - textRect_.w / 2 + offset;
	else {
		clipArea.w += image_w + checkbox_horizontal_padding;
		textx = loc.x + image_w + checkbox_horizontal_padding / 2;
	}

	SDL_Color button_colour = font::BUTTON_COLOUR;

	if (!enabled()) {
		static const Uint32 disabled_btn_color = 0xAAAAAA;
		static const double disabled_btn_adjust = 0.18;
		image = blend_surface(greyscale_image(image), disabled_btn_adjust, disabled_btn_color);
		button_colour = font::GRAY_COLOUR;
	}

	video().blit_surface(loc.x, loc.y, image);
	if (type_ != TYPE_IMAGE){
		clipArea.x += offset;
		clipArea.y += offset;
		clipArea.w -= 2*offset;
		clipArea.h -= 2*offset;
		font::draw_text(&video(), clipArea, font_size, button_colour, label_, textx, texty);
	}

	update_rect(loc);
}

bool button::hit(int x, int y) const
{
	return point_in_rect(x,y,location());
}

static bool not_image(const std::string& str) { return !str.empty() && str[0] != IMAGE_PREFIX; }

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
		else if (type_ != TYPE_CHECK && type_ != TYPE_IMAGE || state_ != PRESSED)
			state_ = NORMAL;
	}
}

void button::mouse_down(SDL_MouseButtonEvent const &event)
{
	if (hit(event.x, event.y) && event.button == SDL_BUTTON_LEFT && type_ != TYPE_CHECK){
		state_ = PRESSED;
		sound::play_UI_sound(game_config::sounds::button_press);
	}
}

void button::release(){
	state_ = NORMAL;
	draw_contents();
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
		sound::play_UI_sound(game_config::sounds::checkbox_release);
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
	case TYPE_IMAGE:
		pressed_ = true;
		break;
	}
}

void button::handle_event(const SDL_Event& event)
{
	if (hidden() || !enabled())
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
