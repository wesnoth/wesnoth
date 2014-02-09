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

#include "gui/widgets/spacer.hpp"

#include "gui/auxiliary/widget_definition/spacer.hpp"
#include "gui/auxiliary/window_builder/spacer.hpp"
#include "gui/widgets/detail/register.tpp"
#include "gui/widgets/settings.hpp"

#include <boost/bind.hpp>

namespace gui2
{

REGISTER_WIDGET(spacer)

tpoint tspacer::calculate_best_size() const
{
	return best_size_ != tpoint(0, 0) ? best_size_
									  : tcontrol::calculate_best_size();
}

void tspacer::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool tspacer::get_active() const
{
	return true;
}

unsigned tspacer::get_state() const
{
	return 0;
}

bool tspacer::disable_click_dismiss() const
{
	return false;
}

void tspacer::impl_draw_background(surface& /*frame_buffer*/)
{
	/* DO NOTHING */
}

void tspacer::impl_draw_background(surface& /*frame_buffer*/
								   ,
								   int /*x_offset*/
								   ,
								   int /*y_offset*/)
{
	/* DO NOTHING */
}

const std::string& tspacer::get_control_type() const
{
	static const std::string type = "spacer";
	return type;
}

} // namespace gui2
