/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/window_builder/toggle_panel.hpp"

#include "config.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/window_builder/helper.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "wml_exception.hpp"

namespace gui2
{

namespace implementation
{

tbuilder_toggle_panel::tbuilder_toggle_panel(const config& cfg)
	: tbuilder_control(cfg)
	, grid(NULL)
	, retval_id_(cfg["return_value_id"])
	, retval_(cfg["return_value"])
{
	const config& c = cfg.child("grid");

	VALIDATE(c, _("No grid defined."));

	grid = new tbuilder_grid(c);
}

twidget* tbuilder_toggle_panel::build() const
{
	ttoggle_panel* widget = new ttoggle_panel();

	init_control(widget);

	widget->set_retval(get_retval(retval_id_, retval_, id));

	DBG_GUI_G << "Window builder: placed toggle panel '" << id
			  << "' with definition '" << definition << "'.\n";

	widget->init_grid(grid);
	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_toggle_panel
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="toggle_panel"}{min=0}{max=-1}{super="generic/widget_instance"}
 * == Toggle panel ==
 *
 * A toggle panel is an item which can hold other items. The difference between
 * a grid and a panel is that it's possible to define how a panel looks. A grid
 * in an invisible container to just hold the items. The toggle panel is a
 * combination of the panel and a toggle button, it allows a toggle button with
 * its own grid.
 *
 * @begin{table}{config}
 *     grid & grid & &                 Defines the grid with the widgets to
 *                                     place on the panel. $
 *     return_value_id & string & "" & The return value id, see
 *                                     [[GUIToolkitWML#Button]] for more
 *                                     information. $
 *     return_value & int & 0 &        The return value, see
 *                                     [[GUIToolkitWML#Button]] for more
 *                                     information. $
 * @end{table}
 * @allow{link}{name="gui/window/resolution/grid"}
 * @end{tag}{name="toggle_panel"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */
