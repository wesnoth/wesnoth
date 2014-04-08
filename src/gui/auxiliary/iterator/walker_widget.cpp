/*
   Copyright (C) 2011 - 2014 by Mark de Wever <koraq@xs4all.nl>
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

#include "asserts.hpp"
#include "gui/widgets/widget.hpp"

namespace gui2
{

namespace iterator
{

namespace walker
{

twidget::twidget(gui2::twidget& widget) : widget_(&widget)
{
}

twalker_::tstate twidget::next(const tlevel level)
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
		case grid:  /* FALL DOWN */
		case child: /* FALL DOWN */
			;
	}

	assert(false);
	return fail;
}

bool twidget::at_end(const tlevel level) const
{
	switch(level) {
		case widget:
			return widget_ == NULL;
		case grid: /* FALL DOWN */
		case child:
			return true;
	}

	assert(false);
	return true;
}

gui2::twidget* twidget::get(const tlevel level)
{
	switch(level) {
		case widget:
			return widget_;
		case grid: /* FALL DOWN */
		case child:
			return NULL;
	}

	assert(false);
	return NULL;
}

} //  namespace walker

} // namespace iterator

} // namespace gui2
