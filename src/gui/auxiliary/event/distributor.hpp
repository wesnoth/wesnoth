/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_AUXILIARY_EVENT_DISTRIBUTOR_HPP_INCLUDED
#define GUI_WIDGETS_AUXILIARY_EVENT_DISTRIBUTOR_HPP_INCLUDED

/**
 * @file gui/auxiliary/event/distributor.hpp
 * Contains the event distributor.
 *
 * The event distributor exists of several classes which are combined in one
 * templated tdistributor class. The classes are closly tight together.
 *
 * All classes have direct access to eachothers members since they should act
 * as one. (Since the buttons are a templated subclass it's not possible to use
 * private subclasses.)
 *
 * The tmouse_motion class handles the mouse motion and holds the owner of us
 * since all classes virtually inherit us.
 *
 * The tmouse_button classes are templated classes per mouse button, the
 * template parameters are used to make the difference between the mouse
 * buttons. Althought it's easily possible to add more mouse buttons in the
 * code several places only expect a left, middle and right button.
 *
 * tdistributor is the main class to be used in the user code. This class
 * contains the handling of the keyboard as well.
 *
 * @todo Finish hovering and tooltips.
 *
 * The code for these functions is available but commented out so that can be
 * used as example for how to implement it.
 */

#include "gui/auxiliary/event/dispatcher.hpp"
#include "gui/widgets/event_executor.hpp"
#include "gui/widgets/helper.hpp"

class t_string;

namespace gui2{

class twidget;

namespace event {

/***** ***** ***** ***** tmouse_motion ***** ***** ***** ***** *****/

class tmouse_motion
{
public:

	tmouse_motion(twidget& owner, const tdispatcher::tposition queue_position);

	~tmouse_motion();

	/**
	 * Captures the mouse input.
	 *
	 * When capturing the widget that has the mouse focus_ does the capturing.
	 *
	 * @param capture             Set or release the capturing.
	 */
	void capture_mouse(//twidget* widget);
			const bool capture = true);
protected:

    /** The widget that currently has the mouse focus_. */
	twidget* mouse_focus_;

	/** Did the current widget capture the focus_? */
	bool mouse_captured_;

	/** The widget that owns us. */
	twidget& owner_;

	/** The timer for the hover event. */
	unsigned long hover_timer_;

	/** The widget which should get the hover event. */
	twidget* hover_widget_;

	/** The anchor point of the hover event. */
	tpoint hover_position_;

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
	void start_hover_timer(twidget* widget, const tpoint& coordinate);

	/** Stops the current hover timer. */
	void stop_hover_timer();

	/**
	 * Called when the mouse enters a widget.
	 *
	 * @param mouse_over          The widget that should receive the event.
	 */
	void mouse_enter(twidget* mouse_over);

	/** Called when the mouse leaves the current widget. */
	void mouse_leave();

private:

	/**
	 * Called when the mouse moves over a widget.
	 *
	 * @param mouse_over          The widget that should receive the event.
	 * @param coordinate          The current screen coordinate of the mouse.
	 */
	void mouse_motion(twidget* mouse_over, const tpoint& coordinate);

	bool signal_handler_sdl_mouse_motion_entered_;
	void signal_handler_sdl_mouse_motion(
			  const event::tevent event
			, bool& handled
			, const tpoint& coordinate);

	void signal_handler_show_hover_tooltip(const event::tevent event);

};

/***** ***** ***** ***** tmouse_button ***** ***** ***** ***** *****/

template<
		  tevent sdl_button_down
		, tevent sdl_button_up
		, tevent button_down
		, tevent button_up
		, tevent button_click
		, tevent button_double_click
>
class tmouse_button
	: public virtual tmouse_motion
{
public:
	tmouse_button(
			  const std::string& name_
			, twidget& owner
			, const tdispatcher::tposition queue_position
			);
protected:
	/** The time of the last click used for double clicking. */
	Uint32 last_click_stamp_;

	/** The widget the last click was on, used for double clicking. */
	twidget* last_clicked_widget_;

	/**
	 * If the mouse isn't captured we need to verify the up is on the same
	 * widget as the down so we send a proper click, also needed to send the
	 * up to the right widget.
	 */
	twidget* focus_;

private:
	/** used for debug messages. */
	const std::string name_;

	/** Is the button down? */
	bool is_down_;

	bool signal_handler_sdl_button_down_entered_;
	void signal_handler_sdl_button_down(
		  const event::tevent event
		, bool& handled
		, const tpoint& coordinate);

	bool signal_handler_sdl_button_up_entered_;
	void signal_handler_sdl_button_up(
		  const event::tevent event
		, bool& handled
		, const tpoint& coordinate);


	void mouse_button_click(twidget* widget);
};

/***** ***** ***** ***** tdistributor ***** ***** ***** ***** *****/

typedef	tmouse_button<
		  SDL_LEFT_BUTTON_DOWN
		, SDL_LEFT_BUTTON_UP
		, LEFT_BUTTON_DOWN
		, LEFT_BUTTON_UP
		, LEFT_BUTTON_CLICK
		, LEFT_BUTTON_DOUBLE_CLICK
		> tmouse_button_left;

typedef	tmouse_button<
		  SDL_MIDDLE_BUTTON_DOWN
		, SDL_MIDDLE_BUTTON_UP
		, MIDDLE_BUTTON_DOWN
		, MIDDLE_BUTTON_UP
		, MIDDLE_BUTTON_CLICK
		, MIDDLE_BUTTON_DOUBLE_CLICK
		> tmouse_button_middle;

typedef	tmouse_button<
		  SDL_RIGHT_BUTTON_DOWN
		, SDL_RIGHT_BUTTON_UP
		, RIGHT_BUTTON_DOWN
		, RIGHT_BUTTON_UP
		, RIGHT_BUTTON_CLICK
		, RIGHT_BUTTON_DOUBLE_CLICK
		> tmouse_button_right;


/** The event handler class for the widget library. */
class tdistributor
	: public tmouse_button_left
	, public tmouse_button_middle
	, public tmouse_button_right
{
public:
	tdistributor(twidget& owner
			, const tdispatcher::tposition queue_position);

	~tdistributor();

	/**
	 * Captures the keyboard input.
	 *
	 * @param widget              The widget which should capture the keyboard.
	 *                            Sending NULL releases the capturing.
	 */
	void keyboard_capture(twidget* widget);

	/**
	 * Adds the widget to the keyboard chain.
	 *
	 * @param widget              The widget to add to the chain. The widget
	 *                            should be valid widget, which hasn't been
	 *                            added to the chain yet.
	 */
	void keyboard_add_to_chain(twidget* widget);

	/**
	 * Remove the widget from the keyborad chain.
	 *
	 * @parameter widget          The widget to be removed from the chain.
	 */
	void keyboard_remove_from_chain(twidget* widget);

	/**
	 * Shows a tooltip.
	 *
	 * A tooltip is a small shortly visible item which is meant to show the user
	 * extra information. It shows after a short time hovering over a widget and
	 * automatically disappears again after a while. Only one tooltip or help
	 * message can be active at a time.
	 *
	 * @param message             The message to show.
	 * @param timeout             The time the tooltip is shown, 0 means
	 *                            forever.
	 */
	void show_tooltip(const t_string& message, const unsigned timeout);

	/** Removes the currently shown tooltip. */
	void remove_tooltip();

	/**
	 * Shows a help message.
	 *
	 * A help message is like a tooltip, but in general contains more info and
	 * the user needs to trigger it (most of the time with the F1 button).
	 *
	 * @param message             The message to show.
	 * @param timeout             The time the help message is shown, 0 means
	 *                            forever.
	 */
	void show_help_popup(const t_string& message, const unsigned timeout);

	/** Removes the currently show tooltip. */
	void remove_help_popup();

private:

	bool hover_pending_;			   /**< Is there a hover event pending? */
	unsigned hover_id_;                /**< Id of the pending hover event. */
	SDL_Rect hover_box_;               /**< The area the mouse can move in,
										*   moving outside invalidates the
										*   pending hover event.
										*/

	bool had_hover_;                   /**< A widget only gets one hover event
	                                    *   per enter cycle.
										*/

	/** The widget of the currently active tooltip. */
	twidget* tooltip_;

	/** The widget of the currently active help popup. */
	twidget* help_popup_;

	/** The widget that holds the keyboard focus_. */
	twidget* keyboard_focus_;

	/**
	 * Fall back keyboard focus_ items.
	 *
	 * When the focussed widget didn't handle the keyboard event (or no handler
	 * for the keyboard focus_) it is send all widgets in this vector. The order
	 * is from rbegin() to rend().  If the keyboard_focus_ is in the vector it
	 * won't get the event twice. The first item added to the vector should be
	 * the window, so it will be the last handler and can dispatch the hotkeys
	 * registered.
	 */
	std::vector<twidget*> keyboard_focus_chain_;

#if 0
	/**
	 * Raises a hover request.
	 *
	 * @param test_on_widget      Do we need to test whether we're on a widget.
	 */
	void set_hover(const bool test_on_widget = false);

	/**
	 * The function to do the real job of showing the tooltip.
	 *
	 * @param location            The location in the window where to show the
	 *                            tooltip.
	 * @param tooltip             The message to show.
	 */
	virtual void do_show_tooltip(
		const tpoint& location, const t_string& tooltip) = 0;

	/** Function to do the real removal of the tooltip. */
	virtual void do_remove_tooltip() = 0;

	/**
	 * The function to do the real job of showing the help popup.
	 *
	 * @param location            The location in the window where to show the
	 *                            help popup.
	 * @param help_popup          The message to show.
	 */
	virtual void do_show_help_popup(
		const tpoint& location, const t_string& help_popup) = 0;

	/** Function to do the real removal of the help popup. */
	virtual void do_remove_help_popup() = 0;

	/**
	 * Handler for the click dismiss functionallity.
	 *
	 * @returns                   True if the click dismiss action is performed,
	 *                            false otherwise.
	 */
	virtual bool click_dismiss() = 0;
#endif

	/**
	 * Set of functions that handle certain events and sends them to the proper
	 * widget. These functions are called by the SDL event handling functions.
	 */

	void signal_handler_sdl_key_down(const SDLKey key
			, const SDLMod modifier
			, const Uint16 unicode);


	template<tevent event>
	void signal_handler_sdl_wheel();

	void signal_handler_notify_removal(tdispatcher& widget, const tevent event);
};

} // namespace event

} // namespace gui2

#endif
