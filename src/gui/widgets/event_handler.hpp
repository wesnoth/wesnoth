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

//! @file event_handler.hpp
//! Contains the information with an event.

#ifndef GUI_WIDGETS_EVENT_INFO_HPP_INCLUDED
#define GUI_WIDGETS_EVENT_INFO_HPP_INCLUDED

#include "events.hpp"
#include "gui/widgets/helper.hpp"
#include "gui/widgets/widget.hpp"

#include "SDL.h"

class t_string;

namespace gui2{

class twindow;

class tevent_handler : public events::handler
{
public:
	tevent_handler();

	virtual ~tevent_handler() { leave(); }

	void process_events() { events::pump(); }

	//! Implement events::handler::handle_event().
	void handle_event(const SDL_Event& event);

	virtual twindow& get_window() = 0;
	virtual const twindow& get_window() const = 0;

	/** See twidget::find_widget() for the description. */
	virtual twidget* find_widget(const tpoint& coordinate, 
			const bool must_be_active) = 0;

	/** The const version of find_widget. */
	virtual const twidget* find_widget(const tpoint& coordinate, 
		const bool must_be_active) const = 0;

	void mouse_capture(const bool capture = true);
	void keyboard_capture(twidget* widget) { keyboard_focus_ = widget; }

	/** Adds the widget to the chain, widgets may only be added once. */
	void add_to_keyboard_chain(twidget* widget);

	/** Remove the widget (if in the vector) from the chain. */
	void remove_from_keyboard_chain(twidget* widget);

	tpoint get_mouse() const;

	//! We impement the handling of the tip, but call the do functions
	//! which are virtual.
	void show_tooltip(const t_string& tooltip, const unsigned timeout);
	void remove_tooltip();
	void show_help_popup(const t_string& help_popup, const unsigned timeout);
	void remove_help_popup();

private:

	struct tmouse_button {
		
		tmouse_button(const std::string& name, 
			void (tevent_executor::*down) (tevent_handler&),
			void (tevent_executor::*up) (tevent_handler&),
			void (tevent_executor::*click) (tevent_handler&),
			void (tevent_executor::*double_click) (tevent_handler&),
			bool (tevent_executor::*wants_double_click) () const) :
				last_click_stamp(0),
				focus(0),
				name(name),
				down(down),
				up(up),
				click(click),
				double_click(double_click),
				wants_double_click(wants_double_click),
				is_down(false)
			{}

		//! The time of the last click used for double clicking.
		Uint32 last_click_stamp;

		//! If the mouse isn't captured we need to verify the up
		//! is on the same widget as the down so we send a proper
		//! click, also needed to send the up to the right widget.
		twidget* focus;

		//! used for debug messages.
		const std::string name;

		//! Pointers to member functions, this way we can call the proper
		//! function indirect without writing a case for which button to
		//! use.
		void (tevent_executor::*down) (tevent_handler&);
		void (tevent_executor::*up) (tevent_handler&);
		void (tevent_executor::*click) (tevent_handler&);
		void (tevent_executor::*double_click) (tevent_handler&);
		bool (tevent_executor::*wants_double_click) () const;

		//! Is the button down?
		bool is_down;
	};

	//! we create a new event context so we're always modal.
	//! Maybe this has to change, but not sure yet.
	events::event_context event_context_;

	int mouse_x_;                      //! The current mouse x.
	int mouse_y_;                      //! The current mouse y.

	tmouse_button left_;
	tmouse_button middle_;
	tmouse_button right_;

	bool hover_pending_;			   //! Is there a hover event pending?
	unsigned hover_id_;                //! Id of the pending hover event.
	SDL_Rect hover_box_;               //! The area the mouse can move in, moving outside
	                                   //! invalidates the pending hover event.
    bool had_hover_;                   //! A widget only gets one hover event per enter cycle. 

	//! The widget that created the tooltip / tooltip.
	twidget* tooltip_;
	twidget* help_popup_;


	twidget* mouse_focus_;
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

	void mouse_enter(const SDL_Event& event, twidget* mouse_over);
	void mouse_move(const SDL_Event& event, twidget* mouse_over);
	void mouse_hover(const SDL_Event& event, twidget* mouse_over);
	void mouse_leave(const SDL_Event& event, twidget* mouse_over);


	void mouse_button_down(const SDL_Event& event, twidget* mouse_over, tmouse_button& button);
	void mouse_button_up(const SDL_Event& event, twidget* mouse_over, tmouse_button& button);
	void mouse_click(twidget* widget, tmouse_button& button);

	void set_hover(const bool test_on_widget = false);

	void key_down(const SDL_Event& event);

	virtual void do_show_tooltip(const tpoint& location, const t_string& tooltip) = 0;
	virtual void do_remove_tooltip() = 0;
	virtual void do_show_help_popup(const tpoint& location, const t_string& help_popup) = 0;
	virtual void do_remove_help_popup() = 0;
};

} // namespace gui2

#endif
