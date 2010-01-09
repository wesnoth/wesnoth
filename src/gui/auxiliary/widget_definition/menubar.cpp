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

#include "gui/auxiliary/widget_definition/menubar.hpp"

#include "gui/auxiliary/log.hpp"

namespace gui2 {

tmenubar_definition::tmenubar_definition(const config& cfg) :
	tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing menubar " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tmenubar_definition::tresolution::tresolution(const config& cfg) :
	tresolution_definition_(cfg)
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_menubar
 *
 * == Menubar ==
 *
 * The definition of a normal menubar.
 *
 * The following states exist:
 * * state_enabled, the menubar is enabled.
 * * state_disabled, the menubar is disabled.
 *
 */
	// Note the order should be the same as the enum tstate is menubar.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
}

} // namespace gui2

