/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_CONTAINER_HPP_INCLUDED
#define GUI_WIDGETS_CONTAINER_HPP_INCLUDED

#include "gui/widgets/grid.hpp"
#include "gui/widgets/control.hpp"
#include "gui/auxiliary/window_builder.hpp"

namespace gui2 {

/**
 * A generic container base class.
 *
 * A container is a class build with multiple items either acting as one
 * widget.
 *
 */
class tcontainer_ : public tcontrol
{
	friend class tdebug_layout_graph;
public:
	tcontainer_(const unsigned canvas_count) :
		tcontrol(canvas_count),
		grid_()
	{
		grid_.set_parent(this);
	}

	/**
	 * Returns the size of the client area.
	 *
	 * The client area is the area available for widgets.
	 */
	virtual SDL_Rect get_client_rect() const { return get_rect(); }

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

	/** Inherited from tcontrol. */
	void layout_init(const bool full_initialization);

	/**
	 * Tries to reduce the width of a container.
	 *
	 * @see @ref layout_algorihm for more information.
	 *
	 * @param maximum_width       The wanted maximum width.
	 */
	void reduce_width(const unsigned maximum_width);

	/** Inherited from tcontrol. */
	void request_reduce_width(const unsigned maximum_width);

	/** Inherited from twidget. */
	void demand_reduce_width(const unsigned maximum_width);

	/**
	 * Tries to reduce the height of a container.
	 *
	 * @see @ref layout_algorihm for more information.
	 *
	 * @param maximum_height      The wanted maximum height.
	 */
	void reduce_height(const unsigned maximum_height);

	/** Inherited from twidget. */
	void request_reduce_height(const unsigned maximum_height);

	/** Inherited from twidget. */
	void demand_reduce_height(const unsigned maximum_height);

private:
	/** Inherited from twidget. */
	tpoint calculate_best_size() const;
public:

	/** Inherited from twidget. */
	bool can_wrap() const { return grid_.can_wrap() || twidget::can_wrap(); }

	/** Inherited from twidget. */
	void place(const tpoint& origin, const tpoint& size);

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** Inherited from twidget.*/
	bool has_widget(const twidget* widget) const
		{ return grid_.has_widget(widget); }

	/** Inherited from twidget. */
	void set_origin(const tpoint& origin);

	/** Inherited from twidget. */
	void set_visible_area(const SDL_Rect& area);

	/** Inherited from twidget. */
	void impl_draw_children(surface& frame_buffer);

protected:

	/** Inherited from twidget. */
	void child_populate_dirty_list(twindow& caller,
			const std::vector<twidget*>& call_stack);

public:

	/** Inherited from tcontrol. */
	twidget* find_at(const tpoint& coordinate, const bool must_be_active)
		{ return grid_.find_at(coordinate, must_be_active); }

	/** Inherited from tcontrol. */
	const twidget* find_at(const tpoint& coordinate,
			const bool must_be_active) const
		{ return grid_.find_at(coordinate, must_be_active); }

	/** Inherited from tcontrol.*/
	twidget* find(const std::string& id, const bool must_be_active)
	{
		twidget* result = tcontrol::find(id, must_be_active);
		return result ? result : grid_.find(id, must_be_active);
	}

	/** Inherited from tcontrol.*/
	const twidget* find(const std::string& id, const bool must_be_active) const
	{
		const twidget* result = tcontrol::find(id, must_be_active);
		return result ? result : grid_.find(id, must_be_active);
	}

	/** Inherited from tcontrol. */
	void set_active(const bool active);

	/** Inherited from tcontrol. */
	bool disable_click_dismiss() const;

	/**
	 * Initializes and builds the grid.
	 *
	 * This function should only be called upon an empty grid. This grid is
	 * returned by initial_grid();
	 *
	 * @param grid_builder        The builder for the grid.
	 */
	void init_grid(const boost::intrusive_ptr<tbuilder_grid>& grid_builder);

	/***** **** ***** ***** wrappers to the grid **** ********* *****/

	tgrid::iterator begin() { return grid_.begin(); }
	tgrid::iterator end() { return grid_.end(); }

	unsigned add_row(const unsigned count = 1)
		{ return grid_.add_row(count); }

	void set_rows(const unsigned rows) { grid_.set_rows(rows); }
	unsigned int get_rows() const { return grid_.get_rows(); }

	void set_cols(const unsigned cols) { grid_.set_cols(cols); }
	unsigned int get_cols() const { return grid_.get_cols(); }

	void set_rows_cols(const unsigned rows, const unsigned cols)
		{ grid_.set_rows_cols(rows, cols); }

	void set_child(twidget* widget, const unsigned row,
		const unsigned col, const unsigned flags, const unsigned border_size)
		{ grid_.set_child(widget, row, col, flags, border_size); }

	void set_row_grow_factor(const unsigned row, const unsigned factor)
		{ grid_.set_row_grow_factor(row, factor); }

	void set_column_grow_factor(const unsigned column, const unsigned factor)
		{ grid_.set_column_grow_factor(column, factor); }

public:
	/***** ***** ***** setters / getters for members ***** ****** *****/

	// Public due to the fact that window needs to be able to swap the
	// children, might be protected again later.
	const tgrid& grid() const { return grid_; }
	tgrid& grid() { return grid_; }

private:

	/** The grid which holds the child objects. */
	tgrid grid_;

	/**
	 * Returns the grid to initialize while building.
	 *
	 * @todo Evaluate whether this function is overridden if not remove.
	 */
	virtual tgrid& initial_grid() { return grid_; }

	/** Returns the space used by the border. */
	virtual tpoint border_space() const { return tpoint(0, 0); }

	/**
	 * Helper for set_active.
	 *
	 * This function should set the control itself active. It's called by
	 * set_active if the state needs to change. The widget is set to dirty() by
	 * set_active so we only need to change the state.
	 */
	virtual void set_self_active(const bool active) = 0;
};

} // namespace gui2

#endif

