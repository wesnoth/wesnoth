/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "gui/widgets/widget_helpers.hpp"

#include "gui/widgets/grid.hpp"

#include <cassert>

namespace gui2
{
void swap_grid(grid* g, grid* content_grid, std::unique_ptr<widget> widget, const std::string& id)
{
	assert(content_grid);
	assert(widget);

	// Make sure the new child has same id.
	widget->set_id(id);

	// Get the container containing the wanted widget.
	grid* parent_grid = nullptr;
	if(g) {
		parent_grid = g->find_widget<grid>(id, false, false);
	}

	if(!parent_grid) {
		parent_grid = content_grid->find_widget<grid>(id, true, false);
	}

	parent_grid = dynamic_cast<grid*>(parent_grid->parent());
	assert(parent_grid);

	// Replace the child.
	auto old = parent_grid->swap_child(id, std::move(widget), false);
	assert(old);
}
}
