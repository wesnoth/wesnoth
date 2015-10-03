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

#include "gui/auxiliary/window_builder/multi_page.hpp"

#include "config.hpp"
#include "foreach.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/multi_page.hpp"
#include "gui/widgets/multi_page.hpp"
#include "wml_exception.hpp"

namespace gui2 {

namespace implementation {

tbuilder_multi_page::tbuilder_multi_page(const config& cfg) :
	implementation::tbuilder_control(cfg),
	builder(NULL),
	data()
{
	const config &page = cfg.child("page_definition");

	VALIDATE(page, _("No page defined."));
	builder = new tbuilder_grid(page);
	assert(builder);

	/** @todo This part is untested. */
	const config &d = cfg.child("page_data");
	if(!d){
		return;
	}

	BOOST_FOREACH(const config &row, d.child_range("row")) {
		unsigned col = 0;

		BOOST_FOREACH(const config &column, row.child_range("column")) {
			data.push_back(string_map());
			BOOST_FOREACH(const config::attribute &i, column.attribute_range()) {
				data.back()[i.first] = i.second;
			}
			++col;
		}

		VALIDATE(col == builder->cols, _("'list_data' must have "
			"the same number of columns as the 'list_definition'."));
	}
}

twidget* tbuilder_multi_page::build() const
{
	tmulti_page *widget = new tmulti_page();

	init_control(widget);

	widget->set_page_builder(builder);

	DBG_GUI_G << "Window builder: placed multi_page '"
			<< id << "' with defintion '"
			<< definition << "'.\n";

	boost::intrusive_ptr<const tmulti_page_definition::tresolution> conf =
		boost::dynamic_pointer_cast
		<const tmulti_page_definition::tresolution>(widget->config());
	assert(conf);

	widget->init_grid(conf->grid);

	widget->finalize(data);

	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI_MACRO
 * @start_macro = multi_page_description
 *
 *        A multi page is a control that contains serveral 'pages' of which
 *        only one is visible. The pages can contain the same of different
 *        info.
 * @end_macro
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_multi_page
 *
 * == Multi page ==
 *
 * @macro = multi_page_description
 *
 * List with the multi page specific variables:
 * @start_table = config
 *     page_definition (section)       This defines how a listbox item
 *                                     looks. It must contain the grid
 *                                     definition for 1 row of the list.
 *
 *     page_data(section = [])         A grid alike section which stores the
 *                                     initial data for the listbox. Every row
 *                                     must have the same number of columns as
 *                                     the 'list_definition'.
 * @end_table
 */

