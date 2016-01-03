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
 * Placement helper for the vertical list.
 */

#ifndef GUI_AUXILIARY_PLACER_VERTICAL_LIST_HPP_INCLUDED
#define GUI_AUXILIARY_PLACER_VERTICAL_LIST_HPP_INCLUDED

#include "gui/auxiliary/placer.hpp"

#include <vector>

namespace gui2
{

namespace implementation
{

/**
 * The placement class for a vertical list.
 *
 * See @ref tplacer_ for more information.
 */
class tplacer_vertical_list : public tplacer_
{
public:
	/***** ***** Constructor, destructor, assignment. ***** *****/

	explicit tplacer_vertical_list(const unsigned maximum_columns);


	/***** ***** Inherited operations. ***** *****/

	virtual void initialise();

	virtual void add_item(const tpoint& size);

	virtual tpoint get_size() const;

	virtual tpoint get_origin(const unsigned index) const;


	/***** ***** Members. ***** *****/

private:
	/**
	 * The maximum number of columns to use.
	 *
	 * This value is determined by the @p parallel_items parameter of
	 * @ref tplacer_::build).
	 */
	unsigned maximum_columns_;

	/**
	 * Holds the row sizes
	 *
	 * The pair contains the following values:
	 * * first                    The origin of a row.
	 * * second                   The height of a row.
	 */
	std::vector<std::pair<int, int> > rows_;

	/** Holds the widths of the columns. */
	std::vector<int> columns_;

	/** The row to add an item to. */
	unsigned row_;

	/** The column to add an item to. */
	unsigned column_;
};

} // namespace implementation

} // namespace gui2

#endif
