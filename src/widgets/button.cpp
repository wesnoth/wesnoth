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

const int font_size = 12;
const int horizontal_padding = 6;
const int vertical_padding = 12;

button::button(display& disp, const std::string& label, button::TYPE type,
               std::string button_image_name) :
                          label_(label), display_(&disp),
						  image_(NULL), pressedImage_(NULL), activeImage_(NULL), pressedActiveImage_(NULL),
                          x_(0), y_(0), button_(true),
                          state_(UNINIT), type_(type)
{
	log_scope("button constructor");
	if(button_image_name.empty() && type == TYPE_PRESS) {
		button_image_name = "button";
	} else if(button_image_name.empty() && type == TYPE_CHECK) {
		button_image_name = "checkbox";
	}

	const std::string button_image_file = "buttons/" + button_image_name + ".png";
	scoped_sdl_surface button_image(image::get_image(button_image_file,image::UNSCALED));
	scoped_sdl_surface pressed_image(image::get_image("buttons/" + button_image_name + "-pressed.png", image::UNSCALED));
	scoped_sdl_surface active_image(image::get_image("buttons/" + button_image_name + "-active.png", image::UNSCALED));
	scoped_sdl_surface pressed_active_image(image::get_image("buttons/" + button_image_name + "-active-pressed.png", image::UNSCALED));

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

	h_ = maximum(textRect_.h+horizontal_padding,button_image->h);

	if(type == TYPE_PRESS) {
		w_ = maximum(textRect_.w+horizontal_padding,button_image->w);

		image_.assign(scale_surface(button_image,w_,h_));
		pressedImage_.assign(scale_surface(pressed_image,w_,h_));
		activeImage_.assign(scale_surface(active_image,w_,h_));
		pressedActiveImage_.assign(scale_surface(pressed_active_image,w_,h_));
	} else {
		w_ = horizontal_padding + textRect_.w + button_image->w;
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

void button::draw()
{
	if(type_ == TYPE_CHECK) {
		restorer_.restore();
		const SDL_Rect area = {x_,y_,w_,h_};
		restorer_ = surface_restorer(&display_->video(),area);
	}

	SDL_Surface* image = image_;
	const int image_w = image_->w;
	const int image_h = image_->h;
	int offset = 0;
	switch(state_) {
		case ACTIVE: image = activeImage_;
		             break;
		case PRESSED: image = pressedImage_;
			          if(type_ == TYPE_PRESS) { offset = 1; }
			          break;
		case PRESSED_ACTIVE: image = pressedActiveImage_;
		                     break;
		case UNINIT:
		case NORMAL:
		default: break;
	}

	const SDL_Rect clipArea = display_->screen_area();
	const int texty = y_ + h_/2 - textRect_.h/2 + offset;
	int textx;

	if(type_ == TYPE_PRESS) {
		textx = x_ + image->w/2 - textRect_.w/2 + offset;
	} else {
		textx = x_ + image_w + horizontal_padding/2;
	}

	display_->blit_surface(x_,y_,image);
	font::draw_text(display_,clipArea,font_size,
					font::BUTTON_COLOUR,label_,textx,texty);

	update_rect(x_,y_,width(),height());
}

bool button::hit(int x, int y) const
{
	if(x > x_ && x < x_ + w_ &&
	   y > y_ && y < y_ + h_) {

		if(type_ == TYPE_CHECK)
			return true;

		x -= x_;
		y -= y_;
		int row_width = image_->w + is_odd(image_->w);

		surface_lock lock(image_);
	
		if(*(lock.pixels()+y*row_width+x) != 0)
			return true;
	}

	return false;
}

void button::set_x(int val) { x_ = val; }
void button::set_y(int val) { y_ = val; }
void button::set_xy(int valx, int valy) { x_ = valx; y_ = valy; }
void button::set_label(std::string val)
{
	label_ = val;
	textRect_ = display_->screen_area();
	textRect_ = font::draw_text(NULL,textRect_,font_size,
	                            font::BUTTON_COLOUR,label_,0,0);
}

int button::width() const
{
	return w_;
}

int button::height() const
{
	return h_;
}

bool button::process(int mousex, int mousey, bool button)
{
	enum MOUSE_STATE { UNCHANGED, UP, DOWN };
	MOUSE_STATE mouse_state = UNCHANGED;
	if(button && !button_)
		mouse_state = DOWN;
	else if(!button && button_)
		mouse_state = UP;

	button_ = button;

	const STATE start_state = state_;

	if(type_ == TYPE_PRESS) {

		switch(state_) {
		case UNINIT:
			state_ = NORMAL;
			break;
		case NORMAL:
			if(hit(mousex,mousey))
				state_ = ACTIVE;
			break;
		case ACTIVE:
			if(mouse_state == DOWN && hit(mousex,mousey))
				state_ = PRESSED;
			else if(!hit(mousex,mousey))
				state_ = NORMAL;
			break;
		case PRESSED:
			if(mouse_state == UP) {
				if(hit(mousex,mousey)) {
					state_ = ACTIVE;
					draw();
					return true;
					} else {
					state_ = NORMAL;
				}
			}
		}
	} else if(type_ == TYPE_CHECK) {

		const bool is_hit = hit(mousex,mousey);
		switch(state_) {
		case NORMAL:
			if(is_hit) {
				state_ = ACTIVE;
				draw();
				return true;
			}

			break;

		case PRESSED:
			if(is_hit) {
				state_ = PRESSED_ACTIVE;
				draw();
				return true;
			}

			break;

		case UNINIT:
			break;
		case ACTIVE:
			if(!is_hit) {
				state_ = NORMAL;
				draw();
				return true;
			} else if(mouse_state == UP) {
				state_ = PRESSED_ACTIVE;
				draw();
				return true;
			}

			break;

		case PRESSED_ACTIVE:
			if(!is_hit) {
				state_ = PRESSED;
				draw();
				return true;
			} else if(mouse_state == UP) {
				state_ = ACTIVE;
				draw();
				return true;
			}

			break;
		}
	}

	if(state_ != start_state) {
		draw();
	}

	return false;
}

}
