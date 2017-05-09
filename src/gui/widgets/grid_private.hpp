/*
   Copyright (C) 2009 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

/**
 * @file
 * Helper for header for the grid.
 *
 * @note This file should only be included by grid.cpp.
 *
 * This file is being used for a small experiment in which some private
 * functions of grid are no longer in grid but moved in a friend class with
 * static functions. The goal is to have less header recompilations, when
 * there's a need to add or remove a private function.
 * Also non-trivial functions like 'const foo& bar() const' and 'foo& bar()'
 * are wrapped in a template to avoid code duplication (for typing not for the
 * binary) to make maintenance easier.
 */

#include "gui/widgets/grid.hpp"

#include "utils/const_clone.hpp"

namespace gui2
{

/**
 * Helper to implement private functions without modifying the header.
 *
 * The class is a helper to avoid recompilation and only has static
 * functions.
 */
struct grid_implementation
{
	/**
	 * Implementation for the wrappers for
	 * [const] widget* grid::find_at(const point&, const bool) [const].
	 *
	 * @tparam W                  widget or const widget.
	 */
	template <class W>
	static W* find_at(typename utils::const_clone<grid, W>::reference grid,
					  const point& coordinate,
					  const bool must_be_active)
	{
		typedef typename utils::const_clone<grid::child, W>::type hack;
		for(hack & child : grid.children_)
		{

			W* widget = child.get_widget();
			if(!widget) {
				continue;
			}

			widget = widget->find_at(coordinate, must_be_active);
			if(widget) {
				return widget;
			}
		}

		return 0;
	}

	/**
	 * Implementation for the wrappers for
	 * [const] widget* grid::find(const std::string&,
	 * const bool) [const].
	 *
	 * @tparam W                  widget or const widget.
	 */
	template <class W>
	static W* find(typename utils::const_clone<grid, W>::reference grid,
				   const std::string& id,
				   const bool must_be_active)
	{
		// Inherited.
		W* widget = grid.widget::find(id, must_be_active);
		if(widget) {
			return widget;
		}

		typedef typename utils::const_clone<grid::child, W>::type hack;
		for(hack & child : grid.children_)
		{

			widget = child.get_widget();
			if(!widget) {
				continue;
			}

			widget = widget->find(id, must_be_active);
			if(widget) {
				return widget;
			}
		}

		return 0;
	}

	/**
	 * Helper function to do the resizing of a row.
	 *
	 * @param grid                The grid to operate upon.
	 * @param row                 The row to resize.
	 * @param maximum_height      The wanted maximum height.
	 *
	 * @returns                   The required row height after resizing.
	 */
	static unsigned row_request_reduce_height(grid& grid,
											  const unsigned row,
											  const unsigned maximum_height);

	/**
	 * Helper function to do the resizing of a column.
	 *
	 * @param grid                The grid to operate upon.
	 * @param column              The column to resize.
	 * @param maximum_width       The wanted maximum width.
	 *
	 * @returns                   The required column width after resizing.
	 */
	static unsigned column_request_reduce_width(grid& grid,
												const unsigned column,
												const unsigned maximum_width);

private:
	/**
	 * Helper function to do the resizing of a widget.
	 *
	 * @param child               The cell whose widget needs to be resized.
	 * @param maximum_height      The wanted maximum height.
	 */
	static void cell_request_reduce_height(grid::child& child,
										   const unsigned maximum_height);

	/**
	 * Helper function to do the resizing of a widget.
	 *
	 * @param child               The cell whose widget needs to be resized.
	 * @param maximum_width      The wanted maximum width.
	 */
	static void cell_request_reduce_width(grid::child& child,
										  const unsigned maximum_width);
};

} // namespace gui2
