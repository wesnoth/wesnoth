/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/widgets/widget_helpers.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/widget.hpp"

#include <cassert>

namespace gui2
{
void swap_grid(grid* g, grid* content_grid, widget* widget, const std::string& id)
{
	assert(content_grid);
	assert(widget);

	// Make sure the new child has same id.
	widget->set_id(id);

	// Get the container containing the wanted widget.
	grid* parent_grid = nullptr;
	if(g) {
		parent_grid = find_widget<grid>(g, id, false, false);
	}

	if(!parent_grid) {
		parent_grid = find_widget<grid>(content_grid, id, true, false);
	}

	parent_grid = dynamic_cast<grid*>(parent_grid->parent());
	assert(parent_grid);

	// Replace the child.
	auto old = parent_grid->swap_child(id, widget, false);
	assert(old);
}
}
