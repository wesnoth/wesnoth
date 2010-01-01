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

#ifndef GUI_AUXILIARY_WINDOW_BUILDER_LISTBOX_HPP_INCLUDED
#define GUI_AUXILIARY_WINDOW_BUILDER_LISTBOX_HPP_INCLUDED

#include "gui/auxiliary/window_builder/control.hpp"

#include "gui/widgets/scrollbar_container.hpp"

namespace gui2 {

namespace implementation {

struct tbuilder_listbox
	: public tbuilder_control
{
	tbuilder_listbox(const config& cfg);

	twidget* build () const;

	tscrollbar_container::tscrollbar_mode
			vertical_scrollbar_mode,
			horizontal_scrollbar_mode;

	tbuilder_grid_ptr header;
	tbuilder_grid_ptr footer;

	tbuilder_grid_ptr list_builder;

	/**
	 * Listbox data.
	 *
	 * Contains a vector with the data to set in every cell, it's used to
	 * serialize the data in the config, so the config is no longer required.
	 */
	std::vector<string_map> list_data;
};

} // namespace implementation

} // namespace gui2

#endif

