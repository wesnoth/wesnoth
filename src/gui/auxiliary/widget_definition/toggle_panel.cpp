/*
   Copyright (C) 2007 - 2015 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/widget_definition/toggle_panel.hpp"

#include "gui/auxiliary/log.hpp"

namespace gui2
{

ttoggle_panel_definition::ttoggle_panel_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing toggle panel " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_toggle_panel
 *
 * == Toggle panel ==
 *
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="toggle_panel_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * The definition of a toggle panel. A toggle panel is like a toggle button, but
 * instead of being a button it's a panel. This means it can hold multiple child
 * items.
 *
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super=generic/widget_definition/resolution}
 * The resolution for a toggle panel also contains the following keys:
 * @begin{table}{config}
 *     top_border & unsigned & 0 &     The size which isn't used for the client
 *                                   area. $
 *     bottom_border & unsigned & 0 &  The size which isn't used for the client
 *                                   area. $
 *     left_border & unsigned & 0 &    The size which isn't used for the client
 *                                   area. $
 *     right_border & unsigned & 0 &   The size which isn't used for the client
 *                                   area. $
 * @end{table}
 *
 * The following states exist:
 * * state_enabled, the panel is enabled and not selected.
 * * state_disabled, the panel is disabled and not selected.
 * * state_focussed, the mouse is over the panel and not selected.
 *
 * * state_enabled_selected, the panel is enabled and selected.
 * * state_disabled_selected, the panel is disabled and selected.
 * * state_focussed_selected, the mouse is over the panel and selected.
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @begin{tag}{name="state_disabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled"}
 * @begin{tag}{name="state_focussed"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_focussed"}
 * @begin{tag}{name="state_enabled_selected"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled_selected"}
 * @begin{tag}{name="state_disabled_selected"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled_selected"}
 * @begin{tag}{name="state_focussed_selected"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_focussed_selected"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="toggle_panel_definition"}
 * @end{parent}{name="gui/"}
 */
ttoggle_panel_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, top_border(cfg["top_border"])
	, bottom_border(cfg["bottom_border"])
	, left_border(cfg["left_border"])
	, right_border(cfg["right_border"])
{
	// Note the order should be the same as the enum tstate in toggle_panel.hpp.
	FOREACH(const AUTO& c, cfg.child_range("state"))
	{
		state.push_back(tstate_definition(c.child("enabled")));
		state.push_back(tstate_definition(c.child("disabled")));
		state.push_back(tstate_definition(c.child("focussed")));
	}
}

} // namespace gui2
