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

#ifndef __GUI_WIDGETS_TOGGLE_BUTTON_HPP_INCLUDED__
#define __GUI_WIDGETS_TOGGLE_BUTTON_HPP_INCLUDED__

#include "gui/widgets/control.hpp"

namespace gui2 {

// Class for a toggle button
class ttoggle_button : public tcontrol
{
public:
	ttoggle_button() : 
		tcontrol(COUNT),
		state_(ENABLED_UP)
	{
		load_config();
	}

	void mouse_enter(tevent_handler&);
	void mouse_leave(tevent_handler&);

	void mouse_left_button_click(tevent_handler&);

	void set_active(const bool active);
	bool get_active() const
		{ return state_ != DISABLED_UP && state_ != DISABLED_DOWN; }
	unsigned get_state() const { return state_; }

	/** Is the widget in the up state. */
	bool is_up() const { return state_ < ENABLED_DOWN; }

	/** Set the button in the wanted state. */
	void set_up(const bool up = true);

protected:
	
private:
	//! Note the order of the states must be the same as defined in settings.hpp.
	//! Also note the internals do assume the order for up and down to be the same
	//! and also that 'up' is before 'down'.
	enum tstate { 
		ENABLED_UP,   DISABLED_UP,   FOCUSSED_UP, 
		ENABLED_DOWN, DISABLED_DOWN, FOCUSSED_DOWN, 
		COUNT };

	void set_state(tstate state);
	tstate state_;
 
	//! Inherited from tcontrol.
	const std::string& get_control_type() const 
		{ static const std::string type = "toggle_button"; return type; }
};


} // namespace gui2

#endif


