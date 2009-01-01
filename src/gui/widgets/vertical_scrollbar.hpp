/* $Id$ */
/*
   copyright (C) 2008 - 2009 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#ifndef GUI_WIDGETS_VERTICAL_SCROLLBAR_HPP_INCLUDED
#define GUI_WIDGETS_VERTICAL_SCROLLBAR_HPP_INCLUDED

#include "gui/widgets/scrollbar.hpp"

namespace gui2 {

/** A vertical scrollbar. */
class tvertical_scrollbar : public tscrollbar_
{
public:

	tvertical_scrollbar() :
		tscrollbar_()
	{
	}

private:

	/** Inherited from tscrollbar. */
	unsigned get_length() const { return get_height(); }

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
	int get_length_difference(const tpoint& original, const tpoint& current) const
		{ return current.y - original.y; }

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const
		{ static const std::string type = "vertical_scrollbar"; return type; }
};

} // namespace gui2

#endif

