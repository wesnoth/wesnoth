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

#include "gui/auxiliary/widget_definition.hpp"
#include "gui/auxiliary/window_builder.hpp"

#include "gui/widgets/control.hpp"
#include "gui/widgets/selectable.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

/**
 * Simple push button.
 */
class tcombobox : public tcontrol, public tselectable_
{
public:
	tcombobox();

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref tcontrol::set_active. */
	virtual void set_active(const bool active) OVERRIDE;

	/** See @ref tcontrol::get_active. */
	virtual bool get_active() const OVERRIDE;

	/** See @ref tcontrol::get_state. */
	virtual unsigned get_state() const OVERRIDE;

	/** Inherited from tclickable. */
	void connect_click_handler(const event::tsignal_function& signal)
	{
		connect_signal_mouse_left_click(*this, signal);
	}

	/** Inherited from tclickable. */
	void disconnect_click_handler(const event::tsignal_function& signal)
	{
		disconnect_signal_mouse_left_click(*this, signal);
	}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_retval(const int retval)
	{
		retval_ = retval;
	}
	void set_values(const std::vector<std::string>& values, int selected = 0);
	void set_selected(int selected);
	
	/** See tselectable_::set_callback_state_change. */
	boost::function<void(twidget&)> callback_state_change_;

	/** Inherited from tselectable_ */
	virtual unsigned get_value() const OVERRIDE { return selected_; }

	/** Inherited from tselectable_ */
	virtual void set_value(const unsigned value ) OVERRIDE { set_selected(value); }

	/** Inherited from tselectable_ */
	virtual unsigned num_states() const OVERRIDE { return values_.size(); }

	/** Inherited from tselectable_ */
	virtual void set_callback_state_change(boost::function<void(twidget&)> callback)
	{
		selected_callback_ = callback;
	}

	/** Returns the value of the selected row */
	std::string get_value_string() const { return values_[selected_]; }

private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum tstate {
		ENABLED,
		DISABLED,
		PRESSED,
		FOCUSED,
		COUNT
	};

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
	 * If this value is not 0 and the button is clicked it sets the retval of
	 * the window and the window closes itself.
	 */
	int retval_;
	/**
	 */
	std::vector<std::string> values_;
	/**
	 */
	int selected_;

	boost::function<void(twidget&)> selected_callback_;

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const OVERRIDE;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_mouse_enter(const event::tevent event, bool& handled);

	void signal_handler_mouse_leave(const event::tevent event, bool& handled);

	void signal_handler_left_button_down(const event::tevent event,
										 bool& handled);

	void signal_handler_left_button_up(const event::tevent event,
									   bool& handled);

	void signal_handler_left_button_click(const event::tevent event,
										  bool& handled);
};

// }---------- DEFINITION ---------{

struct tcombobox_definition : public tcontrol_definition
{
	explicit tcombobox_definition(const config& cfg);

	struct tresolution : public tresolution_definition_
	{
		explicit tresolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{

class tcontrol;

namespace implementation
{

struct tbuilder_combobox : public tbuilder_control
{
public:
	explicit tbuilder_combobox(const config& cfg);

	using tbuilder_control::build;

	twidget* build() const;

private:
	std::string retval_id_;
	int retval_;
	std::vector<std::string> options_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
