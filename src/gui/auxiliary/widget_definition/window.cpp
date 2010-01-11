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

#include "gui/auxiliary/widget_definition/window.hpp"

#include "gui/auxiliary/log.hpp"

namespace gui2 {

twindow_definition::twindow_definition(const config& cfg)
	: tcontrol_definition(cfg)
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
 */

	DBG_GUI_P << "Parsing window " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

twindow_definition::tresolution::tresolution(const config& cfg)
	: tpanel_definition::tresolution(cfg)
	, grid(NULL)
{
	const config &child = cfg.child("grid");
//	VALIDATE(child, _("No grid defined."));

	/** @todo Evaluate whether the grid should become mandatory. */
	if(child) {
		grid = new tbuilder_grid(child);
	}
}

} // namespace gui2

