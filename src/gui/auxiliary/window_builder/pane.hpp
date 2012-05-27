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

#ifndef GUI_AUXILIARY_WINDOW_BUILDER_PANE_HPP_INCLUDED
#define GUI_AUXILIARY_WINDOW_BUILDER_PANE_HPP_INCLUDED

#include "gui/auxiliary/placer.hpp"
#include "gui/widgets/pane.hpp"

namespace gui2 {

class tpane;

namespace implementation {

struct tbuilder_pane
	: public tbuilder_widget
{
	explicit tbuilder_pane(const config& cfg);

	tpane* build() const;

	tplacer_::tgrow_direction grow_direction;

	unsigned parallel_items;

	tbuilder_grid_ptr item_definition;
};

} // namespace implementation

} // namespace gui2

#endif
