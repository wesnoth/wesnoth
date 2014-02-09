/*
   Copyright (C) 2008 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_CONTAINER_HPP_INCLUDED
#define GUI_WIDGETS_CONTAINER_HPP_INCLUDED

#include "gui/widgets/grid.hpp"
#include "gui/widgets/control.hpp"
#include "gui/auxiliary/window_builder.hpp"

namespace gui2
{

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
	explicit tcontainer_(const unsigned canvas_count)
		: tcontrol(canvas_count), grid_()
	{
		grid_.set_parent(this);
	}

	/**
	 * Returns the client rect.
	 *
	 * The client rect is the area which is used for child items. The rest of
	 * the area of this widget is used for its own decoration.
	 *
	 * @returns                   The client rect.
	 */
	virtual SDL_Rect get_client_rect() const;

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

	/** See @ref twidget::layout_initialise. */
	virtual void layout_initialise(const bool full_initialisation) OVERRIDE;

	/**
	 * Tries to reduce the width of a container.
	 *
	 * See @ref layout_algorithm for more information.
	 *
	 * @param maximum_width       The wanted maximum width.
	 */
	void reduce_width(const unsigned maximum_width);

	/** See @ref twidget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned maximum_width) OVERRIDE;

	/** See @ref twidget::demand_reduce_width. */
	virtual void demand_reduce_width(const unsigned maximum_width) OVERRIDE;

	/**
	 * Tries to reduce the height of a container.
	 *
	 * See @ref layout_algorithm for more information.
	 *
	 * @param maximum_height      The wanted maximum height.
	 */
	void reduce_height(const unsigned maximum_height);

	/** See @ref twidget::request_reduce_height. */
	virtual void request_reduce_height(const unsigned maximum_height) OVERRIDE;

	/** See @ref twidget::demand_reduce_height. */
	virtual void demand_reduce_height(const unsigned maximum_height) OVERRIDE;

private:
	/** See @ref twidget::calculate_best_size. */
	virtual tpoint calculate_best_size() const OVERRIDE;

public:
	/** See @ref twidget::can_wrap. */
	virtual bool can_wrap() const OVERRIDE;

	/** See @ref twidget::place. */
	virtual void place(const tpoint& origin, const tpoint& size) OVERRIDE;

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref twidget::has_widget. */
	virtual bool has_widget(const twidget& widget) const OVERRIDE;

	/** See @ref twidget::set_origin. */
	virtual void set_origin(const tpoint& origin) OVERRIDE;

	/** See @ref twidget::set_visible_rectangle. */
	virtual void set_visible_rectangle(const SDL_Rect& rectangle) OVERRIDE;

	/** See @ref twidget::impl_draw_children. */
	virtual void impl_draw_children(surface& frame_buffer) OVERRIDE;

	/** See @ref twidget::impl_draw_children. */
	virtual void impl_draw_children(surface& frame_buffer,
									int x_offset,
									int y_offset) OVERRIDE;

protected:
	/** See @ref twidget::layout_children. */
	virtual void layout_children() OVERRIDE;

	/** See @ref twidget::child_populate_dirty_list. */
	virtual void
	child_populate_dirty_list(twindow& caller,
							  const std::vector<twidget*>& call_stack) OVERRIDE;

public:
	/** See @ref twidget::find_at. */
	virtual twidget* find_at(const tpoint& coordinate,
							 const bool must_be_active) OVERRIDE;

	/** See @ref twidget::find_at. */
	virtual const twidget* find_at(const tpoint& coordinate,
								   const bool must_be_active) const OVERRIDE;

	/** See @ref twidget::find. */
	twidget* find(const std::string& id, const bool must_be_active) OVERRIDE;

	/** See @ref twidget::find. */
	const twidget* find(const std::string& id,
						const bool must_be_active) const OVERRIDE;

	/** See @ref tcontrol::set_active. */
	virtual void set_active(const bool active) OVERRIDE;

	/** See @ref twidget::disable_click_dismiss. */
	bool disable_click_dismiss() const OVERRIDE;

	/**
	 * See @ref twidget::create_walker.
	 *
	 * @todo Implement properly.
	 */
	virtual iterator::twalker_* create_walker() OVERRIDE
	{
		return NULL;
	}

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

	tgrid::iterator begin()
	{
		return grid_.begin();
	}
	tgrid::iterator end()
	{
		return grid_.end();
	}

	unsigned add_row(const unsigned count = 1)
	{
		return grid_.add_row(count);
	}

	void set_rows(const unsigned rows)
	{
		grid_.set_rows(rows);
	}
	unsigned int get_rows() const
	{
		return grid_.get_rows();
	}

	void set_cols(const unsigned cols)
	{
		grid_.set_cols(cols);
	}
	unsigned int get_cols() const
	{
		return grid_.get_cols();
	}

	void set_rows_cols(const unsigned rows, const unsigned cols)
	{
		grid_.set_rows_cols(rows, cols);
	}

	void set_child(twidget* widget,
				   const unsigned row,
				   const unsigned col,
				   const unsigned flags,
				   const unsigned border_size)
	{
		grid_.set_child(widget, row, col, flags, border_size);
	}

	void set_row_grow_factor(const unsigned row, const unsigned factor)
	{
		grid_.set_row_grow_factor(row, factor);
	}

	void set_column_grow_factor(const unsigned column, const unsigned factor)
	{
		grid_.set_column_grow_factor(column, factor);
	}

public:
	/***** ***** ***** setters / getters for members ***** ****** *****/

	// Public due to the fact that window needs to be able to swap the
	// children, might be protected again later.
	const tgrid& grid() const
	{
		return grid_;
	}
	tgrid& grid()
	{
		return grid_;
	}

private:
	/** The grid which holds the child objects. */
	tgrid grid_;

	/**
	 * Returns the grid to initialize while building.
	 *
	 * @todo Evaluate whether this function is overridden if not remove.
	 */
	virtual tgrid& initial_grid()
	{
		return grid_;
	}

	/** Returns the space used by the border. */
	virtual tpoint border_space() const;

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
