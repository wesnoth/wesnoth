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

textbox::textbox(display& d, int width, const std::string& text, bool editable)
           : widget(d), text_(string_to_wstring(text)), text_pos_(0),
             cursor_(text_.size()), selstart_(-1), selend_(-1), grabmouse_(false),
	         editable_(editable), show_cursor_(true), show_cursor_at_(0), text_image_(NULL),
			 scrollbar_(d,this),
             uparrow_(d,"",gui::button::TYPE_PRESS,"uparrow-button"),
             downarrow_(d,"",gui::button::TYPE_PRESS,"downarrow-button"),
			 scroll_bottom_(false), wrap_(false)
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
	text_.clear();
	cursor_ = 0;
	cursor_pos_ = 0;
	text_pos_ = 0;
	set_dirty(true);
	update_text_cache(true);
}

void textbox::draw_cursor(int pos, display &disp) const
{
	if(show_cursor_ && editable_) {
		SDL_Rect rect = {location().x + pos, location().y, 1, location().h };
		SDL_Surface* const frame_buffer = disp.video().getSurface();
		SDL_FillRect(frame_buffer,&rect,SDL_MapRGB(frame_buffer->format,255,255,255));
	}
}

void textbox::draw()
{
	if(location().x == 0 || !dirty()) {
		uparrow_.draw();
		downarrow_.draw();
		return;
	}

	bg_restore();

	const bool has_scrollbar = show_scrollbar();
	SDL_Rect loc = location();
	if(has_scrollbar && loc.w > scrollbar_.get_max_width()) {
		scrollbar_.set_dirty();
		loc.w -= scrollbar_.get_max_width();
	}

	gui::draw_solid_tinted_rectangle(loc.x,loc.y,loc.w,loc.h,0,0,0,
	                          focus() ? 0.2 : 0.4, disp().video().getSurface());
	
	SDL_Rect src;

	// Fills the selected area
	if(is_selection()) {
		int x = minimum<int>(char_pos_[selstart_], char_pos_[selend_]) - text_pos_ + loc.x;
		int w = abs(char_pos_[selstart_] - char_pos_[selend_]);

		if(!((x > loc.x + loc.w) || ((x + w) < loc.x))) {
			src.x = maximum<int>(x, loc.x);
			src.y = loc.y;
			src.w = src.x + w > loc.x + loc.w ? loc.x + loc.w - src.x : w;
			src.h = loc.h;

			Uint16 colour = Uint16(SDL_MapRGB(disp().video().getSurface()->format, 160, 0, 0));
			fill_rect_alpha(src,colour,140,disp().video().getSurface());
		}
	}

	if(text_image_ != NULL) {
		src.y = 0;
		src.w = minimum<size_t>(loc.w,text_image_->w);
		src.h = minimum<size_t>(loc.h,text_image_->h);
		src.x = text_pos_;
		SDL_Rect dest = disp().screen_area();
		dest.x = loc.x;
		dest.y = loc.y;

		scrollbar_.enable(has_scrollbar);

		if(has_scrollbar && text_image_->h > 0 && loc.h > uparrow_.height() + downarrow_.height()) {
			SDL_Rect scroll_loc = {loc.x + loc.w,loc.y + uparrow_.height(),
			                       scrollbar_.get_max_width(),loc.h - uparrow_.height() - downarrow_.height()};
			scrollbar_.set_location(scroll_loc);

			uparrow_.set_location(loc.x + loc.w,loc.y);
			downarrow_.set_location(loc.x + loc.w,loc.y + loc.h - downarrow_.height());

			const size_t max_height = scrollbar_.location().h;
			const size_t proportion = (loc.h*100)/text_image_->h;
			const size_t grip_height = maximum<size_t>((max_height*proportion)/100,scrollbar_.get_minimum_grip_height());
			scrollbar_.set_grip_height(grip_height);

			const size_t max_y = text_image_->h - loc.h;

			const size_t max_grip_y = scrollbar_.location().h - grip_height;

			if(scroll_bottom_) {
				scrollbar_.set_grip_position(max_grip_y);
			}

			if(max_grip_y > 0) {
				const size_t grip_y = scrollbar_.get_grip_position();

				uparrow_.hide(grip_y == 0);
				downarrow_.hide(grip_y == max_grip_y);

				src.y = (max_y*(grip_y*100)/max_grip_y)/100;
				std::cerr << "set src.y to " << src.y << "/" << max_grip_y << "\n";
			}

			uparrow_.set_dirty(true);
			downarrow_.set_dirty(true);
			uparrow_.draw();
			downarrow_.draw();

			scrollbar_.redraw();
		} else {
			uparrow_.hide();
			downarrow_.hide();
		}

		scroll_bottom_ = false;

		SDL_BlitSurface(text_image_,&src,disp().video().getSurface(),&dest);
	}

	draw_cursor((cursor_pos_ == 0 ? 0 : cursor_pos_ - 1), disp());

	set_dirty(false);
	update_rect(loc);
}

void textbox::process()
{
	if(show_scrollbar()) {
		if(uparrow_.pressed()) {
			scrollbar_.set_grip_position(scrollbar_.get_grip_position() - scrollbar_.get_grip_height()/5);
			set_dirty(true);
		}

		if(downarrow_.pressed()) {
			scrollbar_.set_grip_position(scrollbar_.get_grip_position() + scrollbar_.get_grip_height()/5);
			set_dirty(true);
		}
	}

	if(editable_) {
		if(focus()) {
			const int ticks = SDL_GetTicks();
			if(ticks > show_cursor_at_+500) {
				show_cursor_ = !show_cursor_;
				show_cursor_at_ = ticks;
				set_dirty();
			}
		} else if(show_cursor_ == true) {
			show_cursor_ = false;
			set_dirty();
		}
	}
	
	draw();
}

void textbox::set_editable(bool value)
{
	editable_ = value;
}

bool textbox::editable() const
{
	return editable_;
}

void textbox::scroll_to_bottom()
{
	scroll_bottom_ = true;
	set_dirty(true);
}

void textbox::set_wrap(bool val)
{
	if(wrap_ != val) {
		wrap_ = val;
		update_text_cache(true);
		set_dirty(true);
	}
}

void textbox::scroll(int pos)
{
	set_dirty(true);
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

		wide_string wrapped_text;
		
		for(wide_string::const_iterator itor = text_.begin(); itor != text_.end(); itor++) {
			wide_string s;
			push_back(s,*itor);
			visible_string.append(wstring_to_string(s));

			if(char(*itor) == '\n') {
				visible_string = "";
			}

			int w = font::line_width(visible_string, font_size);

			if(wrap_ && w >= location().w - scrollbar_.get_max_width()) {
				push_back(wrapped_text,wchar_t('\n'));
				char_pos_.push_back(0);
				visible_string = wstring_to_string(s);
				w = font::line_width(visible_string, font_size);	
			} else {
				char_pos_.push_back(w);
			}

			push_back(wrapped_text,*itor);
		}

		text_size_.x = 0;
		text_size_.y = 0;
		const std::string s = wstring_to_string(wrapped_text);
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
	
	wide_string::iterator itor = text_.begin() + minimum(selstart_, selend_);
	text_.erase(itor, itor + abs(selend_ - selstart_));
	cursor_ = minimum(selstart_, selend_);
	selstart_ = selend_ = -1;
}

void textbox::handle_event(const SDL_Event& event)
{
	bool changed = false;
	
	if(location().x == 0 || editable_ == false) {
		return;
	}

	int mousex, mousey;
	const Uint8 mousebuttons = SDL_GetMouseState(&mousex,&mousey);
	if(!(mousebuttons & SDL_BUTTON(1))) {
		grabmouse_ = false;
	}

	//if we don't have the focus, then see if we gain the focus,
	//otherwise return
	if(focus() == false) {
		if(event.type == SDL_MOUSEMOTION &&
		   mousex >= location().x && mousey >= location().y &&
		   mousex < location().x + location().w && mousey < location().y + location().h) {
			events::focus_handler(this);
		}
		   
		return;
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
	
	if(c == SDLK_BACKSPACE) {
		changed = true;
		if(is_selection()) {
			erase_selection();
		} else if(cursor_ > 0) {
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

	//movement characters may have a "Unicode" field on some platforms, so ignore it.
	if(!(c == SDLK_UP || c == SDLK_DOWN || c == SDLK_LEFT || c == SDLK_RIGHT ||
	   c == SDLK_DELETE || c == SDLK_BACKSPACE)) {
		if(character != 0)
			std::cerr << "Char: " << character << ", c = " << c << "\n";
	
		if(character >= 32 && character != 127) {
			changed = true;
			if(is_selection()) 
				erase_selection();

			text_.insert(text_.begin()+cursor_,character);
			++cursor_;		
		}
	}

	if(is_selection() && (selend_ != cursor_))
		selstart_ = selend_ = -1;

	//since there has been cursor activity, make the cursor appear for
	//at least the next 500ms.
	show_cursor_ = true;
	show_cursor_at_ = SDL_GetTicks();

	update_text_cache(changed);
	set_dirty(true);
	draw();
}

bool textbox::show_scrollbar() const
{
	return text_image_ != NULL && text_image_->h > location().h;
}

} //end namespace gui
