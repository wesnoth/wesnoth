/*
   Copyright (C) 2011 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_AUXILIARY_WALKER_VISITOR_GRID_HPP_INCLUDED
#define GUI_WIDGETS_AUXILIARY_WALKER_VISITOR_GRID_HPP_INCLUDED

#include "gui/auxiliary/iterator/walker.hpp"

#include "gui/widgets/grid.hpp"

namespace gui2
{

namespace iterator
{

/** A walker for a @ref gui2::tgrid. */
class tgrid : public twalker_
{
public:
	/**
	 * Constructor.
	 *
	 * @param grid                The grid which the walker is attached to.
	 */
	explicit tgrid(gui2::tgrid& grid);

	/** Inherited from @ref gui2::iterator::twalker_. */
	virtual tstate next(const tlevel level);

	/** Inherited from @ref gui2::iterator::twalker_. */
	virtual bool at_end(const tlevel level) const;

	/** Inherited from @ref gui2::iterator::twalker_. */
	virtual gui2::twidget* get(const tlevel level);

private:
	/** The grid which the walker is attached to. */
	gui2::tgrid& grid_;

	/**
	 * The grid which the walker is attached to.
	 *
	 * This variable is used to track whether the @ref
	 * gui2::iterator::twalker_::widget level has been visited.
	 */
	gui2::twidget* widget_;

	/**
	 * The iterator to the children of @ref grid_.
	 *
	 * This variable is used to track where the @ref
	 * gui2::iterator::twalker_::child level visiting is.
	 */
	gui2::tgrid::iterator itor_;
};

} // namespace iterator

} // namespace gui2

#endif
