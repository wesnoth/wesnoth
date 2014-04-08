/*
   Copyright (C) 2008 - 2014 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/window_builder/scroll_label.hpp"

#include "config.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/window_builder/helper.hpp"
#include "gui/auxiliary/widget_definition/scroll_label.hpp"
#include "gui/widgets/scroll_label.hpp"

namespace gui2
{

namespace implementation
{

tbuilder_scroll_label::tbuilder_scroll_label(const config& cfg)
	: implementation::tbuilder_control(cfg)
	, vertical_scrollbar_mode(
			  get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			  get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
{
}

twidget* tbuilder_scroll_label::build() const
{
	tscroll_label* widget = new tscroll_label();

	init_control(widget);

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	boost::intrusive_ptr<const tscroll_label_definition::tresolution>
	conf = boost::
			dynamic_pointer_cast<const tscroll_label_definition::tresolution>(
					widget->config());
	assert(conf);

	widget->init_grid(conf->grid);
	widget->finalize_setup();

	DBG_GUI_G << "Window builder: placed scroll label '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI_MACRO
 * @begin{macro}{scroll_label_description}
 *
 *        A scroll label is a label that wraps its text and also has a
 *        vertical scrollbar. This way a text can't be too long to be shown
 *        for this widget.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_scroll_label
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="scroll_label"}{min="0"}{max="-1"}{super="generic/widget_instance"}
 * == Scroll label ==
 *
 * @macro = scroll_label_description
 *
 * List with the scroll label specific variables:
 * @begin{table}{config}
 *     vertical_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 *     horizontal_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 * @end{table}
 * @end{tag}{name="scroll_label"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */
