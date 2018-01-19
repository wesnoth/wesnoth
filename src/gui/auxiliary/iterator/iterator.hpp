/*
   Copyright (C) 2011 - 2018 by Mark de Wever <koraq@xs4all.nl>
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
 * Contains the base iterator class for the gui2 widgets.
 *
 * See @ref gui2_iterator for more information.
 */

#pragma once

#include "gui/auxiliary/iterator/policy_order.hpp"

namespace gui2
{

namespace iteration
{

/**
 * The iterator class.
 *
 * See @ref gui2_iterator_iterator for more information.
 */
template <class order>
class iterator : private order
{
public:
	iterator(const iterator&) = delete;
	iterator& operator=(const iterator&) = delete;

	/**
	 * Constructor.
	 *
	 * @param root                The widget where to start the iteration.
	 */
	iterator(widget& root) : order(root)
	{
	}

	/**
	 * Has the iterator reached the end?
	 *
	 * @returns                   The status.
	 * @retval [true]             At the end.
	 * @retval [false]            Not at the end.
	 */
	bool at_end() const
	{
		return order::at_end();
	}

	/**
	 * Visit the next widget.
	 *
	 * @pre                       The following assertion holds:
	 *                            @code at_end() == false @endcode
	 *
	 * @throws                    A @ref range_error exception upon pre
	 *                            condition violation.
	 *
	 * @returns                   Whether the next widget can be safely
	 *                            deferred.
	 */
	bool next()
	{
		return order::next();
	}

	/** See @ref next. */
	iterator<order>& operator++()
	{
		order::next();
		return *this;
	}

	/**
	 * Returns the current widget.
	 *
	 * @returns                   The current widget.
	 */
	widget& operator*()
	{
		return order::operator*();
	}

	/** See @ref operator*. */
	widget* operator->()
	{
		return &(operator*());
	}
};

} // namespace iteration

} // namespace gui2
