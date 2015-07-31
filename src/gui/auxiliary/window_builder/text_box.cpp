/*
   Copyright (C) 2008 - 2015 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/window_builder/text_box.hpp"

#include "config.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/text_box.hpp"

namespace gui2
{

namespace implementation
{

tbuilder_text_box::tbuilder_text_box(const config& cfg)
	: tbuilder_control(cfg), history(cfg["history"])
{
}

twidget* tbuilder_text_box::build() const
{
	ttext_box* widget = new ttext_box();

	init_control(widget);

	// A textbox doesn't have a label but a text
	widget->set_value(label);

	if(!history.empty()) {
		widget->set_history(history);
	}

	DBG_GUI_G << "Window builder: placed text box '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_text_box
 *
 * == Text box ==
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="text_box"}{min="0"}{max="-1"}{super="generic/widget_instance"}
 * @begin{table}{config}
 *     label & t_string & "" &          The initial text of the text box. $
 *     history & string & "" &         The name of the history for the text
 *                                     box.
 *                                     A history saves the data entered in a
 *                                     text box between the games. With the up
 *                                     and down arrow it can be accessed. To
 *                                     create a new history item just add a
 *                                     new unique name for this field and the
 *                                     engine will handle the rest. $
 * @end{table}
 * @end{tag}{name="text_box"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */
