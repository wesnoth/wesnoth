/*
	Copyright (C) 2009 - 2024
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

#include "gui/widgets/scrollbar_container.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

namespace implementation
{
struct builder_scrollbar_panel;
}

/**
 * @ingroup GUIWidgetWML
 *
 * Visible container to hold multiple widgets.
 *
 * This widget can draw items beyond the widgets it holds and in front of them.
 * A panel is always active so these functions return dummy values.
 *
 * A panel is a container holding other elements in its grid.
 * It uses the states as layers to draw on.
 *
 * Key          |Type                        |Default  |Description
 * -------------|----------------------------|---------|-----------
 * grid         | @ref guivartype_grid "grid"|mandatory|A grid containing the widgets for main widget.
 * The following layers exist:
 * * background - the background of the panel.
 * * foreground - the foreground of the panel.
 * List with the scrollbar_panel specific variables:
 * Key                      |Type                                            |Default     |Description
 * -------------------------|------------------------------------------------|------------|-----------
 * vertical_scrollbar_mode  | @ref guivartype_scrollbar_mode "scrollbar_mode"|initial_auto|Determines whether or not to show the scrollbar.
 * horizontal_scrollbar_mode| @ref guivartype_scrollbar_mode "scrollbar_mode"|initial_auto|Determines whether or not to show the scrollbar.
 * definition               | @ref guivartype_section "section"              |mandatory   |This defines how a scrollbar_panel item looks. It must contain the grid definition for 1 row of the list.
 */
class scrollbar_panel : public scrollbar_container
{
	friend struct implementation::builder_scrollbar_panel;

public:
	/**
	 * Constructor.
	 */
	explicit scrollbar_panel(const implementation::builder_scrollbar_panel& builder);

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	/** See @ref container_base::set_self_active. */
	virtual void set_self_active(const bool active) override;
};

// }---------- DEFINITION ---------{

struct scrollbar_panel_definition : public styled_widget_definition
{

	explicit scrollbar_panel_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		builder_grid_ptr grid;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_scrollbar_panel : public builder_styled_widget
{
	explicit builder_scrollbar_panel(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

	scrollbar_container::scrollbar_mode vertical_scrollbar_mode;
	scrollbar_container::scrollbar_mode horizontal_scrollbar_mode;

	builder_grid_ptr grid_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
