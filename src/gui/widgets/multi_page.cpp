/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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
#include "gui/widgets/widget_helpers.hpp"
#include "gui/widgets/generator.hpp"

#include "gettext.hpp"

#include "utils/functional.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(multi_page)

multi_page::multi_page(const implementation::builder_multi_page& builder)
	: container_base(builder, get_control_type())
	, generator_(generator_base::build(true, true, generator_base::independent, false))
	, page_builders_()
{
}

grid& multi_page::add_page(const string_map& item)
{
	assert(generator_);
	grid& page = generator_->create_item(-1, page_builders_.begin()->second, item, nullptr);

	return page;
}

grid& multi_page::add_page(const std::string& type, int insert_pos, const string_map& item)
{
	assert(generator_);
	auto it_builder = page_builders_.find(type);
	VALIDATE(it_builder != page_builders_.end(), "invalid page type '" + type + "'");
	return generator_->create_item(insert_pos, it_builder->second, item, nullptr);
}

grid& multi_page::add_page(
	const std::map<std::string /* widget id */, string_map>& data)
{
	assert(generator_);
	grid& page = generator_->create_item(-1, page_builders_.begin()->second, data, nullptr);

	return page;
}

grid& multi_page::add_page(
	const std::string& type, int insert_pos, const std::map<std::string /* widget id */, string_map>& data)
{
	assert(generator_);
	auto it_builder = page_builders_.find(type);
	VALIDATE(it_builder != page_builders_.end(), "invalid page type '" + type + "'");
	return generator_->create_item(insert_pos, it_builder->second, data, nullptr);
}

void multi_page::remove_page(const unsigned page, unsigned count)
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

void multi_page::clear()
{
	assert(generator_);
	generator_->clear();
}

unsigned multi_page::get_page_count() const
{
	assert(generator_);
	return generator_->get_item_count();
}

void multi_page::select_page(const unsigned page, const bool select)
{
	assert(generator_);
	generator_->select_item(page, select);
}

int multi_page::get_selected_page() const
{
	assert(generator_);
	return generator_->get_selected_item();
}

const grid& multi_page::page_grid(const unsigned page) const
{
	assert(generator_);
	return generator_->item(page);
}

grid& multi_page::page_grid(const unsigned page)
{
	assert(generator_);
	return generator_->item(page);
}

bool multi_page::get_active() const
{
	return true;
}

unsigned multi_page::get_state() const
{
	return 0;
}

void multi_page::finalize(const std::vector<string_map>& page_data)
{
	assert(generator_);
	generator_->create_items(-1, page_builders_.begin()->second, page_data, nullptr);
	swap_grid(nullptr, &get_grid(), generator_, "_content_grid");
}

void multi_page::impl_draw_background(int /*x_offset*/, int /*y_offset*/)
{
	/* DO NOTHING */
}

void multi_page::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

// }---------- DEFINITION ---------{

multi_page_definition::multi_page_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing multipage " << id << '\n';

	load_resolutions<resolution>(cfg);
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
multi_page_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid(nullptr)
{
	const config& child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = std::make_shared<builder_grid>(child);
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{multi_page_description}
 *
 *        A multi page is a styled_widget that contains several 'pages' of which
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

builder_multi_page::builder_multi_page(const config& cfg)
	: implementation::builder_styled_widget(cfg), builders(), data()
{
	for (const config& page : cfg.child_range("page_definition"))
	{
		auto builder = std::make_shared<builder_grid>(page);
		assert(builder);
		builders[page["id"]] = builder;
	}
	VALIDATE(!builders.empty(), _("No page defined."));

	/** @todo This part is untested. */
	const config& d = cfg.child("page_data");
	if(!d) {
		return;
	}

	auto builder = builders.begin()->second;
	for(const auto & row : d.child_range("row"))
	{
		unsigned col = 0;

		for(const auto & column : row.child_range("column"))
		{
			data.emplace_back();
			for(const auto & i : column.attribute_range())
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

widget* builder_multi_page::build() const
{
	multi_page* widget = new multi_page(*this);

	widget->set_page_builders(builders);

	DBG_GUI_G << "Window builder: placed multi_page '" << id
			  << "' with definition '" << definition << "'.\n";

	const auto conf = widget->cast_config_to<multi_page_definition>();
	assert(conf);

	widget->init_grid(conf->grid);

	widget->finalize(data);

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
