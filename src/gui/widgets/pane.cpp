/* $Id$ */
/*
   Copyright (C) 2012 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/log.hpp"
#include "gui/widgets/grid.hpp"

#define LOG_SCOPE_HEADER "tpane [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2 {

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
	template<class W>
	static typename tconst_clone<twidget, W>::pointer
	find_at(
			  W pane
			, tpoint coordinate
			, const bool must_be_active
			)
	{

		/*
		 * First test whether the mouse is at the pane.
		 */
		if(pane->twidget::find_at(coordinate, must_be_active) != pane) {
			return NULL;
		}

		/*
		 * The widgets are placed at coordinate 0,0 so adjust the offset to
		 * that coordinate system.
		 */
		coordinate.x -= pane->get_x();
		coordinate.y -= pane->get_y();

		typedef typename tconst_clone<tpane::titem, W>::reference thack;
		BOOST_FOREACH(thack item, pane->items_) {

			/*
			 * If the adjusted coordinate is in the item's grid let the grid
			 * resolve the coordinate.
			 */
			const SDL_Rect rect = item.grid->get_rect();
			if(
					   coordinate.x >= rect.x
					&& coordinate.y >= rect.y
					&& coordinate.x < rect.x + rect.w
					&& coordinate.y < rect.y + rect.h) {

				return item.grid->find_at(coordinate, must_be_active);
			}

			/*
			 * No hit then adjust the coordinate with the size of the widget
			 * that did not hit it. This is required since every item's grid
			 * has its own coodinate systeme starting at 0,0.
			 */
			coordinate.y -= rect.h;

			/*
			 * If the position to test is outside our virtual area, we bail out.
			 */
			if(coordinate.y < 0) {
				return NULL;
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
{
}

unsigned tpane::create_item(
		  const std::map<std::string, string_map>& item_data
		, const std::map<std::string, std::string>& tags)
{
	titem item = { item_id_generator_++, tags, item_builder_->build() };

	item.grid->set_parent(this);

	typedef std::pair<std::string, string_map> hack ;
	BOOST_FOREACH(const hack& data, item_data) {
		tcontrol* control = find_widget<tcontrol>(
				  item.grid
				, data.first
				, false
				, false);

		if(control) {
			control->set_members(data.second);
		}
	}

	items_.push_back(item);
	return item.id;
}

void tpane::place(const tpoint& origin, const tpoint& size)
{
	DBG_GUI_L << LOG_HEADER << '\n';
	twidget::place(origin, size);

	place_children();
}

void tpane::layout_init(const bool full_initialization)
{
	DBG_GUI_D << LOG_HEADER << '\n';

	twidget::layout_init(full_initialization);

	BOOST_FOREACH(titem& item, items_) {
		if(item.grid->get_visible() != twidget::INVISIBLE) {
			item.grid->layout_init(full_initialization);
		}
	}
}

void tpane::impl_draw_children(
		  surface& frame_buffer
		, int x_offset
		, int y_offset)
{
	DBG_GUI_D << LOG_HEADER << '\n';

	BOOST_FOREACH(titem& item, items_) {
		if(item.grid->get_visible() != twidget::INVISIBLE) {
			item.grid->draw_children(frame_buffer, x_offset, y_offset);
		}
	}
}

void tpane::child_populate_dirty_list(twindow& caller,
			const std::vector<twidget*>& call_stack)
{
	BOOST_FOREACH(titem& item, items_) {
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
	BOOST_FOREACH(titem& item, items_) {
		item.grid->set_visible(
				filter_functor(item)
					? twidget::VISIBLE
					: twidget::INVISIBLE);
	}

	set_origin_children();
}

void tpane::request_reduce_width(const unsigned /*maximum_width*/)
{
}

twidget* tpane::find_at(
		  const tpoint& coordinate
		, const bool must_be_active)
{
	return tpane_implementation::find_at(
			  this
			, coordinate
			, must_be_active);
}

const twidget* tpane::find_at(
		  const tpoint& coordinate
		, const bool must_be_active) const
{
	return tpane_implementation::find_at(
			  this
			, coordinate
			, must_be_active);
}

tpoint tpane::calculate_best_size() const
{
	return tpoint(500, 500);
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

void tpane::place_children()
{
	unsigned y = 0;

	BOOST_FOREACH(titem& item, items_) {
		if(item.grid->get_visible() == twidget::INVISIBLE) {
			continue;
		}

		DBG_GUI_L << LOG_HEADER << " offset " << y << '\n';
		item.grid->place(tpoint(0, y), item.grid->get_best_size());
		y += item.grid->get_height();
	}
}

void tpane::set_origin_children()
{
	unsigned y = 0;

	BOOST_FOREACH(titem& item, items_) {
		if(item.grid->get_visible() == twidget::INVISIBLE) {
			continue;
		}

		DBG_GUI_L << LOG_HEADER << " offset " << y << '\n';
		item.grid->set_origin(tpoint(0, y));
		y += item.grid->get_height();
	}
}

} // namespace gui2
