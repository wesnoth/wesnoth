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

#include "gui/auxiliary/widget_definition/vertical_scrollbar.hpp"

#include "gui/auxiliary/log.hpp"
#include "wml_exception.hpp"

namespace gui2 {

tvertical_scrollbar_definition::tvertical_scrollbar_definition(
		const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing vertical scrollbar " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tvertical_scrollbar_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, minimum_positioner_length(cfg["minimum_positioner_length"])
	, maximum_positioner_length(cfg["maximum_positioner_length"])
	, top_offset(cfg["top_offset"])
	, bottom_offset(cfg["bottom_offset"])
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_vertical_scrollbar
 *
 * == Vertical scrollbar ==
 *
 * The definition of a vertical scrollbar. This class is most of the time not
 * used directly. Instead it's used to build other items with scrollbars.
 *
 * The resolution for a vertical scrollbar also contains the following keys:
 * @start_table = config
 *     minimum_positioner_length (unsigned)
 *                                     The minumum size the positioner is
 *                                     allowed to be. The engine needs to know
 *                                     this in order to calculate the best size
 *                                     for the positioner.
 *     maximum_positioner_length (unsigned = 0)
 *                                     The maximum size the positioner is
 *                                     allowed to be. If minimum and maximum are
 *                                     the same value the positioner is fixed
 *                                     size. If the maximum is 0 (and the
 *                                     minimum not) there's no maximum.
 *     top_offset (unsigned = 0)       The number of pixels at the top which
 *                                     can't be used by the positioner.
 *     bottom_offset (unsigned = 0)    The number of pixels at the bottom which
 *                                     can't be used by the positioner.
 * @end_table
 *
 * The following states exist:
 * * state_enabled, the vertical scrollbar is enabled.
 * * state_disabled, the vertical scrollbar is disabled.
 * * state_pressed, the left mouse button is down on the positioner of the
 *   vertical scrollbar.
 * * state_focussed, the mouse is over the positioner of the vertical scrollbar.
 */

	VALIDATE(minimum_positioner_length
			, missing_mandatory_wml_key(
				  "resolution"
				, "minimum_positioner_length"));

	// Note the order should be the same as the enum tstate in scrollbar.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
	state.push_back(tstate_definition(cfg.child("state_pressed")));
	state.push_back(tstate_definition(cfg.child("state_focussed")));
}

} // namespace gui2

