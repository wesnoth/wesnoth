/*
   Copyright (C) 2010 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/detail/register.hpp"
#include "gui/widgets/settings.hpp"

#include <boost/bind.hpp>

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(drawing)

tpoint tdrawing::calculate_best_size() const
{
	return best_size_ != tpoint(0, 0) ? best_size_
									  : tcontrol::calculate_best_size();
}

void tdrawing::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool tdrawing::get_active() const
{
	return true;
}

unsigned tdrawing::get_state() const
{
	return 0;
}

bool tdrawing::disable_click_dismiss() const
{
	return false;
}

const std::string& tdrawing::get_control_type() const
{
	static const std::string type = "drawing";
	return type;
}

// }---------- DEFINITION ---------{

tdrawing_definition::tdrawing_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing drawing " << id << '\n';

	load_resolutions<tresolution>(cfg);
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
tdrawing_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
{
	/*
	 * Note the order should be the same as the enum tstate in drawing.hpp.
	 * Normally the [draw] section is in the config, but for this widget the
	 * original draw section is ignored, so send a dummy.
	 */
	static const config dummy("draw");
	state.push_back(tstate_definition(dummy));
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

tbuilder_drawing::tbuilder_drawing(const config& cfg)
	: tbuilder_control(cfg)
	, width(cfg["width"])
	, height(cfg["height"])
	, draw(cfg.child("draw"))
{
	assert(!draw.empty());
}

twidget* tbuilder_drawing::build() const
{
	tdrawing* widget = new tdrawing();

	init_control(widget);

	const game_logic::map_formula_callable& size = get_screen_size_variables();

	const unsigned w = width(size);
	const unsigned h = height(size);

	if(w || h) {
		widget->set_best_size(tpoint(w, h));
	}

	widget->canvas().front().set_cfg(draw);

	DBG_GUI_G << "Window builder: placed drawing '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
