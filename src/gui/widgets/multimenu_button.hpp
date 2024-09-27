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


namespace gui2
{
namespace implementation
{
struct builder_multimenu_button;
}

// ------------ WIDGET -----------{

class multimenu_button : public options_button
{
public:
	explicit multimenu_button(const implementation::builder_multimenu_button& builder);

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
	unsigned get_max_shown() const
	{
		return max_shown_;
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

	void update_label() override;

	menu_item& add_row(config row, const int index) override;

private:
	unsigned max_shown_;

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

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
