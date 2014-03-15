/*
   Copyright (C) 2014 by 
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_NUM_BOX_HPP_INCLUDED
#define GUI_WIDGETS_NUM_BOX_HPP_INCLUDED

#include "gui/widgets/integer_selector.hpp"
#include "gui/widgets/text_box.hpp"


/**
 * A class inherited from ttext_box that only takes numerical values
 */
namespace gui2
{

class tnum_box : public ttext_box, tinteger_selector_
{
public:
	tnum_box() : ttext_box(), value_(0), minimum_value_(0), maximum_value_(32767)
	{
	}

	/** Inherited from ttext_. */
	void set_value(const std::string& text);

	void set_value(const int value)
	{
		validate(value);
	}
	
	int get_value() const
	{
		return value_;
	}

	std::string get_text() const
	{
		return ttext_box::get_value();
	}
	
	void set_minimum_value(const int minimum_value)
	{
		minimum_value_ = minimum_value;
		if (minimum_value < get_value()) {
			validate(minimum_value);
		}
	}

	int get_minimum_value() const
	{
		return minimum_value_;
	}

	void set_maximum_value(const int maximum_value)
	{
		maximum_value_ = maximum_value;
		if (maximum_value > get_value()) {
			validate(maximum_value);
		}
	}

	int get_maximum_value() const
	{
		return maximum_value_;
	}


protected:
	void insert_char(const Uint16 unicode);
	void delete_char(const bool before_cursor);
	void paste_selection(const bool mouse);

	// optimization, especially for increment/decrement at bonds
	bool set_validated(const int value, const int cursor_shift)
	{
		if (value_ == value && !cursor_shift) {
			return false;
		}
		value_ = value;
		return true;
	}

private:
	void handle_key_backspace(SDLMod modifier, bool& handled);
	void handle_key_delete(SDLMod modifier, bool& handled);
	
	void handle_key_up_arrow(SDLMod /*modifier*/, bool& handled) {
		handled = true;
		validate(get_value()+1);
	}
	void handle_key_down_arrow(SDLMod /*modifier*/, bool& handled) {
		handled = true;
		validate(get_value()-1);
	}

	int value_;
	int minimum_value_;
	int maximum_value_;
	
	void validate(const int value, const int cursor_shift = 0);
	
	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const OVERRIDE;
};

} // namespace gui2

#endif
