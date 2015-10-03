/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/window_builder/horizontal_listbox.hpp"

#include "foreach.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/listbox.hpp"
#include "gui/auxiliary/window_builder/helper.hpp"
#include "gui/widgets/listbox.hpp"
#include "wml_exception.hpp"

namespace gui2 {

namespace implementation {

tbuilder_horizontal_listbox::tbuilder_horizontal_listbox(const config& cfg)
	: tbuilder_control(cfg)
	, vertical_scrollbar_mode(
			get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, list_builder(NULL)
	, list_data()
{
	const config &l = cfg.child("list_definition");

	VALIDATE(l, _("No list defined."));
	list_builder = new tbuilder_grid(l);
	assert(list_builder);
	VALIDATE(list_builder->rows == 1
			, _("A 'list_definition' should contain one row."));

	const config &data = cfg.child("list_data");
	if (!data) return;

	BOOST_FOREACH(const config &row, data.child_range("row")) {
		unsigned col = 0;

		BOOST_FOREACH(const config &c, row.child_range("column")) {
			list_data.push_back(string_map());
			BOOST_FOREACH (const config::attribute &i, c.attribute_range()) {
				list_data.back()[i.first] = i.second;
			}
			++col;
		}

		VALIDATE(col == list_builder->cols, _("'list_data' must have "
			"the same number of columns as the 'list_definition'."));
	}
}

twidget* tbuilder_horizontal_listbox::build() const
{
	tlistbox *widget = new tlistbox(
			true, true, tgenerator_::horizontal_list, true);

	init_control(widget);

	widget->set_list_builder(list_builder); // FIXME in finalize???

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	DBG_GUI_G << "Window builder: placed listbox '"
		<< id << "' with defintion '"
		<< definition << "'.\n";

	boost::intrusive_ptr<const tlistbox_definition::tresolution> conf =
		boost::dynamic_pointer_cast
		<const tlistbox_definition::tresolution>(widget->config());
	assert(conf);

	widget->init_grid(conf->grid);

	widget->finalize(NULL, NULL, list_data);

	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI_MACRO
 * @start_macro = horizontal_listbox_description
 *
 *        A horizontal listbox is a control that holds several items of the
 *        same type.  Normally the items in a listbox are ordered in rows,
 *        this version orders them vertically instead.
 * @end_macro
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_horizontal_listbox
 *
 * == Horizontal listbox ==
 *
 * @macro = horizontal_listbox_description
 *
 * List with the horizontal listbox specific variables:
 * @start_table = config
 *     vertical_scrollbar_mode (scrollbar_mode = initial_auto)
 *                                     Determines whether or not to show the
 *                                     scrollbar.
 *     horizontal_scrollbar_mode (scrollbar_mode = initial_auto)
 *                                     Determines whether or not to show the
 *                                     scrollbar.
 *
 *     list_definition (section)       This defines how a listbox item
 *                                     looks. It must contain the grid
 *                                     definition for 1 row of the list.
 *
 *     list_data(section = [])         A grid alike section which stores the
 *                                     initial data for the listbox. Every row
 *                                     must have the same number of columns as
 *                                     the 'list_definition'.
 *
 * @end_table
 *
 * In order to force widgets to be the same size inside a horizontal listbox,
 * the widgets need to be inside a linked_group.
 *
 * Inside the list section there are only the following widgets allowed
 * * grid (to nest)
 * * selectable widgets which are
 * ** toggle_button
 * ** toggle_panel
 *
 */

