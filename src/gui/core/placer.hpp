/*
   Copyright (C) 2012 - 2018 by Mark de Wever <koraq@xs4all.nl>
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
 * Base class for the placement helper.
 *
 * Some items can create new child items and these items are placed in some way
 * this code contains helpers for the placement by calculating the positions
 * for the items and the best size for the children.
 */

#pragma once

#include "utils/make_enum.hpp"

struct point;

namespace gui2
{

/**
 * Base class for the placement helper.
 *
 * The normal operation for the usage of the class is:
 * * Call @ref initialize().
 * * For every visible child item call @ref add_item() with the wanted size of
 *   the widget.
 * Once this is done the required size for all children can be retrieved with
 * @ref get_size(). It is also possible to place all children now. In order to
 * do so loop again over all children in the same order as @ref add_item() and
 * call @ref get_origin(). The @p index parameter is an increasing counter.
 *
 * @note The origins can only be retrieved after all items are added since the
 * adding of a later item may influence a previous item. E.g. in a vertical
 * list with two columns the position of the second column depends on the width
 * of the first and a later row may have a wider column 1 as an earlier row.
 */
class placer_base
{
public:
	/***** ***** Types. ***** *****/

	/** The direction the placer should grow towards. */
	MAKE_ENUM(tgrow_direction,
		(horizontal, "horizontal")
		(vertical, "vertical")
	)


	/***** ***** Constructor, destructor, assignment. ***** *****/

public:
	/**
	 * Builder function.
	 *
	 * @pre                       @p parallel_items > 0
	 *
	 * @param grow_direction      The direction in which the items will be
	 *                            added.
	 * @param parallel_items      The direction perpendicular towards the grow
	 *                            direction has a fixed number of items. This
	 *                            value sets that limit. For a list containing
	 *                            only horizontally or vertically placed items
	 *                            the value should be 1.
	 */
	static placer_base* build(const tgrow_direction grow_direction,
						   const unsigned parallel_items);

	virtual ~placer_base();


	/***** ***** Operations. ***** *****/

	/**
	 * Initialises the placer.
	 *
	 * When the placement needs to be calculated the state often needs to be
	 * reset, items are placed, removed or changed visibility causing the old
	 * placement to be invalid.
	 */
	virtual void initialize() = 0;

	/**
	 * Adds a item to be placed.
	 *
	 * @param size                The required size for the item.
	 */
	virtual void add_item(const point& size) = 0;

	/**
	 * Gets the required size of all items.
	 *
	 * @returns                   The required size.
	 */
	virtual point get_size() const = 0;

	/**
	 * Gets the origin for an item.
	 *
	 * @param index               The index of the item whose origin to return.
	 *                            The index is the index of the call to
	 *                            @ref add_item().
	 *
	 * @returns                   The origin where to place the widget.
	 */
	virtual point get_origin(const unsigned index) const = 0;
};

} // namespace gui2
