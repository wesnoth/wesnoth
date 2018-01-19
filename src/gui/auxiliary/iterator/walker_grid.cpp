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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/iterator/walker_grid.hpp"

#include <cassert>

namespace gui2
{

namespace iteration
{

grid::grid(gui2::grid& grid)
	: grid_(grid), widget_(&grid), itor_(grid.begin())
{
}

walker_base::state_t grid::next(const level level)
{
	if(at_end(level)) {
		return fail;
	}

	switch(level) {
		case self:
			if(widget_) {
				widget_ = nullptr;
				return invalid;
			}
			FALLTHROUGH;
		case internal:
			assert(false);
			return fail;
		case child:
			if(itor_ != grid_.end()) {
				++itor_;
				return itor_ == grid_.end() ? invalid : valid;
			}
	}

	assert(false);
	return fail;
}

bool grid::at_end(const level level) const
{
	switch(level) {
		case self:
			return widget_ == nullptr;
		case internal:
			return true;
		case child:
			return (itor_ == grid_.end());
	}

	assert(false);
	return true;
}

gui2::widget* grid::get(const level level)
{
	switch(level) {
		case self:
			return widget_;
		case internal:
			return nullptr;
		case child:
			if(itor_ == grid_.end()) {
				return nullptr;
			} else {
				return *itor_;
			}
	}

	assert(false);
	return nullptr;
}

} // namespace iteration

} // namespace gui2
