/*
   Copyright (C) 2009 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_SCROLLBAR_PANEL_HPP_INCLUDED
#define GUI_WIDGETS_SCROLLBAR_PANEL_HPP_INCLUDED

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
 * Visible container to hold multiple widgets.
 *
 * This widget can draw items beyond the widgets it holds and in front of
 * them. A panel is always active so these functions return dummy values.
 */
class tscrollbar_panel : public tscrollbar_container
{
	friend struct implementation::builder_scrollbar_panel;

public:
	/**
	 * Constructor.
	 *
	 * @param canvas_count        The canvas count for tcontrol.
	 */
	explicit tscrollbar_panel(const unsigned canvas_count = 2)
		: tscrollbar_container(canvas_count)
	{
	}

	/** See @ref tcontrol::get_active. */
	virtual bool get_active() const override;

	/** See @ref tcontrol::get_state. */
	virtual unsigned get_state() const override;

private:
	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const override;

	/** See @ref tcontainer_::set_self_active. */
	virtual void set_self_active(const bool active) override;
};

// }---------- DEFINITION ---------{

struct scrollbar_panel_definition : public control_definition
{

	explicit scrollbar_panel_definition(const config& cfg);

	struct tresolution : public resolution_definition
	{
		explicit tresolution(const config& cfg);

		builder_grid_ptr grid;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_scrollbar_panel : public builder_control
{
	explicit builder_scrollbar_panel(const config& cfg);

	using builder_control::build;

	twidget* build() const;

	tscrollbar_container::tscrollbar_mode vertical_scrollbar_mode;
	tscrollbar_container::tscrollbar_mode horizontal_scrollbar_mode;

	builder_grid_ptr grid;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
