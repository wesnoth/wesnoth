/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_BUTTON_HPP_INCLUDED
#define GUI_WIDGETS_BUTTON_HPP_INCLUDED

#include "gui/widgets/control.hpp"

namespace gui2 {

/**
 * Simple push button.
 */
class tbutton : public tcontrol
{
public:
	tbutton() :
		tcontrol(COUNT),
		state_(ENABLED),
		retval_(0),
		callback_mouse_left_click_(0)
	{
	}

	void set_callback_mouse_left_click(void (*callback) (twidget*))
		{ callback_mouse_left_click_ = callback; }

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** Inherted from tevent_executor. */
	void mouse_enter(tevent_handler&);

	/** Inherted from tevent_executor. */
	void mouse_leave(tevent_handler&);


	/** Inherted from tevent_executor. */
	void mouse_left_button_down(tevent_handler& event);

	/** Inherted from tevent_executor. */
	void mouse_left_button_up(tevent_handler&);

	/** Inherted from tevent_executor. */
	void mouse_left_button_click(tevent_handler&);


	/** Inherited from tcontrol. */
	void set_active(const bool active)
		{ if(get_active() != active) set_state(active ? ENABLED : DISABLED); };

	/** Inherited from tcontrol. */
	bool get_active() const { return state_ != DISABLED; }

	/** Inherited from tcontrol. */
	unsigned get_state() const { return state_; }

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_retval(const int retval) { retval_ = retval; }

private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum tstate { ENABLED, DISABLED, PRESSED, FOCUSSED, COUNT };

	void set_state(const tstate state);
	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	tstate state_;

 	/**
	 * The return value of the button.
	 *
	 * If this value is not 0 and the button is clicked it sets the retval of
	 * the window and the window closes itself.
	 */
	int retval_;

	/**
	 * This callback is used when the control gets a left click. Except when the
	 * button has a retval_, then retval_ is set.
	 */
	void (*callback_mouse_left_click_) (twidget*);

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const
		{ static const std::string type = "button"; return type; }
};


} // namespace gui2

#endif

