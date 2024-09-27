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



namespace gui2
{
namespace implementation
{
struct builder_multimenu_button;
}

// ------------ WIDGET -----------{

class multimenu_button : public styled_widget
{
public:
	explicit multimenu_button(const implementation::builder_multimenu_button& builder);

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref styled_widget::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	/**
	 * Sets the maximum number of selected elements shown on the label.
	 * If more are selected, the label will say "and N others".
	 *
	 * @param max      The maximum number of elements to show
	 */
	void set_max_shown(const unsigned max)
	{
		max_shown_ = max;
	}

	/**
	 * Get the maximum number of selected elements shown on the label.
	 *
	 * @returns        The maximum number of elements to show
	 */
	unsigned get_max_shown()
	{
		return max_shown_;
	}

	/**
	 * Get the number of options available in the menu
	 *
	 * @returns        The number of options in the menu
	 */
	unsigned num_options()
	{
		return values_.size();
	}

	/**
	 * Select an option in the menu
	 *
	 * @param option   The option to select
	 * @param selected True to select it, or false to deselect it
	 */
	void select_option(const unsigned option, const bool selected = true);

	/**
	 * Set the options selected in the menu.
	 *
	 * @param states   A mask specifying which options to select and deselect
	 */
	void select_options(const boost::dynamic_bitset<>& states);

	/**
	 * Set the available menu options.
	 *
	 * @param values   A list of options to show in the menu
	 */
	void set_values(const std::vector<::config>& values);

	/**
	 * Get the current state of the menu options.
	 *
	 * @returns        A mask specifying which options are selected
	 */
	boost::dynamic_bitset<> get_toggle_states() const
	{
		return toggle_states_;
	}

	/**
	 * Deselect all the menu options.
	 */
	void reset_toggle_states();

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
	 * The maximum number of selected states to list in the label
	 */
	unsigned max_shown_;

	std::vector<::config> values_;

	boost::dynamic_bitset<> toggle_states_;

	dialogs::drop_down_menu* droplist_;

	void update_config_from_toggle_states();
	void update_label();

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

	void signal_handler_notify_changed();
};

// }---------- DEFINITION ---------{

struct multimenu_button_definition : public styled_widget_definition
{
	explicit multimenu_button_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{


namespace implementation
{

struct builder_multimenu_button : public builder_styled_widget
{
public:
	explicit builder_multimenu_button(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

private:
	unsigned max_shown_;
	std::vector<::config> options_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
