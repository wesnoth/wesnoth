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
           : widget(), disp_(disp), text_(text), firstOnScreen_(0),
             cursor_(text.size()), buffer_(NULL), focus_(true)
{
	static const SDL_Rect area = disp.screen_area();
	const int height = font::draw_text(NULL,area,font_size,font::NORMAL_COLOUR,"ABCD",0,0).h;
	const SDL_Rect starting_rect = {0,0,width,height};
	set_location(starting_rect);
}

const std::string& textbox::text() const
{
	return text_;
}

void textbox::set_text(std::string text)
{
	text_ = text;
	cursor_ = text_.size();
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

	if(show_cursor) {
		SDL_Rect rect = {location().x + pos, location().y, 1, location().h };
		SDL_Surface* const frame_buffer = disp_.video().getSurface();
		SDL_FillRect(frame_buffer,&rect,SDL_MapRGB(frame_buffer->format,255,255,255));
	}
}

void textbox::draw() const
{
	if(location().x == 0)
		return;

	if(buffer_.get() != NULL) {
		SDL_Rect rect = location();
		SDL_BlitSurface(buffer_,NULL,disp_.video().getSurface(),&rect);
	}

	gui::draw_solid_tinted_rectangle(location().x,location().y,location().w,location().h,0,0,0,
	                          focus_ ? 0.2 : 0.4, disp_.video().getSurface());

	if(cursor_ == 0)
		draw_cursor(0);

	int pos = 1;
	std::string str(1,'x');
	const SDL_Rect clip = disp_.screen_area();

	//draw the text
	for(size_t i = firstOnScreen_; i < text_.size(); ++i) {
		str[0] = text_[i];
		const SDL_Rect area =
		    font::draw_text(NULL,clip,font_size,font::NORMAL_COLOUR,str,0,0,
		                    NULL,false,font::NO_MARKUP);

		//if we can't fit the next character on screen
		if(pos + area.w > location().w) {
			if(cursor_ > i)
				++firstOnScreen_;
			break;
		}

		font::draw_text(&disp_,clip,font_size,font::NORMAL_COLOUR,str,
		                location().x + pos, location().y, NULL, false, font::NO_MARKUP);

		pos += area.w;

		if(cursor_ == i+1)
			draw_cursor(pos-1);
	}

	update_rect(location());
}

void textbox::handle_event(const SDL_Event& event)
{
	if(location().x == 0)
		return;

	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);

	if(event.type != SDL_KEYDOWN || !focus_)
		return;

	const SDL_keysym& key = reinterpret_cast<const SDL_KeyboardEvent&>(event).keysym;
	
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

	if(isgraph(character) || character == ' ') {
		text_.insert(text_.begin()+cursor_,character);
		++cursor_;
	}
}

void textbox::set_position(int x, int y)
{
	SDL_Rect rect = {x,y,location().w,location().h};
	set_location(rect);
	buffer_.assign(get_surface_portion(disp_.video().getSurface(),rect));
}

void textbox::set_width(int w)
{
	SDL_Rect rect = location();
	rect.w = w;
	set_location(rect);
}

void textbox::set_focus(bool new_focus)
{
	focus_ = new_focus;
}

}
