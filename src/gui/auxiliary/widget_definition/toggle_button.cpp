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

#include "gui/auxiliary/widget_definition/toggle_button.hpp"

#include "gui/auxiliary/log.hpp"

namespace gui2 {

ttoggle_button_definition::ttoggle_button_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing toggle button " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

ttoggle_button_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_toggle_button
 *
 * == Toggle button ==
 *
 * The definition of a toggle button.
 *
 * The following states exist:
 * * state_enabled, the button is enabled and not selected.
 * * state_disabled, the button is disabled and not selected.
 * * state_focussed, the mouse is over the button and not selected.
 *
 * * state_enabled_selected, the button is enabled and selected.
 * * state_disabled_selected, the button is disabled and selected.
 * * state_focussed_selected, the mouse is over the button and selected.
 */

	// Note the order should be the same as the enum tstate in
	// toggle_button.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
	state.push_back(tstate_definition(cfg.child("state_focussed")));

	state.push_back(tstate_definition(cfg.child("state_enabled_selected")));
	state.push_back(tstate_definition(cfg.child("state_disabled_selected")));
	state.push_back(tstate_definition(cfg.child("state_focussed_selected")));
}

} // namespace gui2

