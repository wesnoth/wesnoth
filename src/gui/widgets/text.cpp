/* $Id$ */
/*
   copyright (c) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/widgets/text.hpp"

#include "log.hpp"

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

void ttext_::key_press(tevent_handler& /*event*/, bool& handled, SDLKey key, SDLMod modifier, Uint16 unicode)
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
void ttext_::handle_key_backspace(SDLMod /*modifier*/, bool& handled)
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
void ttext_::handle_key_delete(SDLMod /*modifier*/, bool& handled)
{
	DBG_G_E << "Text_box: key press: delete.\n";

	handled = true;
	if(sel_len_ != 0) {
		delete_selection();
	} else if (sel_start_ < text_.size()) {
		delete_char(false);
	}
}

void ttext_::handle_key_default(bool& handled, SDLKey /*key*/, SDLMod /*modifier*/, Uint16 unicode)
{
	DBG_G_E << "Text_box: key press: default.\n";

	if(unicode >= 32 && unicode != 127) {
		handled = true;
		insert_char(unicode);
	}
}




} // namespace gui2

