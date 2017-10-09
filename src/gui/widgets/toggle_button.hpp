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

#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/selectable_item.hpp"

namespace gui2
{
namespace implementation
{
	struct builder_toggle_button;
}

// ------------ WIDGET -----------{

/**
 * Class for a toggle button.
 *
 * A toggle button is a button with two states 'up' and 'down' or 'selected' and
 * 'deselected'. When the mouse is pressed on it the state changes.
 */
class toggle_button : public styled_widget, public selectable_item
{
public:
	explicit toggle_button(const implementation::builder_toggle_button& builder);

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref styled_widget::set_members. */
	void set_members(const string_map& data) override;

	/** See @ref styled_widget::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

	/** Inherited from styled_widget. */
	void update_canvas() override;

	/** Inherited from selectable_item */
	unsigned get_value() const override
	{
		return state_num_;
	}
	/** Inherited from selectable_item */
	unsigned num_states() const override;
	/** Inherited from selectable_item */
	void set_value(const unsigned selected) override;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_retval(const int retval);

	void set_icon_name(const std::string& icon_name)
	{
		icon_name_ = icon_name;
		update_canvas();
	}
	const std::string& icon_name() const
	{
		return icon_name_;
	}

	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 * Also note the internals do assume the order for 'up' and 'down' to be the
	 * same and also that 'up' is before 'down'. 'up' has no suffix, 'down' has
	 * the SELECTED suffix.
	 */
	enum state_t {
		ENABLED,
		DISABLED,
		FOCUSED,
		COUNT
	};

private:

	void set_state(const state_t state);

	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	state_t state_;
	/**
	 *	Usually 1 for selected and 0 for not selected, can also have higher values in tristate buttons.
	 */
	unsigned state_num_;
	/**
	 * The return value of the button.
	 *
	 * If this value is not 0 and the button is double clicked it sets the
	 * retval of the window and the window closes itself.
	 */
	int retval_;

	/**
	 * The toggle button can contain an icon next to the text.
	 * Maybe this will move the the styled_widget class if deemed needed.
	 */
	std::string icon_name_;

	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_mouse_enter(const event::ui_event event, bool& handled);

	void signal_handler_mouse_leave(const event::ui_event event, bool& handled);

	void signal_handler_left_button_click(const event::ui_event event,
										  bool& handled);

	void signal_handler_left_button_double_click(const event::ui_event event,
												 bool& handled);
};

// }---------- DEFINITION ---------{

struct toggle_button_definition : public styled_widget_definition
{
	explicit toggle_button_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_toggle_button : public builder_styled_widget
{
	explicit builder_toggle_button(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;

private:
	std::string icon_name_;
	std::string retval_id_;
	int retval_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
