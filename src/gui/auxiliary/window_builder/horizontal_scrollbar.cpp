/*
   Copyright (C) 2008 - 2015 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/window_builder/horizontal_scrollbar.hpp"

#include "gui/auxiliary/log.hpp"
#include "gui/widgets/horizontal_scrollbar.hpp"

namespace gui2
{

namespace implementation
{

tbuilder_horizontal_scrollbar::tbuilder_horizontal_scrollbar(const config& cfg)
	: tbuilder_control(cfg)
{
}

twidget* tbuilder_horizontal_scrollbar::build() const
{
	thorizontal_scrollbar* widget = new thorizontal_scrollbar();

	init_control(widget);

	DBG_GUI_G << "Window builder:"
			  << " placed horizontal scrollbar '" << id << "' with definition '"
			  << definition << "'.\n";

	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI_MACRO
 * @begin{macro}{horizontal_scrollbar_description}
 *
 *        A horizontal scrollbar is a widget that shows a horizontal scrollbar.
 *        This widget is most of the time used in a container to control the
 *        scrolling of its contents.
 * @end{macro}
 */

/*WIKI
 * @page = GUIToolkitWML
 * @order = 2_horizontal_scrollbar
 *
 * == Horizontal scrollbar ==
 *
 * @macro = horizontal_scrollbar_description
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="horizontal_scrollbar"}{min=0}{max=-1}{super="generic/widget_instance"}
 * @end{tag}{name="horizontal_scrollbar"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 * A horizontal scrollbar has no special fields.
 */
