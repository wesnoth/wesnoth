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

#include "gui/auxiliary/widget_definition/toggle_panel.hpp"

#include "gui/auxiliary/log.hpp"

namespace gui2 {

ttoggle_panel_definition::ttoggle_panel_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing toggle panel " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

ttoggle_panel_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, top_border(lexical_cast_default<unsigned>(cfg["top_border"]))
	, bottom_border(lexical_cast_default<unsigned>(cfg["bottom_border"]))
	, left_border(lexical_cast_default<unsigned>(cfg["left_border"]))
	, right_border(lexical_cast_default<unsigned>(cfg["right_border"]))
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_toggle_panel
 *
 * == Toggle panel ==
 *
 * The definition of a toggle panel. A toggle panel is like a toggle button, but
 * instead of being a button it's a panel. This means it can hold multiple child
 * items.
 *
 * The resolution for a toggle panel also contains the following keys:
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
 * The following states exist:
 * * state_enabled, the panel is enabled and not selected.
 * * state_disabled, the panel is disabled and not selected.
 * * state_focussed, the mouse is over the panel and not selected.
 *
 * * state_enabled_selected, the panel is enabled and selected.
 * * state_disabled_selected, the panel is disabled and selected.
 * * state_focussed_selected, the mouse is over the panel and selected.
 */
	// Note the order should be the same as the enum tstate in toggle_panel.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
	state.push_back(tstate_definition(cfg.child("state_focussed")));

	state.push_back(tstate_definition(cfg.child("state_enabled_selected")));
	state.push_back(tstate_definition(cfg.child("state_disabled_selected")));
	state.push_back(tstate_definition(cfg.child("state_focussed_selected")));
}

} // namespace gui2

