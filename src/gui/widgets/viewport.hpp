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

#ifndef GUI_WIDGETS_VIEWPORT_HPP_INCLUDED
#define GUI_WIDGETS_VIEWPORT_HPP_INCLUDED

#include "gui/widgets/widget.hpp"

namespace gui2 {

class tgrid;

class tviewport
	: public twidget
{
public:

	tviewport();

	/** Inherited from twidget. */
	void request_reduce_width(const unsigned maximum_width);

private:
	/** Inherited from twidget. */
	tpoint calculate_best_size() const;

public:
	/** Inherited from twidget. */
	bool disable_click_dismiss() const;

	/** Inherited from twidget. */
	virtual iterator::twalker_* create_walker();

private:

};

} // namespace gui2

#endif
