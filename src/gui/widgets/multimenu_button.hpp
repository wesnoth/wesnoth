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

// ------------ WIDGET -----------{

/**
 * Simple push button.
 */
class multimenu_button : public styled_widget
{
public:
	multimenu_button();

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

	/**
	 * Sets the maximum number of selected elements shown on the label.
	 * If more are selected, the label will say "and N others".
	 *
	 * @param max      The maximum number of elements to show
	 */
	void set_max_shown(const int max)
	{
		max_shown_ = max;
	}

	/**
	 * Get the maximum number of selected elements shown on the label.
	 *
	 * @returns        The maximum number of elements to show
	 */
	int get_max_shown()
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
	void select_options(boost::dynamic_bitset<> states);

	/**
	 * Set the available menu options.
	 *
	 * @param values   A list of options to show in the menu
	 */
	void set_values(const std::vector<::config>& values);

	/**
	 * Sets a callback that will be called immediately when any toggle button is selected or deselected.
	 */
	virtual void set_callback_toggle_state_change(std::function<void(boost::dynamic_bitset<>)> callback)
	{
		callback_toggle_state_change_ = callback;
	}

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
	 * The return value of the button.
	 *
	 * If this value is not 0 and the button is clicked it sets the retval of
	 * the window and the window closes itself.
	 */
	int retval_;

	/**
	 * The maximum number of selected states to list in the label
	 */
	int max_shown_;

	std::vector<::config> values_;

	boost::dynamic_bitset<> toggle_states_;

	dialogs::drop_down_menu* droplist_;

	std::function<void(boost::dynamic_bitset<>)> callback_toggle_state_change_;

	void update_config_from_toggle_states();
	void update_label();

	/** See @ref styled_widget::get_control_type. */
	virtual const std::string& get_control_type() const override;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_mouse_enter(const event::ui_event event, bool& handled);

	void signal_handler_mouse_leave(const event::ui_event event, bool& handled);

	void signal_handler_left_button_down(const event::ui_event event, bool& handled);

	void signal_handler_left_button_up(const event::ui_event event, bool& handled);

	void signal_handler_left_button_click(const event::ui_event event, bool& handled);

	void toggle_state_changed();
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

class styled_widget;

namespace implementation
{

struct builder_multimenu_button : public builder_styled_widget
{
public:
	explicit builder_multimenu_button(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;

private:
	std::string retval_id_;
	int retval_, max_shown_;
	std::vector<::config> options_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
