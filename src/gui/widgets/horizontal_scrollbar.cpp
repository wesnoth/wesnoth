/*
   Copyright (C) 2008 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/horizontal_scrollbar.hpp"

#include "gui/widgets/detail/register.tpp"
#include "gui/widgets/settings.hpp"
#include "gui/auxiliary/widget_definition/horizontal_scrollbar.hpp"
#include "gui/auxiliary/window_builder/horizontal_scrollbar.hpp"

#include <boost/bind.hpp>

namespace gui2
{

REGISTER_WIDGET(horizontal_scrollbar)

unsigned thorizontal_scrollbar::minimum_positioner_length() const
{
	boost::intrusive_ptr<const thorizontal_scrollbar_definition::tresolution>
	conf = boost::dynamic_pointer_cast<const thorizontal_scrollbar_definition::
											   tresolution>(config());

	assert(conf);
	return conf->minimum_positioner_length;
}

unsigned thorizontal_scrollbar::maximum_positioner_length() const
{
	boost::intrusive_ptr<const thorizontal_scrollbar_definition::tresolution>
	conf = boost::dynamic_pointer_cast<const thorizontal_scrollbar_definition::
											   tresolution>(config());

	assert(conf);
	return conf->maximum_positioner_length;
}

unsigned thorizontal_scrollbar::offset_before() const
{
	boost::intrusive_ptr<const thorizontal_scrollbar_definition::tresolution>
	conf = boost::dynamic_pointer_cast<const thorizontal_scrollbar_definition::
											   tresolution>(config());

	assert(conf);
	return conf->left_offset;
}

unsigned thorizontal_scrollbar::offset_after() const
{
	boost::intrusive_ptr<const thorizontal_scrollbar_definition::tresolution>
	conf = boost::dynamic_pointer_cast<const thorizontal_scrollbar_definition::
											   tresolution>(config());
	assert(conf);

	return conf->right_offset;
}

bool thorizontal_scrollbar::on_positioner(const tpoint& coordinate) const
{
	// Note we assume the positioner is over the entire height of the widget.
	return coordinate.x >= static_cast<int>(get_positioner_offset())
		   && coordinate.x < static_cast<int>(get_positioner_offset()
											  + get_positioner_length())
		   && coordinate.y > 0 && coordinate.y < static_cast<int>(get_height());
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
	} else if(static_cast<size_t>(coordinate.x) > get_positioner_offset()
												  + get_positioner_length()) {

		return 1;
	} else {
		return 0;
	}
}

const std::string& thorizontal_scrollbar::get_control_type() const
{
	static const std::string type = "horizontal_scrollbar";
	return type;
}

} // namespace gui2
