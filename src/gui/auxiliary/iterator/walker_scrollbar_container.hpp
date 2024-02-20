/*
	Copyright (C) 2011 - 2024
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

#include "gui/auxiliary/iterator/walker.hpp"

#include "gui/widgets/scrollbar_container.hpp"

namespace gui2::iteration
{

/** A walker for a @ref gui2::container_base. */
class scrollbar_container : public walker_base
{
public:
	/**
	 * Constructor.
	 *
	 * @param container                The grid which the walker is attached to.
	 */
	explicit scrollbar_container(gui2::scrollbar_container& container);

	/** Inherited from @ref gui2::iteration::walker_base. */
	virtual state_t next(const level level);

	/** Inherited from @ref gui2::iteration::walker_base. */
	virtual bool at_end(const level level) const;

	/** Inherited from @ref gui2::iteration::walker_base. */
	virtual gui2::widget* get(const level level);

private:
	/** The container which the walker is attached to. */
	gui2::scrollbar_container& container_;

	/**
	 * The widget which the walker is attached to.
	 *
	 * This variable is used to track whether the
	 * gui2::iteration::walker_base::widget level has been visited.
	 */
	gui2::widget* widget_;

	/**
	 * Whether the grid has been yielded
	 *
	 * This variable is used to track whether the
	 * gui2::iteration::walker_base::internal level has been visited.
	 */
	bool entered_grid = false, in_grid = false, entered_children = false;

	/**
	 * The iterator to the children of @ref container_.
	 *
	 * This variable is used to track where the
	 * gui2::iteration::walker_base::child level and
	 * gui2::iteration::walker_base::internal level visiting is.
	 */
	gui2::grid::iterator itor_;
};

} // namespace gui2::iteration
