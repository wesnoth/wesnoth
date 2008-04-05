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

#ifndef __GUI_WIDGETS_BUTTON_HPP_INCLUDED__
#define __GUI_WIDGETS_BUTTON_HPP_INCLUDED__

#include "gui/widgets/control.hpp"

#include "gui/widgets/settings.hpp"

namespace gui2 {

// Class for a simple push button
class tbutton : public tcontrol
{
public:
	tbutton() : 
		tcontrol(COUNT),
		state_(ENABLED),
		definition_(),
		retval_(0)
		{
		}

	void mouse_enter(tevent_handler&);
	void mouse_hover(tevent_handler&);
	void mouse_leave(tevent_handler&);

	void mouse_left_button_down(tevent_handler& event);
	void mouse_left_button_up(tevent_handler&);
	void mouse_left_button_click(tevent_handler&);
	void mouse_left_button_double_click(tevent_handler&); //FIXME remove

	// note we should check whether the label fits in the button
	tpoint get_best_size() const;

	void set_best_size(const tpoint& origin);

	void set_retval(const int retval) { retval_ = retval; }

	//! Default button values, values are subject to change.
	//! Note this might be moved somewhere else since it will
	//! force people to include the button, while it should
	//! be and implementation detail for most callers.
	enum RETVAL {
		NONE = 0,                      //!< Dialog is closed with no return 
		                               //!< value, should be rare but eg a
		                               //!< message popup can do it.
		OK = -1,                       //!< Dialog is closed with ok button.
		CANCEL = -2,                   //!< Dialog is closed with the cancel 
		                               //!<  button.
		};

	//! Gets the retval for the default buttons.
	static RETVAL get_retval_by_id(const std::string& id);

	void set_active(const bool active);
	bool get_active() const;
	unsigned get_state() const { return state_; }
	bool full_redraw() const { return false; /* FIXME IMPLEMENT */ }

protected:
	
private:
	enum tstate { ENABLED, DISABLED, PRESSED, FOCUSSED, COUNT };

	void set_state(tstate state);
	tstate state_;

	std::vector<tbutton_definition::tresolution>::const_iterator definition_;

	void resolve_definition();

	int retval_;
};


} // namespace gui2

#endif

