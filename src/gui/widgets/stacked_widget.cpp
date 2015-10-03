/* $Id$ */
/*
   Copyright (C) 2009 - 2011 by Mark de Wever <koraq@xs4all.nl>
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

#include "foreach.hpp"
#include "gui/widgets/generator.hpp"

namespace gui2 {

tstacked_widget::tstacked_widget()
	: tcontainer_(1)
	, generator_(NULL)
{
	generator_ = tgenerator_::build(
			false, false, tgenerator_::independant, false);
}

void tstacked_widget::layout_children()
{
	assert(generator_);
	for(unsigned i = 0; i < generator_->get_item_count(); ++i) {
		generator_->item(i).layout_children();
	}
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

void tstacked_widget::finalize(
		std::vector<tbuilder_grid_const_ptr> widget_builder)
{
	assert(generator_);
	string_map empty_data;
	BOOST_FOREACH(const tbuilder_grid_const_ptr& builder, widget_builder) {
		generator_->create_item(-1, builder, empty_data, NULL);
	}
	swap_grid(NULL, &grid(), generator_, "_content_grid");

	for(size_t i = 0; i < generator_->get_item_count(); ++i) {
		generator_->select_item(i, true);
	}
}

const std::string& tstacked_widget::get_control_type() const
{
    static const std::string type = "stacked_widget";
    return type;
}

} // namespace gui2

