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

#include "gui/auxiliary/window_builder/toggle_button.hpp"

#include "config.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/window_builder/helper.hpp"
#include "gui/widgets/toggle_button.hpp"

namespace gui2
{

namespace implementation
{

tbuilder_toggle_button::tbuilder_toggle_button(const config& cfg)
	: tbuilder_control(cfg)
	, icon_name_(cfg["icon"])
	, retval_id_(cfg["return_value_id"])
	, retval_(cfg["return_value"])
{
}

twidget* tbuilder_toggle_button::build() const
{
	ttoggle_button* widget = new ttoggle_button();

	init_control(widget);

	widget->set_icon_name(icon_name_);
	widget->set_retval(get_retval(retval_id_, retval_, id));

	DBG_GUI_G << "Window builder: placed toggle button '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI
 * @page = GUIToolkitWML
 * @order = 2_toggle_button
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="toggle_button"}{min=0}{max=-1}{super="generic/widget_instance"}
 * == Toggle button ==
 *
 * @begin{table}{config}
 *     icon & f_string & "" &          The name of the icon file to show. $
 *     return_value_id & string & "" & The return value id, see
 *                                     [[GUIToolkitWML#Button]] for more
 *                                     information. $
 *     return_value & int & 0 &        The return value, see
 *                                     [[GUIToolkitWML#Button]] for more
 *                                     information. $
 * @end{table}
 * @end{tag}{name="toggle_button"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */
