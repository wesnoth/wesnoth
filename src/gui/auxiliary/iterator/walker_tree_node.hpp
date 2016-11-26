/*
   Copyright (C) 2011 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include <boost/ptr_container/ptr_vector.hpp>

namespace gui2
{

class tree_view_node;

namespace iteration
{

/** A walker for a @ref gui2::tree_view_node. */
class tree_node : public walker_base
{
public:
	/**
	 * Constructor.
	 *
	 * @param node                The tree view node which the walker is attached to.
	 * @param children            The node's children.
	 */
	tree_node(gui2::tree_view_node& node, boost::ptr_vector<gui2::tree_view_node>& children);

	/** Inherited from @ref gui2::iteration::walker_base. */
	virtual state_t next(const level level);

	/** Inherited from @ref gui2::iteration::walker_base. */
	virtual bool at_end(const level level) const;

	/** Inherited from @ref gui2::iteration::walker_base. */
	virtual gui2::widget* get(const level level);

private:
	/** The children of the node which the walker is attached to. */
	boost::ptr_vector<gui2::tree_view_node>& children_;

	/**
	 * The node which the walker is attached to.
	 *
	 * This variable is used to track whether the @ref
	 * gui2::iteration::walker_base::widget level has been visited.
	 */
	gui2::widget* widget_;

	/**
	 * The iterator to the children of @ref node_.
	 *
	 * This variable is used to track where the @ref
	 * gui2::iteration::walker_base::child level visiting is.
	 */
	boost::ptr_vector<gui2::tree_view_node>::iterator itor_;
};

} // namespace iteration

} // namespace gui2

#endif
