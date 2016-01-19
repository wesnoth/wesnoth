/*
   Copyright (C) 2007 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/widget_definition/window.hpp"

#include "gui/auxiliary/log.hpp"

namespace gui2
{

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_window
 *
 * == Window ==
 *
 * The definition of a window. A window is a kind of panel see the panel for
 * which fields exist
 *
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="window_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="gui/panel_definition/resolution"}
 * @allow{link}{name="gui/window/resolution/grid"}
 * @allow{link}{name="gui/panel_definition/resolution/background"}
 * @allow{link}{name="gui/panel_definition/resolution/foreground"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="window_definition"}
 * @end{parent}{name="gui/"}
 */
twindow_definition::twindow_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing window " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

twindow_definition::tresolution::tresolution(const config& cfg)
	: tpanel_definition::tresolution(cfg), grid(NULL)
{
	const config& child = cfg.child("grid");
	// VALIDATE(child, _("No grid defined."));

	/** @todo Evaluate whether the grid should become mandatory. */
	if(child) {
		grid = new tbuilder_grid(child);
	}
}

} // namespace gui2
