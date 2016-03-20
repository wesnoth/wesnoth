/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_HORIZONTAL_SCROLLBAR_HPP_INCLUDED
#define GUI_WIDGETS_HORIZONTAL_SCROLLBAR_HPP_INCLUDED

#include "gui/widgets/scrollbar.hpp"

#include "gui/auxiliary/widget_definition.hpp"
#include "gui/auxiliary/window_builder.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

/** A horizontal scrollbar. */
class thorizontal_scrollbar : public tscrollbar_
{
public:
	thorizontal_scrollbar() : tscrollbar_()
	{
	}

private:
	/** Inherited from tscrollbar. */
	unsigned get_length() const
	{
		return get_width();
	}

	/** Inherited from tscrollbar. */
	unsigned minimum_positioner_length() const;

	/** Inherited from tscrollbar. */
	unsigned maximum_positioner_length() const;

	/** Inherited from tscrollbar. */
	unsigned offset_before() const;

	/** Inherited from tscrollbar. */
	unsigned offset_after() const;

	/** Inherited from tscrollbar. */
	bool on_positioner(const tpoint& coordinate) const;

	/** Inherited from tscrollbar. */
	int on_bar(const tpoint& coordinate) const;

	/** Inherited from tscrollbar. */
	bool in_orthogonal_range(const tpoint& coordinate) const;

	/** Inherited from tscrollbar. */
	int get_length_difference(const tpoint& original, const tpoint& current)
			const
	{
		return current.x - original.x;
	}

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const OVERRIDE;
};

// }---------- DEFINITION ---------{

struct thorizontal_scrollbar_definition : public tcontrol_definition
{
	explicit thorizontal_scrollbar_definition(const config& cfg);

	struct tresolution : public tresolution_definition_
	{
		explicit tresolution(const config& cfg);

		unsigned minimum_positioner_length;
		unsigned maximum_positioner_length;

		unsigned left_offset;
		unsigned right_offset;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct tbuilder_horizontal_scrollbar : public tbuilder_control
{
	explicit tbuilder_horizontal_scrollbar(const config& cfg);

	using tbuilder_control::build;

	twidget* build() const;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
