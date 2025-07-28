/*
	Copyright (C) 2011 - 2025
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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/iterator/walker_scrollbar_container.hpp"

#include <cassert>

namespace gui2::iteration
{

scrollbar_container::scrollbar_container(gui2::scrollbar_container& container)
	: container_(container), widget_(&container), itor_(container.begin())
{
}

walker_base::state_t scrollbar_container::next(const level level)
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
			assert(false);
			return fail;
		case internal:
			if(!entered_grid) {
				entered_grid = true;
				in_grid = true;
				return valid;
			}
			if(in_grid) {
				++itor_;
				if(itor_ == container_.end()) {
					in_grid = false;
					entered_children = true;
					itor_ = container_.content_grid()->begin();
					return invalid;
				}
				return valid;
			}
			assert(false);
			return fail;
		case child:
			if(itor_ != container_.content_grid()->end()) {
				++itor_;
				return itor_ == container_.content_grid()->end() ? invalid : valid;
			}
	}

	assert(false);
	return fail;
}

bool scrollbar_container::at_end(const level level) const
{
	switch(level) {
		case self:
			return widget_ == nullptr;
		case internal:
			return entered_grid && !in_grid;
		case child:
			return entered_children && (itor_ == container_.content_grid()->end());
	}

	assert(false);
	return true;
}

gui2::widget* scrollbar_container::get(const level level)
{
	switch(level) {
		case self:
			return widget_;
		case internal:
			if(!in_grid || itor_ == container_.end()) {
				return nullptr;
			} else {
				return *itor_;
			}
		case child:
			if(!entered_children || itor_ == container_.content_grid()->end()) {
				return nullptr;
			} else {
				return *itor_;
			}
	}

	assert(false);
	return nullptr;
}

} // namespace gui2::iteration
