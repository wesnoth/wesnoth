/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/drop_down_menu.hpp"

#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/selectable_item.hpp"

#include <boost/dynamic_bitset.hpp>

class config;

namespace gui2
{
namespace implementation
{
	struct builder_menu_button;
}

// ------------ WIDGET -----------{

/**
 * Simple push button.
 */
class menu_button : public styled_widget, public selectable_item
{
public:
	explicit menu_button(const implementation::builder_menu_button& builder);

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref styled_widget::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
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
	std::string get_value_string() const
	{
		return values_[selected_]["label"];
	}

	void set_keep_open(const bool keep_open)
	{
		keep_open_ = keep_open;
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

	std::vector<::config> values_;

	int selected_;

	bool keep_open_;

	dialogs::drop_down_menu* droplist_;

	/** See selectable_item::set_callback_state_change. */
	std::function<void(widget&)> callback_state_change_;

	/** See @ref styled_widget::get_control_type. */
	virtual const std::string& get_control_type() const override;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_mouse_enter(const event::ui_event event, bool& handled);

	void signal_handler_mouse_leave(const event::ui_event event, bool& handled);

	void signal_handler_left_button_down(const event::ui_event event, bool& handled);

	void signal_handler_left_button_up(const event::ui_event event, bool& handled);

	void signal_handler_left_button_click(const event::ui_event event, bool& handled);
};

// }---------- DEFINITION ---------{

struct menu_button_definition : public styled_widget_definition
{
	explicit menu_button_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{

class styled_widget;

namespace implementation
{

struct builder_menu_button : public builder_styled_widget
{
public:
	explicit builder_menu_button(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;

private:
	std::string retval_id_;
	int retval_;
	std::vector<::config> options_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
