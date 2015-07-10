/*
   Copyright (C) 2009 - 2015 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/stacked_widget.hpp"

#include "gui/auxiliary/find_widget.tpp"
#include "gui/auxiliary/widget_definition/stacked_widget.hpp"
#include "gui/auxiliary/window_builder/stacked_widget.hpp"
#include "gui/widgets/detail/register.tpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/generator.hpp"
#include "utils/foreach.tpp"

#include <boost/bind.hpp>

namespace gui2
{

REGISTER_WIDGET(stacked_widget)

tstacked_widget::tstacked_widget()
	: tcontainer_(1)
	, generator_(
			  tgenerator_::build(false, false, tgenerator_::independent, false))
	, selected_layer_(-1)
{
}

bool tstacked_widget::get_active() const
{
	return true;
}

unsigned tstacked_widget::get_state() const
{
	return 0;
}

void tstacked_widget::layout_children()
{
	assert(generator_);
	for(unsigned i = 0; i < generator_->get_item_count(); ++i) {
		generator_->item(i).layout_children();
	}
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

void
tstacked_widget::finalize(std::vector<tbuilder_grid_const_ptr> widget_builder)
{
	assert(generator_);
	string_map empty_data;
	FOREACH(const AUTO & builder, widget_builder)
	{
		generator_->create_item(-1, builder, empty_data, NULL);
	}
	swap_grid(NULL, &grid(), generator_, "_content_grid");

	select_layer(-1);
}

const std::string& tstacked_widget::get_control_type() const
{
	static const std::string type = "stacked_widget";
	return type;
}

void tstacked_widget::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

void tstacked_widget::select_layer_internal(const unsigned int layer, const bool select) const
{
	// Selecting a layer that's already selected appears to actually deselect
	// it, so make sure to only perform changes we want.
	if(generator_->is_selected(layer) != select) {
		generator_->select_item(layer, select);
	}
}

void tstacked_widget::select_layer(const int layer)
{
	const unsigned int num_layers = generator_->get_item_count();
	selected_layer_ = std::max(-1, std::min<int>(layer, num_layers - 1));

	for(unsigned int i = 0; i < num_layers; ++i) {
		if(selected_layer_ >= 0) {
			const bool selected = i == static_cast<unsigned int>(selected_layer_);
			// Select current layer, leave the rest unselected.
			select_layer_internal(i, selected);
			generator_->item(i).set_visible(selected
											? twidget::tvisible::visible
											: twidget::tvisible::hidden);
		} else {
			// Select everything.
			select_layer_internal(i, true);
			generator_->item(i).set_visible(twidget::tvisible::visible);
		}
	}
}

} // namespace gui2
