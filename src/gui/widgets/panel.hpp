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

#include "gui/widgets/container_base.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{
namespace implementation
{
struct builder_panel;
}

// ------------ WIDGET -----------{

/**
 * @ingroup GUIWidgetWML
 *
 * A panel is a visible container to hold multiple widgets.
 * The difference between a grid and a panel is that it's possible to define how a panel looks.
 * A grid in an invisible container to just hold the items.
 * A panel is always enabled and can't be disabled.
 * Instead it uses the states as layers to draw on and can draw items beyond the widgets it holds and in front of them.
 * A panel is always active so some functions return dummy values.
 *
 * The widget instance has the following:
 * Key          |Type                        |Default  |Description
 * -------------|----------------------------|---------|-----------
 * grid         | @ref guivartype_grid "grid"|mandatory|Defines the grid with the widgets to place on the panel.
 *
 * The resolution for a panel also contains the following keys:
 * Key          |Type                                |Default|Description
 * -------------|------------------------------------|-------|-------------
 * top_border   | @ref guivartype_unsigned "unsigned"|0      |The size which isn't used for the client area.
 * bottom_border| @ref guivartype_unsigned "unsigned"|0      |The size which isn't used for the client area.
 * left_border  | @ref guivartype_unsigned "unsigned"|0      |The size which isn't used for the client area.
 * right_border | @ref guivartype_unsigned "unsigned"|0      |The size which isn't used for the client area.
 * The following layers exist:
 * * background - the background of the panel.
 * * foreground - the foreground of the panel.
 */
class panel : public container_base
{
public:
	/**
	 * Constructor.
	 */
	panel(const implementation::builder_styled_widget& builder, const std::string& control_type = "");

	/** See @ref container_base::get_client_rect. */
	virtual SDL_Rect get_client_rect() const override;

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

private:
	/** See @ref widget::impl_draw_background. */
	virtual bool impl_draw_background() override;

	/** See @ref widget::impl_draw_foreground. */
	virtual bool impl_draw_foreground() override;

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	/** See @ref container_base::border_space. */
	virtual point border_space() const override;

	/** See @ref container_base::set_self_active. */
	virtual void set_self_active(const bool active) override;
};

// }---------- DEFINITION ---------{

struct panel_definition : public styled_widget_definition
{
	explicit panel_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		unsigned top_border;
		unsigned bottom_border;

		unsigned left_border;
		unsigned right_border;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_panel : public builder_styled_widget
{
	explicit builder_panel(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

	builder_grid_ptr grid;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
