/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/widgets/horizontal_scrollbar.hpp"

namespace gui2 {

unsigned thorizontal_scrollbar::minimum_positioner_length() const
{
	boost::intrusive_ptr
		<const thorizontal_scrollbar_definition::tresolution> conf =
		boost::dynamic_pointer_cast
		<const thorizontal_scrollbar_definition::tresolution>(config());

	assert(conf);
	return conf->minimum_positioner_length;
}

unsigned thorizontal_scrollbar::maximum_positioner_length() const
{
	boost::intrusive_ptr
		<const thorizontal_scrollbar_definition::tresolution> conf =
		boost::dynamic_pointer_cast
		<const thorizontal_scrollbar_definition::tresolution>(config());

	assert(conf);
	return conf->maximum_positioner_length;
}

unsigned thorizontal_scrollbar::offset_before() const
{
	boost::intrusive_ptr
		<const thorizontal_scrollbar_definition::tresolution> conf =
		boost::dynamic_pointer_cast
		<const thorizontal_scrollbar_definition::tresolution>(config());

	assert(conf);
	return conf->left_offset;
}

unsigned thorizontal_scrollbar::offset_after() const
{
	boost::intrusive_ptr
		<const thorizontal_scrollbar_definition::tresolution> conf =
		boost::dynamic_pointer_cast
		<const thorizontal_scrollbar_definition::tresolution>(config());
	assert(conf);

	return conf->right_offset;
}

bool thorizontal_scrollbar::on_positioner(const tpoint& coordinate) const
{
	// Note we assume the positioner is over the entire height of the widget.
	return coordinate.x >= static_cast<int>(get_positioner_offset())
		&& coordinate.x < static_cast<int>(get_positioner_offset()
				+ get_positioner_length())
		&& coordinate.y > 0
		&& coordinate.y < static_cast<int>(get_height());
}

int thorizontal_scrollbar::on_bar(const tpoint& coordinate) const
{
	// Not on the widget, leave.
	if(static_cast<size_t>(coordinate.x) > get_width()
			|| static_cast<size_t>(coordinate.y) > get_height()) {
		return 0;
	}

	// we also assume the bar is over the entire width of the widget.
	if(static_cast<size_t>(coordinate.x) < get_positioner_offset()) {
		return -1;
	} else if(static_cast<size_t>(coordinate.x) >
			get_positioner_offset() + get_positioner_length()) {

		return 1;
	} else {
		return 0;
	}
}

} // namespace gui2

