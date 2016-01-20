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

#include "gui/auxiliary/window_builder/combobox.hpp"

#include "config.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/window_builder/helper.hpp"
#include "gui/widgets/combobox.hpp"
#include "utils/foreach.tpp"

namespace gui2
{

namespace implementation
{

tbuilder_combobox::tbuilder_combobox(const config& cfg)
	: tbuilder_control(cfg)
	, retval_id_(cfg["return_value_id"])
	, retval_(cfg["return_value"])
	, options_()
{
	FOREACH(const AUTO& option, cfg.child_range("option")) {
		options_.push_back(option["label"]);
	}
}

twidget* tbuilder_combobox::build() const
{
	tcombobox* widget = new tcombobox();

	init_control(widget);

	widget->set_retval(get_retval(retval_id_, retval_, id));
	widget->set_values(options_);

	DBG_GUI_G << "Window builder: placed combobox '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI_MACRO
 * @begin{macro}{combobox_description}
 *
 *        A combobox is a control to choose an element from a list of elements.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_combobox
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="combobox"}{min=0}{max=-1}{super="generic/widget_instance"}
 * == combobox ==
 *
 * @macro = combobox_description
 *
 * Instance of a combobox. When a combobox has a return value it sets the
 * return value for the window. Normally this closes the window and returns
 * this value to the caller. The return value can either be defined by the
 * user or determined from the id of the combobox. The return value has a
 * higher precedence as the one defined by the id. (Of course it's weird to
 * give a combobox an id and then override its return value.)
 *
 * When the combobox doesn't have a standard id, but you still want to use the
 * return value of that id, use return_value_id instead. This has a higher
 * precedence as return_value.
 *
 * List with the combobox specific variables:
 * @begin{table}{config}
 *     return_value_id & string & "" &   The return value id. $
 *     return_value & int & 0 &          The return value. $
 *
 * @end{table}
 * @end{tag}{name="combobox"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */
