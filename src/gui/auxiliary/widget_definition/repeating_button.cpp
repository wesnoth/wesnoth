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

#include "gui/auxiliary/widget_definition/repeating_button.hpp"

#include "gui/auxiliary/log.hpp"

namespace gui2
{

trepeating_button_definition::trepeating_button_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing repeating button " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_repeating_button
 *
 * == Repeating button ==
 *
 * @macro = repeating_button_description
 *
 * The following states exist:
 * * state_enabled, the repeating_button is enabled.
 * * state_disabled, the repeating_button is disabled.
 * * state_pressed, the left mouse repeating_button is down.
 * * state_focused, the mouse is over the repeating_button.
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="repeating_button_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @begin{tag}{name="state_disabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled"}
 * @begin{tag}{name="state_pressed"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_pressed"}
 * @begin{tag}{name="state_focused"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_focused"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="repeating_button_definition"}
 * @end{parent}{name="gui/"}
 */
trepeating_button_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
{
	// Note the order should be the same as the enum tstate in
	// repeating_button.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
	state.push_back(tstate_definition(cfg.child("state_pressed")));
	state.push_back(tstate_definition(cfg.child("state_focused")));
}

} // namespace gui2
