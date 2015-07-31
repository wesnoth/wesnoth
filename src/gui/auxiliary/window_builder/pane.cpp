/*
   Copyright (C) 2012 - 2015 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/window_builder/pane.hpp"

#include "config.hpp"
#include "gettext.hpp"
#include "gui/widgets/pane.hpp"
#include "wml_exception.hpp"

namespace gui2
{

namespace implementation
{

tbuilder_pane::tbuilder_pane(const config& cfg)
	: tbuilder_widget(cfg)
	, grow_direction(
			  lexical_cast<tplacer_::tgrow_direction>(cfg["grow_direction"]))
	, parallel_items(cfg["parallel_items"])
	, item_definition(new tbuilder_grid(cfg.child("item_definition", "[pane]")))
{
	VALIDATE(parallel_items > 0, _("Need at least 1 parallel item."));
}

twidget* tbuilder_pane::build() const
{
	return build(treplacements());
}

twidget* tbuilder_pane::build(const treplacements& /*replacements*/) const
{
	return tpane::build(*this);
}

} // namespace implementation

} // namespace gui2

/*WIKI_MACRO
 * @begin{macro}{pane_description}
 *
 *        A pane is a container where new members can be added and removed
 *        during run-time.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_viewport
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="pane"}{min=0}{max=-1}{super="generic/widget_instance"}
 * == Label ==
 *
 * @macro = viewport_description
 *
 * List with the label specific variables:
 * @begin{table}{config}
 *     grow_direction & grow_direction & &
 *                                The direction in which new items grow. $
 *     parallel_items & unsigned & &
 *                                The number of items that are growing in
 *                                parallel. $
 *     item_definition & section & &
 *                                The definition of a new item. $
 * @end{table}
 *
 * @begin{tag}{name="item_definition"}{min=1}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="item_definition"}
 * @end{tag}{name="pane"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */
