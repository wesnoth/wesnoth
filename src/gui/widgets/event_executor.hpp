/*
   Copyright (C) 2007 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_EVENT_EXECUTOR_HPP_INCLUDED
#define GUI_WIDGETS_EVENT_EXECUTOR_HPP_INCLUDED

namespace gui2
{

class event_handler;

/**
 * Event execution calls.
 *
 * Base class with all possible events, most widgets can ignore most of these,
 * but they are available. In order to use an event simply override the
 * execution function and implement the wanted behavior. The default behavior
 * defined here is to do nothing.
 *
 * For more info about the event handling have a look at the event_handler
 * class which 'translates' sdl events into 'widget' events.
 */
class event_executor
{
public:
	event_executor()
		: wants_mouse_hover_(false)
		, wants_mouse_left_double_click_(false)
		, wants_mouse_middle_double_click_(false)
		, wants_mouse_right_double_click_(false)
	{
	}

	virtual ~event_executor()
	{
	}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_wants_mouse_hover(const bool hover = true)
	{
		wants_mouse_hover_ = hover;
	}
	bool wants_mouse_hover() const
	{
		return wants_mouse_hover_;
	}

	void set_wants_mouse_left_double_click(const bool click = true)
	{
		wants_mouse_left_double_click_ = click;
	}
	bool wants_mouse_left_double_click() const
	{
		return wants_mouse_left_double_click_;
	}

	void set_wants_mouse_middle_double_click(const bool click = true)
	{
		wants_mouse_middle_double_click_ = click;
	}
	bool wants_mouse_middle_double_click() const
	{
		return wants_mouse_middle_double_click_;
	}

	event_executor& set_wants_mouse_right_double_click(const bool click = true)
	{
		wants_mouse_right_double_click_ = click;
		return *this;
	}
	bool wants_mouse_right_double_click() const
	{
		return wants_mouse_right_double_click_;
	}

private:
	/** Does the widget want a hover event? See mouse_hover. */
	bool wants_mouse_hover_;

	/**
	 * Does the widget want a left button double click? See
	 * mouse_left_button_double_click.
	 */
	bool wants_mouse_left_double_click_;

	/** See wants_mouse_left_double_click_ */
	bool wants_mouse_middle_double_click_;

	/** See wants_mouse_left_double_click_ */
	bool wants_mouse_right_double_click_;
};

} // namespace gui2

#endif
