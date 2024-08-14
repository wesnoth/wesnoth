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
#include "gui/widgets/options_button.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/selectable_item.hpp"

namespace gui2
{
namespace implementation
{
	struct builder_menu_button;
}

// ------------ WIDGET -----------{

class menu_button : public options_button, public selectable_item
{
public:
	explicit menu_button(const implementation::builder_menu_button& builder);

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/
	/** Inherited from selectable_item */
	virtual unsigned get_value() const override { return selected_; }

	/** Inherited from selectable_item */
	virtual void set_value(unsigned value, bool fire_event = false) override { set_selected(value, fire_event); }

	/** Inherited from selectable_item */
	virtual unsigned num_states() const override { return values_.size(); }

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

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

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
