/*
   Copyright (C) 2012 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/pane.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/log.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/window.hpp"
#include "utils/const_clone.hpp"
#include "gui/core/event/message.hpp"
#include "gettext.hpp"
#include "lexical_cast.hpp"
#include "sdl/rect.hpp"

#include "utils/functional.hpp"

#define LOG_SCOPE_HEADER "pane [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

/**
 * Helper to implement private functions without modifying the header.
 *
 * The class is a helper to avoid recompilation and only has static
 * functions. It also facilitates to create duplicates of functions for a const
 * and a non-const member function.
 */
struct pane_implementation
{
	/**
	 * Implementation for the wrappers for
	 * [const] widget* pane::find_at(const point&, const bool) [const].
	 *
	 * @tparam W                  A pointer to the pane.
	 */
	template <class W>
	static utils::const_clone_ptr<widget, W>
	find_at(W pane, point coordinate, const bool must_be_active)
	{

		/*
		 * First test whether the mouse is at the pane.
		 */
		if(pane->widget::find_at(coordinate, must_be_active) != pane) {
			return nullptr;
		}

		for(auto item : pane->items_)
		{

			if(item.item_grid->get_visible() == widget::visibility::invisible) {
				continue;
			}

			/*
			 * If the adjusted coordinate is in the item's grid let the grid
			 * resolve the coordinate.
			 */
			if(sdl::point_in_rect(coordinate, item.item_grid->get_rectangle())) {
				return item.item_grid->find_at(coordinate, must_be_active);
			}
		}

		return nullptr;
	}

	/**
	 * Implementation for the wrappers for
	 * [const] grid* pane::grid(const unsigned id) [const].
	 *
	 * @tparam W                  A pointer to the pane.
	 */
	template <class W>
	static utils::const_clone_ptr<grid, W>
	get_grid(W pane, const unsigned id)
	{
		for(auto item : pane->items_)
		{

			if(item.id == id) {
				return item.item_grid;
			}
		}

		return nullptr;
	}
};

pane::pane(const builder_grid_ptr item_builder)
	: widget()
	, items_()
	, item_builder_(item_builder)
	, item_id_generator_(0)
	, placer_(placer_base::build(placer_base::tgrow_direction::vertical, 1))
{
	connect_signal<event::REQUEST_PLACEMENT>(
			std::bind(
					&pane::signal_handler_request_placement, this, _1, _2, _3),
			event::dispatcher::back_pre_child);
}

pane::pane(const implementation::builder_pane& builder)
	: widget(builder)
	, items_()
	, item_builder_(builder.item_definition)
	, item_id_generator_(0)
	, placer_(placer_base::build(builder.grow_direction, builder.parallel_items))
{
	connect_signal<event::REQUEST_PLACEMENT>(
			std::bind(
					&pane::signal_handler_request_placement, this, _1, _2, _3),
			event::dispatcher::back_pre_child);
}

pane* pane::build(const implementation::builder_pane& builder)
{
	return new pane(builder);
}

unsigned pane::create_item(const std::map<std::string, string_map>& item_data,
							const std::map<std::string, std::string>& tags)
{
	item item = { item_id_generator_++, tags, item_builder_->build() };

	item.item_grid->set_parent(this);

	for(const auto & data : item_data)
	{
		styled_widget* control
				= find_widget<styled_widget>(item.item_grid, data.first, false, false);

		if(control) {
			control->set_members(data.second);
		}
	}

	items_.push_back(item);

	event::message message;
	fire(event::REQUEST_PLACEMENT, *this, message);

	return item.id;
}

void pane::place(const point& origin, const point& size)
{
	DBG_GUI_L << LOG_HEADER << '\n';
	widget::place(origin, size);

	assert(origin.x == 0);
	assert(origin.y == 0);

	place_children();
}

void pane::layout_initialize(const bool full_initialization)
{
	DBG_GUI_D << LOG_HEADER << '\n';

	widget::layout_initialize(full_initialization);

	for(auto & item : items_)
	{
		if(item.item_grid->get_visible() != widget::visibility::invisible) {
			item.item_grid->layout_initialize(full_initialization);
		}
	}
}

void
pane::impl_draw_children(surface& frame_buffer, int x_offset, int y_offset)
{
	DBG_GUI_D << LOG_HEADER << '\n';

	for(auto & item : items_)
	{
		if(item.item_grid->get_visible() != widget::visibility::invisible) {
			item.item_grid->draw_children(frame_buffer, x_offset, y_offset);
		}
	}
}

void pane::child_populate_dirty_list(window& caller,
									  const std::vector<widget*>& call_stack)
{
	for(auto & item : items_)
	{
		std::vector<widget*> child_call_stack = call_stack;
		item.item_grid->populate_dirty_list(caller, child_call_stack);
	}
}

void pane::sort(const tcompare_functor& compare_functor)
{
	items_.sort(compare_functor);

	set_origin_children();
}

void pane::filter(const tfilter_functor& filter_functor)
{
	for(auto & item : items_)
	{
		item.item_grid->set_visible(filter_functor(item)
									   ? widget::visibility::visible
									   : widget::visibility::invisible);
	}

	set_origin_children();
}

void pane::request_reduce_width(const unsigned /*maximum_width*/)
{
}

widget* pane::find_at(const point& coordinate, const bool must_be_active)
{
	return pane_implementation::find_at(this, coordinate, must_be_active);
}

const widget* pane::find_at(const point& coordinate,
							  const bool must_be_active) const
{
	return pane_implementation::find_at(this, coordinate, must_be_active);
}

point pane::calculate_best_size() const
{
	prepare_placement();
	return placer_->get_size();
}

bool pane::disable_click_dismiss() const
{
	return false;
}

iteration::walker_base* pane::create_walker()
{
	/**
	 * @todo Implement properly.
	 */
	return nullptr;
}

grid* pane::get_grid(const unsigned id)
{
	return pane_implementation::get_grid(this, id);
}

const grid* pane::get_grid(const unsigned id) const
{
	return pane_implementation::get_grid(this, id);
}

void pane::place_children()
{
	prepare_placement();
	unsigned index = 0;
	for(auto & item : items_)
	{
		if(item.item_grid->get_visible() == widget::visibility::invisible) {
			continue;
		}

		const point origin = placer_->get_origin(index);
		item.item_grid->place(origin, item.item_grid->get_best_size());
		++index;
	}
}

void pane::set_origin_children()
{
	prepare_placement();
	unsigned index = 0;
	for(auto & item : items_)
	{
		if(item.item_grid->get_visible() == widget::visibility::invisible) {
			continue;
		}

		const point origin = placer_->get_origin(index);
		item.item_grid->set_origin(origin);
		++index;
	}
}

void pane::place_or_set_origin_children()
{
	prepare_placement();
	unsigned index = 0;
	for(auto & item : items_)
	{
		if(item.item_grid->get_visible() == widget::visibility::invisible) {
			continue;
		}

		const point origin = placer_->get_origin(index);
		if(item.item_grid->get_size() != item.item_grid->get_best_size()) {
			item.item_grid->place(origin, item.item_grid->get_best_size());
		} else {
			item.item_grid->set_origin(origin);
		}
		++index;
	}
}

void pane::prepare_placement() const
{
	assert(placer_.get());
	placer_->initialize();

	for(const auto & item : items_)
	{
		if(item.item_grid->get_visible() == widget::visibility::invisible) {
			continue;
		}

		placer_->add_item(item.item_grid->get_best_size());
	}
}

void pane::signal_handler_request_placement(dispatcher& dispatcher,
											 const event::ui_event event,
											 bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	widget* wgt = dynamic_cast<widget*>(&dispatcher);
	if(wgt) {
		for(auto & item : items_)
		{
			if(item.item_grid->has_widget(*wgt)) {
				if(item.item_grid->get_visible() != widget::visibility::invisible) {

					/*
					 * This time we call init layout but also the linked widget
					 * update this makes things work properly for the
					 * addon_list. This code can use some more tuning,
					 * polishing and testing.
					 */
					item.item_grid->layout_initialize(false);
					get_window()->layout_linked_widgets();

					/*
					 * By not calling init layout it uses its previous size
					 * what seems to work properly when showing and hiding
					 * items. Might fail with new items (haven't tested yet).
					 */
					item.item_grid->place(point(), item.item_grid->get_best_size());
				}
				place_or_set_origin_children();
				DBG_GUI_E << LOG_HEADER << ' ' << event << " handled.\n";
				handled = true;
				return;
			}
		}
	}

	DBG_GUI_E << LOG_HEADER << ' ' << event << " failed to handle.\n";
	assert(false);
	handled = false;
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{pane_description}
 *
 *        A pane is a container where new members can be added and removed
 *        during run-time.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_viewport
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="pane"}{min=0}{max=-1}{super="generic/widget_instance"}
 * == Label ==
 *
 * @macro = viewport_description
 *
 * List with the label specific variables:
 * @begin{table}{config}
 *     grow_direction & grow_direction & &
 *                                The direction in which new items grow. $
 *     parallel_items & unsigned & &
 *                                The number of items that are growing in
 *                                parallel. $
 *     item_definition & section & &
 *                                The definition of a new item. $
 * @end{table}
 *
 * @begin{tag}{name="item_definition"}{min=1}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="item_definition"}
 * @end{tag}{name="pane"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

builder_pane::builder_pane(const config& cfg)
	: builder_widget(cfg)
	, grow_direction(
			  lexical_cast<placer_base::tgrow_direction>(cfg["grow_direction"]))
	, parallel_items(cfg["parallel_items"])
	, item_definition(new builder_grid(cfg.child("item_definition", "[pane]")))
{
	VALIDATE(parallel_items > 0, _("Need at least 1 parallel item."));
}

widget* builder_pane::build() const
{
	return build(replacements_map());
}

widget* builder_pane::build(const replacements_map& /*replacements*/) const
{
	return pane::build(*this);
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
