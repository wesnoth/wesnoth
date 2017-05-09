/*
   Copyright (C) 2009 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/clickable_item.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

class repeating_button : public styled_widget, public clickable_item
{
public:
	repeating_button();
	~repeating_button();

	/**
	 * Connects a signal handler for a left mouse button down.
	 *
	 * This event is triggered when the button is pressed and, as long as the
	 * button stays down, every x ms afterwards.
	 *
	 * @param signal              The signal to connect.
	 */
	void connect_signal_mouse_left_down(const event::signal_function& signal);

	/**
	 * Disconnects a signal handler for a left mouse button down.
	 *
	 * @param signal              The signal to disconnect (should be the same
	 *                            as send to the connect call.
	 */
	void
	disconnect_signal_mouse_left_down(const event::signal_function& signal);

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref styled_widget::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

	/** Inherited from tclickable. */
	void connect_click_handler(const event::signal_function& signal) override
	{
		connect_signal_mouse_left_down(signal);
	}

	/** Inherited from tclickable. */
	void disconnect_click_handler(const event::signal_function& signal) override
	{
		disconnect_signal_mouse_left_down(signal);
	}

private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum state_t {
		ENABLED,
		DISABLED,
		PRESSED,
		FOCUSED,
		COUNT
	};

	void set_state(const state_t state);
	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	state_t state_;

	/** The timer for the repeating events. */
	size_t repeat_timer_;

	/** See @ref styled_widget::get_control_type. */
	virtual const std::string& get_control_type() const override;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_mouse_enter(const event::ui_event event, bool& handled);

	void signal_handler_mouse_leave(const event::ui_event event, bool& handled);

	void signal_handler_left_button_down(const event::ui_event event,
										 bool& handled);

	void signal_handler_left_button_up(const event::ui_event event,
									   bool& handled);
};

// }---------- DEFINITION ---------{

struct repeating_button_definition : public styled_widget_definition
{
	explicit repeating_button_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_repeating_button : public builder_styled_widget
{
public:
	explicit builder_repeating_button(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
