/* $Id$ */
/*
   Copyright (C) 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_GRID_PRIVATE_HPP_INCLUDED
#define GUI_WIDGETS_GRID_PRIVATE_HPP_INCLUDED

/**
 * @file gui/widgets/grid_private.hpp
 * Helper for header for the grid.
 *
 * @note This file should only be included by grid.cpp.
 *
 * This fils is being used for a small experiment in which some private
 * functions of tgrid are no longer in tgrid but moved in a friend class with
 * static functions. The goal is to have less header recompilations, when
 * there's a need to add or remove a private function.
 * Also non-trivial functions like 'const foo& bar() const' and 'foo& bar()'
 * are wrapped in a template to avoid code duplication (for typing not for the
 * binary) to make maintenance easier.
 */

#include "gui/widgets/grid.hpp"

#include "foreach.hpp"

namespace gui2 {

/**
 * Helper to implement private functions without modifing the header.         
 *  
 * The class is a helper to avoid recompilation and only has static           
 * functions.                                                                 
 */
struct tgrid_implementation
{
	/** 
	 * Wrapper function for
	 * twidget* tgrid::find_widget(const tpoint&, const bool).
	 */
	static twidget* find_widget(
			tgrid& grid, const tpoint& coordinate, const bool must_be_active)
	{
		return find_widget_implementation<twidget*, tgrid, tgrid::tchild>
			(grid, coordinate, must_be_active);
	}

	/** 
	 * Wrapper function for
	 * const twidget* tgrid::find_widget(const tpoint&, const bool) const.
	 */
	static const twidget* find_widget(const tgrid& grid,
			const tpoint& coordinate, const bool must_be_active)
	{
		return find_widget_implementation<const twidget*,
				const tgrid, const tgrid::tchild>
			(grid, coordinate, must_be_active);
	}

private:

	/**
	 * Implementation for the wrappers for 
	 * [const] twidget* tgrid::find_widget(const tpoint&, const bool) [const].
	 *
	 * @param W                   twidget* or const twidget*.
	 * @param G                   if W == twidget* -> tgrid else const tgrid
	 * @param C                   if W == twidget* -> tgrid::tchild
	 *                            else const tgrid::tchild.
	 */
	template<class W, class G, class C>
	static W find_widget_implementation(G& grid, const tpoint& coordinate,
			const bool must_be_active)
	{
		foreach(C& child, grid.children_) {

			W widget = child.widget();
			if(!widget) {
				continue;
			}

			widget = widget->find_widget(coordinate, must_be_active);
			if(widget) {
				return widget;
			}

		}

		return 0;
	}
};

} // namespace gui2

#endif
