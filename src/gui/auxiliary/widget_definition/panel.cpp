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

#include "gui/auxiliary/widget_definition/panel.hpp"

#include "gui/auxiliary/log.hpp"

namespace gui2 {

tpanel_definition::tpanel_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing panel " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tpanel_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, top_border(cfg["top_border"])
	, bottom_border(cfg["bottom_border"])
	, left_border(cfg["left_border"])
	, right_border(cfg["right_border"])
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_panel
 *
 * == Panel ==
 *
 * @macro = panel_description
 *
 * A panel is always enabled and can't be disabled. Instead it uses the
 * states as layers to draw on.
 *
 * The resolution for a panel also contains the following keys:
 * @start_table = config
 *     top_border (unsigned = 0)     The size which isn't used for the client
 *                                   area.
 *     bottom_border (unsigned = 0)  The size which isn't used for the client
 *                                   area.
 *     left_border (unsigned = 0)    The size which isn't used for the client
 *                                   area.
 *     right_border (unsigned = 0)   The size which isn't used for the client
 *                                   area.
 * @end_table
 *
 * The following layers exist:
 * * background, the background of the panel.
 * * foreground, the foreground of the panel.
 */

	// The panel needs to know the order.
	state.push_back(tstate_definition(cfg.child("background")));
	state.push_back(tstate_definition(cfg.child("foreground")));
}

} // namespace gui2

