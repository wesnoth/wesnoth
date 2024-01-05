/*
	Copyright (C) 2023 - 2023
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

#include "gui/widgets/integer_selector.hpp"
#include "gui/widgets/styled_widget.hpp"

#include "gui/auxiliary/typed_formula.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{
namespace implementation
{
struct builder_spinbox;
}

// ------------ WIDGET -----------{

/**
 * @ingroup GUIWidgetWML
 * A implementation of a spinner/spinbox widget that allows to enter numeric values
 * with buttons for increasing/decreasing the value.
 */

class spinbox : public styled_widget, public integer_selector
{

public:
	spinbox(const implementation::builder_spinbox& builder);

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref styled_widget::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

	/** Inherited from integer_selector. */
	virtual void set_value(int value) override
	{
		set_label(std::to_string(value));
	}

	/** Inherited from integer_selector. */
	virtual int get_value() const override
	{
		return std::stoi(get_label());
	}

	/** Inherited from integer_selector. */
	virtual int get_minimum_value() const override
	{
		return minimum_value_;
	}

	/** Inherited from integer_selector. */
	virtual int get_maximum_value() const override
	{
		// No max value for spinbox
		return std::stoi(get_label());
	}

	void prev()
	{
		// Allow negatives?
		if (get_value() > 0) {
			set_value(get_value() - step_size_);
		}
	}

	void next()
	{
		// No max value
		set_value(get_value() + step_size_);
	}


//protected:
	/** See @ref styled_widget::update_canvas. */
//	virtual void update_canvas() override;

private:

	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum state_t {
		ENABLED,
		DISABLED,
		FOCUSED,
		HOVERED,
	};

	void set_state(const state_t state);

	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	state_t state_;

	int minimum_value_, step_size_; // label is the current value

	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_sdl_key_down(const event::ui_event event, bool& handled, const SDL_Keycode key, SDL_Keymod /*modifier*/);

	void signal_handler_mouse_enter(const event::ui_event event, bool& handled);

	void signal_handler_mouse_leave(const event::ui_event event, bool& handled);

	void signal_handler_left_button_down(const event::ui_event event, bool& handled);

	void signal_handler_left_button_up(const event::ui_event event, bool& handled);

	void signal_handler_left_button_click(const event::ui_event event, bool& handled);
};

// }---------- DEFINITION ---------{

struct spinbox_definition : public styled_widget_definition
{
	explicit spinbox_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_spinbox : public builder_styled_widget
{
	explicit builder_spinbox(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
