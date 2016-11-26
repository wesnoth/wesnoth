/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/spacer.hpp"

#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"

#include "utils/functional.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(spacer)

point spacer::calculate_best_size() const
{
	return best_size_ != point() ? best_size_
									  : styled_widget::calculate_best_size();
}

void spacer::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool spacer::get_active() const
{
	return true;
}

unsigned spacer::get_state() const
{
	return 0;
}

bool spacer::disable_click_dismiss() const
{
	return false;
}

void spacer::impl_draw_background(surface& /*frame_buffer*/
								   ,
								   int /*x_offset*/
								   ,
								   int /*y_offset*/)
{
	/* DO NOTHING */
}

const std::string& spacer::get_control_type() const
{
	static const std::string type = "spacer";
	return type;
}

// }---------- DEFINITION ---------{

spacer_definition::spacer_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing spacer " << id << '\n';

	load_resolutions<resolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_spacer
 *
 * == Spacer ==
 *
 * @macro = spacer_description
 *
 * A spacer has no states so nothing to load.
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="spacer_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @end{tag}{name="spacer_definition"}
 * @end{parent}{name="gui/"}
 */
spacer_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{spacer_description}
 *
 *        A spacer is a dummy item to either fill in a widget since no empty
 *        items are allowed or to reserve a fixed space.
 * @end{macro}
 */


/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_spacer
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="spacer"}{min=0}{max=-1}{super="generic/widget_instance"}
 * == Spacer ==
 *
 * @macro = spacer_description
 *
 * If either the width or the height is non-zero the spacer functions as a
 * fixed size spacer.
 *
 * @begin{table}{config}
 *     width & f_unsigned & 0 &          The width of the spacer. $
 *     height & f_unsigned & 0 &         The height of the spacer. $
 * @end{table}
 *
 * The variable available are the same as for the window resolution see
 * http://www.wesnoth.org/wiki/GUIToolkitWML#Resolution_2 for the list of
 * items.
 * @end{tag}{name="spacer"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

builder_spacer::builder_spacer(const config& cfg)
	: builder_styled_widget(cfg), width_(cfg["width"]), height_(cfg["height"])
{
}

widget* builder_spacer::build() const
{
	spacer* widget = new spacer();

	init_control(widget);

	const game_logic::map_formula_callable& size = get_screen_size_variables();

	const unsigned width = width_(size);
	const unsigned height = height_(size);

	if(width || height) {
		widget->set_best_size(point(width, height));
	}

	DBG_GUI_G << "Window builder: placed spacer '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
