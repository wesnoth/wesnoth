/*
   Copyright (C) 2011 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/iterator/walker_widget.hpp"

#include <cassert>
#include "gui/widgets/widget.hpp"

namespace gui2
{

namespace iteration
{

namespace walker
{

widget::widget(gui2::widget& widget) : widget_(&widget)
{
}

walker_base::state_t widget::next(const level level)
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
		case internal: FALLTHROUGH;
		case child: FALLTHROUGH;
	}

	assert(false);
	return fail;
}

bool widget::at_end(const level level) const
{
	switch(level) {
		case self:
			return widget_ == nullptr;
		case internal: /* FALL DOWN */
		case child:
			return true;
	}

	assert(false);
	return true;
}

gui2::widget* widget::get(const level level)
{
	switch(level) {
		case self:
			return widget_;
		case internal: /* FALL DOWN */
		case child:
			return nullptr;
	}

	assert(false);
	return nullptr;
}

} //  namespace walker

} // namespace iteration

} // namespace gui2
