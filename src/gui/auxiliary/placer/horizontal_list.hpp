/*
   Copyright (C) 2012 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Placement helper for the horizontal list.
 */

#ifndef GUI_AUXILIARY_PLACER_HORIZONTAL_LIST_HPP_INCLUDED
#define GUI_AUXILIARY_PLACER_HORIZONTAL_LIST_HPP_INCLUDED

#include "gui/auxiliary/placer.hpp"

#include <vector>

namespace gui2
{

namespace implementation
{

/**
 * The placement class for a horizontal list.
 *
 * See @ref tplacer_ for more information.
 */
class tplacer_horizontal_list : public tplacer_
{

public:
	/***** ***** Constructor, destructor, assignment. ***** *****/

	explicit tplacer_horizontal_list(const unsigned maximum_rows);


	/***** ***** Inherited operations. ***** *****/

	virtual void initialise();

	virtual void add_item(const tpoint& size);

	virtual tpoint get_size() const;

	virtual tpoint get_origin(const unsigned index) const;


	/***** ***** Members. ***** *****/

private:
	/**
	 * The maximum number of rows to use.
	 *
	 * This value is determined by the @p parallel_items parameter of
	 * @ref tplacer_::build).
	 */
	unsigned maximum_rows_;

	/** Holds the heights of the rows. */
	std::vector<int> rows_;

	/**
	 * Holds the column sizes
	 *
	 * The pair contains the following values:
	 * * first                    The origin of a column.
	 * * second                   The width of a column.
	 */
	std::vector<std::pair<int, int> > columns_;

	/** The row to add an item to. */
	unsigned row_;

	/** The column to add an item to. */
	unsigned column_;
};

} // namespace implementation

} // namespace gui2

#endif
