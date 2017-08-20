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

#include "gui/widgets/scrollbar.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{
namespace implementation
{
struct builder_horizontal_scrollbar;
}

// ------------ WIDGET -----------{

/** A horizontal scrollbar. */
class horizontal_scrollbar : public scrollbar_base
{
	friend struct implementation::builder_horizontal_scrollbar;

public:
	explicit horizontal_scrollbar(const implementation::builder_horizontal_scrollbar& builder);

private:
	/** Inherited from tscrollbar. */
	unsigned get_length() const override
	{
		return get_width();
	}

	/** Inherited from tscrollbar. */
	unsigned minimum_positioner_length() const override;

	/** Inherited from tscrollbar. */
	unsigned maximum_positioner_length() const override;

	/** Inherited from tscrollbar. */
	unsigned offset_before() const override;

	/** Inherited from tscrollbar. */
	unsigned offset_after() const override;

	/** Inherited from tscrollbar. */
	bool on_positioner(const point& coordinate) const override;

	/** Inherited from tscrollbar. */
	int on_bar(const point& coordinate) const override;

	/** Inherited from tscrollbar. */
	bool in_orthogonal_range(const point& coordinate) const override;

	/** Inherited from tscrollbar. */
	int get_length_difference(const point& original, const point& current) const override
	{
		return current.x - original.x;
	}

	/** See @ref styled_widget::get_control_type. */
	virtual const std::string& get_control_type() const override;
};

// }---------- DEFINITION ---------{

struct horizontal_scrollbar_definition : public styled_widget_definition
{
	explicit horizontal_scrollbar_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		unsigned minimum_positioner_length;
		unsigned maximum_positioner_length;

		unsigned left_offset;
		unsigned right_offset;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_horizontal_scrollbar : public builder_styled_widget
{
	explicit builder_horizontal_scrollbar(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
