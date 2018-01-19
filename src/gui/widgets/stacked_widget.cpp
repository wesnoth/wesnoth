/*
   Copyright (C) 2009 - 2018 by Mark de Wever <koraq@xs4all.nl>
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
#include "gui/widgets/widget_helpers.hpp"
#include "gui/widgets/generator.hpp"
#include "gettext.hpp"
#include "utils/const_clone.hpp"
#include "utils/general.hpp"

#include "utils/functional.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(stacked_widget)

struct stacked_widget_implementation
{
	template<typename W>
	static W* find(utils::const_clone_ref<stacked_widget, W> stack,
			const std::string& id,
			const bool must_be_active)
	{
		// Use base method if find-in-all-layer isn't set.
		if(!stack.find_in_all_layers_) {
			return stack.container_base::find(id, must_be_active);
		}

		for(unsigned i = 0; i < stack.get_layer_count(); ++i) {
			if(W* res = stack.get_layer_grid(i)->find(id, must_be_active)) {
				return res;
			}
		}

		return stack.container_base::find(id, must_be_active);
	}
};

stacked_widget::stacked_widget(const implementation::builder_stacked_widget& builder)
	: container_base(builder, get_control_type())
	, generator_(generator_base::build(false, false, generator_base::independent, false))
	, selected_layer_(-1)
	, find_in_all_layers_(false)
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

void stacked_widget::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

void stacked_widget::select_layer_impl(std::function<bool(unsigned int i)> display_condition)
{
	const unsigned int num_layers = get_layer_count();

	// Deselect all layers except the chosen ones.
	for(unsigned int i = 0; i < num_layers; ++i) {
		const bool selected = display_condition(i);

		/* Selecting a previously selected item will deselect it, regardless of the what is passed to
		 * select_item. This causes issues if this function is called when all layers are visible (for
		 * example, initialization). For layers other than the chosen one, this is the desired behavior.
		 * However the chosen layer could *also* be deselected undesirably due to the conditions outlined
		 * above, and as this widget's generator does not stipulate a minimum selection, it's possible to
		 * end up with no layers visible at all.
		 *
		 * This works around that by performing no selection unless necessary to change states.
		 */
		if(generator_->is_selected(i) != selected) {
			generator_->select_item(i, selected);
		}
	}

	// If we already have our chosen layers, exit.
	if(selected_layer_ >= 0) {
		return;
	}

	// Else, re-show all layers.
	for(unsigned int i = 0; i < num_layers; ++i) {
		/* By design, only the last selected item will receive events even if multiple items are visible
		 * and said item is not at the top of the stack. If this point is reached, all layers have already
		 * been hidden by the loop above, so the last layer selected will be the top-most one, as desired.
		 */
		generator_->select_item(i, true);
	}
}

void stacked_widget::update_selected_layer_index(const int i)
{
	selected_layer_ = utils::clamp<int>(i, -1, get_layer_count() - 1);
}

bool stacked_widget::layer_selected(const unsigned layer)
{
	assert(layer < get_layer_count());
	return generator_->is_selected(layer);
}

void stacked_widget::select_layer(const int layer)
{
	update_selected_layer_index(layer);

	select_layer_impl([this](unsigned int i)
	{
		return i == static_cast<unsigned int>(selected_layer_);
	});
}

void stacked_widget::select_layers(const boost::dynamic_bitset<>& mask)
{
	assert(mask.size() == get_layer_count());

	select_layer_impl([&](unsigned int i)
	{
		if(mask[i]) {
			update_selected_layer_index(i);
		}

		return mask[i];
	});
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

const grid* stacked_widget::get_layer_grid(unsigned int i) const
{
	assert(generator_);
	return &generator_->item(i);
}

widget* stacked_widget::find(const std::string& id, const bool must_be_active)
{
	return stacked_widget_implementation::find<widget>(*this, id, must_be_active);
}

const widget* stacked_widget::find(const std::string& id, const bool must_be_active) const
{
	return stacked_widget_implementation::find<const widget>(*this, id, must_be_active);
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
	state.emplace_back(dummy);

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
		stack.emplace_back(std::make_shared<builder_grid>(layer));
	}
}

widget* builder_stacked_widget::build() const
{
	stacked_widget* widget = new stacked_widget(*this);

	DBG_GUI_G << "Window builder: placed stacked widget '" << id
			  << "' with definition '" << definition << "'.\n";

	const auto conf = widget->cast_config_to<stacked_widget_definition>();
	assert(conf);

	widget->init_grid(conf->grid);

	widget->finalize(stack);

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
