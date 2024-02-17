/*
	Copyright (C) 2008 - 2024
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

#include "gui/core/widget_definition.hpp"
#include "gui/dialogs/drop_down_menu.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/selectable_item.hpp"

namespace gui2
{
namespace implementation
{
	struct builder_menu_button;
}

// ------------ WIDGET -----------{

/**
 * @ingroup GUIWidgetWML
 *
 * A menu_button is a styled_widget to choose an element from a list of elements.
 *
 * When a menu_button has a return value it sets the return value for the window.
 * Normally this closes the window and returns this value to the caller.
 * The return value can either be defined by the user or determined from the id of the menu_button.
 * The return value has a higher precedence as the one defined by the id.
 * (Of course it's weird to give a menu_button an id and then override its return value.)
 *
 * When the menu_button doesn't have a standard id, but you still want to use the return value of that id, use return_value_id instead.
 * This has a higher precedence as return_value.
 *
 * List with the menu_button specific variables:
 * Key            |Type                                |Default  |Description
 * ---------------|------------------------------------|---------|-----------
 * return_value_id| @ref guivartype_string "string"    |""       |The return value id.
 * return_value   | @ref guivartype_int "int"          |0        |The return value.
 *
 * The following states exist:
 * * state_enabled - the menu_button is enabled.
 * * state_disabled - the menu_button is disabled.
 * * state_pressed - the left mouse menu_button is down.
 * * state_focused - the mouse is over the menu_button.
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

namespace implementation
{

struct builder_menu_button : public builder_styled_widget
{
public:
	explicit builder_menu_button(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

private:
	std::vector<::config> options_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
