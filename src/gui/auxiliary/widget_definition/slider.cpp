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

#include "gui/auxiliary/widget_definition/slider.hpp"

#include "gui/auxiliary/log.hpp"
#include "wml_exception.hpp"

namespace gui2 {

tslider_definition::tslider_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing slider " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tslider_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, minimum_positioner_length(cfg["minimum_positioner_length"])
	, maximum_positioner_length(cfg["maximum_positioner_length"])
	, left_offset(cfg["left_offset"])
	, right_offset(cfg["right_offset"])
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_slider
 *
 * == Slider ==
 *
 * @macro = slider_description
 *
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
 *     left_offset (unsigned = 0)      The number of pixels at the left side
 *                                     which can't be used by the positioner.
 *     right_offset (unsigned = 0)     The number of pixels at the right side
 *                                     which can't be used by the positioner.
 * @end_table
 *
 * The following states exist:
 * * state_enabled, the slider is enabled.
 * * state_disabled, the slider is disabled.
 * * state_pressed, the left mouse button is down on the positioner of the
 *   slider.
 * * state_focussed, the mouse is over the positioner of the slider.
 */
	VALIDATE(minimum_positioner_length
			, missing_mandatory_wml_key(
				  "resolution"
				, "minimum_positioner_length"));

	// Note the order should be the same as the enum tstate is slider.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
	state.push_back(tstate_definition(cfg.child("state_pressed")));
	state.push_back(tstate_definition(cfg.child("state_focussed")));
}

} // namespace gui2

