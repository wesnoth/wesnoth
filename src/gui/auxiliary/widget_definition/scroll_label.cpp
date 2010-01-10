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

#include "gui/auxiliary/widget_definition/scroll_label.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "wml_exception.hpp"

namespace gui2 {

tscroll_label_definition::tscroll_label_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing scroll label " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tscroll_label_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, grid(NULL)
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_scroll_label
 *
 * == Scroll label ==
 *
 * @macro = scroll_label_description
 *
 * This widget is slower as a normal label widget so only use this widget
 * when the scrollbar is required (or expected to become required).
 *
 * @start_table = config
 *     grid (grid)                     A grid containing the widgets for main
 *                                     widget.
 * @end_table
 *
 * TODO we need one definition for a vertical scrollbar since this is the second
 * time we use it.
 *
 * @start_table = container
 *     _content_grid (grid)            A grid which should only contain one
 *                                     label widget.
 *     _scrollbar_grid (grid)          A grid for the scrollbar
 *                                     (Merge with listbox info.)
 * @end_table
 *
 * The following states exist:
 * * state_enabled, the scroll label is enabled.
 * * state_disabled, the scroll label is disabled.
 *
 */
	// Note the order should be the same as the enum tstate is scroll_label.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));

	const config &child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = new tbuilder_grid(child);
}

} // namespace gui2

