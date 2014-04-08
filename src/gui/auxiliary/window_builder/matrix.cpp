/*
   Copyright (C) 2012 - 2014 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/window_builder/matrix.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/matrix.hpp"
#include "gui/auxiliary/window_builder/helper.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/matrix.hpp"
#include "gui/widgets/pane.hpp"
#include "gui/widgets/viewport.hpp"
#include "gui/widgets/settings.hpp"
#include "wml_exception.hpp"

namespace gui2
{

namespace implementation
{

tbuilder_matrix::tbuilder_matrix(const config& cfg)
	: tbuilder_control(cfg)
	, vertical_scrollbar_mode(
			  get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			  get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, builder_top(NULL)
	, builder_bottom(NULL)
	, builder_left(NULL)
	, builder_right(NULL)
	, builder_main(create_builder_widget(cfg.child("main", "[matrix]")))
{
	if(const config& top = cfg.child("top")) {
		builder_top = new tbuilder_grid(top);
	}

	if(const config& bottom = cfg.child("bottom")) {
		builder_bottom = new tbuilder_grid(bottom);
	}

	if(const config& left = cfg.child("left")) {
		builder_left = new tbuilder_grid(left);
	}

	if(const config& right = cfg.child("right")) {
		builder_right = new tbuilder_grid(right);
	}
}

twidget* tbuilder_matrix::build() const
{
	return tmatrix::build(*this);
}

} // namespace implementation

} // namespace gui2

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_matrix
 *
 * == Listbox ==
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="matrix"}{min=0}{max=-1}{super="generic/widget_instance"}
 *
 *
 * List with the matrix specific variables:
 * @begin{table}{config}
 *     vertical_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 *     horizontal_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 * @end{table}
 *
 *
 * @begin{tag}{name="top"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="top"}
 * @begin{tag}{name="bottom"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="bottom"}
 *
 * @begin{tag}{name="left"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="left"}
 * @begin{tag}{name="right"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="right"}
 *
 * @begin{tag}{name="main"}{min="1"}{max="1"}{super="gui/window/resolution/grid/row/column"}
 * @end{tag}{name="main"}
 * @end{tag}{name="matrix"}
 *
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */
