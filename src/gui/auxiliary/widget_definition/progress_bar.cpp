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

#include "gui/auxiliary/widget_definition/progress_bar.hpp"

#include "gui/auxiliary/log.hpp"

namespace gui2 {

tprogress_bar_definition::tprogress_bar_definition(const config& cfg) :
	tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing progress bar " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tprogress_bar_definition::tresolution::tresolution(const config& cfg) :
	tresolution_definition_(cfg)
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_progress_bar
 *
 * == Progress bar ==
 *
 * @macro = progress_bar_description
 *
 * The definition of a progress bar. This object shows the progress of a certain
 * action, or the value state of a certain item.
 *
 * The following states exist:
 * * state_enabled, the progress bar is enabled.
 *
 */

	// Note the order should be the same as the enum tstate is progress_bar.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
}

} // namespace gui2

