/*
	Copyright (C) 2008 - 2025
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

#pragma once

#include "gui/widgets/widget.hpp"

#include <string>

namespace gui2
{
class grid;

/**
 * Swaps an item in a grid for another one.
 */
void swap_grid(grid* g, grid* content_grid, std::unique_ptr<widget> widget, const std::string& id);
}
