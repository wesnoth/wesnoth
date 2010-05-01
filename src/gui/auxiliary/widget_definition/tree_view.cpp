/* $Id$ */
/*
   Copyright (C) 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/widget_definition/tree_view.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "wml_exception.hpp"

namespace gui2 {

ttree_view_definition::ttree_view_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing tree view " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

ttree_view_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, grid(NULL)
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_tree_view
 *
 * == Tree view ==
 *
 * @macro = tree_view_description
 *
 * The documentation is not written yet.
 *
 * The following states exist:
 * * state_enabled, the listbox is enabled.
 * * state_disabled, the listbox is disabled.
 */

	// Note the order should be the same as the enum tstate is listbox.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));

	const config &child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = new tbuilder_grid(child);
}

} // namespace gui2

