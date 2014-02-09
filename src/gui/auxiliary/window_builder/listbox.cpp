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

#include "gui/auxiliary/window_builder/listbox.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/listbox.hpp"
#include "gui/auxiliary/window_builder/helper.hpp"
#include "gui/widgets/grid.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/pane.hpp"
#include "gui/widgets/viewport.hpp"
#include "gui/widgets/settings.hpp"
#include "utils/foreach.tpp"
#include "wml_exception.hpp"

namespace gui2
{

namespace implementation
{

tbuilder_listbox::tbuilder_listbox(const config& cfg)
	: tbuilder_control(cfg)
	, vertical_scrollbar_mode(
			  get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			  get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, header(NULL)
	, footer(NULL)
	, list_builder(NULL)
	, list_data()
{
	if(const config& h = cfg.child("header")) {
		header = new tbuilder_grid(h);
	}

	if(const config& f = cfg.child("footer")) {
		footer = new tbuilder_grid(f);
	}

	const config& l = cfg.child("list_definition");

	VALIDATE(l, _("No list defined."));
	list_builder = new tbuilder_grid(l);
	assert(list_builder);
	VALIDATE(list_builder->rows == 1,
			 _("A 'list_definition' should contain one row."));

	const config& data = cfg.child("list_data");
	if(!data) {
		return;
	}

	FOREACH(const AUTO & row, data.child_range("row"))
	{
		unsigned col = 0;

		FOREACH(const AUTO & c, row.child_range("column"))
		{
			list_data.push_back(string_map());
			FOREACH(const AUTO & i, c.attribute_range())
			{
				list_data.back()[i.first] = i.second;
			}
			++col;
		}

		VALIDATE(col == list_builder->cols,
				 _("'list_data' must have the same number of "
				   "columns as the 'list_definition'."));
	}
}

twidget* tbuilder_listbox::build() const
{
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	tlist* widget = new tlist(
			true, true, tgenerator_::vertical_list, true, list_builder);

	init_control(widget);
	if(!list_data.empty()) {
		widget->append_rows(list_data);
	}
	return widget;
#else
	if(new_widgets) {

		tpane* pane = new tpane(list_builder);
		pane->set_id(id);


		tgrid* grid = new tgrid();
		grid->set_rows_cols(1, 1);
#if 0
		grid->set_child(
				  pane
				, 0
				, 0
				, tgrid::VERTICAL_GROW_SEND_TO_CLIENT
					| tgrid::HORIZONTAL_GROW_SEND_TO_CLIENT
				, tgrid::BORDER_ALL);
#else
		tviewport* viewport = new tviewport(*pane);
		grid->set_child(viewport,
						0,
						0,
						tgrid::VERTICAL_GROW_SEND_TO_CLIENT
						| tgrid::HORIZONTAL_GROW_SEND_TO_CLIENT,
						tgrid::BORDER_ALL);
#endif
		return grid;
	}

	tlistbox* widget
			= new tlistbox(true, true, tgenerator_::vertical_list, true);

	init_control(widget);

	widget->set_list_builder(list_builder); // FIXME in finalize???

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	DBG_GUI_G << "Window builder: placed listbox '" << id
			  << "' with definition '" << definition << "'.\n";

	boost::intrusive_ptr<const tlistbox_definition::tresolution>
	conf = boost::dynamic_pointer_cast<const tlistbox_definition::tresolution>(
			widget->config());
	assert(conf);

	widget->init_grid(conf->grid);

	widget->finalize(header, footer, list_data);

	return widget;
#endif
}

} // namespace implementation

} // namespace gui2

/*WIKI_MACRO
 * @begin{macro}{listbox_description}
 *
 *        A listbox is a control that holds several items of the same type.
 *        Normally the items in a listbox are ordered in rows, this version
 *        might allow more options for ordering the items in the future.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_listbox
 *
 * == Listbox ==
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="listbox"}{min=0}{max=-1}{super="generic/widget_instance"}
 * @macro = listbox_description
 *
 * List with the listbox specific variables:
 * @begin{table}{config}
 *     vertical_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 *     horizontal_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 *
 *     header & grid & [] &            Defines the grid for the optional
 *                                     header. (This grid will automatically
 *                                     get the id _header_grid.) $
 *     footer & grid & [] &            Defines the grid for the optional
 *                                     footer. (This grid will automatically
 *                                     get the id _footer_grid.) $
 *
 *     list_definition & section & &   This defines how a listbox item
 *                                     looks. It must contain the grid
 *                                     definition for 1 row of the list. $
 *
 *     list_data & section & [] &      A grid alike section which stores the
 *                                     initial data for the listbox. Every row
 *                                     must have the same number of columns as
 *                                     the 'list_definition'. $
 *
 * @end{table}
 * @begin{tag}{name="header"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="header"}
 * @begin{tag}{name="footer"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="footer"}
 * @begin{tag}{name="list_definition"}{min=0}{max=1}
 * @begin{tag}{name="row"}{min=1}{max=1}{super="generic/listbox_grid/row"}
 * @end{tag}{name="row"}
 * @end{tag}{name="list_definition"}x
 * @begin{tag}{name="list_data"}{min=0}{max=1}{super="generic/listbox_grid"}
 * @end{tag}{name="list_data"}
 *
 * In order to force widgets to be the same size inside a listbox, the widgets
 * need to be inside a linked_group.
 *
 * Inside the list section there are only the following widgets allowed
 * * grid (to nest)
 * * selectable widgets which are
 * ** toggle_button
 * ** toggle_panel
 * @end{tag}{name="listbox"}
 *
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

/*WIKI
 * @begin{parent}{name="generic/"}
 * @begin{tag}{name="listbox_grid"}{min="0"}{max="-1"}
 * @begin{tag}{name="row"}{min="0"}{max="-1"}
 * @begin{table}{config}
 *     grow_factor & unsigned & 0 &      The grow factor for a row. $
 * @end{table}
 * @begin{tag}{name="column"}{min="0"}{max="-1"}{super="gui/window/resolution/grid/row/column"}
 * @begin{table}{config}
 *     label & t_string & "" &  $
 *     tooltip & t_string & "" &  $
 *     icon & t_string & "" &  $
 * @end{table}
 * @allow{link}{name="gui/window/resolution/grid/row/column/toggle_button"}
 * @allow{link}{name="gui/window/resolution/grid/row/column/toggle_panel"}
 * @end{tag}{name="column"}
 * @end{tag}{name="row"}
 * @end{tag}{name="listbox_grid"}
 * @end{parent}{name="generic/"}
 */
