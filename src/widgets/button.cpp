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
#include "../game.hpp"
#include "../font.hpp"
#include "../util.hpp"

namespace gui {

const int font_size = 12;
const int horizontal_padding = 10;
const int vertical_padding = 10;

button::button(display& disp, const std::string& label, button::TYPE type,
               const std::string& button_image_name) :
                          label_(label), display_(&disp),
						  image_(NULL), pressedImage_(NULL),
                          x_(0), y_(0), button_(true),
                          state_(UNINIT), type_(type)
{
	SDL_Surface* button_image =
	       disp.getImage("buttons/button.png",display::UNSCALED);
	SDL_Surface* pressed_image =
	       disp.getImage("buttons/button-pressed.png", display::UNSCALED);
	SDL_Surface* active_image =
	       disp.getImage("buttons/button-active.png", display::UNSCALED);


	if(!button_image_name.empty()) {
		button_image = disp.getImage("buttons/" + button_image_name +
		                             "-button.png", display::UNSCALED);
		pressed_image = disp.getImage("buttons/" + button_image_name +
		                              "-button-pressed.png",display::UNSCALED);
		active_image = disp.getImage("buttons/" + button_image_name +
		                              "-button-active.png",display::UNSCALED);
	}

	if(pressed_image == NULL)
		pressed_image = button_image;

	if(active_image == NULL)
		active_image = button_image;

	if(button_image == NULL)
		throw error();

	textRect_.x = 0;
	textRect_.y = 0;
	textRect_.w = disp.x();
	textRect_.h = disp.y();

	textRect_ = font::draw_text(NULL,textRect_,font_size,
	                            font::NORMAL_COLOUR,label_,0,0);
	const int width = maximum(textRect_.w+horizontal_padding,button_image->w);
	const int height = maximum(textRect_.h+horizontal_padding,button_image->h);

	image_ = scale_surface(button_image,width,height);
	pressedImage_ = scale_surface(pressed_image,width,height);
	activeImage_ = scale_surface(active_image,width,height);
}

button::button(const button& b) : label_(b.label_), display_(b.display_),
                                  image_(NULL), pressedImage_(NULL),
								  x_(b.x_), y_(b.y_), textRect_(b.textRect_),
								  button_(b.button_), state_(b.state_),
                                  type_(b.type_)
{
	image_ = scale_surface(b.image_,b.image_->w,b.image_->h);
	pressedImage_ = scale_surface(b.pressedImage_,b.pressedImage_->w,
	                                              b.pressedImage_->h);
	activeImage_ = scale_surface(b.activeImage_,b.activeImage_->w,
	                                            b.activeImage_->h);
}

button& button::operator=(const button& b)
{
	if(image_ != NULL)
		SDL_FreeSurface(image_);

	if(pressedImage_ != NULL)
		SDL_FreeSurface(pressedImage_);

	label_ = b.label_;
	display_ = b.display_;
	image_ = scale_surface(b.image_,b.image_->w,b.image_->h);
	pressedImage_ = scale_surface(b.pressedImage_,b.pressedImage_->w,
	                                              b.pressedImage_->h);
	activeImage_ = scale_surface(b.activeImage_,b.activeImage_->w,
	                                            b.activeImage_->h);
	x_ = b.x_;
	y_ = b.y_;
	textRect_ = b.textRect_;
	button_ = b.button_;
	state_ = b.state_;
	type_ = b.type_;

	return *this;
}

button::~button()
{
	if(pressedImage_ != NULL)
		SDL_FreeSurface(pressedImage_);

	if(activeImage_ != NULL)
		SDL_FreeSurface(activeImage_);

	if(image_ != NULL)
		SDL_FreeSurface(image_);
}

void button::set_check(bool check)
{
	if(type_ == TYPE_CHECK)
		state_ = check ? PRESSED : NORMAL;
}

bool button::checked() const
{
	return state_ == PRESSED;
}

void button::draw()
{
	SDL_Surface* image = image_;
	int offset = 0;
	switch(state_) {
		case ACTIVE: image = activeImage_;
		             break;
		case PRESSED: image = pressedImage_;
		              offset = 1;
			      break;
		case UNINIT:
		case NORMAL:
		default: break;
	}

	const SDL_Rect clipArea = {0,0,display_->x(),display_->y()};
	const int textx = x_ + image->w/2 - textRect_.w/2 + offset;
	const int texty = y_ + image->h/2 - textRect_.h/2 + offset;

	display_->blit_surface(x_,y_,image);
	font::draw_text(display_,clipArea,font_size,
					font::NORMAL_COLOUR,label_,textx,texty);

	display_->video().flip();
}

bool button::hit(int x, int y) const
{
	if(x > x_ && x < x_ + image_->w &&
	   y > y_ && y < y_ + image_->h) {
		x -= x_;
		y -= y_;
		int row_width = image_->w + is_odd(image_->w);
	
		if(*(reinterpret_cast<short*>(image_->pixels)+y*row_width+x) != 0)
			return true;
	}

	return false;
}

void button::set_x(int val) { x_ = val; }
void button::set_y(int val) { y_ = val; }
void button::set_xy(int valx, int valy) { x_ = valx; y_ = valy; }

int button::width() const
{
	return image_->w;
}

int button::height() const
{
	return image_->h;
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

		switch(state_) {
		case NORMAL:
			if(mouse_state == UP && hit(mousex,mousey)) {
				state_ = PRESSED;
				draw();
				return true;
			}
			break;
		case PRESSED:
			if(mouse_state == UP && hit(mousex,mousey)) {
				state_ = NORMAL;
				draw();
				return true;
			}
			break;
		case UNINIT:
			break;
		case ACTIVE:
			break;
		}
	}

	if(state_ != start_state) {
		draw();
	}

	return false;
}

}
