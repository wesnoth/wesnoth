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

#include "gui/auxiliary/window_builder/viewport.hpp"

#include "config.hpp"
#include "gui/widgets/viewport.hpp"

namespace gui2
{

namespace implementation
{

tbuilder_viewport::tbuilder_viewport(const config& cfg)
	: tbuilder_widget(cfg)
	, widget(create_builder_widget(cfg.child("widget", "[viewport]")))
{
}

twidget* tbuilder_viewport::build() const
{
	return build(treplacements());
}

twidget* tbuilder_viewport::build(const treplacements& replacements) const
{
	return tviewport::build(*this, replacements);
}

} // namespace implementation

} // namespace gui2


/*WIKI_MACRO
 * @begin{macro}{viewport_description}
 *
 *        A viewport is an special widget used to view only a part of the
 *        widget it `holds'.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_viewport
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="viewport"}{min=0}{max=-1}{super="generic/widget_instance"}
 * @begin{tag}{name="widget"}{min="1"}{max="1"}{super="gui/window/resolution/grid/row/column"}
 * == Label ==
 *
 * @macro = viewport_description
 *
 * List with the label specific variables:
 * @begin{table}{config}
 *     widget & section & &       Holds a single widget like a grid cell.$
 * @end{table}
 * @end{tag}{name="widget"}
 * @end{tag}{name="viewport"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */
