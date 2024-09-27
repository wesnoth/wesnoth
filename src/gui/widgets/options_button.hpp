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

#include "gui/auxiliary/menu_item.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/dialogs/drop_down_menu.hpp"
#include "gui/widgets/styled_widget.hpp"

#include "boost/container/stable_vector.hpp"

namespace gui2
{
namespace implementation
{
	struct builder_options_button;
}

// ------------ WIDGET -----------{

class options_button : public styled_widget
{
public:
	explicit options_button(const implementation::builder_styled_widget& builder, const std::string& control_type);

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref styled_widget::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	virtual void set_values(const std::vector<::config>& values, unsigned selected = 0);
	virtual void set_values(const boost::container::stable_vector<menu_item>& values, unsigned selected = 0);

	virtual menu_item& add_row(config row, const int index = -1);

	menu_item* get_row(const int index);

	void remove_rows(const unsigned pos, const unsigned number = 1);

	void set_selected(unsigned selected, bool fire_event = true);

	/** Returns the value of the selected row */
	std::string get_value_string() const
	{
		return values_[selected_].label;
	}

	/** Returns the entire config object for the selected row. */
	const ::config get_value_config() const
	{
		return values_[selected_].get_config();
	}

	void set_keep_open(const bool keep_open)
	{
		keep_open_ = keep_open;
	}

	void set_persistent(bool persistent)
	{
       		persistent_ = persistent;
	}

	int get_item_count() const
	{
		return values_.size();
	}

    /**
     * Get the current state of the menu options.
     *
     * @returns        A mask specifying which options are selected
     */
	boost::dynamic_bitset<> get_toggle_states() const;

    /**
     * Deselect all the menu options.
     */
	void reset_toggle_states();

protected:
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

//protected:
	boost::container::stable_vector<menu_item> values_;

	/* toggle_states_ is used only to transfer checkbox values to and from the widget, and may
	 * not be up to date at any other time.  Use values_ for all other operations */
    boost::dynamic_bitset<> toggle_states_;

	dialogs::drop_down_menu* droplist_;

    void update_config_from_toggle_states();

	unsigned selected_;

	/* For widgets that update label depending on their value(s) */
	virtual void update_label() {}

private:
	bool keep_open_;

	/* Whether or not the item selected should be remembered if menu is re-opened */
	/* Does not apply to multimenu_button */
	bool persistent_;

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

	virtual void signal_handler_notify_changed();

};

// }---------- DEFINITION ---------{

struct options_button_definition : public styled_widget_definition
{
	explicit options_button_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_options_button : public builder_styled_widget
{
public:
	explicit builder_options_button(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

private:
	std::vector<::config> options_;

};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
