/* $Id$ */
/*
   Copyright (C) 2007 - 2008 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file event_handler.hpp
 * Contains the information with an event.
 */

#ifndef GUI_WIDGETS_EVENT_INFO_HPP_INCLUDED
#define GUI_WIDGETS_EVENT_INFO_HPP_INCLUDED

#include "events.hpp"
#include "gui/widgets/helper.hpp"
#include "gui/widgets/widget.hpp"

#include "SDL.h"

class t_string;

namespace gui2{

class twindow;

/** The event handler class for the widget library. */
class tevent_handler : public events::handler
{
public:
	tevent_handler();

	~tevent_handler() { leave(); }

	/** Inherited from events::handler. */
	void process_events() { events::pump(); }

	/** Inherited from events::handler. */
	void handle_event(const SDL_Event& event);

	/** Returns the main window. */
	virtual twindow& get_window() = 0;

	/** Returns the main window. */
	virtual const twindow& get_window() const = 0;

	/** See twidget::find_widget() for the description. */
	virtual twidget* find_widget(const tpoint& coordinate,
			const bool must_be_active) = 0;

	/** The const version of find_widget. */
	virtual const twidget* find_widget(const tpoint& coordinate,
		const bool must_be_active) const = 0;

	/**
	 * Captures the mouse input.
	 *
	 * When capturing the widget that has the mouse focus does the capturing.
	 *
	 * @param capture             Set or release the capturing.
	 */
	void mouse_capture(const bool capture = true);

	/**
	 * Captures the keyboard input.
	 *
	 * @param widget              The widget which should capture the keyboard.
	 *                            Sending NULL releases the capturing.
	 */
	void keyboard_capture(twidget* widget) { keyboard_focus_ = widget; }

	/**
	 * Adds the widget to the keyboard chain.
	 *
	 * @param widget              The widget to add to the chain. The widget
	 *                            should be valid widget, which hasn't been
	 *                            added to the chain yet.
	 */
	void add_to_keyboard_chain(twidget* widget);

	/**
	 * Remove the widget from the keyborad chain.
	 *
	 * @parameter widget          The widget to be removed from the chain.
	 */
	void remove_from_keyboard_chain(twidget* widget);

	/** Return the current mouse position. */
	tpoint get_mouse() const;

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

	/**
	 * A mouse button.
	 *
	 * The class tracks the state of the mouse button and which functions to
	 * invoke upon state changes.
	 * */
	struct tmouse_button {

		tmouse_button(const std::string& name,
			void (tevent_executor::*down) (tevent_handler&),
			void (tevent_executor::*up) (tevent_handler&),
			void (tevent_executor::*click) (tevent_handler&),
			void (tevent_executor::*double_click) (tevent_handler&),
			bool (tevent_executor::*wants_double_click) () const) :
				last_click_stamp(0),
				last_clicked_widget(NULL),
				focus(0),
				name(name),
				down(down),
				up(up),
				click(click),
				double_click(double_click),
				wants_double_click(wants_double_click),
				is_down(false)
			{}

		/** The time of the last click used for double clicking. */
		Uint32 last_click_stamp;

		/** The widget the last click was on, used for double clicking. */
		twidget* last_clicked_widget;

		/**
		 * If the mouse isn't captured we need to verify the up is on the same
		 * widget as the down so we send a proper click, also needed to send the
		 * up to the right widget.
		 */
		twidget* focus;

		/** used for debug messages. */
		const std::string name;

		/**
		 * Pointers to member functions, this way we can call the proper
		 * function indirect without writing a case for which button to
		 * use.
		 */
		void (tevent_executor::*down) (tevent_handler&);
		void (tevent_executor::*up) (tevent_handler&);
		void (tevent_executor::*click) (tevent_handler&);
		void (tevent_executor::*double_click) (tevent_handler&);
		bool (tevent_executor::*wants_double_click) () const;

		/** Is the button down? */
		bool is_down;
	};

	/**
	 * We create a new event context so we're always modal. Maybe this has to
	 * change, but not sure yet.
	 */
	events::event_context event_context_;

	int mouse_x_;                      /**< The current mouse x. */
	int mouse_y_;                      /**< The current mouse y. */

	tmouse_button left_;               /**< The left mouse button. */
	tmouse_button middle_;             /**< The middle mouse button. */
	tmouse_button right_;              /**< The right mouse button. */

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

    /** The widget that currently has the moue focus. */
	twidget* mouse_focus_;

	/** Did the current widget capture the focus? */
	bool mouse_captured_;

	/** The widget that holds the keyboard focus. */
	twidget* keyboard_focus_;

	/**
	 * Fall back keyboard focus items.
	 *
	 * When the focussed widget didn't handle the keyboard event (or no handler
	 * for the keyboard focus) it is send all widgets in this vector. The order
	 * is from rbegin() to rend().  If the keyboard_focus_ is in the vector it
	 * won't get the event twice. The first item added to the vector should be
	 * the window, so it will be the last handler and can dispatch the hotkeys
	 * registered.
	 */
	std::vector<twidget*> keyboard_focus_chain_;

	/**
	 * Set of functions that handle certain events and sends them to the proper
	 * widget. These functions are called by the SDL event handling functions.
	 */

	/**
	 * Called when the mouse enters a widget.
	 *
	 * @param event               The SDL_Event which was triggered.
	 * @param mouse_over          The widget that should receive the event.
	 */
	void mouse_enter(const SDL_Event& event, twidget* mouse_over);

	/**
	 * Called when the mouse moves over a widget.
	 *
	 * @param event               The SDL_Event which was triggered.
	 * @param mouse_over          The widget that should receive the event.
	 */
	void mouse_move(const SDL_Event& event, twidget* mouse_over);

	/**
	 * Called when a widget should raises a hover event.
	 *
	 * @param event               The SDL_Event which was triggered.
	 * @param mouse_over          The widget that should receive the event.
	 */
	void mouse_hover(const SDL_Event& event, twidget* mouse_over);

	/**
	 * Called when the mouse leaves a widget.
	 *
	 * @param event               The SDL_Event which was triggered.
	 * @param mouse_over          The widget that should receive the event.
	 */
	void mouse_leave(const SDL_Event& event, twidget* mouse_over);

	/**
	 * Called when a mouse button is pressed on a widget.
	 *
	 * @param event               The SDL_Event which was triggered.
	 * @param mouse_over          The widget that should receive the event. This
	 *                            widget can be NULL and capturing the mouse
	 *                            can send the event to another widget.
	 * @param button              The button which was used to generate the event.
	 */
	void mouse_button_down(
		const SDL_Event& event, twidget* mouse_over, tmouse_button& button);

	/**
	 * Called when a mouse button is released.
	 *
	 * @param event               The SDL_Event which was triggered.
	 * @param mouse_over          The widget that should receive the event. This
	 *                            widget can be NULL and capturing the mouse
	 *                            can send the event to another widget.
	 * @param button              The button which was used to generate the event.
	 */
	void mouse_button_up(
		const SDL_Event& event, twidget* mouse_over, tmouse_button& button);

	/**
	 * Called when a mouse click is generated.
	 *
	 * Note if the widget wants a double click a double click might be send or
	 * the click might be delayed to wait for a double click.
	 *
	 * @param widget              The widget that should receive the event.
	 * @param button              The button which was used to generate the event.
	 */
	void mouse_click(twidget* widget, tmouse_button& button);

	/**
	 * Raises a hover request.
	 *
	 * @param test_on_widget      Do we need to test whether we're on a widget.
	 */
	void set_hover(const bool test_on_widget = false);

	/**
	 * A key has been pressed.
	 *
	 * @param event               The SDL_Event which was triggered.
	 */
	void key_down(const SDL_Event& event);

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

	/** Handler for the easy close functionallity. */
	virtual void easy_close() = 0;
};

} // namespace gui2

#endif
