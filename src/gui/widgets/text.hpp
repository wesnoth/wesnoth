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

#ifndef __GUI_WIDGETS_TEXT_HPP_INCLUDED__
#define __GUI_WIDGETS_TEXT_HPP_INCLUDED__

#include "gui/widgets/control.hpp"

#include <string>

namespace gui2 {

//! Base class for text items will get two base decendends
//! - ttext_box a single line text
//! - ttext_area a multi line text
class ttext_ : public tcontrol
{

public:
	ttext_() :
		tcontrol(COUNT),
		state_(ENABLED),
		text_(),
		sel_start_(0),
		sel_len_(0),
		max_length_(std::string::npos),
		dragging_(false)
	{}

	void set_active(const bool /*active*/) { /*FIXME IMPLEMENT*/ };
	bool get_active() const { return true; /* FIXME IMPLEMENT */ }
	unsigned get_state() const { return state_; }

	void mouse_move(tevent_handler&);

	void mouse_left_button_down(tevent_handler& event);
	void mouse_left_button_up(tevent_handler&);
	void mouse_left_button_double_click(tevent_handler&);

	void key_press(tevent_handler& event, bool& handled, SDLKey key, SDLMod modifier, Uint16 unicode);


	void set_text(const std::string& text); 
	std::string get_text() const { return text_; }
	const std::string& text() const { return text_; }

protected:

	virtual void goto_end_of_line(const bool select = false) = 0;
	void goto_end_of_data(const bool select = false) { set_cursor(text_.size(), select); }

	virtual void goto_start_of_line(const bool select = false) = 0;
	void goto_start_of_data(const bool select = false) { set_cursor(0, select); }

	void select_all() { sel_start_ = 0; goto_end_of_data(true); }

	void set_cursor(const size_t offset, const bool select); // call set dirty

	size_t get_sel_start() const { return sel_start_; }
	void  set_sel_start(const size_t sel_start) { sel_start_ = sel_start; set_dirty(); }

	size_t get_sel_len() const { return sel_len_; }
	void set_sel_len(const unsigned sel_len) { sel_len_ = sel_len; set_dirty(); }

	//! Inserts a character at the cursor.
	virtual void insert_char(Uint16 unicode) = 0;

	//! Deletes the character.
	virtual void delete_char(const bool before_cursor) = 0;

	//! Deletes the current selection.
	virtual void delete_selection() = 0;

	//! Copies the current selection.
	virtual void copy_selection();

	//! Pastes the current selection.
	virtual void paste_selection();

protected:

	std::string& text() { return text_; }

	size_t& sel_start() { return sel_start_; }

	int& sel_len() { return sel_len_; }

private:
	//! Note the order of the states must be the same as defined in settings.hpp.
	enum tstate { ENABLED, DISABLED, FOCUSSED, COUNT };

	void set_state(tstate state);
	tstate state_;


	//! The text in the widget.
	std::string text_;

	//! Calculates the offsets of all chars.
	virtual void calculate_char_offset() = 0;

	size_t sel_start_;
	//! positive sel_len_ means selection to the right.
	//! negative sel_len_ means selection to the left.
	//! sel_len_ == 0 means no selection.
	int sel_len_;
	size_t max_length_;

	//! Is the mouse in dragging mode, this affects selection in mouse movee
	bool dragging_;

	// handling of special keys first the pure virtuals
	virtual void handle_key_up_arrow(SDLMod modifier, bool& handled) = 0;
	virtual void handle_key_down_arrow(SDLMod modifier, bool& handled) = 0;

	// Clears the current line
	virtual void handle_key_clear_line(SDLMod modifier, bool& handled) = 0;

	// Go a character left of not at start of buffer else beep.
	// ctrl moves a word instead of character.
	// shift selects while moving.
	virtual void handle_key_left_arrow(SDLMod modifier, bool& handled);

	// Go a character right of not at end of buffer else beep.
	// ctrl moves a word instead of character.
	// shift selects while moving.
	virtual void handle_key_right_arrow(SDLMod modifier, bool& handled);

	// Go to the beginning of the line.
	// ctrl moves the start of data (except when ctrl-e but caller does that) 
	// shift selects while moving.
	virtual void handle_key_home(SDLMod modifier, bool& handled);

	// Go to the end of the line.
	// ctrl moves the end of data (except when ctrl-a but caller does that) 
	// shift selects while moving.
	virtual void handle_key_end(SDLMod modifier, bool& handled);

	// Deletes the character in front of the cursor (if not at the beginning).
	virtual void handle_key_backspace(SDLMod modifier, bool& handled);

	// Deletes either the selection or the character beyond the cursor
	virtual void handle_key_delete(SDLMod modifier, bool& handled);

	// Default handler, inserts a unicode char if valid
	virtual void handle_key_default(bool& handled, SDLKey key, SDLMod modifier, Uint16 unicode);

	// These are ignored by a single line edit box which is the default behaviour.
	virtual void handle_key_page_up(SDLMod /*modifier*/, bool& /*handled*/) {}
	virtual void handle_key_page_down(SDLMod /*modifier*/, bool& /*handled*/) {}
};

} // namespace gui2

#endif
