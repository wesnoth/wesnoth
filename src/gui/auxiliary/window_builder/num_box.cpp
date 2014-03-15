/*
   Copyright (C) 2014 by 
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

#include "gui/auxiliary/window_builder/num_box.hpp"

#include "config.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/num_box.hpp"


namespace gui2
{

namespace implementation
{

tbuilder_num_box::tbuilder_num_box(const config& cfg)
	: tbuilder_control(cfg)
	, history_(cfg["history"])
	, minimum_value_(cfg["minimum_value"])
	, maximum_value_(cfg["maximum_value"])
	, value_(cfg["value"])
{
}

twidget* tbuilder_num_box::build() const
{
	tnum_box* widget = new tnum_box();

	init_control(widget);

	// set keys for numerical interface
	widget->set_maximum_value(maximum_value_);
	widget->set_minimum_value(minimum_value_);
	widget->set_value(value_);

	if(!history_.empty()) {
		widget->set_history(history_);
	}

	DBG_GUI_G << "Window builder: placed number box '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_num_box
 *
 * == Number box ==
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="num_box"}{min="0"}{max="-1"}{super="generic/widget_instance"}
 * @begin{table}{config}
 *     history & string & "" &          See the description of the text box. $
 *
 *     minimum_value & int & 0 &        The minimum value the number box can hold. $
 *     maximum_value & int & 0 &        The maximum value the number box can hold. $
 *     value & int & 0 &                The value of the number box. $
 * @end{table}
 * @end{tag}{name="num_box"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */
