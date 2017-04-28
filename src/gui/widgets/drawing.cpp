/*
   Copyright (C) 2010 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/drawing.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"

#include "utils/functional.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(drawing)

point drawing::calculate_best_size() const
{
	return best_size_ != point() ? best_size_
									  : styled_widget::calculate_best_size();
}

void drawing::request_reduce_width(const unsigned maximum_width)
{
	if(best_size_ != point()) {
		// This drawing is of fixed size, do nothing.
	} else {
		styled_widget::request_reduce_width(maximum_width);
	}
}

void drawing::request_reduce_height(const unsigned maximum_height)
{
	if(best_size_ != point()) {
		// This drawing is of fixed size, do nothing.
	} else {
		styled_widget::request_reduce_height(maximum_height);
	}
}

void drawing::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool drawing::get_active() const
{
	return true;
}

unsigned drawing::get_state() const
{
	return 0;
}

bool drawing::disable_click_dismiss() const
{
	return false;
}

const std::string& drawing::get_control_type() const
{
	static const std::string type = "drawing";
	return type;
}

// }---------- DEFINITION ---------{

drawing_definition::drawing_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing drawing " << id << '\n';

	load_resolutions<resolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_drawing
 *
 * == Drawing ==
 *
 * @macro = drawing_description
 *
 * The definition of a drawing. The widget normally has no event interaction
 * so only one state is defined.
 *
 * The following states exist:
 * * state_enabled
 *     the drawing is enabled. The state is a dummy since the
 *     things really drawn are placed in the window instance.
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="drawing_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="drawing_definition"}
 * @end{parent}{name="gui/"}
 */
drawing_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	/*
	 * Note the order should be the same as the enum state_t in drawing.hpp.
	 * Normally the [draw] section is in the config, but for this widget the
	 * original draw section is ignored, so send a dummy.
	 */
	static const config dummy("draw");
	state.emplace_back(dummy);
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{drawing_description}
 *
 *        A drawing is widget with a fixed size and gives access to the
 *        canvas of the widget in the window instance. This allows special
 *        display only widgets.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_drawing
 *
 * == Spacer ==
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="drawing"}{min=0}{max=-1}{super="generic/widget_instance"}
 * @macro = drawing_description
 *
 * If either the width or the height is not zero the drawing functions as a
 * fixed size drawing.
 *
 * @begin{table}{config}
 *     width & f_unsigned & 0 &          The width of the drawing. $
 *     height & f_unsigned & 0 &         The height of the drawing. $
 *     draw & config & &                 The config containing the drawing. $
 * @end{table}
 * @allow{link}{name="generic/state/draw"}
 * @end{tag}{name="drawing"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 * The variable available are the same as for the window resolution see
 * http://www.wesnoth.org/wiki/GUIToolkitWML#Resolution_2 for the list of
 * items.
 */

namespace implementation
{

builder_drawing::builder_drawing(const config& cfg)
	: builder_styled_widget(cfg)
	, width(cfg["width"])
	, height(cfg["height"])
	, draw(cfg.child("draw"))
{
	assert(!draw.empty());
}

widget* builder_drawing::build() const
{
	drawing* widget = new drawing();

	init_control(widget);

	const wfl::map_formula_callable& size = get_screen_size_variables();

	const unsigned w = width(size);
	const unsigned h = height(size);

	if(w || h) {
		widget->set_best_size(point(w, h));
	}

	widget->set_drawing_data(draw);

	DBG_GUI_G << "Window builder: placed drawing '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
