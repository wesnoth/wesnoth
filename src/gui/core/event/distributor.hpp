/*
	Copyright (C) 2009 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

/**
 * @file
 * Contains the event distributor.
 *
 * The event distributor exists of several classes which are combined in one
 * templated distributor class. The classes are closely tight together.
 *
 * All classes have direct access to each others members since they should act
 * as one. (Since the buttons are a templated subclass it's not possible to use
 * private subclasses.)
 *
 * The mouse_motion class handles the mouse motion and holds the owner of us
 * since all classes virtually inherit us.
 *
 * The mouse_button classes are templated classes per mouse button, the
 * template parameters are used to make the difference between the mouse
 * buttons. Although it's easily possible to add more mouse buttons in the
 * code several places only expect a left, middle and right button.
 *
 * distributor is the main class to be used in the user code. This class
 * contains the handling of the keyboard as well.
 */

#include "gui/core/event/dispatcher.hpp"
#include "gui/core/event/handler.hpp"
#include "sdl/point.hpp"

#include <chrono>
#include <string>
#include <vector>

namespace gui2
{
class widget;

namespace event
{

/***** ***** ***** ***** mouse_motion ***** ***** ***** ***** *****/

class mouse_motion
{
public:
	mouse_motion(widget& owner, const dispatcher::queue_position queue_position);

	~mouse_motion();

	/**
	 * Captures the mouse input.
	 *
	 * When capturing the widget that has the mouse focus_ does the capturing.
	 *
	 * @param capture             Set or release the capturing.
	 */
	void capture_mouse(const bool capture = true);

protected:
	/** The widget that currently has the mouse focus_. */
	widget* mouse_focus_;

	/** Did the current widget capture the focus_? */
	bool mouse_captured_;

	/** The widget that owns us. */
	widget& owner_;

	/** The timer for the hover event. */
	std::size_t hover_timer_;

	/** The widget which should get the hover event. */
	widget* hover_widget_;

	/** The anchor point of the hover event. */
	point hover_position_;

	/**
	 * Has the hover been shown for the widget?
	 *
	 * A widget won't get a second hover event after the tooltip has been
	 * triggered. Only after (shortly) entering another widget it will be shown
	 * again for this widget.
	 */
	bool hover_shown_;

	/**
	 * Starts the hover timer.
	 *
	 * @param widget                 The widget that wants the tooltip.
	 * @param coordinate             The anchor coordinate.
	 */
	void start_hover_timer(widget* widget, const point& coordinate);

	/** Stops the current hover timer. */
	void stop_hover_timer();

	/**
	 * Called when the mouse enters a widget.
	 *
	 * @param mouse_over          The widget that should receive the event.
	 */
	void mouse_enter(widget* mouse_over);

	/** Called when the mouse leaves the current widget. */
	void mouse_leave();

private:
	/**
	 * Called when the mouse moves over a widget.
	 *
	 * @param mouse_over          The widget that should receive the event.
	 * @param coordinate          The current screen coordinate of the mouse.
	 */
	void mouse_hover(widget* mouse_over, const point& coordinate);

	/** Called when the mouse wants the widget to show its tooltip. */
	void show_tooltip();

	bool signal_handler_sdl_mouse_motion_entered_;
	void signal_handler_sdl_mouse_motion(const event::ui_event event, bool& handled, const point& coordinate);

	void signal_handler_sdl_touch_motion(
		const event::ui_event event, bool& handled, const point& coordinate, const point& distance);

	void signal_handler_sdl_wheel(const event::ui_event event, bool& handled, const point& coordinate);

	void signal_handler_show_helptip(const event::ui_event event, bool& handled, const point& coordinate);
};

/***** ***** ***** ***** mouse_button ***** ***** ***** ***** *****/

template<std::size_t I>
class mouse_button : public virtual mouse_motion
{
public:
	mouse_button(widget& owner, const dispatcher::queue_position queue_position);

	/**
	 * Initializes the state of the button.
	 *
	 * @param button_state The initial state of all buttons, in which the bit corresponding to
	 mouse_button_event_types.mask will be set if the button is down, or unset if it is up.
	 */
	void initialize_state(int32_t button_state);

protected:
	/** The time of the last click used for double clicking. */
	std::chrono::steady_clock::time_point last_click_stamp_;

	/** The widget the last click was on, used for double clicking. */
	widget* last_clicked_widget_;

	/**
	 * If the mouse isn't captured we need to verify the up is on the same
	 * widget as the down so we send a proper click, also needed to send the
	 * up to the right widget.
	 */
	widget* focus_;

private:
	/** Is the button down? */
	bool is_down_;

	bool signal_handler_sdl_button_down_entered_;
	void signal_handler_sdl_button_down(const event::ui_event event, bool& handled, const point& coordinate);

	bool signal_handler_sdl_button_up_entered_;
	void signal_handler_sdl_button_up(const event::ui_event event, bool& handled, const point& coordinate);

	void mouse_button_click(widget* widget);
};

/***** ***** ***** ***** distributor ***** ***** ***** ***** *****/

using mouse_button_left    = mouse_button<0>;
using mouse_button_middle  = mouse_button<1>;
using mouse_button_right   = mouse_button<2>;

/**
 * The event handler class for the widget library.
 *
 * C++ doesn't allow multiple inheritance to directly use more than one instance of a
 * superclass.
 *
 * It's a diamond inheritance, as all of these have virtual base class mouse_motion;
 * refactoring that would allow these multiple classes to be replaced with a simple
 * (distributor has-a std::array<mouse_button, 3>) relationship.
 */
class distributor :
	public mouse_button_left,
	public mouse_button_middle,
	public mouse_button_right
{
public:
	distributor(widget& owner, const dispatcher::queue_position queue_position);

	~distributor();

	/**
	 * Initializes the state of the keyboard and mouse.
	 *
	 * Needed after initialization and reactivation.
	 */
	void initialize_state();

	/**
	 * Captures the keyboard input.
	 *
	 * @param widget              The widget which should capture the keyboard.
	 *                            Sending nullptr releases the capturing.
	 */
	void keyboard_capture(widget* widget);

	/**
	 * Adds the widget to the keyboard chain.
	 *
	 * @param widget              The widget to add to the chain. The widget
	 *                            should be valid widget, which hasn't been
	 *                            added to the chain yet.
	 */
	void keyboard_add_to_chain(widget* widget);

	/**
	 * Remove the widget from the keyboard chain.
	 *
	 * @param widget              The widget to be removed from the chain.
	 */
	void keyboard_remove_from_chain(widget* widget);

	/**
	 * Return the widget currently capturing keyboard input.
	 */
	widget* keyboard_focus() const;

private:
	/** The widget that holds the keyboard focus_. */
	widget* keyboard_focus_;

	/**
	 * Fall back keyboard focus_ items.
	 *
	 * When the focused widget didn't handle the keyboard event (or no handler
	 * for the keyboard focus_) it is send all widgets in this vector. The order
	 * is from rbegin() to rend().  If the keyboard_focus_ is in the vector it
	 * won't get the event twice. The first item added to the vector should be
	 * the window, so it will be the last handler and can dispatch the hotkeys
	 * registered.
	 */
	std::vector<widget*> keyboard_focus_chain_;

	/**
	 * Set of functions that handle certain events and sends them to the proper
	 * widget. These functions are called by the SDL event handling functions.
	 */

	void signal_handler_sdl_key_down(const SDL_Keycode key,
									 const SDL_Keymod modifier,
									 const std::string& unicode);

	void signal_handler_sdl_text_input(const std::string& unicode, int32_t start, int32_t len);
	void signal_handler_sdl_text_editing(const std::string& unicode, int32_t start, int32_t len);

	template<typename Fcn, typename P1, typename P2, typename P3>
	void signal_handler_keyboard_internal(event::ui_event evt, P1&& p1, P2&& p2, P3&& p3);

	void signal_handler_notify_removal(dispatcher& widget, const ui_event event);
};

} // namespace event

} // namespace gui2
