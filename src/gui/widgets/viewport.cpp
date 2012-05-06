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

#include "gui/widgets/viewport.hpp"

#include "gui/auxiliary/log.hpp"

#define LOG_SCOPE_HEADER "tviewport [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2 {

/**
 * Helper to implement private functions without modifying the header.
 *
 * The class is a helper to avoid recompilation and only has static
 * functions. It also facilitates to create duplicates of functions for a const
 * and a non-const member function.
 */
struct tviewport_implementation
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
			  W viewport
			, tpoint coordinate
			, const bool must_be_active
			)
	{

		/*
		 * First test whether the mouse is at the pane.
		 */
		if(viewport->twidget::find_at(coordinate, must_be_active) != viewport) {
			return NULL;
		}

		/*
		 * The widgets are placed at coordinate 0,0 so adjust the offset to
		 * that coordinate system.
		 */
		coordinate.x -= viewport->get_x();
		coordinate.y -= viewport->get_y();

		return viewport->widget_.find_at(coordinate, must_be_active);
	}

	template<class W>
	static typename tconst_clone<twidget, W>::pointer
	find(
			  W viewport
			, const std::string& id
			, const bool must_be_active
			)
	{
		if(viewport->twidget::find(id, must_be_active)) {
			return viewport;
		} else {
			return viewport->widget_.find(id, must_be_active);
		}
	}
};

tviewport::tviewport(twidget& widget)
	: widget_(widget)
{
	widget_.set_parent(this);
}

void tviewport::place(const tpoint& origin, const tpoint& size)
{
	twidget::place(origin, size);

	widget_.place(tpoint(0, 0), widget_.get_best_size());
}

void tviewport::layout_init(const bool full_initialization)
{
	twidget::layout_init(full_initialization);

	if(widget_.get_visible() != twidget::INVISIBLE) {
		widget_.layout_init(full_initialization);
	}
}

void tviewport::impl_draw_children(
		  surface& frame_buffer
		, int x_offset
		, int y_offset)
{
	x_offset += get_x();
	y_offset += get_y();

	if(widget_.get_visible() != twidget::INVISIBLE) {
		widget_.draw_background(frame_buffer, x_offset, y_offset);
		widget_.draw_children(frame_buffer, x_offset, y_offset);
		widget_.draw_foreground(frame_buffer, x_offset, y_offset);
		widget_.set_dirty(false);
	}
}

void tviewport::child_populate_dirty_list(
		  twindow& caller
		, const std::vector<twidget*>& call_stack)
{
	std::vector<twidget*> child_call_stack = call_stack;
	widget_.populate_dirty_list(caller, child_call_stack);
}

void tviewport::request_reduce_width(const unsigned /*maximum_width*/)
{
}

twidget* tviewport::find_at(const tpoint& coordinate, const bool must_be_active)
{
	return tviewport_implementation::find_at(this, coordinate, must_be_active);
}

const twidget* tviewport::find_at(
		  const tpoint& coordinate
		, const bool must_be_active) const
{
	return tviewport_implementation::find_at(this, coordinate, must_be_active);
}

twidget* tviewport::find(const std::string& id, const bool must_be_active)
{
	return tviewport_implementation::find(this, id, must_be_active);
}

const twidget* tviewport::find(
		  const std::string& id
		, const bool must_be_active) const
{
	return tviewport_implementation::find(this, id, must_be_active);
}

tpoint tviewport::calculate_best_size() const
{
	return tpoint(500, 500);
}

bool tviewport::disable_click_dismiss() const
{
	return false;
}

iterator::twalker_* tviewport::create_walker()
{
	/**
	 * @todo Implement properly.
	 */
	return NULL;
}

} // namespace gui2
