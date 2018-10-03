/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_values(const std::vector<::config>& values, unsigned selected = 0);

	void set_selected(unsigned selected, bool fire_event = true);

	/** Inherited from selectable_item */
	virtual unsigned get_value() const override { return selected_; }

	/** Inherited from selectable_item */
	virtual void set_value(unsigned value, bool fire_event = false) override { set_selected(value, fire_event); }

	/** Inherited from selectable_item */
	virtual unsigned num_states() const override { return values_.size(); }

	/** Returns the value of the selected row */
	std::string get_value_string() const
	{
		return values_[selected_]["label"];
	}

	/** Returns the entire config object for the selected row. */
	const ::config& get_value_config() const
	{
		return values_[selected_];
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

	std::vector<::config> values_;

	unsigned selected_;

	bool keep_open_;

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_mouse_enter(const event::ui_event event, bool& handled);

	void signal_handler_mouse_leave(const event::ui_event event, bool& handled);

	void signal_handler_left_button_down(const event::ui_event event, bool& handled);

	void signal_handler_left_button_up(const event::ui_event event, bool& handled);

	void signal_handler_left_button_click(const event::ui_event event, bool& handled);

	void signal_handler_sdl_wheel_up(const event::ui_event event, bool& handled);

	void signal_handler_sdl_wheel_down(const event::ui_event event, bool& handled);
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

	virtual widget_ptr build() const override;

private:
	std::vector<::config> options_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
