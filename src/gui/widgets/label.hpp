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

#ifndef __GUI_WIDGETS_LABEL_HPP_INCLUDED__
#define __GUI_WIDGETS_LABEL_HPP_INCLUDED__

#include "gui/widgets/control.hpp"

namespace gui2 {

class tlabel : public tcontrol
{
public:
	
	tlabel() :
		tcontrol(COUNT),
		state_(ENABLED)
	{
	}

	void set_active(const bool active) { set_state(active ? ENABLED : DISABLED); };
	bool get_active() const { return state_ == ENABLED; }
	unsigned get_state() const { return state_; }

private:
	//! Note the order of the states must be the same as defined in settings.hpp.
	enum tstate { ENABLED, DISABLED, COUNT };

	void set_state(tstate state);
	tstate state_;

	//! Inherited from tcontrol.
	const std::string& get_control_type() const 
		{ static const std::string type = "label"; return type; }
};

} // namespace gui2

#endif

