/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <pango/pango.h>

namespace font {

/**
 * Small helper wrapper for PangoLayoutIter*.
 *
 * Needed to make sure it gets freed properly.
 */
class p_itor
{
public:

	explicit p_itor(PangoLayout* layout_)
		: itor_(pango_layout_get_iter(layout_))
	{
	}

	p_itor(const p_itor &) = delete;
	p_itor & operator = (const p_itor &) = delete;

	~p_itor() { pango_layout_iter_free(itor_); }

	operator PangoLayoutIter*() { return itor_; }

private:

	PangoLayoutIter* itor_;
};

} // end namespace font
