/*
   Copyright (C) 2012 - 2016 by Mark de Wever <koraq@xs4all.nl>
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
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/window.hpp"
#include "utils/const_clone.hpp"
#include "gui/auxiliary/window_builder/pane.hpp"
#include "gui/auxiliary/event/message.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER "tpane [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

/**
 * Helper to implement private functions without modifying the header.
 *
 * The class is a helper to avoid recompilation and only has static
 * functions. It also facilitates to create duplicates of functions for a const
 * and a non-const member function.
 */
struct tpane_implementation
{
	/**
	 * Implementation for the wrappers for
	 * [const] twidget* tpane::find_at(const tpoint&, const bool) [const].
	 *
	 * @tparam W                  A pointer to the pane.
	 */
	template <class W>
	static typename utils::tconst_clone<twidget, W>::pointer
	find_at(W pane, tpoint coordinate, const bool must_be_active)
	{

		/*
		 * First test whether the mouse is at the pane.
		 */
		if(pane->twidget::find_at(coordinate, must_be_active) != pane) {
			return NULL;
		}

		typedef typename utils::tconst_clone<tpane::titem, W>::reference thack;
		BOOST_FOREACH(thack item, pane->items_)
		{

			if(item.grid->get_visible() == twidget::tvisible::invisible) {
				continue;
			}

			/*
			 * If the adjusted coordinate is in the item's grid let the grid
			 * resolve the coordinate.
			 */
			const SDL_Rect rect = item.grid->get_rectangle();
			if(coordinate.x >= rect.x && coordinate.y >= rect.y
			   && coordinate.x < rect.x + rect.w
			   && coordinate.y < rect.y + rect.h) {

				return item.grid->find_at(coordinate, must_be_active);
			}
		}

		return NULL;
	}

	/**
	 * Implementation for the wrappers for
	 * [const] tgrid* tpane::grid(const unsigned id) [const].
	 *
	 * @tparam W                  A pointer to the pane.
	 */
	template <class W>
	static typename utils::tconst_clone<tgrid, W>::pointer
	grid(W pane, const unsigned id)
	{
		typedef typename utils::tconst_clone<tpane::titem, W>::reference thack;
		BOOST_FOREACH(thack item, pane->items_)
		{

			if(item.id == id) {
				return item.grid;
			}
		}

		return NULL;
	}
};

tpane::tpane(const tbuilder_grid_ptr item_builder)
	: twidget()
	, items_()
	, item_builder_(item_builder)
	, item_id_generator_(0)
	, placer_(tplacer_::build(tplacer_::vertical, 1))
{
	connect_signal<event::REQUEST_PLACEMENT>(
			boost::bind(
					&tpane::signal_handler_request_placement, this, _1, _2, _3),
			event::tdispatcher::back_pre_child);
}

tpane::tpane(const implementation::tbuilder_pane& builder)
	: twidget(builder)
	, items_()
	, item_builder_(builder.item_definition)
	, item_id_generator_(0)
	, placer_(tplacer_::build(builder.grow_direction, builder.parallel_items))
{
	connect_signal<event::REQUEST_PLACEMENT>(
			boost::bind(
					&tpane::signal_handler_request_placement, this, _1, _2, _3),
			event::tdispatcher::back_pre_child);
}

tpane* tpane::build(const implementation::tbuilder_pane& builder)
{
	return new tpane(builder);
}

unsigned tpane::create_item(const std::map<std::string, string_map>& item_data,
							const std::map<std::string, std::string>& tags)
{
	titem item = { item_id_generator_++, tags, item_builder_->build() };

	item.grid->set_parent(this);

	FOREACH(const AUTO & data, item_data)
	{
		tcontrol* control
				= find_widget<tcontrol>(item.grid, data.first, false, false);

		if(control) {
			control->set_members(data.second);
		}
	}

	items_.push_back(item);

	event::tmessage message;
	fire(event::REQUEST_PLACEMENT, *this, message);

	return item.id;
}

void tpane::place(const tpoint& origin, const tpoint& size)
{
	DBG_GUI_L << LOG_HEADER << '\n';
	twidget::place(origin, size);

	assert(origin.x == 0);
	assert(origin.y == 0);

	place_children();
}

void tpane::layout_initialise(const bool full_initialisation)
{
	DBG_GUI_D << LOG_HEADER << '\n';

	twidget::layout_initialise(full_initialisation);

	FOREACH(AUTO & item, items_)
	{
		if(item.grid->get_visible() != twidget::tvisible::invisible) {
			item.grid->layout_initialise(full_initialisation);
		}
	}
}

void
tpane::impl_draw_children(surface& frame_buffer, int x_offset, int y_offset)
{
	DBG_GUI_D << LOG_HEADER << '\n';

	FOREACH(AUTO & item, items_)
	{
		if(item.grid->get_visible() != twidget::tvisible::invisible) {
			item.grid->draw_children(frame_buffer, x_offset, y_offset);
		}
	}
}

void tpane::child_populate_dirty_list(twindow& caller,
									  const std::vector<twidget*>& call_stack)
{
	FOREACH(AUTO & item, items_)
	{
		std::vector<twidget*> child_call_stack = call_stack;
		item.grid->populate_dirty_list(caller, child_call_stack);
	}
}

void tpane::sort(const tcompare_functor& compare_functor)
{
	items_.sort(compare_functor);

	set_origin_children();
}

void tpane::filter(const tfilter_functor& filter_functor)
{
	FOREACH(AUTO & item, items_)
	{
		item.grid->set_visible(filter_functor(item)
									   ? twidget::tvisible::visible
									   : twidget::tvisible::invisible);
	}

	set_origin_children();
}

void tpane::request_reduce_width(const unsigned /*maximum_width*/)
{
}

twidget* tpane::find_at(const tpoint& coordinate, const bool must_be_active)
{
	return tpane_implementation::find_at(this, coordinate, must_be_active);
}

const twidget* tpane::find_at(const tpoint& coordinate,
							  const bool must_be_active) const
{
	return tpane_implementation::find_at(this, coordinate, must_be_active);
}

tpoint tpane::calculate_best_size() const
{
	prepare_placement();
	return placer_->get_size();
}

bool tpane::disable_click_dismiss() const
{
	return false;
}

iterator::twalker_* tpane::create_walker()
{
	/**
	 * @todo Implement properly.
	 */
	return NULL;
}

tgrid* tpane::grid(const unsigned id)
{
	return tpane_implementation::grid(this, id);
}

const tgrid* tpane::grid(const unsigned id) const
{
	return tpane_implementation::grid(this, id);
}

void tpane::place_children()
{
	prepare_placement();
	unsigned index = 0;
	FOREACH(AUTO & item, items_)
	{
		if(item.grid->get_visible() == twidget::tvisible::invisible) {
			continue;
		}

		const tpoint origin = placer_->get_origin(index);
		item.grid->place(origin, item.grid->get_best_size());
		++index;
	}
}

void tpane::set_origin_children()
{
	prepare_placement();
	unsigned index = 0;
	FOREACH(AUTO & item, items_)
	{
		if(item.grid->get_visible() == twidget::tvisible::invisible) {
			continue;
		}

		const tpoint origin = placer_->get_origin(index);
		item.grid->set_origin(origin);
		++index;
	}
}

void tpane::place_or_set_origin_children()
{
	prepare_placement();
	unsigned index = 0;
	FOREACH(AUTO & item, items_)
	{
		if(item.grid->get_visible() == twidget::tvisible::invisible) {
			continue;
		}

		const tpoint origin = placer_->get_origin(index);
		if(item.grid->get_size() != item.grid->get_best_size()) {
			item.grid->place(origin, item.grid->get_best_size());
		} else {
			item.grid->set_origin(origin);
		}
		++index;
	}
}

void tpane::prepare_placement() const
{
	assert(placer_.get());
	placer_->initialise();

	FOREACH(const AUTO & item, items_)
	{
		if(item.grid->get_visible() == twidget::tvisible::invisible) {
			continue;
		}

		placer_->add_item(item.grid->get_best_size());
	}
}

void tpane::signal_handler_request_placement(tdispatcher& dispatcher,
											 const event::tevent event,
											 bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	twidget* widget = dynamic_cast<twidget*>(&dispatcher);
	if(widget) {
		FOREACH(AUTO & item, items_)
		{
			if(item.grid->has_widget(*widget)) {
				if(item.grid->get_visible() != twidget::tvisible::invisible) {

					/*
					 * This time we call init layout but also the linked widget
					 * update this makes things work properly for the
					 * addon_list. This code can use some more tuning,
					 * polishing and testing.
					 */
					item.grid->layout_initialise(false);
					get_window()->layout_linked_widgets();

					/*
					 * By not calling init layout it uses its previous size
					 * what seems to work properly when showing and hiding
					 * items. Might fail with new items (haven't tested yet).
					 */
					item.grid->place(tpoint(0, 0), item.grid->get_best_size());
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

} // namespace gui2
