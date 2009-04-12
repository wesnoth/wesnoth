/* $Id$ */
/*
   Copyright (C) 2007 - 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_EVENT_EXECUTOR_HPP_INCLUDED
#define GUI_WIDGETS_EVENT_EXECUTOR_HPP_INCLUDED

#include "SDL.h"

namespace gui2 {

class tevent_handler;

/**
 * Event execution calls.
 *
 * Base class with all possible events, most widgets can ignore most of these,
 * but they are available. In order to use an event simply override the
 * execution function and implement the wanted behaviour. The default behaviour
 * defined here is to do nothing.
 *
 * For more info about the event handling have a look at the tevent_handler
 * class which 'translates' sdl events into 'widget' events.
 */
class tevent_executor
{
public:
	tevent_executor() :
		wants_mouse_hover_(false),
		wants_mouse_left_double_click_(false),
		wants_mouse_middle_double_click_(false),
		wants_mouse_right_double_click_(false)
		{}

	virtual ~tevent_executor() {}

	/***** ***** ***** ***** mouse movement ***** ***** ***** *****/

	/**
	 * The mouse 'enters' the widget.
	 *
	 * Entering happens when the mouse moves on a widget it wasn't on before.
	 * When the mouse is captured by another widget this event does not occur.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void mouse_enter(tevent_handler& /*event_handler*/) {}

	/**
	 * The mouse moves 'over' the widget.
	 *
	 * The mouse either moves over the widget or it has the mouse captured in
	 * which case every move causes a move event for the capturing widget.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void mouse_move(tevent_handler& /*event_handler*/) {}

	/**
	 * The mouse 'hovers' over a widget.
	 *
	 * If the mouse remains a while without moving over a widget this event can
	 * be send. This event can be used to show a tooltip.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void mouse_hover(tevent_handler& /*event_handler*/) {}

	/**
	 * The mouse 'leaves' the widget.
	 *
	 * If the mouse is moves off the widget this event is called. When the leave
	 * occurs while the mouse is captured the event will be send when still of
	 * the widget if the capture is released. This event is only triggered when
	 * wants_mouse_hover_ is set.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void mouse_leave(tevent_handler& /*event_handler*/) {}

	/***** ***** ***** ***** mouse left button ***** ***** ***** *****/

	/**
	 * The left mouse button is pressed down.
	 *
	 * This is a rather low level event, most of the time you want to have a
	 * look at mouse_left_button_click instead.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void mouse_left_button_down(tevent_handler& /*event_handler*/) {}

	/**
	 * The left mouse button is released down.
	 *
	 * This is a rather low level event, most of the time you want to have a
	 * look at mouse_left_button_click instead.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void mouse_left_button_up(tevent_handler& /*event_handler*/) {}

	/**
	 * The left button is clicked.
	 *
	 * This event happens when the left mouse button is pressed and released on
	 * the same widget. It's execution can be a little delayed when
	 * wants_mouse_left_double_click_ is set.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void mouse_left_button_click(tevent_handler& /*event_handler*/) {}

	/**
	 * The left button is double clicked.
	 *
	 * This event happens when the left mouse button is pressed and released on
	 * the same widget twice within a short time. This event will only occur if
	 * wants_mouse_left_double_click_ is set.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void mouse_left_button_double_click(tevent_handler& /*event_handler*/) {}

	/***** ***** ***** ***** mouse middle button ***** ***** ***** *****/

	/** See mouse_left_button_down. */
	virtual void mouse_middle_button_down(tevent_handler&) {}

	/** See mouse_left_button_up. */
	virtual void mouse_middle_button_up(tevent_handler&) {}

	/** See mouse_left_button_click. */
	virtual void mouse_middle_button_click(tevent_handler&) {}

	/** See mouse_left_button_double_click. */
	virtual void mouse_middle_button_double_click(tevent_handler&) {}

	/***** ***** ***** ***** mouse right button ***** ***** ***** *****/

	/** See mouse_left_button_down. */
	virtual void mouse_right_button_down(tevent_handler&) {}

	/** See mouse_left_button_up. */
	virtual void mouse_right_button_up(tevent_handler&) {}

	/** See mouse_left_button_click. */
	virtual void mouse_right_button_click(tevent_handler&) {}

	/** See mouse_left_button_double_click. */
	virtual void mouse_right_button_double_click(tevent_handler&) {}

	/***** ***** ***** ***** mouse wheel ***** ***** ***** *****/

	/**
	 * Scrollwheel up.
	 *
	 * The scrollwheel events are trigger by the scrollwheel.
	 *
	 * @param event_handler       The event handler that send the event.
	 * @param handled             Do we handle the event.
	 */
	virtual void mouse_wheel_up(
			tevent_handler& /*event_handler*/, bool& /*handled*/) {}

	/** Scrollwheel down see mouse_wheel_up.*/
	virtual void mouse_wheel_down(tevent_handler&, bool&) {}

	/** Scrollwheel to the left see mouse_wheel_up.*/
	virtual void mouse_wheel_left(tevent_handler&, bool&) {}

	/** Scrollwheel to the right see mouse_wheel_up.*/
	virtual void mouse_wheel_right(tevent_handler&, bool&) {}

	/***** ***** ***** ***** focus ***** ***** ***** *****/

	/**
	 * The widget receives a focus event.
	 *
	 * Container classes are notified when a child item gets a mouse down
	 * event. This can be used to capture the keyboard.
	 */
	virtual void focus(tevent_handler&) {}

	/***** ***** ***** ***** keyboard ***** ***** ***** *****/

	/**
	 * Called when a widget receives the keyboard focus.
	 *
	 * @todo add to the event schedule, first lose then receive.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void receive_keyboard_focus(tevent_handler& /*event_handler*/) {}

	/**
	 * Called when a widget loses the keyboard focus.
	 *
	 * @todo add to the event schedule, first lose then receive.
	 *
	 * @param event_handler       The event handler that send the event, most
	 *                            of the time the widget will be the one
	 *                            receiving the focus, but that's not
	 *                            guaranteed.
	 */
	virtual void lose_keyboard_focus(tevent_handler& /*event_handler*/) {}

	/**
	 * A key is pressed.
	 *
	 * When a key is pressed it's send to the widget that has the focus, if this
	 * widget doesn't handle the key it is send to the next handler. Some keys
	 * might get captured before send to the widget (eg F1).
	 *
	 * @param event_handler       The event handler that send the event.
	 * @param handled             Do we handle the event.
	 * @param key                 The SDL key code, needed for special keys.
	 * @param modifier            The keyboard modifiers when the key was
	 *                            pressed.
	 * @param unicode             The unicode for the pressed key.
	 */
	virtual void key_press(tevent_handler& /*event_handler*/, bool& /*handled*/,
		SDLKey /*key*/, SDLMod /*modifier*/, Uint16 /*unicode*/) {}

	/**
	 * The F1 key was pressed.
	 *
	 * This event is special since we normally want a help when this key is
	 * pressed.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void help_key(tevent_handler&) {}

	/***** ***** ***** ***** window management ***** ***** ***** *****/

	/**
	 * The window is resized.
	 *
	 * @param event_handler       The event handler that send the event.
	 * @param new_width           Width of the application window after resizing.
	 * @param new_height          Height of the application window after
	 *                            resizing.
	 */
	virtual void window_resize(tevent_handler&, const unsigned /* new_width */,
		const unsigned /* new_height */) {}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_wants_mouse_hover(const bool hover = true)
		{ wants_mouse_hover_ = hover; }
	bool wants_mouse_hover() const { return wants_mouse_hover_; }

	void set_wants_mouse_left_double_click(const bool click = true)
		{ wants_mouse_left_double_click_ = click; }
	bool wants_mouse_left_double_click() const
		{ return wants_mouse_left_double_click_; }

	void set_wants_mouse_middle_double_click(const bool click = true)
		{ wants_mouse_middle_double_click_ = click; }
	bool wants_mouse_middle_double_click() const
		{ return wants_mouse_middle_double_click_; }

	tevent_executor& set_wants_mouse_right_double_click(const bool click = true)
		{ wants_mouse_right_double_click_ = click; return *this; }
	bool wants_mouse_right_double_click() const
		{ return wants_mouse_right_double_click_; }

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
