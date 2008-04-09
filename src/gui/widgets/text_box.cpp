/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/widgets/text_box.hpp"

#include "font.hpp"
#include "foreach.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"

#include <numeric>

#define DBG_G LOG_STREAM_INDENT(debug, gui)
#define LOG_G LOG_STREAM_INDENT(info, gui)
#define WRN_G LOG_STREAM_INDENT(warn, gui)
#define ERR_G LOG_STREAM_INDENT(err, gui)

#define DBG_G_D LOG_STREAM_INDENT(debug, gui_draw)
#define LOG_G_D LOG_STREAM_INDENT(info, gui_draw)
#define WRN_G_D LOG_STREAM_INDENT(warn, gui_draw)
#define ERR_G_D LOG_STREAM_INDENT(err, gui_draw)

#define DBG_G_E LOG_STREAM_INDENT(debug, gui_event)
#define LOG_G_E LOG_STREAM_INDENT(info, gui_event)
#define WRN_G_E LOG_STREAM_INDENT(warn, gui_event)
#define ERR_G_E LOG_STREAM_INDENT(err, gui_event)

#define DBG_G_P LOG_STREAM_INDENT(debug, gui_parse)
#define LOG_G_P LOG_STREAM_INDENT(info, gui_parse)
#define WRN_G_P LOG_STREAM_INDENT(warn, gui_parse)
#define ERR_G_P LOG_STREAM_INDENT(err, gui_parse)


namespace gui2 {

void ttext_::set_cursor(const size_t offset, const bool select)
{
	if(select) {

		if(sel_start_ == offset) {
			sel_len_ = 0;
		} else if (offset > sel_start_) {
			sel_len_ = sel_start_ - offset;
		} else { // sel_start_ < offset
			sel_len_ = - (sel_start_ - offset);
		}
		set_canvas_text();
		set_dirty();

	} else {
		assert(offset <= text_.size());
		sel_start_ = offset;
		sel_len_ = 0;

		set_canvas_text();
		set_dirty();
	}
}

void ttext_::mouse_move(tevent_handler&)
{
	DBG_G_E << "Text_box: mouse move.\n"; 

	// if in select mode select text and move cursor
}

void ttext_::mouse_hover(tevent_handler&)
{
	DBG_G_E << "Text_box: mouse hover.\n"; 
}

void ttext_::mouse_left_button_down(tevent_handler& event) 
{ 
	DBG_G_E << "Text_box: left mouse button down.\n"; 

	//FIXME place cursor
	//set select  mode

	event.keyboard_capture(this);
	event.mouse_capture();
}

void ttext_::mouse_left_button_up(tevent_handler&) 
{ 
	// reset select  mode
	DBG_G_E << "Text_box: left mouse button up.\n";
}

void ttext_::mouse_left_button_double_click(tevent_handler&) 
{ 
	DBG_G_E << "Text_box: left mouse button double click.\n";

	sel_start_ = 0;
	sel_len_ = text_.size();

}

void ttext_::key_press(tevent_handler& event, bool& handled, SDLKey key, SDLMod modifier, Uint16 unicode)
{
	DBG_G_E << "Text_box: key press.\n";

	switch(key) {

		case SDLK_LEFT :
			handle_key_left_arrow(modifier, handled);
			break;

		case SDLK_RIGHT :
			handle_key_right_arrow(modifier, handled);
			break;

		case SDLK_UP :
			handle_key_up_arrow(modifier, handled);
			break;

		case SDLK_DOWN :
			handle_key_down_arrow(modifier, handled);
			break;

		case SDLK_PAGEUP :
			handle_key_page_up(modifier, handled);
			break;

		case SDLK_PAGEDOWN :
			handle_key_page_down(modifier, handled);
			break;

		case SDLK_a :
			if(!(modifier & KMOD_CTRL)) {
				handle_key_default(handled, key, modifier, unicode);
				break;
			}
			
			// If ctrl-a is used for home drop the control modifier
			modifier = static_cast<SDLMod>(modifier &~ KMOD_CTRL);
			/* FALL DOWN */

		case SDLK_HOME :
			handle_key_home(modifier, handled);
			break;
			
		case SDLK_e :
			if(!(modifier & KMOD_CTRL)) {
				handle_key_default(handled, key, modifier, unicode);
				break;
			}
			
			// If ctrl-e is used for end drop the control modifier
			modifier = static_cast<SDLMod>(modifier &~ KMOD_CTRL);
			/* FALL DOWN */

		case SDLK_END :
			handle_key_end(modifier, handled);
			break;

		case SDLK_BACKSPACE :
			handle_key_backspace(modifier, handled);
			break;

		case SDLK_u :
			if(modifier & KMOD_CTRL) {
				handle_key_clear_line(modifier, handled);
			} else {
				handle_key_default(handled, key, modifier, unicode);
			}
			break;

		case SDLK_DELETE :
			handle_key_delete(modifier, handled);
			break;
			
		default :
			handle_key_default(handled, key, modifier, unicode);

	}

}

void ttext_::set_text(const std::string& text)
{ 
	if(text != text_) { 
		text_ = text; 
		calculate_char_offset(); 

		// default to put the cursor at the end of the buffer.
		sel_start_ = text_.size();
		sel_len_ = 0;
		set_canvas_text();
		set_dirty(); 
	} 
}

void ttext_::set_state(tstate state)
{
	if(state != state_) {
		state_ = state;
		set_dirty(true);
	}
}

// Go a character left of not at start of buffer else beep.
// ctrl moves a word instead of character.
// shift selects while moving.
void ttext_::handle_key_left_arrow(SDLMod modifier, bool& handled)
{
	DBG_G_E << "Text_box: key press: left arrow.\n";

	handled = true;
	if(sel_start_) {
		set_cursor(sel_start_ - 1, modifier & KMOD_SHIFT);
	}
}

// Go a character right of not at end of buffer else beep.
// ctrl moves a word instead of character.
// shift selects while moving.
void ttext_::handle_key_right_arrow(SDLMod modifier, bool& handled)
{
	DBG_G_E << "Text_box: key press: right arrow.\n";

	handled = true;
	if(sel_start_ < text_.size()) {
		set_cursor(sel_start_ + 1, modifier & KMOD_SHIFT);
	}
}

// Go to the beginning of the line.
// ctrl moves the start of data (except when ctrl-e but caller does that) 
// shift selects while moving.
void ttext_::handle_key_home(SDLMod modifier, bool& handled)
{
	DBG_G_E << "Text_box: key press: home.\n";

	handled = true;
	if(modifier & KMOD_CTRL) {
		goto_start_of_data(modifier & KMOD_SHIFT);
	} else {
		goto_start_of_line(modifier & KMOD_SHIFT);
	}
}

// Go to the end of the line.
// ctrl moves the end of data (except when ctrl-a but caller does that) 
// shift selects while moving.
void ttext_::handle_key_end(SDLMod modifier, bool& handled)
{
	DBG_G_E << "Text_box: key press: end.\n";

	handled = true;
	if(modifier & KMOD_CTRL) {
		goto_end_of_data(modifier & KMOD_SHIFT);
	} else {
		goto_end_of_line(modifier & KMOD_SHIFT);
	}
}

// Deletes the character in front of the cursor (if not at the beginning).
void ttext_::handle_key_backspace(SDLMod modifier, bool& handled)
{
	DBG_G_E << "Text_box: key press: backspace.\n";

	handled = true;
	if(sel_start_){
		delete_char(true);
	} else {
		// FIXME beep
	}

}

// Deletes either the selection or the character beyond the cursor
void ttext_::handle_key_delete(SDLMod modifier, bool& handled)
{
	DBG_G_E << "Text_box: key press: delete.\n";

	handled = true;
	if(sel_len_ != 0) {
		delete_selection();
	} else if (sel_start_ < text_.size()) {
		delete_char(false);
	}
}

void ttext_::handle_key_default(bool& handled, SDLKey key, SDLMod modifier, Uint16 unicode)
{
	DBG_G_E << "Text_box: key press: default.\n";

	if(unicode >= 32 && unicode != 127) {
		handled = true;
		insert_char(unicode);
	}
}




static surface render_text(const std::string& text, unsigned font_size)
{
	static SDL_Color col = {0, 0, 0, 0};
	return font::get_rendered_text(text, font_size, col, TTF_STYLE_NORMAL);
}

//! Helper function for text more efficient as set_text.
//! Inserts a character at the cursor.
void ttext_box::insert_char(Uint16 unicode)
{
	if(sel_len() > 0) {
		delete_selection();
	}

	// Determine the width of the new character.
	std::string tmp_text;
	tmp_text.insert(tmp_text.begin(), unicode);

	surface surf = render_text(tmp_text, config()->text_font_size);
	assert(surf);
	const unsigned width = surf->w;

	// Insert the char in the buffer, we need to assume it's a wide string.
	wide_string tmp = utils::string_to_wstring(text());
	tmp.insert(tmp.begin() + sel_start(), unicode);
	text() = utils::wstring_to_string(tmp);

	// Update the widths.
	character_offset_.insert(character_offset_.begin() + sel_start(), width);
	if(sel_start() != 0) {
		character_offset_[sel_start()] += character_offset_[sel_start() - 1]; 
	}

	++sel_start();
	for(size_t i = sel_start(); i < character_offset_.size(); ++i) {
		character_offset_[i] += width;
	}

	set_cursor(sel_start(), false);
	set_canvas_text();
	set_dirty();
}

//! Deletes the character.
//!
//! @param before_cursor     If true it deletes the character before the cursor
//!                          (backspace) else the character after the cursor
//!                          (delete). 
void ttext_box::delete_char(const bool before_cursor)
{
	if(before_cursor) {
		--sel_start();
		set_cursor(sel_start(), false);
	}

	sel_len() = 1;

	delete_selection();
}

//! Deletes the current selection.
void ttext_box::delete_selection()
{
	assert(sel_len() != 0);

	// If we have a negative range change it to a positive range.
	// This makes the rest of the algoritms easier.
	if(sel_len() < 0) {
		sel_len() = - sel_len();
		sel_start() -= sel_len();
		set_cursor(sel_start(), false);
	}

	// Update the text, we need to assume it's a wide string.
	wide_string tmp = utils::string_to_wstring(text());
	tmp.erase(tmp.begin() + sel_start(), tmp.begin() + sel_start() + sel_len());
	text() = utils::wstring_to_string(tmp);

	// Update the offsets
	const unsigned width = character_offset_[sel_start() + sel_len() - 1] - 
		(sel_start() ? character_offset_[sel_start() - 1] : 0);

	character_offset_.erase(character_offset_.begin() + sel_start(), character_offset_.begin() + sel_start() + sel_len());
	for(size_t i = sel_start(); i < character_offset_.size(); ++i) {
		character_offset_[i] -= width;
	}

	sel_len() = 0;
	set_canvas_text();
	set_dirty();
}

//! Inherited from tcontrol.
void ttext_box::set_canvas_text()
{
	foreach(tcanvas& tmp, canvas()) {
		tmp.set_variable("text", variant(text()));
		//FIXME add selection info.
		if(text().empty() || sel_start() == 0) {
			tmp.set_variable("cursor_offset", variant(0));
		} else {
			tmp.set_variable("cursor_offset", variant(character_offset_[sel_start() -1] + 0));
		}
	}
}

//! Calculates the offsets of all chars.
void ttext_box::calculate_char_offset()
{
	// If the text is set before the config is loaded do it ourselves.
	// This isn't really clean solution, maybe fix it later.
	if(!config()) {
		load_config();
	}
	assert(config());
	character_offset_.clear();

	std::string rendered_text;
	const unsigned font_size = config()->text_font_size;

	// FIXME we assume the text start at offset 0!!!
	foreach(const wchar_t& unicode, utils::string_to_wstring(text())) {
		rendered_text.insert(rendered_text.end(), unicode);
		surface surf = render_text(rendered_text, font_size);
		assert(surf);
		character_offset_.push_back(surf->w);

	}
}

void ttext_box::handle_key_clear_line(SDLMod modifier, bool& handled)
{
	handled = true;

	set_text("");
}

void ttext_box::load_config()
{
	if(!config()) {
		set_config(get_text_box(definition()));

		assert(canvas().size() == config()->state.size());
		for(size_t i = 0; i < canvas().size(); ++i) {
			canvas(i) = config()->state[i].canvas;
		}

		set_canvas_text();
	}
}

} //namespace gui2


