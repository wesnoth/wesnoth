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

#ifndef GUI_AUXILIARY_WINDOW_BUILDER_SCROLL_LABEL_HPP_INCLUDED
#define GUI_AUXILIARY_WINDOW_BUILDER_SCROLL_LABEL_HPP_INCLUDED

#include "gui/auxiliary/window_builder/control.hpp"

#include "gui/widgets/scrollbar_container.hpp"

namespace gui2 {

namespace implementation {

struct tbuilder_scroll_label
	: public tbuilder_control
{
	explicit tbuilder_scroll_label(const config& cfg);

	twidget* build () const;

private:

	tscrollbar_container::tscrollbar_mode vertical_scrollbar_mode;
	tscrollbar_container::tscrollbar_mode horizontal_scrollbar_mode;
};

} // namespace implementation

} // namespace gui2

#endif

