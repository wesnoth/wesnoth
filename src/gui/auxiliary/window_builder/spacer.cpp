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

#include "gui/auxiliary/window_builder/spacer.hpp"

#include "config.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/spacer.hpp"

namespace gui2
{

namespace implementation
{

tbuilder_spacer::tbuilder_spacer(const config& cfg)
	: tbuilder_control(cfg), width_(cfg["width"]), height_(cfg["height"])
{
}

twidget* tbuilder_spacer::build() const
{
	tspacer* widget = new tspacer();

	init_control(widget);

	const game_logic::map_formula_callable& size = get_screen_size_variables();

	const unsigned width = width_(size);
	const unsigned height = height_(size);

	if(width || height) {
		widget->set_best_size(tpoint(width, height));
	}

	DBG_GUI_G << "Window builder: placed spacer '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

} // namespace gui2

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
