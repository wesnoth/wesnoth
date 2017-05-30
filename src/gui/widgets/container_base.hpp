/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/widgets/grid.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

/**
 * A generic container base class.
 *
 * A container is a class build with multiple items either acting as one
 * widget.
 *
 */
class container_base : public styled_widget
{
	friend class debug_layout_graph;

public:
	container_base();

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

	/** See @ref widget::layout_initialize. */
	virtual void layout_initialize(const bool full_initialization) override;

	/**
	 * Tries to reduce the width of a container.
	 *
	 * See @ref layout_algorithm for more information.
	 *
	 * @param maximum_width       The wanted maximum width.
	 */
	void reduce_width(const unsigned maximum_width);

	/** See @ref widget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned maximum_width) override;

	/** See @ref widget::demand_reduce_width. */
	virtual void demand_reduce_width(const unsigned maximum_width) override;

	/**
	 * Tries to reduce the height of a container.
	 *
	 * See @ref layout_algorithm for more information.
	 *
	 * @param maximum_height      The wanted maximum height.
	 */
	void reduce_height(const unsigned maximum_height);

	/** See @ref widget::request_reduce_height. */
	virtual void request_reduce_height(const unsigned maximum_height) override;

	/** See @ref widget::demand_reduce_height. */
	virtual void demand_reduce_height(const unsigned maximum_height) override;

protected:
	/** See @ref widget::calculate_best_size. */
	virtual point calculate_best_size() const override;

public:
	/** See @ref widget::can_wrap. */
	virtual bool can_wrap() const override;

	/** See @ref widget::place. */
	virtual void place(const point& origin, const point& size) override;

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref widget::has_widget. */
	virtual bool has_widget(const widget& widget) const override;

	/** See @ref widget::set_origin. */
	virtual void set_origin(const point& origin) override;

	/** See @ref widget::set_visible_rectangle. */
	virtual void set_visible_rectangle(const SDL_Rect& rectangle) override;

	/** See @ref widget::impl_draw_children. */
	virtual void impl_draw_children(int x_offset, int y_offset) override;

protected:
	/** See @ref widget::layout_children. */
	virtual void layout_children() override;

public:
	/** See @ref widget::find_at. */
	virtual widget* find_at(const point& coordinate,
							 const bool must_be_active) override;

	/** See @ref widget::find_at. */
	virtual const widget* find_at(const point& coordinate,
								   const bool must_be_active) const override;

	/** See @ref widget::find. */
	widget* find(const std::string& id, const bool must_be_active) override;

	/** See @ref widget::find. */
	const widget* find(const std::string& id,
						const bool must_be_active) const override;

	/** See @ref styled_widget::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref widget::disable_click_dismiss. */
	bool disable_click_dismiss() const override;

	/**
	 * See @ref widget::create_walker.
	 *
	 * @todo Implement properly.
	 */
	virtual iteration::walker_base* create_walker() override
	{
		return nullptr;
	}

	/**
	 * Initializes and builds the grid.
	 *
	 * This function should only be called upon an empty grid. This grid is
	 * returned by initial_grid();
	 *
	 * @param grid_builder        The builder for the grid.
	 */
	void init_grid(const std::shared_ptr<builder_grid>& grid_builder);

	/***** **** ***** ***** wrappers to the grid **** ********* *****/

	grid::iterator begin()
	{
		return grid_.begin();
	}
	grid::iterator end()
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

	void set_child(widget* widget,
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
	const grid& get_grid() const
	{
		return grid_;
	}
	grid& get_grid()
	{
		return grid_;
	}

private:
	/** The grid which holds the child objects. */
	grid grid_;

	/**
	 * Returns the grid to initialize while building.
	 *
	 * @todo Evaluate whether this function is overridden if not remove.
	 */
	virtual grid& initial_grid()
	{
		return grid_;
	}

	/** Returns the space used by the border. */
	virtual point border_space() const;

	/**
	 * Helper for set_active.
	 *
	 * This function should set the styled_widget itself active. It's called by
	 * set_active if the state needs to change. The widget is set to dirty() by
	 * set_active so we only need to change the state.
	 */
	virtual void set_self_active(const bool active) = 0;

	void inject_linked_groups();
};

} // namespace gui2
