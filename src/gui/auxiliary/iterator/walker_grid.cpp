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

#include "gui/auxiliary/iterator/walker_grid.hpp"

#include "asserts.hpp"

namespace gui2
{

namespace iterator
{

tgrid::tgrid(gui2::tgrid& grid)
	: grid_(grid), widget_(&grid), itor_(grid.begin())
{
}

twalker_::tstate tgrid::next(const tlevel level)
{
	if(at_end(level)) {
		return fail;
	}

	switch(level) {
		case widget:
			if(widget_) {
				widget_ = NULL;
				return invalid;
			} else {
				/* FALL DOWN */
			}
		case grid:
			assert(false);
			return fail;
		case child:
			if(itor_ == grid_.end()) {
				/* FALL DOWN */
			} else {
				++itor_;
				return itor_ == grid_.end() ? invalid : valid;
			}
	}

	assert(false);
	return fail;
}

bool tgrid::at_end(const tlevel level) const
{
	switch(level) {
		case widget:
			return widget_ == NULL;
		case grid:
			return true;
		case child:
			return (itor_ == grid_.end());
	}

	assert(false);
	return true;
}

gui2::twidget* tgrid::get(const tlevel level)
{
	switch(level) {
		case widget:
			return widget_;
		case grid:
			return NULL;
		case child:
			if(itor_ == grid_.end()) {
				return NULL;
			} else {
				return *itor_;
			}
	}

	assert(false);
	return NULL;
}

} // namespace iterator

} // namespace gui2
