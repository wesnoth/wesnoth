/* $Id$ */
/*
   Copyright (C) 2007 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/widget_definition/scrollbar_panel.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "wml_exception.hpp"

namespace gui2 {

tscrollbar_panel_definition::tscrollbar_panel_definition(const config& cfg)
	: tcontrol_definition(cfg)
{

	DBG_GUI_P << "Parsing scrollbar panel " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tscrollbar_panel_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, grid()
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_scrollbar_panel
 *
 * == Scrollbar panel ==
 *
 * The definition of a panel with scrollbars. A panel is a container hold
 * other elements in it's grid. A panel is always enabled and can't be
 * disabled. Instead it uses the states as layers to draw on.
 *
 * @start_table = config
 *     grid (grid)                     A grid containing the widgets for main
 *                                     widget.
 * @end_table
 *
 * The following layers exist:
 * * background, the background of the panel.
 * * foreground, the foreground of the panel.
 */

	// The panel needs to know the order.
	state.push_back(tstate_definition(cfg.child("background")));
	state.push_back(tstate_definition(cfg.child("foreground")));

	const config &child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = new tbuilder_grid(child);
}

} // namespace gui2

