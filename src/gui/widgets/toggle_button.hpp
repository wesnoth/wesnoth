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

#ifndef GUI_WIDGETS_TOGGLE_BUTTON_HPP_INCLUDED
#define GUI_WIDGETS_TOGGLE_BUTTON_HPP_INCLUDED

#include "gui/widgets/control.hpp"

namespace gui2 {

/**
 * Class for a toggle button.
 *
 * A toggle button is a button with two states 'up' and 'down' or 'selected' and
 * 'deselected'. When the mouse is pressed on it the state changes.
 */
class ttoggle_button : public tcontrol, public tselectable_
{
public:
	ttoggle_button() : 
		tcontrol(COUNT),
		state_(ENABLED),
		callback_mouse_left_click_(0),
		icon_name_()
	{
	}

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** Inherted from tevent_executor. */
	void mouse_enter(tevent_handler&);

	/** Inherted from tevent_executor. */
	void mouse_leave(tevent_handler&);

	/** Inherted from tevent_executor. */
	void mouse_left_button_click(tevent_handler&);

	/** Inherited from tcontrol. */
	void set_active(const bool active);

	/** Inherited from tcontrol. */
	bool get_active() const
		{ return state_ != DISABLED && state_ != DISABLED_SELECTED; }

	/** Inherited from tcontrol. */
	unsigned get_state() const { return state_; }

	/** Inherited from tcontrol. */
	void set_canvas_text();
	
	/** Inherited from tselectable_ */
	bool is_selected() const { return state_ >= ENABLED_SELECTED; }

	/** Inherited from tselectable_ */
	void set_selected(const bool selected = true);

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_callback_mouse_left_click(void (*callback) (twidget*)) 
		{ callback_mouse_left_click_ = callback; }

	void set_icon_name(const std::string& icon_name) 
		{ icon_name_ = icon_name; set_canvas_text(); }
	const std::string& icon_name() const { return icon_name_; }
	
private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 * Also note the internals do assume the order for 'up' and 'down' to be the
	 * same and also that 'up' is before 'down'. 'up' has no suffix, 'down' has
	 * the SELECTED suffix.
	 */
	enum tstate { 
		ENABLED,          DISABLED,          FOCUSSED, 
		ENABLED_SELECTED, DISABLED_SELECTED, FOCUSSED_SELECTED, 
		COUNT};

	void set_state(tstate state);

	/** 
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	tstate state_;
 
 	/** This callback is used when the control gets a left click. */
	void (*callback_mouse_left_click_) (twidget*);

	/** 
	 * The toggle button can contain an icon next to the text.
	 * Maybe this will move the the tcontrol class if deemed needed.
	 */
	std::string icon_name_;

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const 
		{ static const std::string type = "toggle_button"; return type; }

};

} // namespace gui2

#endif

