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
 * This file is being used for a small experiment in which some private
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
 * Helper struct to get the same constness for T and U.
 * 
 * @param T                       A type to determine the constness.
 * @param U                       Non const type to set the constness off.
 */
template<class T, class U>
struct tconst_duplicator
{
	/** The type to use, if T not const U is also not const. */
	typedef U type;
};

/** Specialialized version of tconst_duplicator when T is a const type. */
template<class T, class U>
struct tconst_duplicator<const T, U>
{
	/** The type to use, const U. */
	typedef const U type;
};

/**
 * Helper to implement private functions without modifing the header.         
 *  
 * The class is a helper to avoid recompilation and only has static           
 * functions.                                                                 
 */
struct tgrid_implementation
{
	/**
	 * Implementation for the wrappers for 
	 * [const] twidget* tgrid::find_widget(const tpoint&, const bool) [const].
	 *
	 * @param W                   twidget or const twidget.
	 */
	template<class W>
	static W* find_widget(typename tconst_duplicator<W, tgrid>::type& grid,
			const tpoint& coordinate, const bool must_be_active)
	{
		typedef typename tconst_duplicator<W, tgrid::tchild>::type hack;
		foreach(hack& child, grid.children_) {

			W* widget = child.widget();
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

	/**
	 * Implementation for the wrappers for 
	 * [const] twidget* tgrid::find_widget(const std::string&,
	 * const bool) [const].
	 *
	 * @param W                   twidget or const twidget.
	 */
	template<class W>
	static W* find_widget(typename tconst_duplicator<W, tgrid>::type& grid,
				const std::string& id, const bool must_be_active)
	{
		// Inherited.
		W* widget = grid.twidget::find_widget(id, must_be_active);
		if(widget) {
			return widget;
		}

		typedef typename tconst_duplicator<W, tgrid::tchild>::type hack;
		foreach(hack& child, grid.children_) {

			widget = child.widget();
			if(!widget) {
				continue;
			}

			widget = widget->find_widget(id, must_be_active);
			if(widget) {
				return widget;
			}

		}

		return 0;
	}
};

} // namespace gui2

#endif
