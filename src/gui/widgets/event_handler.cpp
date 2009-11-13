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

/**
 * @file event_handler.cpp
 * Implementation of event_handler.hpp.
 *
 * More documentation at the end of the file.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

#ifdef GUI2_OLD_EVENT_DISPATCHER

#include "clipboard.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window.hpp"

#include <boost/bind.hpp>

namespace gui2{

/**
 * SDL_AddTimer() callback for the hover event.
 *
 * When this callback is called it pushes a new hover event in the event queue.
 *
 * @param interval                The time parameter of SDL_AddTimer.
 * @param param                   Pointer to parameter structure.
 *
 * @returns                       The new timer interval, 0 to stop.
 */
static Uint32 hover_callback(Uint32 /*interval*/, void *param)
{
	DBG_GUI_E << "Pushing hover event in queue.\n";

	SDL_Event event;
	SDL_UserEvent data;

	data.type = HOVER_EVENT;
	data.code = 0;
	data.data1 = param;
	data.data2 = 0;

	event.type = HOVER_EVENT;
	event.user = data;

	SDL_PushEvent(&event);
	return 0;
}

/**
 * SDL_AddTimer() callback for the popup event.
 *
 * This event makes sure the popup is removed again.
 *
 * @param interval                The time parameter of SDL_AddTimer.
 * @param param                   Pointer to parameter structure.
 *
 * @returns                       The new timer interval, 0 to stop.
 */
static Uint32 popup_callback(Uint32 /*interval*/, void* /*param*/)
{
	DBG_GUI_E << "Pushing popup removal event in queue.\n";

	SDL_Event event;
	SDL_UserEvent data;

	data.type = HOVER_REMOVE_POPUP_EVENT;
	data.code = 0;
	data.data1 = 0;
	data.data2 = 0;

	event.type = HOVER_REMOVE_POPUP_EVENT;
	event.user = data;

	SDL_PushEvent(&event);
	return 0;
}

/**
 * @todo At construction we should get the state and from that moment on we
 * keep track of the changes ourselves, not yet sure what happens when an input
 * blocker is used.
 */
tevent_handler::tevent_handler() :
	  mouse_x_(-1)
	, mouse_y_(-1)
	, left_("left"
			, &tevent_executor::wants_mouse_left_double_click
			, event::SDL_LEFT_BUTTON_DOWN
			, event::SDL_LEFT_BUTTON_UP
			, event::LEFT_BUTTON_DOWN
			, event::LEFT_BUTTON_UP
			, event::LEFT_BUTTON_CLICK
			, event::LEFT_BUTTON_DOUBLE_CLICK)
	, middle_("middle"
			, &tevent_executor::wants_mouse_middle_double_click
			, event::SDL_MIDDLE_BUTTON_DOWN
			, event::SDL_MIDDLE_BUTTON_UP
			, event::MIDDLE_BUTTON_DOWN
			, event::MIDDLE_BUTTON_UP
			, event::MIDDLE_BUTTON_CLICK
			, event::MIDDLE_BUTTON_DOUBLE_CLICK)
	, right_("right"
			, &tevent_executor::wants_mouse_right_double_click
			, event::SDL_RIGHT_BUTTON_DOWN
			, event::SDL_RIGHT_BUTTON_UP
			, event::RIGHT_BUTTON_DOWN
			, event::RIGHT_BUTTON_UP
			, event::RIGHT_BUTTON_CLICK
			, event::RIGHT_BUTTON_DOUBLE_CLICK)
	, hover_pending_(false)
	, hover_id_(0)
	, hover_box_()
	, had_hover_(false)
	, tooltip_(0)
	, help_popup_(0)
	, mouse_focus_(0)
	, mouse_captured_(false)
	, keyboard_focus_(0)
	, keyboard_focus_chain_()
{
	if(SDL_WasInit(SDL_INIT_TIMER) == 0) {
		if(SDL_InitSubSystem(SDL_INIT_TIMER) == -1) {
			assert(false);
		}
	}
}

void tevent_handler::mouse_capture(const bool capture)
{
	assert(mouse_focus_);
	mouse_captured_ = capture;
}

void tevent_handler::keyboard_capture(twidget* widget)
{
	if(keyboard_focus_) {
		get_window().fire(event::LOSE_KEYBOARD_FOCUS, *keyboard_focus_, NULL);
	}

	keyboard_focus_ = widget;

	if(keyboard_focus_) {
		get_window().fire(event::RECEIVE_KEYBOARD_FOCUS
				, *keyboard_focus_
				, NULL);
	}
}

tpoint tevent_handler::get_mouse() const
{
	return tpoint(mouse_x_, mouse_y_);
}

void tevent_handler::init_mouse_location()
{
	// Fix the mouse location by pushing a dummy event.
	int x;
	int y;
	SDL_GetMouseState(&x, &y);

	SDL_Event event;
	event.type = SDL_MOUSEMOTION;
	event.motion.type = SDL_MOUSEMOTION;
	event.motion.x = x;
	event.motion.y = y;

	x = SDL_PushEvent(&event);
}

void tevent_handler::add_to_keyboard_chain(twidget* widget)
{
	assert(widget);
	assert(
		std::find(keyboard_focus_chain_.begin(), keyboard_focus_chain_.end(), widget)
		== keyboard_focus_chain_.end());

	keyboard_focus_chain_.push_back(widget);
}

void tevent_handler::remove_from_keyboard_chain(twidget* widget)
{
	assert(widget);
	std::vector<twidget*>::iterator itor = std::find(
		keyboard_focus_chain_.begin(), keyboard_focus_chain_.end(), widget);

	if(itor != keyboard_focus_chain_.end()) {
		keyboard_focus_chain_.erase(itor);
	}
}

void tevent_handler::show_tooltip(const t_string& message, const unsigned timeout)
{
	DBG_GUI_E << "Event: show tooltip.\n";

	assert(!tooltip_);

	if(help_popup_) {
		remove_help_popup();
	}

	tooltip_ = mouse_focus_;

	do_show_tooltip(tpoint(mouse_x_, mouse_y_), message);

	if(timeout) {
		SDL_AddTimer(timeout, popup_callback, 0);
	}
}

void tevent_handler::remove_tooltip()
{
	if(!tooltip_) {
		return;
	}

	tooltip_ = 0;

	do_remove_tooltip();
}

void tevent_handler::show_help_popup(const t_string& message, const unsigned timeout)
{
	DBG_GUI_E << "Event: show help popup.\n";

	if(help_popup_) {
		DBG_GUI_E << "Help is already there, bailing out.\n";
		return;
	}

	if(tooltip_) {
		remove_tooltip();
	}

	// Kill hover events FIXME not documented.
	had_hover_ = true;
	hover_pending_ = false;

	help_popup_ = mouse_focus_;

	do_show_help_popup(tpoint(mouse_x_, mouse_y_), message);

	if(timeout) {
		SDL_AddTimer(timeout, popup_callback, 0);
	}
}

void tevent_handler::remove_help_popup()
{
	if(!help_popup_) {
		return;
	}

	help_popup_ = 0;

	do_remove_help_popup();
}

void tevent_handler::remove_widget(const twidget* widget)
{
	if(left_.last_clicked_widget == widget) left_.last_clicked_widget = NULL;
	if(left_.focus == widget) left_.focus = NULL;

	if(middle_.last_clicked_widget == widget)
		middle_.last_clicked_widget = NULL;
	if(middle_.focus == widget) middle_.focus = NULL;

	if(right_.last_clicked_widget == widget)
		right_.last_clicked_widget = NULL;
	if(right_.focus == widget) right_.focus = NULL;


	if(mouse_focus_ == widget) mouse_focus_ = NULL;
	if(keyboard_focus_ == widget) keyboard_focus_ = NULL;

	std::vector<twidget*>::iterator itor = std::find(
			  keyboard_focus_chain_.begin()
			, keyboard_focus_chain_.end()
			, widget);
	if(itor != keyboard_focus_chain_.end()) {
		keyboard_focus_chain_.erase(itor);
	}
}

void tevent_handler::mouse_enter(twidget* mouse_over)
{
	DBG_GUI_E << "tevent_handler: mouse enter.\n";

	assert(mouse_over);

	mouse_focus_ = mouse_over;
	get_window().fire(event::MOUSE_ENTER, *mouse_over);
}

void tevent_handler::mouse_motion(twidget* mouse_over, const tpoint& coordinate)
{
	DBG_GUI_E << "tevent_handler: mouse motion.\n";

	assert(mouse_over);

	get_window().fire(event::MOUSE_MOTION, *mouse_over, coordinate);
}

void tevent_handler::mouse_leave()
{
	DBG_GUI_E << "tevent_handler: mouse leave.\n";

	get_window().fire(event::MOUSE_LEAVE, *mouse_focus_);

	mouse_focus_ = 0;
}

void tevent_handler::set_hover(const bool test_on_widget)
{
	// Only one hover event.
	if(had_hover_) {
		return;
	}

	// Don't want a hover.
	if(!mouse_focus_ || !mouse_focus_->wants_mouse_hover()) {
		return;
	}

	// Have an hover and still in the bounding rect.
	if(hover_pending_ && point_in_rect(mouse_x_, mouse_y_, hover_box_)) {
		return;
	}

	// Mouse down, no hovering
	if(left_.is_down || middle_.is_down || right_.is_down) {
		return;
	}

	if(test_on_widget) {
		// FIXME implement
	}

	static unsigned hover_id = 0;

	hover_pending_ = true;
	// FIXME hover dimentions should be from the settings
	// also should check the entire box is on the widget???
	hover_box_ = ::create_rect(mouse_x_ - 5, mouse_y_ - 5, 10, 10);

	unsigned *hover = new unsigned;
	*hover = hover_id;
	hover_id_ = hover_id++;

	SDL_AddTimer(settings::popup_show_delay, hover_callback, hover);
}

void tevent_handler::connect_signals(twidget& w)
{
	w.connect_signal<event::SDL_MOUSE_MOTION>(
			boost::bind(&tevent_handler::sdl_mouse_motion, this, _2, _5));


	w.connect_signal<event::SDL_LEFT_BUTTON_DOWN>(
			boost::bind(&tevent_handler::sdl_left_button_down, this, _5));
	w.connect_signal<event::SDL_LEFT_BUTTON_UP>(
			boost::bind(&tevent_handler::sdl_left_button_up, this, _5));

	w.connect_signal<event::SDL_MIDDLE_BUTTON_DOWN>(
			boost::bind(&tevent_handler::sdl_middle_button_down, this, _5));
	w.connect_signal<event::SDL_MIDDLE_BUTTON_UP>(
			boost::bind(&tevent_handler::sdl_middle_button_up, this, _5));

	w.connect_signal<event::SDL_RIGHT_BUTTON_DOWN>(
			boost::bind(&tevent_handler::sdl_right_button_down, this, _5));
	w.connect_signal<event::SDL_RIGHT_BUTTON_UP>(
			boost::bind(&tevent_handler::sdl_right_button_up, this, _5));


	w.connect_signal<event::SDL_KEY_DOWN>(
			boost::bind(&tevent_handler::sdl_key_down, this, _5, _6, _7));


	w.connect_signal<event::SDL_WHEEL_UP>(
			boost::bind(&tevent_handler::sdl_wheel, this, _2));
	w.connect_signal<event::SDL_WHEEL_DOWN>(
			boost::bind(&tevent_handler::sdl_wheel, this, _2));
	w.connect_signal<event::SDL_WHEEL_LEFT>(
			boost::bind(&tevent_handler::sdl_wheel, this, _2));
	w.connect_signal<event::SDL_WHEEL_RIGHT>(
			boost::bind(&tevent_handler::sdl_wheel, this, _2));
}

void tevent_handler::sdl_mouse_motion(
		  const event::tevent event
		, const tpoint& coordinate)
{
	if(mouse_captured_) {
		assert(mouse_focus_);
		if(!get_window().fire(event, *mouse_focus_, coordinate)) {
			mouse_motion(mouse_focus_, coordinate);
		}
	} else {
		twidget* mouse_over = find_at(coordinate, true);
		if(mouse_over
				&& get_window().fire(event, *mouse_over, coordinate)) {

			return;
		}

		if(!mouse_focus_ && mouse_over) {
			mouse_enter(mouse_over);
		} else if (mouse_focus_ && !mouse_over) {
			mouse_leave();
		} else if(mouse_focus_ && mouse_focus_ == mouse_over) {
			mouse_motion(mouse_over, coordinate);
		} else if(mouse_focus_ && mouse_over) {
			// moved from one widget to the next
			mouse_leave();
			mouse_enter(mouse_over);
		} else {
			assert(!mouse_focus_ && !mouse_over);
		}
	}
}

void tevent_handler::sdl_left_button_down(const tpoint& coordinate)
{
	twidget* mouse_over = find_at(coordinate, true);
	button_down(mouse_over, left_, coordinate);
}

void tevent_handler::sdl_middle_button_down(const tpoint& coordinate)
{
	twidget* mouse_over = find_at(coordinate, true);
	button_down(mouse_over, middle_, coordinate);
}

void tevent_handler::sdl_right_button_down(const tpoint& coordinate)
{
	twidget* mouse_over = find_at(coordinate, true);
	button_down(mouse_over, right_, coordinate);
}

void tevent_handler::button_down(twidget* mouse_over
		, tmouse_button& button
		, const tpoint& coordinate)
{
	if(button.is_down) {
		WRN_GUI_E << "In 'button down' for button '" << button.name
			<< "' but the mouse button is already down, we missed an event.\n";
		return;
	}
	button.is_down = true;

	if(mouse_captured_) {
		assert(mouse_focus_);
		button.focus = mouse_focus_;
		if(!get_window().fire(
				button.sdl_button_down_, *button.focus, coordinate)) {

			get_window().fire(button.button_down_, *button.focus);
		}
	} else {
		if(!mouse_over) {
			return;
		}

		if(mouse_over != mouse_focus_) {
			WRN_GUI_E << "Mouse down event on non focussed widget "
				<< "and mouse not captured, we missed events.\n";
			mouse_focus_ = mouse_over;
		}

		button.focus = mouse_over;
		if(!get_window().fire(
				button.sdl_button_down_, *button.focus, coordinate)) {

			get_window().fire(button.button_down_, *button.focus);
		}
	}
}

void tevent_handler::sdl_left_button_up(const tpoint& coordinate)
{
	twidget* mouse_over = find_at(coordinate, true);
	button_up(mouse_over, left_, coordinate);
}

void tevent_handler::sdl_middle_button_up(const tpoint& coordinate)
{
	twidget* mouse_over = find_at(coordinate, true);
	button_up(mouse_over, middle_, coordinate);
}

void tevent_handler::sdl_right_button_up(const tpoint& coordinate)
{
	twidget* mouse_over = find_at(coordinate, true);
	button_up(mouse_over, right_, coordinate);
}

void tevent_handler::button_up(twidget* mouse_over
		, tmouse_button& button
		, const tpoint& coordinate)
{
	if(!button.is_down) {
		WRN_GUI_E << "In 'button up' for button '" << button.name
			<< "' but the mouse button is already up, we missed an event.\n";
		return;
	}

	button.is_down = false;
	if(button.focus) {
		if(!get_window().fire(
				button.sdl_button_up_, *button.focus, coordinate)) {

			get_window().fire(button.button_up_, *button.focus);
		}
	}

	if(mouse_captured_) {
		if (!left_.is_down && !middle_.is_down && !right_.is_down) {
			mouse_captured_ = false;
		}

		if(mouse_focus_ != mouse_over) {
			mouse_leave();

			if(mouse_over) {
				mouse_enter(mouse_over);
			}
		} else {
			mouse_button_click(mouse_focus_, button);
		}
	} else if(button.focus && button.focus == mouse_over) {
		mouse_button_click(button.focus, button);
	}

	button.focus = 0;
//	set_hover();
//	click_dismiss();
}

void tevent_handler::mouse_button_click(twidget* widget, tmouse_button& button)
{
	if((widget->*button.wants_double_click)()) {
		Uint32 stamp = SDL_GetTicks();
		if(button.last_click_stamp + settings::double_click_time >= stamp
				&& button.last_clicked_widget == widget) {

			get_window().fire(button.button_double_click_, *widget);
			button.last_click_stamp = 0;
			button.last_clicked_widget = NULL;

		} else {

			get_window().fire(button.button_click_, *widget);
			button.last_click_stamp = stamp;
			button.last_clicked_widget = widget;
		}

	} else {

		get_window().fire(button.button_click_, *widget);
	}
}

void tevent_handler::sdl_key_down(const SDLKey key
		, const SDLMod modifier
		, const Uint16 unicode)
{
	if(keyboard_focus_) {
		if(get_window().fire(event::SDL_KEY_DOWN
				, *keyboard_focus_, key, modifier, unicode)) {
			return;
		}
	}

	for(std::vector<twidget*>::reverse_iterator
				ritor = keyboard_focus_chain_.rbegin()
			; ritor != keyboard_focus_chain_.rend()
			; ++ritor) {

		if(*ritor == keyboard_focus_) {
			continue;
		}

		if(*ritor == dynamic_cast<twidget*>(this)) {
			/**
			 * @todo Make sure we're not in the event chain.
			 *
			 * No idea why we're here, but needs to be fixed, otherwise we keep
			 * calling this function recursively upon unhandled events...
			 */
			continue;
		}

		if(get_window().fire(event::SDL_KEY_DOWN
				, **ritor, key, modifier, unicode)) {

			return;
		}
	}
}

void tevent_handler::sdl_wheel(const event::tevent event)
{
	if(keyboard_focus_) {
		get_window().fire(event, *keyboard_focus_);
	}
}

/**
 * The event handling system.
 *
 * In this system there are two kind of focus
 * - mouse focus, the widget that will get the mouse events. There are two modes
 *   captured and uncaptured. Basically when a mouse button is pressed that
 *   widget will capture the mouse and all following mouse events are send to
 *   that widget.
 *
 * - keyboard focus, the widget that will get the keyboard events.
 *
 * Keyboard events are first processed by the top level window and if not
 * handled it will be send to the item with the keyboard focus (if any).
 *
 * * Mouse events *
 *
 * Note by button the X can be:
 * - left button
 * - middle button
 * - right button
 *
 * For the mouse the following events are defined:
 * - mouse enter, the mouse moves on a widget it wasn't on before.
 * - mouse move, the mouse moves. This can either be that the mouse moves over
 *   the widget under the mouse or it's send to the widget that captured the
 *   focus.
 * - mouse leave, the mouse leaves the area that bounds the mouse, in captured
 *   mode the release is delayed until the focus is released.
 * - hover, a widget needs to tell it wants this event. The event will be send
 *   when the user doesn't move (or just a little) for a while. (Times are
 *   themable.)
 *
 * - mouse_button_X_down, the mouse button is being pressed on this widget.
 * - mouse_button_X_up, the mouse button has been moved up again.
 * - mouse_button_X_click, a single click on a widget.
 * - mouse_button_X_double_click, a double click on the widget. The widget
 *   needs to subscribe to this event and when doing so a single click will be
 *   delayed a bit. (The double click time is themable.)
 *
 * * Mouse event flow chart *
 *
 *       --------------------
 *      ( mouse somewhere    )
 *       --------------------
 *                 |
 *  -------------->|
 * |               |
 * |               V
 * |     --------------------        --------------------
 * |    | moves upon widget  | -->  / fire mouse enter  /
 * |     --------------------      ---------------------
 * |                                        |
 * |                                        V
 * |                                        /\  Want hover event?
 * |                                    no /  \ yes     --------------------
 * |               ----------------------- \  / -----> / place hover event /
 * |              |                         \/         --------------------
 * |              |                                            |
 * |              |<--------------------------------------------
 * |              |
 * |              V
 * |     --------------------        --------------------
 * |    | moves on widget    | -->  / fire mouse move   /
 * |     --------------------      ---------------------
 * |                                        |
 * |                                        V
 * |                                        /\  Want hover event?
 * |                                    no /  \
 * |               ----------------------- \  /
 * |              |                         \/
 * |              |                         | yew
 * |              |                         V
 * |              |                         /\  Hover location outside threshold?
 * |              |                     no /  \ yes     --------------------
 * |              |<---------------------- \  / -----> / place hover event /
 * |              |                         \/         --------------------
 * |              |                                            |
 * |              |<--------------------------------------------
 * |              |
 * |              V
 * |     --------------------        --------------------
 * |    | receive hover event| -->  / set hover shown   /
 * |     --------------------      ---------------------
 * |                                        |
 * |                                        V
 * |                                 --------------------
 * |                                / fire hover event  /
 * |                               ---------------------
 * |                                        |
 * |               -------------------------
 * |              |
 * |              |--------------------------------------------------
 * |              V                                                  |
 * |     --------------------        --------------------            |
 * |    | moves off widget   | -->  / fire mouse leave  /            |
 * |     --------------------      ---------------------             |
 * |                                         |                       |
 * |                                         V                       |
 * |                                 ---------------------           |
 * |                                / cancel pending     /           |
 * |                               /  hover events      /            |
 * |                               ---------------------             |
 * |                                         |                       |
 * |                                         V                       |
 * |                                 --------------------            |
 * |                                / reset hover shown /            |
 * |                               ---------------------             |
 * |                                         |                       |
 * |<----------------------------------------                        |
 * |                                                                 |
 * |                                                                 |
 * |                                                                 |
 * |                                                                 |
 * |                                                                 |
 * |               --------------------------------------------------
 * |              |
 * |              V
 * |     --------------------        --------------------
 * |    | mouse down         | -->  / cancel pending    /
 * |     --------------------      /  hover events     /
 * |                               --------------------
 * |                                        |
 * |                                        V
 * |                                 --------------------
 * |                                / fire 'mouse down' /
 * |                               ---------------------
 * |                                        |
 * |               -------------------------
 * |              |
 * |              V
 * |     --------------------        --------------------
 * |    | moves on widget    | -->  / fire mouse move   /
 * |     --------------------      ---------------------
 * |                                        |
 * |               --------------------------------------------------
 * |              |                                                  |
 * |              V                                                  |
 * |     --------------------        --------------------            |
 * |    | moves off widget   | -->  / fire mouse move   /            |
 * |     --------------------      ---------------------             |
 * |                                        |                        |
 * |               -------------------------                         |
 * |              |                                                  |
 * |              V                                                  |
 * |     --------------------        --------------------            |
 * |    | moves up           | -->  / release capture   /            |
 * |     --------------------      ---------------------             |
 * |                                        |                        |
 * |                                        V                        |
 * |                                 --------------------            |
 * |                                / fire mouse up     /            |
 * |                               ---------------------             |
 * |                                        |                        |
 * |                                        V                        |
 * |                                 --------------------            |
 * |                                / fire mouse leave  /            |
 * |                               ---------------------             |
 * |                                         |                       |
 * |<----------------------------------------                        |
 * |                                                                 |
 * |                                                                 |
 * |                                                                 |
 * |                                                                 |
 * |                                                                 |
 * |               --------------------------------------------------
 * |              |
 * |              V
 * |     --------------------
 * |    | capture mouse      |
 * |     --------------------
 * |              |
 * |              V
 * |     --------------------        --------------------
 * |    | moves up           | -->  / release capture   /
 * |     --------------------      ---------------------
 * |                                        |
 * |                                        V
 * |                                 --------------------
 * |                                / fire mouse up     /
 * |                               ---------------------
 * |                                        |
 * |                                        V
 * |                                        /\
 * |                                       /  \ Want double click?
 * |                                       \  / no       --------------------
 * |                                        \/  ------> / fire click        /
 * |                                   yes  |          ---------------------
 * |                                        |                  |
 * |                                        V                   -----------------
 * |                                        /\                                   |
 * |                                       /  \ Has click pending?               |
 * |         -----------------------  yes  \  / no       --------------------    |
 * |        / remove click pending /<------ \/  ------> / set click pending /    |
 * |        -----------------------                    ---------------------     |
 * |                 |                                         |                 |
 * |                 V                                         V                 |
 * |         -----------------------                     -------------------     |
 * |        / fire double click    /                    / fire click       /     |
 * |       ------------------------                     -------------------      |
 * |                 |                                         |                 |
 * |                 V                                         V                 |
 *  -----------------------------------------------------------------------------
 *
 */

} // namespace gui2
#endif

