/* $Id$ */
/*
   Copyright (C) 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/stacked_widget.hpp"

#include "gui/widgets/generator.hpp"

namespace gui2 {

tstacked_widget::tstacked_widget()
	: tcontainer_(1)
	, generator_(NULL)
	, item_builder_(NULL)
{
	generator_ = tgenerator_::build(
			false, false, tgenerator_::independant, false);
}

void tstacked_widget::add_item(const string_map& item)
{
	assert(generator_);
	generator_->create_item(-1, item_builder_, item, NULL);
	generator_->select_item(get_item_count() - 1, true);
}

void tstacked_widget::add_item(
		const std::map<std::string /* widget id */, string_map>& data)
{
	assert(generator_);
	generator_->create_item(-1, item_builder_, data, NULL);
	generator_->select_item(get_item_count() - 1, true);
}

unsigned tstacked_widget::get_item_count() const
{
	assert(generator_);
	return generator_->get_item_count();
}

namespace {

/**
 * Swaps an item in a grid for another one.*/
void swap_grid(tgrid* grid,
		tgrid* content_grid, twidget* widget, const std::string& id)
{
	assert(content_grid);
	assert(widget);

	// Make sure the new child has same id.
	widget->set_id(id);

	// Get the container containing the wanted widget.
	tgrid* parent_grid = NULL;
	if(grid) {
		parent_grid = grid->find_widget<tgrid>(id, false, false);
	}
	if(!parent_grid) {
		parent_grid = content_grid->find_widget<tgrid>(id, true, false);
	}
	parent_grid = dynamic_cast<tgrid*>(parent_grid->parent());
	assert(parent_grid);

	// Replace the child.
	widget = parent_grid->swap_child(id, widget, false);
	assert(widget);

	delete widget;
}

} // namespace

void tstacked_widget::finalize(
		const std::vector<string_map>& item_data)
{
	assert(generator_);
	generator_->create_items(-1, item_builder_, item_data, NULL);
	swap_grid(NULL, &grid(), generator_, "_content_grid");

	for(size_t i = 0; i < get_item_count(); ++i) {
		generator_->select_item(i, true);
	}
}

} // namespace gui2

