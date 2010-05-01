/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_AUXILIARY_WINDOW_BUILDER_STACKED_WIDGET_HPP_INCLUDED
#define GUI_AUXILIARY_WINDOW_BUILDER_STACKED_WIDGET_HPP_INCLUDED

#include "gui/auxiliary/window_builder/control.hpp"

#include <vector>

namespace gui2 {

namespace implementation {

struct tbuilder_stacked_widget
	: public tbuilder_control
{
	explicit tbuilder_stacked_widget(const config& cfg);

	twidget* build () const;

	/** The builders for all layers of the stack .*/
	std::vector<tbuilder_grid_const_ptr> stack;
};

} // namespace implementation

} // namespace gui2

#endif

