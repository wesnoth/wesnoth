/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "widgets/textbox.hpp"

#include "desktop/clipboard.hpp"
#include "font/sdl_ttf.hpp"
#include "log.hpp"
#include "sdl/rect.hpp"
#include "serialization/string_utils.hpp"
#include "video.hpp"

static lg::log_domain log_display("display");
#define WRN_DP LOG_STREAM(warn, log_display)
#define DBG_G LOG_STREAM(debug, lg::general())

namespace gui {

textbox::textbox(CVideo &video, int width, const std::string& text, bool editable, size_t max_size, int font_size, double alpha, double alpha_focus, const bool auto_join)
	   : scrollarea(video, auto_join), max_size_(max_size), font_size_(font_size), text_(unicode_cast<ucs4::string>(text)),
	     cursor_(text_.size()), selstart_(-1), selend_(-1),
	     grabmouse_(false), text_pos_(0), editable_(editable),
	     show_cursor_(true), show_cursor_at_(0), text_image_(nullptr),
	     wrap_(false), line_height_(0), yscroll_(0), alpha_(alpha),
	     alpha_focus_(alpha_focus),
	     edit_target_(nullptr)
		,listening_(false)
{
	// static const SDL_Rect area = video.screen_area();
	// const int height = font::draw_text(nullptr,area,font_size,font::NORMAL_COLOR,"ABCD",0,0).h;
	set_measurements(width, font::get_max_height(font_size_));
	set_scroll_rate(font::get_max_height(font_size_) / 2);
	update_text_cache(true);
}

textbox::~textbox()
{
}

void textbox::update_location(SDL_Rect const &rect)
{
	scrollarea::update_location(rect);
	update_text_cache(true);
	set_dirty(true);
}

void textbox::set_inner_location(SDL_Rect const &rect)
{
	bg_register(rect);
	if (text_image_.null()) return;
	text_pos_ = 0;
	update_text_cache(false);
}

const std::string textbox::text() const
{
	const std::string &ret = unicode_cast<utf8::string>(text_);
	return ret;
}

// set_text does not respect max_size_
void textbox::set_text(const std::string& text, const color_t& color)
{
	text_ = unicode_cast<ucs4::string>(text);
	cursor_ = text_.size();
	text_pos_ = 0;
	selstart_ = -1;
	selend_ = -1;
	set_dirty(true);
	update_text_cache(true, color);
	handle_text_changed(text_);
}

void textbox::append_text(const std::string& text, bool auto_scroll, const color_t& color)
{
	if(text_image_.get() == nullptr) {
		set_text(text, color);
		return;
	}

	//disallow adding multi-line text to a single-line text box
	if(wrap_ == false && std::find_if(text.begin(),text.end(),utils::isnewline) != text.end()) {
		return;
	}
	const bool is_at_bottom = get_position() == get_max_position();
	const ucs4::string& wtext = unicode_cast<ucs4::string>(text);

	surface new_text = add_text_line(wtext, color);
	surface new_surface = create_compatible_surface(text_image_,std::max<size_t>(text_image_->w,new_text->w),text_image_->h+new_text->h);

	adjust_surface_alpha(new_text, SDL_ALPHA_TRANSPARENT);
	adjust_surface_alpha(text_image_, SDL_ALPHA_TRANSPARENT);
	SDL_SetSurfaceBlendMode(text_image_, SDL_BLENDMODE_NONE);
	sdl_blit(text_image_,nullptr,new_surface,nullptr);
	SDL_SetSurfaceBlendMode(text_image_, SDL_BLENDMODE_BLEND);

	SDL_Rect target {
			  0
			, text_image_->h
			, new_text->w
			, new_text->h
	};
	SDL_SetSurfaceBlendMode(new_text, SDL_BLENDMODE_NONE);
	sdl_blit(new_text,nullptr,new_surface,&target);
	text_image_.assign(new_surface);

	text_.insert(text_.end(), wtext.begin(), wtext.end());

	set_dirty(true);
	update_text_cache(false);
	if(auto_scroll && is_at_bottom) scroll_to_bottom();
	handle_text_changed(text_);
}

void textbox::clear()
{
	text_.clear();
	cursor_ = 0;
	cursor_pos_ = 0;
	text_pos_ = 0;
	selstart_ = -1;
	selend_ = -1;
	set_dirty(true);
	update_text_cache(true);
	handle_text_changed(text_);
}

void textbox::set_selection(const int selstart, const int selend)
{
	if (!editable_) {
		return;
	}
	if (selstart < 0 || selend < 0 || size_t(selstart) > text_.size() ||
		size_t(selend) > text_.size()) {
		WRN_DP << "out-of-boundary selection" << std::endl;
		return;
	}
	selstart_= selstart;
	selend_ = selend;
	set_dirty(true);
}

void textbox::set_cursor_pos(const int cursor_pos)
{
	if (!editable_) {
		return;
	}
	if (cursor_pos < 0 || size_t(cursor_pos) > text_.size()) {
		WRN_DP << "out-of-boundary selection" << std::endl;
		return;
	}

	cursor_ = cursor_pos;
	update_text_cache(false);
	set_dirty(true);
}

void textbox::draw_cursor(int pos) const
{
	if(show_cursor_ && editable_ && enabled()) {
		SDL_Rect rect {
				  location().x + pos
				, location().y
				, 1
				, location().h
		};

		sdl::fill_rectangle(rect, {255, 255, 255, 255});
	}
}

void textbox::draw_contents()
{
	SDL_Rect const &loc = inner_location();

	surface& surf = video().getSurface();

	color_t c(0, 0, 0);

	double& alpha = focus(nullptr) ? alpha_focus_ : alpha_;
	c.a = 255 * alpha;

	sdl::fill_rectangle(loc, c);

	SDL_Rect src;

	if(text_image_ == nullptr) {
		update_text_cache(true);
	}

	if(text_image_ != nullptr) {
		src.y = yscroll_;
		src.w = std::min<size_t>(loc.w,text_image_->w);
		src.h = std::min<size_t>(loc.h,text_image_->h);
		src.x = text_pos_;
		SDL_Rect dest = video().screen_area();
		dest.x = loc.x;
		dest.y = loc.y;

		// Fills the selected area
		if(enabled() && is_selection()) {
			const int start = std::min<int>(selstart_,selend_);
			const int end = std::max<int>(selstart_,selend_);
			int startx = char_x_[start];
			int starty = char_y_[start];
			const int endx = char_x_[end];
			const int endy = char_y_[end];

			while(starty <= endy) {
				const size_t right = starty == endy ? endx : text_image_->w;
				if(right <= size_t(startx)) {
					break;
				}

				SDL_Rect rect = sdl::create_rect(loc.x + startx
						, loc.y + starty - src.y
						, right - startx
						, line_height_);

				const clip_rect_setter clipper(surf, &loc);

				color_t c2(0, 0, 160, 140);
				sdl::fill_rectangle(rect, c2);

				starty += int(line_height_);
				startx = 0;
			}
		}

		if(enabled()) {
			sdl_blit(text_image_, &src, surf, &dest);
		} else {
			// HACK: using 30% opacity allows white text to look as though it is grayed out,
			// while not changing any applicable non-grayscale AA. Actual colored text will
			// not look as good, but this is not currently a concern since GUI1 textboxes
			// are not used much nowadays, and they will eventually all go away.
			adjust_surface_alpha(text_image_, ftofxp(0.3));
			sdl_blit(text_image_, &src, surf, &dest);
		}
	}

	draw_cursor(cursor_pos_ == 0 ? 0 : cursor_pos_ - 1);
}

void textbox::set_editable(bool value)
{
	editable_ = value;
}

bool textbox::editable() const
{
	return editable_;
}

int textbox::font_size() const
{
	return font_size_;
}

void textbox::set_font_size(int fs)
{
	font_size_ = fs;
}

void textbox::scroll_to_bottom()
{
	set_position(get_max_position());
}

void textbox::set_wrap(bool val)
{
	if(wrap_ != val) {
		wrap_ = val;
		update_text_cache(true);
		set_dirty(true);
	}
}

void textbox::scroll(unsigned int pos)
{
	yscroll_ = pos;
	set_dirty(true);
}

surface textbox::add_text_line(const ucs4::string& text, const color_t& color)
{
	line_height_ = font::get_max_height(font_size_);

	if(char_y_.empty()) {
		char_y_.push_back(0);
	} else {
		char_y_.push_back(char_y_.back() + line_height_);
	}

	char_x_.push_back(0);

	// Re-calculate the position of each glyph. We approximate this by asking the
	// width of each substring, but this is a flawed assumption which won't work with
	// some more complex scripts (that is, RTL languages). This part of the work should
	// actually be done by the font-rendering system.
	std::string visible_string;
	ucs4::string wrapped_text;

	ucs4::string::const_iterator backup_itor = text.end();

	ucs4::string::const_iterator itor = text.begin();
	while(itor != text.end()) {
		//If this is a space, save copies of the current state so we can roll back
		if(char(*itor) == ' ') {
			backup_itor = itor;
		}
		visible_string.append(unicode_cast<utf8::string>(*itor));

		if(char(*itor) == '\n') {
			backup_itor = text.end();
			visible_string = "";
		}

		int w = font::line_width(visible_string, font_size_);

		if(wrap_ && w >= inner_location().w) {
			if(backup_itor != text.end()) {
				int backup = itor - backup_itor;
				itor = backup_itor + 1;
				if(backup > 0) {
					char_x_.erase(char_x_.end()-backup, char_x_.end());
					char_y_.erase(char_y_.end()-backup, char_y_.end());
					wrapped_text.erase(wrapped_text.end()-backup, wrapped_text.end());
				}
			} else {
				if (visible_string == std::string("").append(unicode_cast<utf8::string>(*itor))) {
					break;	//breaks infinite loop where when running with a fake display, we word wrap a single character infinitely.
				}
			}
			backup_itor = text.end();
			wrapped_text.push_back(ucs4::char_t('\n'));
			char_x_.push_back(0);
			char_y_.push_back(char_y_.back() + line_height_);
			visible_string = "";
		} else {
			wrapped_text.push_back(*itor);
			char_x_.push_back(w);
			char_y_.push_back(char_y_.back() + (char(*itor) == '\n' ? line_height_ : 0));
			++itor;
		}
	}

	const std::string s = unicode_cast<utf8::string>(wrapped_text);
	const surface res(font::get_rendered_text(s, font_size_, color));

	return res;
}


void textbox::update_text_cache(bool changed, const color_t& color)
{
	if(changed) {
		char_x_.clear();
		char_y_.clear();

		text_image_.assign(add_text_line(text_, color));
	}

	int cursor_x = char_x_[cursor_];

	if(cursor_x - text_pos_ > location().w) {
		text_pos_ = cursor_x - location().w;
	} else if(cursor_x - text_pos_ < 0) {
		text_pos_ = cursor_x;
	}
	cursor_pos_ = cursor_x - text_pos_;

	if (!text_image_.null()) {
		set_full_size(text_image_->h);
		set_shown_size(location().h);
	}
}

bool textbox::is_selection()
{
	return (selstart_ != -1) && (selend_ != -1) && (selstart_ != selend_);
}

void textbox::erase_selection()
{
	if(!is_selection())
		return;

	ucs4::string::iterator itor = text_.begin() + std::min(selstart_, selend_);
	text_.erase(itor, itor + std::abs(selend_ - selstart_));
	cursor_ = std::min(selstart_, selend_);
	selstart_ = selend_ = -1;
}

namespace {
	const unsigned int copypaste_modifier =
#ifdef __APPLE__
		KMOD_LGUI | KMOD_RGUI
#else
		KMOD_CTRL
#endif
		;
}

bool textbox::requires_event_focus(const SDL_Event* event) const
{
	if(!focus_ || hidden() || !enabled()) {
		return false;
	}
	if(event == nullptr) {
		//when event is not specified, signal that focus may be desired later
		return true;
	}

	if(event->type == SDL_KEYDOWN) {
		SDL_Keycode key = event->key.keysym.sym;
		switch(key) {
		case SDLK_UP:
		case SDLK_DOWN:
		case SDLK_PAGEUP:
		case SDLK_PAGEDOWN:
			//in the future we may need to check for input history or multi-line support
			//for now, just return false since these events are not handled.
			return false;
		default:
			return true;
		}
	}
	//mouse events are processed regardless of focus
	return false;
}

void textbox::handle_event(const SDL_Event& event)
{
	gui::widget::handle_event(event);
	handle_event(event, false);
}

bool textbox::handle_text_input(const SDL_Event& event)
{
	bool changed = false;
	utf8::string str = event.text.text;
	ucs4::string s = unicode_cast<ucs4::string>(str);

	DBG_G << "Char: " << str << "\n";

	if (editable_) {
		changed = true;
		if (is_selection())
			erase_selection();

		if (text_.size() + 1 <= max_size_) {

			text_.insert(text_.begin() + cursor_, s.begin(), s.end());
			cursor_ += s.size();
		}
	} else {
		pass_event_to_target(event);
	}
	return changed;
}

bool textbox::handle_key_down(const SDL_Event &event)
{
	bool changed = false;

	const SDL_Keysym& key = reinterpret_cast<const SDL_KeyboardEvent&>(event).keysym;
	const SDL_Keymod modifiers = SDL_GetModState();

	const int c = key.sym;
	const int old_cursor = cursor_;

	listening_ = true;

	if(editable_) {
		if(c == SDLK_LEFT && cursor_ > 0)
			--cursor_;

		if(c == SDLK_RIGHT && cursor_ < static_cast<int>(text_.size()))
			++cursor_;

		// ctrl-a, ctrl-e and ctrl-u are readline style shortcuts, even on Macs
		if(c == SDLK_END || (c == SDLK_e && (modifiers & KMOD_CTRL)))
			cursor_ = text_.size();

		if(c == SDLK_HOME || (c == SDLK_a && (modifiers & KMOD_CTRL)))
			cursor_ = 0;

		if((old_cursor != cursor_) && (modifiers & KMOD_SHIFT)) {
			if(selstart_ == -1)
				selstart_ = old_cursor;
			selend_ = cursor_;
		}
	} else if(c == SDLK_LEFT || c == SDLK_RIGHT || c == SDLK_END || c == SDLK_HOME) {
		pass_event_to_target(event);
	}

	if(editable_) {
		if(c == SDLK_BACKSPACE) {
			changed = true;
			if(is_selection()) {
				erase_selection();
			} else if(cursor_ > 0) {
				--cursor_;
				text_.erase(text_.begin()+cursor_);
			}
		}

		if(c == SDLK_u && (modifiers & KMOD_CTRL)) { // clear line
			changed = true;
			cursor_ = 0;
			text_.resize(0);
		}

		if(c == SDLK_DELETE && !text_.empty()) {
			changed = true;
			if(is_selection()) {
				erase_selection();
			} else {
				if(cursor_ < static_cast<int>(text_.size())) {
					text_.erase(text_.begin()+cursor_);
				}
			}
		}
	} else if(c == SDLK_BACKSPACE || c == SDLK_DELETE || (c == SDLK_u && (modifiers & KMOD_CTRL))) {
		pass_event_to_target(event);
	}


	//movement characters may have a "Unicode" field on some platforms, so ignore it.
	if(!(c == SDLK_UP || c == SDLK_DOWN || c == SDLK_LEFT || c == SDLK_RIGHT ||
			c == SDLK_DELETE || c == SDLK_BACKSPACE || c == SDLK_END || c == SDLK_HOME ||
			c == SDLK_PAGEUP || c == SDLK_PAGEDOWN)) {
		if((event.key.keysym.mod & copypaste_modifier)
				//on windows SDL fires for AltGr lctrl+ralt (needed to access @ etc on certain keyboards)
#ifdef _WIN32
				&& !(event.key.keysym.mod & KMOD_ALT)
#endif
		) {
			switch(c) {
			case SDLK_v: // paste
			{
				if(!editable()) {
					pass_event_to_target(event);
					break;
				}

				changed = true;
				if(is_selection())
					erase_selection();

				std::string str = desktop::clipboard::copy_from_clipboard(false);

				//cut off anything after the first newline
				str.erase(std::find_if(str.begin(),str.end(),utils::isnewline),str.end());

				ucs4::string s = unicode_cast<ucs4::string>(str);

				if(text_.size() < max_size_) {
					if(s.size() + text_.size() > max_size_) {
						s.resize(max_size_ - text_.size());
					}
					text_.insert(text_.begin()+cursor_, s.begin(), s.end());
					cursor_ += s.size();
				}

			}

			break;

			case SDLK_c: // copy
			{
				if(is_selection())
				{
					const size_t beg = std::min<size_t>(size_t(selstart_),size_t(selend_));
					const size_t end = std::max<size_t>(size_t(selstart_),size_t(selend_));

					ucs4::string ws(text_.begin() + beg, text_.begin() + end);
					std::string s = unicode_cast<utf8::string>(ws);
					desktop::clipboard::copy_to_clipboard(s, false);
				}
			}
			break;
			}
		}
		else {
			pass_event_to_target(event);
		}
	}

	return changed;
}

void textbox::handle_event(const SDL_Event& event, bool was_forwarded)
{
	if(!enabled())
		return;

	scrollarea::handle_event(event);
	if(hidden())
		return;

	bool changed = false;

	const int old_selstart = selstart_;
	const int old_selend = selend_;

	//Sanity check: verify that selection start and end are within text
	//boundaries
	if(is_selection() && !(size_t(selstart_) <= text_.size() && size_t(selend_) <= text_.size())) {
		WRN_DP << "out-of-boundary selection" << std::endl;
		selstart_ = selend_ = -1;
	}

	int mousex, mousey;
	const uint8_t mousebuttons = SDL_GetMouseState(&mousex,&mousey);
	if(!(mousebuttons & SDL_BUTTON(1))) {
		grabmouse_ = false;
	}

	SDL_Rect const &loc = inner_location();
	bool clicked_inside = !mouse_locked() && (event.type == SDL_MOUSEBUTTONDOWN
					   && (mousebuttons & SDL_BUTTON(1))
					   && sdl::point_in_rect(mousex, mousey, loc));
	if(clicked_inside) {
		set_focus(true);
	}
	if ((grabmouse_ && (!mouse_locked() && event.type == SDL_MOUSEMOTION)) || clicked_inside) {
		const int x = mousex - loc.x + text_pos_;
		const int y = mousey - loc.y;
		int pos = 0;
		int distance = x;

		for(unsigned int i = 1; i < char_x_.size(); ++i) {
			if(static_cast<int>(yscroll_) + y < char_y_[i]) {
				break;
			}

			// Check individually each distance (if, one day, we support
			// RTL languages, char_x_[c] may not be monotonous.)
			if(std::abs(x - char_x_[i]) < distance && yscroll_ + y < char_y_[i] + line_height_) {
				pos = i;
				distance = std::abs(x - char_x_[i]);
			}
		}

		cursor_ = pos;

		if(grabmouse_)
			selend_ = cursor_;

		update_text_cache(false);

		if(!grabmouse_ && (mousebuttons & SDL_BUTTON(1))) {
			grabmouse_ = true;
			selstart_ = selend_ = cursor_;
		} else if (! (mousebuttons & SDL_BUTTON(1))) {
			grabmouse_ = false;
		}

		set_dirty();
	}

	//if we don't have the focus, then see if we gain the focus,
	//otherwise return
	if(!was_forwarded && focus(&event) == false) {
		if (!mouse_locked() && event.type == SDL_MOUSEMOTION && sdl::point_in_rect(mousex, mousey, loc))
			events::focus_handler(this);

		return;
	}

	const int old_cursor = cursor_;

	if (event.type == SDL_TEXTINPUT && listening_) {
		changed = handle_text_input(event);
	} else
		if (event.type == SDL_KEYDOWN) {
			changed = handle_key_down(event);
	}
	else {
		if(event.type != SDL_KEYDOWN || (!was_forwarded && focus(&event) != true)) {
			draw();
			return;
		}
	}


	if(is_selection() && (selend_ != cursor_))
		selstart_ = selend_ = -1;

	//since there has been cursor activity, make the cursor appear for
	//at least the next 500ms.
	show_cursor_ = true;
	show_cursor_at_ = SDL_GetTicks();

	if(changed || old_cursor != cursor_ || old_selstart != selstart_ || old_selend != selend_) {
		text_image_ = nullptr;
		handle_text_changed(text_);
	}

	set_dirty(true);
}

void textbox::pass_event_to_target(const SDL_Event& event)
{
	if(edit_target_ && edit_target_->editable()) {
		edit_target_->handle_event(event, true);
	}
}

void textbox::set_edit_target(textbox* target)
{
	edit_target_ = target;
}

} //end namespace gui
