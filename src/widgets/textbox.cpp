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
#include "textbox.hpp"
#include "../font.hpp"
#include "SDL.h"

#include <algorithm>
#include <cctype>

namespace gui {

const int font_size = 16;

textbox::textbox(display& disp, int width, const std::string& text)
           : disp_(disp), text_(text), firstOnScreen_(0),
             cursor_(text.size()), height_(-1), width_(width), x_(-1), y_(-1),
             lastLArrow_(false), lastRArrow_(false),
             lastDelete_(false), lastBackspace_(false), buffer_(NULL)
{
	std::fill(previousKeyState_,
	          previousKeyState_+CHAR_LENGTH,false);
	static const SDL_Rect area = {0,0,1024,768};
	height_ = font::draw_text(NULL,area,font_size,font::NORMAL_COLOUR,
	                          "ABCD",0,0).h;
}

int textbox::height() const
{
	return height_;
}

int textbox::width() const
{
	return width_;
}

const std::string& textbox::text() const
{
	return text_;
}

void textbox::draw_cursor(int pos) const
{
	const bool show_cursor = (SDL_GetTicks()%1000) > 500;
	static const short cursor_colour = 0xFFFF;

	if(show_cursor) {
		short* dst = reinterpret_cast<short*>(
		               disp_.video().getSurface()->pixels) + y_*disp_.x() +
					   x_ + pos;

		for(int i = 0; i != height(); ++i, dst += disp_.x()) {
			*dst = cursor_colour;
		}
	}
}

void textbox::draw() const
{
	if(x_ == -1)
		return;

	if(buffer_.get() != NULL) {
		SDL_Rect rect = { x_, y_, width(), height() };
		SDL_BlitSurface(buffer_,NULL,disp_.video().getSurface(),&rect);
	}

	if(cursor_ == 0)
		draw_cursor(0);

	int pos = 1;
	std::string str(1,'x');
	static const SDL_Rect clip = {0,0,1024,768};

	//draw the text
	for(int i = firstOnScreen_; i < text_.size(); ++i) {
		str[0] = text_[i];
		const SDL_Rect area =
		    font::draw_text(NULL,clip,font_size,font::NORMAL_COLOUR,str,0,0);

		//if we can't fit the next character on screen
		if(pos + area.w > width()) {
			break;
		}

		font::draw_text(&disp_,clip,font_size,font::NORMAL_COLOUR,str,
		                x_ + pos, y_);

		pos += area.w;

		if(cursor_ == i+1)
			draw_cursor(pos-1);
	}

	disp_.video().update(x_,y_,width(),height());
}

void textbox::process()
{
	if(key_[KEY_LEFT] && !lastLArrow_ && cursor_ > 0) {
		--cursor_;
		if(cursor_ < firstOnScreen_)
			--firstOnScreen_;
	}

	if(key_[KEY_RIGHT] && !lastRArrow_ && cursor_ < text_.size()) {
		++cursor_;
	}

	if(key_[KEY_BACKSPACE] && !lastBackspace_ && cursor_ > 0) {
		--cursor_;
		text_.erase(text_.begin()+cursor_);
		if(cursor_ < firstOnScreen_)
			--firstOnScreen_;
	}

	if(key_[KEY_DELETE] && !lastDelete_ && !text_.empty()) {
		if(cursor_ == text_.size()) {
			text_.resize(text_.size()-1);
			--cursor_;
		} else {
			text_.erase(text_.begin()+cursor_);
		}
	}

	lastLArrow_ = key_[KEY_LEFT];
	lastRArrow_ = key_[KEY_RIGHT];
	lastBackspace_ = key_[KEY_BACKSPACE];
	lastDelete_ = key_[KEY_DELETE];

	for(char c = INPUT_CHAR_START; c != INPUT_CHAR_END; ++c) {
		char character = c;
		if(islower(character) && (key_[KEY_LSHIFT] || key_[KEY_RSHIFT])) {
			character = toupper(character);
		}

		const bool val = key_[c];
		if(val && !previousKeyState_[c-INPUT_CHAR_START]) {
			text_.insert(text_.begin()+cursor_,character);
			++cursor_;
		}

		previousKeyState_[c-INPUT_CHAR_START] = val;
	}

	draw();
}

void textbox::set_location(int x, int y)
{
	x_ = x;
	y_ = y;

	SDL_Rect portion;
	portion.x = x_;
	portion.y = y_;
	portion.w = width();
	portion.h = height();
	buffer_.assign(get_surface_portion(disp_.video().getSurface(),portion));
}

}
