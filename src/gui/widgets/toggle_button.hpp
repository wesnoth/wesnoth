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
#include "gui/widgets/selectable.hpp"

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
		retval_(0),
		callback_state_change_(0),
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

	/** Inherted from tevent_executor. */
	void mouse_left_button_double_click(tevent_handler&);

	/** 
	 * Inherited from tcontrol.
	 *
	 * Sets the additional member
	 *  * icon_name_              icon
	 */
	void set_members(const string_map& data);

	/** Inherited from tcontrol. */
	void set_active(const bool active);

	/** Inherited from tcontrol. */
	bool get_active() const
		{ return state_ != DISABLED && state_ != DISABLED_SELECTED; }

	/** Inherited from tcontrol. */
	unsigned get_state() const { return state_; }

	/** Inherited from tcontrol. */
	bool does_block_easy_close() const { return true; }

	/** Inherited from tcontrol. */
	void update_canvas();
	
	/** Inherited from tselectable_ */
	bool get_value() const { return state_ >= ENABLED_SELECTED; }

	/** Inherited from tselectable_ */
	void set_value(const bool selected);

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_retval(const int retval);

	/** Inherited from tselectable_. */
	void set_callback_state_change(void (*callback) (twidget*)) 
		{ callback_state_change_ = callback; }

	void set_icon_name(const std::string& icon_name) 
		{ icon_name_ = icon_name; update_canvas(); }
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
	 * If this value is not 0 and the button is double clicked it sets the
	 * retval of the window and the window closes itself.
	 */
	int retval_;

	/** See tselectable_::set_callback_state_change. */
	void (*callback_state_change_) (twidget*);

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

