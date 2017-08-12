/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/horizontal_scrollbar.hpp"

#include "gui/core/register_widget.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

#include "gui/widgets/settings.hpp"

#include "wml_exception.hpp"

#include "utils/functional.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(horizontal_scrollbar)

unsigned horizontal_scrollbar::minimum_positioner_length() const
{
	const auto conf = cast_config_to<horizontal_scrollbar_definition>();
	assert(conf);

	return conf->minimum_positioner_length;
}

unsigned horizontal_scrollbar::maximum_positioner_length() const
{
	const auto conf = cast_config_to<horizontal_scrollbar_definition>();
	assert(conf);

	return conf->maximum_positioner_length;
}

unsigned horizontal_scrollbar::offset_before() const
{
	const auto conf = cast_config_to<horizontal_scrollbar_definition>();
	assert(conf);

	return conf->left_offset;
}

unsigned horizontal_scrollbar::offset_after() const
{
	const auto conf = cast_config_to<horizontal_scrollbar_definition>();
	assert(conf);

	return conf->right_offset;
}

bool horizontal_scrollbar::on_positioner(const point& coordinate) const
{
	// Note we assume the positioner is over the entire height of the widget.
	return coordinate.x >= static_cast<int>(get_positioner_offset())
		   && coordinate.x < static_cast<int>(get_positioner_offset()
											  + get_positioner_length())
		   && coordinate.y > 0 && coordinate.y < static_cast<int>(get_height());
}

int horizontal_scrollbar::on_bar(const point& coordinate) const
{
	// Not on the widget, leave.
	if(static_cast<size_t>(coordinate.x) > get_width()
	   || static_cast<size_t>(coordinate.y) > get_height()) {
		return 0;
	}

	// we also assume the bar is over the entire width of the widget.
	if(static_cast<size_t>(coordinate.x) < get_positioner_offset()) {
		return -1;
	} else if(static_cast<size_t>(coordinate.x) > get_positioner_offset()
												  + get_positioner_length()) {

		return 1;
	} else {
		return 0;
	}
}

bool horizontal_scrollbar::in_orthogonal_range(const point& coordinate) const
{
	return static_cast<size_t>(coordinate.x) < get_width();
}

const std::string& horizontal_scrollbar::get_control_type() const
{
	static const std::string type = "horizontal_scrollbar";
	return type;
}

// }---------- DEFINITION ---------{

horizontal_scrollbar_definition::horizontal_scrollbar_definition(
		const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing horizontal scrollbar " << id << '\n';

	load_resolutions<resolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_vertical_scrollbar
 *
 * == Horizontal scrollbar ==
 *
 * @macro = horizontal_scrollbar_description
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="horizontal_scrollbar_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * The resolution for a horizontal scrollbar also contains the following keys:
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * @begin{table}{config}
 *     minimum_positioner_length & unsigned & &
 *                                     The minimum size the positioner is
 *                                     allowed to be. The engine needs to know
 *                                     this in order to calculate the best size
 *                                     for the positioner. $
 *     maximum_positioner_length & unsigned & 0 &
 *                                     The maximum size the positioner is
 *                                     allowed to be. If minimum and maximum are
 *                                     the same value the positioner is fixed
 *                                     size. If the maximum is 0 (and the
 *                                     minimum not) there's no maximum. $
 *     left_offset & unsigned & 0 &      The number of pixels at the left which
 *                                     can't be used by the positioner. $
 *     right_offset & unsigned & 0 &     The number of pixels at the right which
 *                                     can't be used by the positioner. $
 * @end{table}
 *
 * The following states exist:
 * * state_enabled, the horizontal scrollbar is enabled.
 * * state_disabled, the horizontal scrollbar is disabled.
 * * state_pressed, the left mouse button is down on the positioner of the
 *   horizontal scrollbar.
 * * state_focused, the mouse is over the positioner of the horizontal
 *   scrollbar.
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @begin{tag}{name="state_disabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled"}
 * @begin{tag}{name="state_pressed"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_pressed"}
 * @begin{tag}{name="state_focused"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_focused"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="horizontal_scrollbar_definition"}
 * @end{parent}{name="gui/"}
 */
horizontal_scrollbar_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
	, minimum_positioner_length(cfg["minimum_positioner_length"])
	, maximum_positioner_length(cfg["maximum_positioner_length"])
	, left_offset(cfg["left_offset"])
	, right_offset(cfg["right_offset"])
{
	VALIDATE(minimum_positioner_length,
			 missing_mandatory_wml_key("resolution",
									   "minimum_positioner_length"));

	// Note the order should be the same as the enum state_t is scrollbar.hpp.
	state.emplace_back(cfg.child("state_enabled"));
	state.emplace_back(cfg.child("state_disabled"));
	state.emplace_back(cfg.child("state_pressed"));
	state.emplace_back(cfg.child("state_focused"));
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{horizontal_scrollbar_description}
 *
 *        A horizontal scrollbar is a widget that shows a horizontal scrollbar.
 *        This widget is most of the time used in a container to control the
 *        scrolling of its contents.
 * @end{macro}
 */

/*WIKI
 * @page = GUIToolkitWML
 * @order = 2_horizontal_scrollbar
 *
 * == Horizontal scrollbar ==
 *
 * @macro = horizontal_scrollbar_description
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="horizontal_scrollbar"}{min=0}{max=-1}{super="generic/widget_instance"}
 * @end{tag}{name="horizontal_scrollbar"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 * A horizontal scrollbar has no special fields.
 */

namespace implementation
{

builder_horizontal_scrollbar::builder_horizontal_scrollbar(const config& cfg)
	: builder_styled_widget(cfg)
{
}

widget* builder_horizontal_scrollbar::build() const
{
	horizontal_scrollbar* widget = new horizontal_scrollbar();

	init_control(widget);

	DBG_GUI_G << "Window builder:"
			  << " placed horizontal scrollbar '" << id << "' with definition '"
			  << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
