/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_AUXILIARY_WINDOW_BUILDER_MENUBAR_HPP_INCLUDED
#define GUI_AUXILIARY_WINDOW_BUILDER_MENUBAR_HPP_INCLUDED

#include "gui/auxiliary/window_builder/control.hpp"

#include "gui/auxiliary/window_builder_private.hpp"
#include "gui/widgets/menubar.hpp"

namespace gui2 {

namespace implementation {

struct tbuilder_menubar
	: public tbuilder_control
{
	tbuilder_menubar(const config& cfg);

	twidget* build () const;

private:
	bool must_have_one_item_selected_;

	tmenubar::tdirection direction_;

	int selected_item_;

	std::vector<tbuilder_gridcell> cells_;
};

} // namespace implementation

} // namespace gui2

#endif


