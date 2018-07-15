/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/scrollbar.hpp"

namespace gui2
{
namespace implementation
{
struct builder_vertical_scrollbar;
}

// ------------ WIDGET -----------{

/** A vertical scrollbar. */
class vertical_scrollbar : public scrollbar_base
{
	friend struct implementation::builder_vertical_scrollbar;

public:
	explicit vertical_scrollbar(const implementation::builder_vertical_scrollbar& builder);

private:
	/** Inherited from scrollbar_base. */
	virtual unsigned get_length() const override
	{
		return get_height();
	}

	/** Inherited from scrollbar_base. */
	virtual unsigned minimum_positioner_length() const override;

	/** Inherited from scrollbar_base. */
	virtual unsigned maximum_positioner_length() const override;

	/** Inherited from scrollbar_base. */
	virtual unsigned offset_before() const override;

	/** Inherited from scrollbar_base. */
	virtual unsigned offset_after() const override;

	/** Inherited from scrollbar_base. */
	virtual bool on_positioner(const point& coordinate) const override;

	/** Inherited from scrollbar_base. */
	virtual int on_bar(const point& coordinate) const override;

	/** Inherited from scrollbar_base. */
	virtual bool in_orthogonal_range(const point& coordinate) const override;

	/** Inherited from scrollbar_base. */
	virtual int get_length_difference(const point& original, const point& current) const override
	{
		return current.y - original.y;
	}

	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;
};

// }---------- DEFINITION ---------{

struct vertical_scrollbar_definition : public styled_widget_definition
{
	explicit vertical_scrollbar_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		unsigned minimum_positioner_length;
		unsigned maximum_positioner_length;

		unsigned top_offset;
		unsigned bottom_offset;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_vertical_scrollbar : public builder_styled_widget
{
	explicit builder_vertical_scrollbar(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
