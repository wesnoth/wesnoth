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

#include "gui/auxiliary/window_builder/password_box.hpp"

#include "config.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/password_box.hpp"


namespace gui2
{

namespace implementation
{

tbuilder_password_box::tbuilder_password_box(const config& cfg)
	: tbuilder_control(cfg), history_(cfg["history"])
{
}

twidget* tbuilder_password_box::build() const
{
	tpassword_box* widget = new tpassword_box();

	init_control(widget);

	// A password box doesn't have a label but a text.
	// It also has no history.
	widget->set_value(label);

	DBG_GUI_G << "Window builder: placed password box '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_password_box
 *
 * == Password box ==
 *
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="password_box"}{min=0}{max=-1}{super="generic/widget_instance"}
 * @begin{table}{config}
 *     label & t_string & "" &         The initial text of the password box. $
 * @end{table}
 * @end{tag}{name="password_box"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */
