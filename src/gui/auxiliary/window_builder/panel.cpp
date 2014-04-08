/*
   Copyright (C) 2008 - 2014 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/window_builder/panel.hpp"

#include "config.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/panel.hpp"
#include "wml_exception.hpp"

namespace gui2
{

namespace implementation
{

tbuilder_panel::tbuilder_panel(const config& cfg)
	: tbuilder_control(cfg), grid(NULL)
{
	const config& c = cfg.child("grid");

	VALIDATE(c, _("No grid defined."));

	grid = new tbuilder_grid(c);
}

twidget* tbuilder_panel::build() const
{
	tpanel* widget = new tpanel();

	init_control(widget);

	DBG_GUI_G << "Window builder: placed panel '" << id << "' with definition '"
			  << definition << "'.\n";

	widget->init_grid(grid);
	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI_MACRO
 * @begin{macro}{panel_description}
 *
 *        A panel is an item which can hold other items. The difference
 *        between a grid and a panel is that it's possible to define how a
 *        panel looks. A grid in an invisible container to just hold the
 *        items.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_panel
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="panel"}{min="0"}{max="-1"}{super="generic/widget_instance"}
 * == Panel ==
 *
 * @macro = panel_description
 *
 * @begin{table}{config}
 *     grid & grid & &                 Defines the grid with the widgets to
 *                                     place on the panel. $
 * @end{table}
 * @allow{link}{name="gui/window/resolution/grid"}
 * @end{tag}{name="panel"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */
