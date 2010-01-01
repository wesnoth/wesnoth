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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/window_builder/vertical_scrollbar.hpp"

#include "config.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/vertical_scrollbar.hpp"

namespace gui2 {

namespace implementation {

tbuilder_vertical_scrollbar::tbuilder_vertical_scrollbar(const config& cfg)
	: tbuilder_control(cfg)
{
}

twidget* tbuilder_vertical_scrollbar::build() const
{
	tvertical_scrollbar *widget = new tvertical_scrollbar();

	init_control(widget);

	DBG_GUI_G << "Window builder:"
			<< " placed vertical scrollbar '" << id
			<< "' with defintion '" << definition
			<< "'.\n";

	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_vertical_scrollbar
 *
 * == Vertical scrollbar ==
 *
 * A vertical scrollbar has no special fields.
 *
 */

