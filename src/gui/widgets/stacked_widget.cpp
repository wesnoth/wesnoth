/*
   Copyright (C) 2009 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/generator.hpp"
#include "gettext.hpp"

#include "utils/functional.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(stacked_widget)

stacked_widget::stacked_widget()
	: container_base(1)
	, generator_(
			  generator_base::build(false, false, generator_base::independent, false))
	, selected_layer_(-1)
{
}

bool stacked_widget::get_active() const
{
	return true;
}

unsigned stacked_widget::get_state() const
{
	return 0;
}

void stacked_widget::layout_children()
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
void swap_grid(grid* g,
			   grid* content_grid,
			   widget* widget,
			   const std::string& id)
{
	assert(content_grid);
	assert(widget);

	// Make sure the new child has same id.
	widget->set_id(id);

	// Get the container containing the wanted widget.
	grid* parent_grid = nullptr;
	if(g) {
		parent_grid = find_widget<grid>(g, id, false, false);
	}
	if(!parent_grid) {
		parent_grid = find_widget<grid>(content_grid, id, true, false);
	}
	parent_grid = dynamic_cast<grid*>(parent_grid->parent());
	assert(parent_grid);

	// Replace the child.
	widget = parent_grid->swap_child(id, widget, false);
	assert(widget);

	delete widget;
}

} // namespace

void
stacked_widget::finalize(std::vector<builder_grid_const_ptr> widget_builder)
{
	assert(generator_);
	string_map empty_data;
	for(const auto & builder : widget_builder)
	{
		generator_->create_item(-1, builder, empty_data, nullptr);
	}
	swap_grid(nullptr, &get_grid(), generator_, "_content_grid");

	select_layer(-1);
}

const std::string& stacked_widget::get_control_type() const
{
	static const std::string type = "stacked_widget";
	return type;
}

void stacked_widget::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

void stacked_widget::select_layer_internal(const unsigned int layer, const bool select) const
{
	// Selecting a layer that's already selected appears to actually deselect
	// it, so make sure to only perform changes we want.
	if(generator_->is_selected(layer) != select) {
		generator_->select_item(layer, select);
	}
}

void stacked_widget::select_layer(const int layer)
{
	const unsigned int num_layers = generator_->get_item_count();
	selected_layer_ = std::max(-1, std::min<int>(layer, num_layers - 1));

	for(unsigned int i = 0; i < num_layers; ++i) {
		if(selected_layer_ >= 0) {
			const bool selected = i == static_cast<unsigned int>(selected_layer_);
			// Select current layer, leave the rest unselected.
			select_layer_internal(i, selected);
			generator_->item(i).set_visible(selected
											? widget::visibility::visible
											: widget::visibility::hidden);
		} else {
			// Select everything.
			select_layer_internal(i, true);
			generator_->item(i).set_visible(widget::visibility::visible);
		}
	}
}

unsigned int stacked_widget::get_layer_count() const
{
	return generator_->get_item_count();
}

grid* stacked_widget::get_layer_grid(unsigned int i)
{
	assert(generator_);
	return &generator_->item(i);
}

// }---------- DEFINITION ---------{

stacked_widget_definition::stacked_widget_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing stacked widget " << id << '\n';

	load_resolutions<resolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_stacked_widget
 *
 * == Stacked widget ==
 *
 * A stacked widget holds several widgets on top of each other. This can be used
 * for various effects; add an optional overlay to an image, stack it with a
 * spacer to force a minimum size of a widget. The latter is handy to avoid
 * making a separate definition for a single instance with a fixed size.
 *
 * A stacked widget has no states.
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="stacked_widget_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * @allow{link}{name="gui/window/resolution/grid"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="stacked_widget_definition"}
 * @end{parent}{name="gui/"}
 */
stacked_widget_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid(nullptr)
{
	// Add a dummy state since every widget needs a state.
	static config dummy("draw");
	state.push_back(state_definition(dummy));

	const config& child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = std::make_shared<builder_grid>(child);
}

// }---------- BUILDER -----------{

/*WIKI
 * @page = GUIToolkitWML
 * @order = 2_stacked_widget
 *
 * == Stacked widget ==
 *
 * A stacked widget is a set of widget stacked on top of each other. The
 * widgets are drawn in the layers, in the order defined in the the instance
 * config. By default the last drawn item is also the 'active' layer for the
 * event handling.
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="stacked_widget"}{min="0"}{max="-1"}{super="generic/widget_instance"}
 * @begin{table}{config}
 * @end{table}
 * @begin{tag}{name="layer"}{min=0}{max=-1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="layer"}
 * @end{tag}{name="stacked_widget"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

builder_stacked_widget::builder_stacked_widget(const config& real_cfg)
	: builder_styled_widget(real_cfg), stack()
{
	const config& cfg = real_cfg.has_child("stack") ? real_cfg.child("stack") : real_cfg;
	if(&cfg != &real_cfg) {
		lg::wml_error() << "Stacked widgets no longer require a [stack] tag. Instead, place [layer] tags directly in the widget definition.\n";
	}
	VALIDATE(cfg.has_child("layer"), _("No stack layers defined."));
	for(const auto & layer : cfg.child_range("layer"))
	{
		stack.push_back(std::make_shared<builder_grid>(layer));
	}
}

widget* builder_stacked_widget::build() const
{
	stacked_widget* widget = new stacked_widget();

	init_control(widget);

	DBG_GUI_G << "Window builder: placed stacked widget '" << id
			  << "' with definition '" << definition << "'.\n";

	std::shared_ptr<const stacked_widget_definition::resolution>
	conf = std::static_pointer_cast<const stacked_widget_definition::resolution>(
					widget->config());
	assert(conf);

	widget->init_grid(conf->grid);

	widget->finalize(stack);

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
