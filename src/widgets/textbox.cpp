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
#include "../show_dialog.hpp"
#include "../video.hpp"
#include "SDL.h"

#include <algorithm>
#include <cctype>

namespace gui {

const int font_size = 16;

textbox::textbox(display& disp, int width, const std::string& text)
           : disp_(disp), text_(text), firstOnScreen_(0),
             cursor_(text.size()), height_(-1), width_(width),
             buffer_(NULL), x_(-1), y_(-1), focus_(true)
{
	std::fill(previousKeyState_,
	          previousKeyState_+CHAR_LENGTH,true);
	static const SDL_Rect area = disp.screen_area();
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

void textbox::clear()
{
	text_ = "";
	cursor_ = 0;
	firstOnScreen_ = 0;
}

void textbox::draw_cursor(int pos) const
{
	const bool show_cursor = (SDL_GetTicks()%1000) > 500;
	static const short cursor_colour = 0xFFFF;

	if(show_cursor) {
		surface_lock lock(disp_.video().getSurface());
		short* dst = lock.pixels() + y_*disp_.x() + x_ + pos;

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

	gui::draw_solid_tinted_rectangle(x_,y_,width(),height(),0,0,0,
	                          focus_ ? 0.2 : 0.4, disp_.video().getSurface());

	if(cursor_ == 0)
		draw_cursor(0);

	int pos = 1;
	std::string str(1,'x');
	static const SDL_Rect clip = disp_.screen_area();

	//draw the text
	for(size_t i = firstOnScreen_; i < text_.size(); ++i) {
		str[0] = text_[i];
		const SDL_Rect area =
		    font::draw_text(NULL,clip,font_size,font::NORMAL_COLOUR,str,0,0,
		                    NULL,false,font::NO_MARKUP);

		//if we can't fit the next character on screen
		if(pos + area.w > width()) {
			break;
		}

		font::draw_text(&disp_,clip,font_size,font::NORMAL_COLOUR,str,
		                x_ + pos, y_, NULL, false, font::NO_MARKUP);

		pos += area.w;

		if(cursor_ == i+1)
			draw_cursor(pos-1);
	}

	update_rect(x_,y_,width(),height());
}

void textbox::handle_event(const SDL_Event& event)
{
	if(event.type != SDL_KEYDOWN || !focus_)
		return;

	const SDL_keysym& key
	           = reinterpret_cast<const SDL_KeyboardEvent&>(event).keysym;
	
	const int c = key.sym;

	if(c == SDLK_LEFT && cursor_ > 0) {
		--cursor_;
		if(cursor_ < firstOnScreen_)
			--firstOnScreen_;
	}

	if(c == SDLK_RIGHT && cursor_ < text_.size()) {
		++cursor_;
	}

	if(c == SDLK_BACKSPACE && cursor_ > 0) {
		--cursor_;
		text_.erase(text_.begin()+cursor_);
		if(cursor_ < firstOnScreen_)
			--firstOnScreen_;
	}

	if(c == SDLK_DELETE && !text_.empty()) {
		if(cursor_ == text_.size()) {
			text_.resize(text_.size()-1);
			--cursor_;
		} else {
			text_.erase(text_.begin()+cursor_);
		}
	}

	const char character = static_cast<char>(key.unicode);

	if(character >= INPUT_CHAR_START && character < INPUT_CHAR_END) {
		text_.insert(text_.begin()+cursor_,character);
		++cursor_;
	}
}

void textbox::process()
{
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

void textbox::set_width(int w)
{
	width_ = w;
	set_location(x_, y_);
}

void textbox::set_focus(bool new_focus)
{
	focus_ = new_focus;
}

}
