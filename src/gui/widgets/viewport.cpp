/* $Id$ */
/*
   Copyright (C) 2012 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/viewport.hpp"

#include "gui/auxiliary/log.hpp"

#define LOG_SCOPE_HEADER "tviewport [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2 {

tviewport::tviewport()
{
}

void tviewport::request_reduce_width(const unsigned /*maximum_width*/)
{
}

tpoint tviewport::calculate_best_size() const
{
	return tpoint(500, 500);
}

bool tviewport::disable_click_dismiss() const
{
	return false;
}

iterator::twalker_* tviewport::create_walker()
{
	/**
	 * @todo Implement properly.
	 */
	return NULL;
}

} // namespace gui2
