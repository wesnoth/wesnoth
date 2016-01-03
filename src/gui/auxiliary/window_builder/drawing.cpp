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

#include "gui/auxiliary/window_builder/drawing.hpp"

#include "config.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/drawing.hpp"

namespace gui2
{

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

} // namespace gui2

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
