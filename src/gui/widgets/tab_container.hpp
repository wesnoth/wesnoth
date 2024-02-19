/*
	Copyright (C) 2024
	by babaissarkar(Subhraman Sarkar) <suvrax@gmail.com>
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

#include "gui/widgets/container_base.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

namespace implementation
{
struct builder_tab_container;
}

/**
 * @ingroup GUIWidgetWML
 *
 * tab_container widget.
 *
 * A widget with a text_box and two button (named _prev and _next) that allows user to increase
 * or decrease the numeric value inside the text_box. Non-numeric values are considered as zero.
 *
 * Key          |Type                        |Default  |Description
 * -------------|----------------------------|---------|-----------
 * grid         | @ref guivartype_grid "grid"|mandatory|A grid containing the widgets for main widget.
 */

class tab_container : public container_base
{
	friend struct implementation::builder_tab_container;

public:
	explicit tab_container(const implementation::builder_tab_container& builder);

	/** See @ref container_base::set_self_active. */
	virtual void set_self_active(const bool active) override;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

	bool can_wrap() const override;

	void add_tab_entry(const widget_data row);

	void select_tab(unsigned index);
private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum state_t {
		ENABLED,
		DISABLED,
	};

	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	state_t state_;

	builder_grid_map builders_;
	std::vector<widget_data> list_items_;

	listbox& get_internal_list();

	void finalize_setup();

	void change_selection();

	void set_items(std::vector<widget_data> list_items)
	{
		list_items_ = list_items;
	}

	void set_builders(builder_grid_map builders) {
		builders_ = builders;
	}

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/***** ***** ***** inherited ****** *****/

	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

//	void place(const point& origin, const point& size);

};

// }---------- DEFINITION ---------{

struct tab_container_definition : public styled_widget_definition
{
	explicit tab_container_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		builder_grid_ptr grid;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_tab_container : public builder_styled_widget
{
	explicit builder_tab_container(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

	builder_grid_map builders;

	std::vector<widget_data> list_items;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
