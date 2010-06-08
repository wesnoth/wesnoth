/* $Id$ */
/*
 Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2
 or at your option any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file manager.cpp
 */

#include "manager.hpp"

#include "resources.hpp"
#include "arrow.hpp"

namespace wb {

manager::manager(): active_(false), move_arrow_(NULL)
{
}

void manager::apply_temp_modifiers()
{
	mapbuilder_.reset(new mapbuilder_visitor(*resources::units));
}
void manager::remove_temp_modifiers()
{
	mapbuilder_.reset();
}

void manager::set_route(const std::vector<map_location> &steps)
{
	route_ = steps;
	if (route_.size() > 1)
	{
		if (move_arrow_ == NULL)
		{
			display *screen = (display*) resources::screen;
			move_arrow_ = new arrow(screen);
			screen->add_arrow(*move_arrow_);
		}
		move_arrow_->set_path(route_);
	}
}

} // end namespace wb
