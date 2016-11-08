/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

#include "gui/widgets/control.hpp"
#include "gui/widgets/selectable.hpp"

class config;

namespace gui2
{

// ------------ WIDGET -----------{

/**
 * Simple push button.
 */
class menu_button : public control, public selectable_item
{
public:
	menu_button();

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref control::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref control::get_active. */
	virtual bool get_active() const override;

	/** See @ref control::get_state. */
	virtual unsigned get_state() const override;

	/** Inherited from tclickable. */
	void connect_click_handler(const event::signal_function& signal)
	{
		connect_signal_mouse_left_click(*this, signal);
	}

	/** Inherited from tclickable. */
	void disconnect_click_handler(const event::signal_function& signal)
	{
		disconnect_signal_mouse_left_click(*this, signal);
	}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_retval(const int retval)
	{
		retval_ = retval;
	}
	void set_values(const std::vector<::config>& values, int selected = 0);
	void set_selected(int selected);

	/** See selectable_item::set_callback_state_change. */
	std::function<void(widget&)> callback_state_change_;

	/** Inherited from selectable_item */
	virtual unsigned get_value() const override { return selected_; }

	/** Inherited from selectable_item */
	virtual void set_value(const unsigned value ) override { set_selected(value); }

	/** Inherited from selectable_item */
	virtual unsigned num_states() const override { return values_.size(); }

	/** Inherited from selectable_item */
	virtual void set_callback_state_change(std::function<void(widget&)> callback) override
	{
		callback_state_change_ = callback;
	}

	/** Returns the value of the selected row */
	std::string get_value_string() const { return values_[selected_]["label"]; }

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

	/**
	 * The return value of the button.
	 *
	 * If this value is not 0 and the button is clicked it sets the retval of
	 * the window and the window closes itself.
	 */
	int retval_;
	/**
	 */
	std::vector<::config> values_;
	/**
	 */
	int selected_;

	/** See @ref control::get_control_type. */
	virtual const std::string& get_control_type() const override;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_mouse_enter(const event::event_t event, bool& handled);

	void signal_handler_mouse_leave(const event::event_t event, bool& handled);

	void signal_handler_left_button_down(const event::event_t event,
										 bool& handled);

	void signal_handler_left_button_up(const event::event_t event,
									   bool& handled);

	void signal_handler_left_button_click(const event::event_t event,
										  bool& handled);
};

// }---------- DEFINITION ---------{

struct menu_button_definition : public control_definition
{
	explicit menu_button_definition(const config& cfg);

	struct tresolution : public resolution_definition
	{
		explicit tresolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{

class control;

namespace implementation
{

struct builder_menu_button : public builder_control
{
public:
	explicit builder_menu_button(const config& cfg);

	using builder_control::build;

	widget* build() const;

private:
	std::string retval_id_;
	int retval_;
	std::vector<::config> options_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
