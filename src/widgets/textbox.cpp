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
#include "../util.hpp"
#include "../language.hpp"
#include "SDL.h"

#include <algorithm>
#include <cctype>

namespace gui {

const int font_size = 16;

textbox::textbox(display& d, int width, const std::string& text)
           : widget(d), text_(string_to_wstring(text)), text_pos_(0),
             cursor_(text_.size()), selstart_(-1), selend_(-1), grabmouse_(false),
	     show_cursor_(true), text_image_(NULL)
{
	static const SDL_Rect area = d.screen_area();
	const int height = font::draw_text(NULL,area,font_size,font::NORMAL_COLOUR,"ABCD",0,0).h;
	const SDL_Rect starting_rect = {0,0,width,height};
	set_location(starting_rect);
	update_text_cache(true);
}

const std::string textbox::text() const
{
	const std::string &ret = wstring_to_string(text_);
	return ret;
}

void textbox::set_text(std::string text)
{
	text_ = string_to_wstring(text);
	cursor_ = text_.size();
	set_dirty(true);
	update_text_cache(true);
}

void textbox::clear()
{
	text_ = L"";
	cursor_ = 0;
	cursor_pos_ = 0;
	text_pos_ = 0;
	set_dirty(true);
	update_text_cache(true);
}

void textbox::draw_cursor(int pos, display &disp) const
{
	if(show_cursor_) {
		SDL_Rect rect = {location().x + pos, location().y, 1, location().h };
		SDL_Surface* const frame_buffer = disp.video().getSurface();
		SDL_FillRect(frame_buffer,&rect,SDL_MapRGB(frame_buffer->format,255,255,255));
	}
}

void textbox::draw()
{
	if(location().x == 0 || !dirty())
		return;

	bg_restore();

	gui::draw_solid_tinted_rectangle(location().x,location().y,location().w,location().h,0,0,0,
	                          focus() ? 0.2 : 0.4, disp().video().getSurface());

	
	SDL_Rect src;

	// Fills the selected area
	if(is_selection()) {
		int x = minimum<int>(char_pos_[selstart_], char_pos_[selend_]) - text_pos_ + location().x;
		int w = abs(char_pos_[selstart_] - char_pos_[selend_]);

		if(!((x > location().x + location().w) || ((x + w) < location().x))) {
			src.x = maximum<int>(x, location().x);
			src.y = location().y;
			src.w = src.x + w > location().x + location().w ? location().x + location().w - src.x : w;
			src.h = location().h;

			Uint16 colour = Uint16(SDL_MapRGB(disp().video().getSurface()->format,
							  font::YELLOW_COLOUR.r, font::YELLOW_COLOUR.g, font::YELLOW_COLOUR.b));
			SDL_FillRect(disp().video().getSurface(), &src, colour);
		}
	}
			
	src.y = 0;
	src.w = text_size_.w;
	src.h = text_size_.h;
	src.x = text_pos_;
	SDL_Rect dest = disp().screen_area();
	dest.x = location().x;
	dest.y = location().y;
	SDL_BlitSurface(text_image_,&src,disp().video().getSurface(),&dest);

	draw_cursor((cursor_pos_ == 0 ? 0 : cursor_pos_ - 1), disp());

	set_dirty(false);
	update_rect(location());
}

void textbox::process()
{
	//Blink the cursor
	bool old_cursor = show_cursor_;
	show_cursor_ = (SDL_GetTicks()%1000) > 500;
	if (old_cursor != show_cursor_)
		set_dirty(true);
	
	draw();
}

void textbox::update_text_cache(bool changed)
{
	if(changed) {
		char_pos_.clear();
		char_pos_.push_back(0);

		// Re-calculate the position of each glyph. We approximate this by asking the
		// width of each substring, but this is a flawed assumption which won't work with
		// some more complex scripts (that is, RTL languages). This part of the work should
		// actually be done by the font-rendering system.
		std::string visible_string;
		
		for(std::wstring::const_iterator itor = text_.begin(); itor != text_.end(); itor++) {
			std::wstring s;
			push_back(s,*itor);
			visible_string.append(wstring_to_string(s));
			const int w = font::line_width(visible_string, font_size);		
		
			char_pos_.push_back(w);
		}

		text_size_.x = 0;
		text_size_.y = 0;
		const std::string s = wstring_to_string(text_);
		text_size_.w = font::line_width(s, font_size);
		text_size_.h = location().h;

		text_image_.assign(font::get_rendered_text(s, font_size, font::NORMAL_COLOUR));		
	}

	int cursor_x = char_pos_[cursor_];

	if(cursor_x - text_pos_ > location().w) {
		text_pos_ = cursor_x - location().w;
	} else if(cursor_x - text_pos_ < 0) {
		text_pos_ = cursor_x;
	}
	cursor_pos_ = cursor_x - text_pos_;
}

bool textbox::is_selection() 
{
	return (selstart_ != -1) && (selend_ != -1) && (selstart_ != selend_);
}

void textbox::erase_selection()
{
	if(!is_selection())
		return;
	
	std::wstring::iterator itor = text_.begin() + minimum(selstart_, selend_);
	text_.erase(itor, itor + abs(selend_ - selstart_));
	cursor_ = minimum(selstart_, selend_);
	selstart_ = selend_ = -1;
}

void textbox::handle_event(const SDL_Event& event)
{
	bool changed = false;
	
	if(location().x == 0)
		return;

	int mousex, mousey;
	const Uint8 mousebuttons = SDL_GetMouseState(&mousex,&mousey);
	if(!(mousebuttons & SDL_BUTTON(1))) {
		grabmouse_ = false;
	}

	if( (grabmouse_ && (event.type == SDL_MOUSEMOTION)) ||  (
		    event.type == SDL_MOUSEBUTTONDOWN  && (mousebuttons & SDL_BUTTON(1))  && ! 
		   (mousex < location().x || mousex > location().x + location().w ||
		    mousey < location().y || mousey > location().y + location().h))) {

		const int x = mousex - location().x + text_pos_;
		const int y = mousey - location().y;
		int pos = 0;
		int distance = x;

		for(int i = 1; i < char_pos_.size(); ++i) {
			// Check individually each distance (if, one day, we support
			// RTL languages, char_pos[c] may not be monotonous.)
			if(abs(x - char_pos_[i]) < distance) {
				pos = i;
				distance = abs(x - char_pos_[i]);
			}
		}

		cursor_ = pos;

		if(grabmouse_)
			selend_ = cursor_;
		
		update_text_cache(false);

		if(!grabmouse_ && mousebuttons & SDL_BUTTON(1)) {
			grabmouse_ = true;
			selstart_ = selend_ = cursor_;
		} else if (! (mousebuttons & SDL_BUTTON(1))) {
			grabmouse_ = false;
		}
	}
	
	if(event.type != SDL_KEYDOWN || focus() != true) {
		draw();
		return;
	}

	const SDL_keysym& key = reinterpret_cast<const SDL_KeyboardEvent&>(event).keysym;
	const SDLMod modifiers = SDL_GetModState();
	
	const int c = key.sym;
	
	int old_cursor = cursor_;
	
	if(c == SDLK_LEFT && cursor_ > 0)
		--cursor_;

	if(c == SDLK_RIGHT && cursor_ < text_.size())
		++cursor_;

	if(c == SDLK_END)
		cursor_ = text_.size();
	
	if(c == SDLK_HOME)
		cursor_ = 0;

	if((old_cursor != cursor_) && (modifiers & KMOD_SHIFT)) {
		if(selstart_ == -1) 
			selstart_ = old_cursor;
		selend_ = cursor_;
	} 
	
	if(c == SDLK_BACKSPACE && cursor_ > 0) {
		changed = true;
		if(is_selection()) {
			erase_selection();
		} else {
			--cursor_;
			text_.erase(text_.begin()+cursor_);
		}
	}

	if(c == SDLK_DELETE && !text_.empty()) {
		changed = true;
		if(is_selection()) {
			erase_selection();
		} else {
			if(cursor_ == text_.size()) {
				text_.resize(text_.size()-1);
				--cursor_;
			} else {
				text_.erase(text_.begin()+cursor_);
			}
		}
	}

	wchar_t character = key.unicode;

	if(character != 0)
		std::cerr << "Char: " << character << ", c = " << c << "\n";
	
	if(/*isgraph(character) || character == ' '*/ character >= 32 && character != 127) {
		changed = true;
		if(is_selection()) 
			erase_selection();

		text_.insert(text_.begin()+cursor_,character);
		++cursor_;		
	}

	if(is_selection() && (selend_ != cursor_))
		selstart_ = selend_ = -1;

	update_text_cache(changed);
	set_dirty(true);
	draw();
}

}
