/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/window_builder/horizontal_scrollbar.hpp"

#include "gui/auxiliary/log.hpp"
#include "gui/widgets/horizontal_scrollbar.hpp"

namespace gui2 {

namespace implementation {

twidget* tbuilder_horizontal_scrollbar::build() const
{
	thorizontal_scrollbar *widget = new thorizontal_scrollbar();

	init_control(widget);

	DBG_GUI_G << "Window builder:"
		<< " placed horizontal scrollbar '" << id
		<< "' with defintion '" << definition
		<< "'.\n";

	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI
 * @page = GUIToolkitWML
 * @order = 2_horizontal_scrollbar
 *
 * == Horizontal scrollbar ==
 *
 * A horizontal scrollbar has no special fields.
 *
 */

