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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/iterator/walker_tree_node.hpp"
#include "gui/widgets/tree_view_node.hpp"

#include "asserts.hpp"

namespace gui2
{

namespace iterator
{

ttree_node::ttree_node(gui2::ttree_view_node& node, boost::ptr_vector<gui2::ttree_view_node>& children)
	: children_(children), widget_(&node), itor_(children.begin())
{
}

twalker_::tstate ttree_node::next(const tlevel level)
{
	if(at_end(level)) {
		return fail;
	}

	switch(level) {
		case widget:
			if(widget_) {
				widget_ = nullptr;
				return invalid;
			} else {
				/* FALL DOWN */
			}
		case grid:
			assert(false);
			return fail;
		case child:
			if(itor_ == children_.end()) {
				/* FALL DOWN */
			} else {
				++itor_;
				return itor_ == children_.end() ? invalid : valid;
			}
	}

	assert(false);
	return fail;
}

bool ttree_node::at_end(const tlevel level) const
{
	switch(level) {
		case widget:
			return widget_ == nullptr;
		case grid:
			return true;
		case child:
			return (itor_ == children_.end());
	}

	assert(false);
	return true;
}

gui2::twidget* ttree_node::get(const tlevel level)
{
	switch(level) {
		case widget:
			return widget_;
		case grid:
			return nullptr;
		case child:
			if(itor_ == children_.end()) {
				return nullptr;
			} else {
				return itor_.operator->();
			}
	}

	assert(false);
	return nullptr;
}

} // namespace iterator

} // namespace gui2
