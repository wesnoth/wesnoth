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

#include "gui/auxiliary/window_builder/multi_page.hpp"

#include "config.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/multi_page.hpp"
#include "gui/widgets/multi_page.hpp"
#include "utils/foreach.tpp"
#include "wml_exception.hpp"

namespace gui2
{

namespace implementation
{

tbuilder_multi_page::tbuilder_multi_page(const config& cfg)
	: implementation::tbuilder_control(cfg), builder(NULL), data()
{
	const config& page = cfg.child("page_definition");

	VALIDATE(page, _("No page defined."));
	builder = new tbuilder_grid(page);
	assert(builder);

	/** @todo This part is untested. */
	const config& d = cfg.child("page_data");
	if(!d) {
		return;
	}

	FOREACH(const AUTO & row, d.child_range("row"))
	{
		unsigned col = 0;

		FOREACH(const AUTO & column, row.child_range("column"))
		{
			data.push_back(string_map());
			FOREACH(const AUTO & i, column.attribute_range())
			{
				data.back()[i.first] = i.second;
			}
			++col;
		}

		VALIDATE(col == builder->cols,
				 _("'list_data' must have "
				   "the same number of columns as the 'list_definition'."));
	}
}

twidget* tbuilder_multi_page::build() const
{
	tmulti_page* widget = new tmulti_page();

	init_control(widget);

	widget->set_page_builder(builder);

	DBG_GUI_G << "Window builder: placed multi_page '" << id
			  << "' with definition '" << definition << "'.\n";

	boost::intrusive_ptr<const tmulti_page_definition::tresolution>
	conf = boost::
			dynamic_pointer_cast<const tmulti_page_definition::tresolution>(
					widget->config());
	assert(conf);

	widget->init_grid(conf->grid);

	widget->finalize(data);

	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI_MACRO
 * @begin{macro}{multi_page_description}
 *
 *        A multi page is a control that contains several 'pages' of which
 *        only one is visible. The pages can contain the same widgets containing
 *        the same 'kind' of data or look completely different.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_multi_page
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="multi_page"}{min=0}{max=-1}{super="generic/widget_instance"}
 * == Multi page ==
 *
 * @macro = multi_page_description
 *
 * List with the multi page specific variables:
 * @begin{table}{config}
 *     page_definition & section & &   This defines how a multi page item
 *                                     looks. It must contain the grid
 *                                     definition for at least one page. $
 *
 *     page_data & section & [] &      A grid alike section which stores the
 *                                     initial data for the multi page. Every
 *                                     row must have the same number of columns
 *                                     as the 'page_definition'. $
 *     horizontal_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar.
 *     vertical_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar.
 * @end{table}
 * @begin{tag}{name="page_definition"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="page_definition"}
 * @begin{tag}{name="page_data"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="page_data"}
 * @end{tag}{name="multi_page"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */
