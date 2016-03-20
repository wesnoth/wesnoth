/*
   Copyright (C) 2009 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_GRID_PRIVATE_HPP_INCLUDED
#define GUI_WIDGETS_GRID_PRIVATE_HPP_INCLUDED

/**
 * @file
 * Helper for header for the grid.
 *
 * @note This file should only be included by grid.cpp.
 *
 * This file is being used for a small experiment in which some private
 * functions of tgrid are no longer in tgrid but moved in a friend class with
 * static functions. The goal is to have less header recompilations, when
 * there's a need to add or remove a private function.
 * Also non-trivial functions like 'const foo& bar() const' and 'foo& bar()'
 * are wrapped in a template to avoid code duplication (for typing not for the
 * binary) to make maintenance easier.
 */

#include "gui/widgets/grid.hpp"

#include "utils/const_clone.hpp"

#include <boost/foreach.hpp>

namespace gui2
{

/**
 * Helper to implement private functions without modifying the header.
 *
 * The class is a helper to avoid recompilation and only has static
 * functions.
 */
struct tgrid_implementation
{
	/**
	 * Implementation for the wrappers for
	 * [const] twidget* tgrid::find_at(const tpoint&, const bool) [const].
	 *
	 * @tparam W                  twidget or const twidget.
	 */
	template <class W>
	static W* find_at(typename utils::tconst_clone<tgrid, W>::reference grid,
					  const tpoint& coordinate,
					  const bool must_be_active)
	{
		typedef typename utils::tconst_clone<tgrid::tchild, W>::type hack;
		BOOST_FOREACH(hack & child, grid.children_)
		{

			W* widget = child.widget();
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
	 * [const] twidget* tgrid::find(const std::string&,
	 * const bool) [const].
	 *
	 * @tparam W                  twidget or const twidget.
	 */
	template <class W>
	static W* find(typename utils::tconst_clone<tgrid, W>::reference grid,
				   const std::string& id,
				   const bool must_be_active)
	{
		// Inherited.
		W* widget = grid.twidget::find(id, must_be_active);
		if(widget) {
			return widget;
		}

		typedef typename utils::tconst_clone<tgrid::tchild, W>::type hack;
		BOOST_FOREACH(hack & child, grid.children_)
		{

			widget = child.widget();
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
	static unsigned row_request_reduce_height(tgrid& grid,
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
	static unsigned column_request_reduce_width(tgrid& grid,
												const unsigned column,
												const unsigned maximum_width);

private:
	/**
	 * Helper function to do the resizing of a widget.
	 *
	 * @param child               The cell whose widget needs to be resized.
	 * @param maximum_height      The wanted maximum height.
	 */
	static void cell_request_reduce_height(tgrid::tchild& child,
										   const unsigned maximum_height);

	/**
	 * Helper function to do the resizing of a widget.
	 *
	 * @param child               The cell whose widget needs to be resized.
	 * @param maximum_width      The wanted maximum width.
	 */
	static void cell_request_reduce_width(tgrid::tchild& child,
										  const unsigned maximum_width);
};

} // namespace gui2

#endif
