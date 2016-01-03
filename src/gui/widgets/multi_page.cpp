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

#include "gui/auxiliary/find_widget.tpp"
#include "gui/auxiliary/widget_definition/multi_page.hpp"
#include "gui/auxiliary/window_builder/multi_page.hpp"
#include "gui/widgets/detail/register.tpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/generator.hpp"

#include <boost/bind.hpp>

namespace gui2
{

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

} // namespace gui2
