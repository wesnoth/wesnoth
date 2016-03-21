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

#include "gui/widgets/multi_page.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/generator.hpp"

#include "gettext.hpp"

#include <boost/bind.hpp>

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(multi_page)
tmulti_page::tmulti_page()
	: tcontainer_(0)
	, generator_(
			  tgenerator_::build(true, true, tgenerator_::independent, false))
	, page_builder_(NULL)
{
}

void tmulti_page::add_page(const string_map& item)
{
	assert(generator_);
	generator_->create_item(-1, page_builder_, item, NULL);
}

void tmulti_page::add_page(
		const std::map<std::string /* widget id */, string_map>& data)
{
	assert(generator_);
	generator_->create_item(-1, page_builder_, data, NULL);
}

void tmulti_page::remove_page(const unsigned page, unsigned count)
{
	assert(generator_);

	if(page >= get_page_count()) {
		return;
	}

	if(!count || count > get_page_count()) {
		count = get_page_count();
	}

	for(; count; --count) {
		generator_->delete_item(page);
	}
}

void tmulti_page::clear()
{
	assert(generator_);
	generator_->clear();
}

unsigned tmulti_page::get_page_count() const
{
	assert(generator_);
	return generator_->get_item_count();
}

void tmulti_page::select_page(const unsigned page, const bool select)
{
	assert(generator_);
	generator_->select_item(page, select);
}

int tmulti_page::get_selected_page() const
{
	assert(generator_);
	return generator_->get_selected_item();
}

const tgrid& tmulti_page::page_grid(const unsigned page) const
{
	assert(generator_);
	return generator_->item(page);
}

tgrid& tmulti_page::page_grid(const unsigned page)
{
	assert(generator_);
	return generator_->item(page);
}

bool tmulti_page::get_active() const
{
	return true;
}

unsigned tmulti_page::get_state() const
{
	return 0;
}

namespace
{

/**
 * Swaps an item in a grid for another one.*/
void swap_grid(tgrid* grid,
			   tgrid* content_grid,
			   twidget* widget,
			   const std::string& id)
{
	assert(content_grid);
	assert(widget);

	// Make sure the new child has same id.
	widget->set_id(id);

	// Get the container containing the wanted widget.
	tgrid* parent_grid = NULL;
	if(grid) {
		parent_grid = find_widget<tgrid>(grid, id, false, false);
	}
	if(!parent_grid) {
		parent_grid = find_widget<tgrid>(content_grid, id, true, false);
	}
	parent_grid = dynamic_cast<tgrid*>(parent_grid->parent());
	assert(parent_grid);

	// Replace the child.
	widget = parent_grid->swap_child(id, widget, false);
	assert(widget);

	delete widget;
}

} // namespace

void tmulti_page::finalize(const std::vector<string_map>& page_data)
{
	assert(generator_);
	generator_->create_items(-1, page_builder_, page_data, NULL);
	swap_grid(NULL, &grid(), generator_, "_content_grid");
}

void tmulti_page::impl_draw_background(surface& /*frame_buffer*/
									   ,
									   int /*x_offset*/
									   ,
									   int /*y_offset*/)
{
	/* DO NOTHING */
}

const std::string& tmulti_page::get_control_type() const
{
	static const std::string type = "multi_page";
	return type;
}

void tmulti_page::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

// }---------- DEFINITION ---------{

tmulti_page_definition::tmulti_page_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing multipage " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_multi_page
 *
 * == Multi page ==
 *
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="multi_page_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @macro = multi_page_description
 *
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * @begin{table}{config}
 *     grid & grid & &                    A grid containing the widgets for main
 *                                     widget. $
 * @end{table}
 * @allow{link}{name="gui/window/resolution/grid"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="multi_page_definition"}
 * @end{parent}{name="gui/"}
 * A multipage has no states.
 */
tmulti_page_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg), grid(NULL)
{
	const config& child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = new tbuilder_grid(child);
}

// }---------- BUILDER -----------{

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

// }------------ END --------------

} // namespace gui2
