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

#include "gui/widgets/widget.hpp"

namespace gui2
{

/**
 * Base container class.
 *
 * This class holds a number of widgets and their wanted layout parameters. It
 * also layouts the items in the grid and handles their drawing.
 */
class grid : public widget
{
	friend class debug_layout_graph;
	friend struct grid_implementation;

public:
	explicit grid(const unsigned rows = 0, const unsigned cols = 0);

	/** Delete the copy constructor. */
	grid(const grid&) = delete;

	/** Delete the move assignment operator. */
	grid& operator=(const grid&) = delete;

	virtual ~grid();

	/***** ***** ***** ***** LAYOUT FLAGS ***** ***** ***** *****/
	static const unsigned VERTICAL_SHIFT                 = 0;
	static const unsigned VERTICAL_GROW_SEND_TO_CLIENT   = 1 << VERTICAL_SHIFT;
	static const unsigned VERTICAL_ALIGN_TOP             = 2 << VERTICAL_SHIFT;
	static const unsigned VERTICAL_ALIGN_CENTER          = 3 << VERTICAL_SHIFT;
	static const unsigned VERTICAL_ALIGN_BOTTOM          = 4 << VERTICAL_SHIFT;
	static const unsigned VERTICAL_MASK                  = 7 << VERTICAL_SHIFT;

	static const unsigned HORIZONTAL_SHIFT               = 3;
	static const unsigned HORIZONTAL_GROW_SEND_TO_CLIENT = 1 << HORIZONTAL_SHIFT;
	static const unsigned HORIZONTAL_ALIGN_LEFT          = 2 << HORIZONTAL_SHIFT;
	static const unsigned HORIZONTAL_ALIGN_CENTER        = 3 << HORIZONTAL_SHIFT;
	static const unsigned HORIZONTAL_ALIGN_RIGHT         = 4 << HORIZONTAL_SHIFT;
	static const unsigned HORIZONTAL_MASK                = 7 << HORIZONTAL_SHIFT;

	static const unsigned BORDER_TOP                     = 1 << 6;
	static const unsigned BORDER_BOTTOM                  = 1 << 7;
	static const unsigned BORDER_LEFT                    = 1 << 8;
	static const unsigned BORDER_RIGHT                   = 1 << 9;
	static const unsigned BORDER_ALL = BORDER_TOP | BORDER_BOTTOM | BORDER_LEFT | BORDER_RIGHT;

	/***** ***** ***** ***** ROW COLUMN MANIPULATION ***** ***** ***** *****/

	/**
	 * Adds a row to end of the grid.
	 *
	 * @param count               Number of rows to add, should be > 0.
	 *
	 * @returns                   The row number of the first row added.
	 */
	unsigned add_row(const unsigned count = 1);

	/**
	 * Sets the grow factor for a row.
	 *
	 * @todo refer to a page with the layout manipulation info.
	 *
	 * @param row                 The row to modify.
	 * @param factor              The grow factor.
	 */
	void set_row_grow_factor(const unsigned row, const unsigned factor)
	{
		assert(row < row_grow_factor_.size());
		row_grow_factor_[row] = factor;
		set_is_dirty(true);
	}

	/**
	 * Sets the grow factor for a column.
	 *
	 * @todo refer to a page with the layout manipulation info.
	 *
	 * @param column              The column to modify.
	 * @param factor              The grow factor.
	 */
	void set_column_grow_factor(const unsigned column, const unsigned factor)
	{
		assert(column < col_grow_factor_.size());
		col_grow_factor_[column] = factor;
		set_is_dirty(true);
	}

	/***** ***** ***** ***** CHILD MANIPULATION ***** ***** ***** *****/

	/**
	 * Sets a child in the grid.
	 *
	 * When the child is added to the grid the grid 'owns' the widget.
	 * The widget is put in a cell, and every cell can only contain 1 widget if
	 * the wanted cell already contains a widget, that widget is freed and
	 * removed.
	 *
	 * @param widget              The widget to put in the grid.
	 * @param row                 The row of the cell.
	 * @param col                 The column of the cell.
	 * @param flags               The flags for the widget in the cell.
	 * @param border_size         The size of the border for the cell, how the
	 *                            border is applied depends on the flags.
	 */
	void set_child(widget* widget,
				   const unsigned row,
				   const unsigned col,
				   const unsigned flags,
				   const unsigned border_size);

	/**
	 * Exchanges a child in the grid.
	 *
	 * It replaced the child with a certain id with the new widget but doesn't
	 * touch the other settings of the child.
	 *
	 * @param id                  The id of the widget to free.
	 * @param w                   The widget to put in the grid.
	 * @param recurse             Do we want to decent into the child grids.
	 * @param new_parent          The new parent for the swapped out widget.
	 *
	 * returns                    The widget which got removed (the parent of
	 *                            the widget is cleared). If no widget found
	 *                            and thus not replace nullptr will returned.
	 */
	std::unique_ptr<widget> swap_child(const std::string& id,
						widget* w,
						const bool recurse,
						widget* new_parent = nullptr);

	/**
	 * Removes and frees a widget in a cell.
	 *
	 * @param row                 The row of the cell.
	 * @param col                 The column of the cell.
	 */
	void remove_child(const unsigned row, const unsigned col);

	/**
	 * Removes and frees a widget in a cell by it's id.
	 *
	 * @param id                  The id of the widget to free.
	 * @param find_all            If true if removes all items with the id,
	 *                            otherwise it stops after removing the first
	 *                            item (or once all children have been tested).
	 */
	void remove_child(const std::string& id, const bool find_all = false);

	/**
	 * Activates all children.
	 *
	 * If a child inherits from styled_widget or is a grid it will call
	 * set_active() for the child otherwise it ignores the widget.
	 *
	 * @param active              Parameter for set_active.
	 */
	void set_active(const bool active);

	/** Returns the widget in the selected cell. */
	const widget* get_widget(const unsigned row, const unsigned col) const
	{
		return get_child(row, col).get_widget();
	}

	/** Returns the widget in the selected cell. */
	widget* get_widget(const unsigned row, const unsigned col)
	{
		return get_child(row, col).get_widget();
	}

	virtual bool can_mouse_focus() const override { return false; }
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

	/**
	 * Recalculates the best size.
	 *
	 * This is used for scrollbar containers when they try to update their
	 * contents size before falling back to the 'global' invalidate_layout.
	 *
	 * @returns                   The newly calculated size.
	 */
	point recalculate_best_size();

	/**
	 * Modifies the widget alignment data of a child cell containing a specific widget.
	 *
	 * @param widget              The widget whose cell to modify.
	 * @param set_flag            The alignment flag to set.
	 * @param mode_mask           Whether to affect horizontal or vertical alignment.
	 *                            Use either HORIZONTAL_MASK or VERTICAL_MASK
	 */
	void set_child_alignment(widget* widget, unsigned set_flag, unsigned mode_mask);

private:
	/** See @ref widget::calculate_best_size. */
	virtual point calculate_best_size() const override;

	/**
	* Attempts to lay out the grid without laying out the entire window.
	*/
	void request_placement(dispatcher& dispatcher, const event::ui_event event, bool& handled, bool& halt);

public:
	/** See @ref widget::can_wrap. */
	virtual bool can_wrap() const override;

public:
	/** See @ref widget::place. */
	virtual void place(const point& origin, const point& size) override;

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref widget::set_origin. */
	virtual void set_origin(const point& origin) override;

	/** See @ref widget::set_visible_rectangle. */
	virtual void set_visible_rectangle(const SDL_Rect& rectangle) override;

	/** See @ref widget::layout_children. */
	virtual void layout_children() override;

	/** See @ref widget::child_populate_dirty_list. */
	virtual void
	child_populate_dirty_list(window& caller,
							  const std::vector<widget*>& call_stack) override;

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

	/** See @ref widget::has_widget. */
	virtual bool has_widget(const widget& widget) const override;

	/** See @ref widget::disable_click_dismiss. */
	bool disable_click_dismiss() const override;

	/** See @ref widget::create_walker. */
	virtual iteration::walker_base* create_walker() override;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_rows(const unsigned rows);
	unsigned int get_rows() const
	{
		return rows_;
	}

	void set_cols(const unsigned cols);
	unsigned int get_cols() const
	{
		return cols_;
	}

	/**
	 * Wrapper to set_rows and set_cols.
	 *
	 * @param rows                Parameter to call set_rows with.
	 * @param cols                Parameter to call set_cols with.
	 */
	void set_rows_cols(const unsigned rows, const unsigned cols);

private:
	/** Child item of the grid. */
	class child
	{
		friend struct grid_implementation;

	public:
		child() : flags_(0), border_size_(0), widget_(nullptr)

		// Fixme make a class where we can store some properties in the cache
		// regarding size etc.
		{
		}

		/** Returns the best size for the cell. */
		point get_best_size() const;

		/**
		 * Places the widget in the cell.
		 *
		 * @param origin          The origin (x, y) for the widget.
		 * @param size            The size for the widget.
		 */
		void place(point origin, point size);

		/** Forwards @ref grid::layout_initialize to the cell. */
		void layout_initialize(const bool full_initialization);

		/** Returns the can_wrap for the cell. */
		bool can_wrap() const
		{
			return widget_ ? widget_->can_wrap() : false;
		}

		/** Returns the id of the widget/ */
		const std::string& id() const;

		unsigned get_flags() const
		{
			return flags_;
		}
		void set_flags(const unsigned flags)
		{
			flags_ = flags;
		}

		unsigned get_border_size() const
		{
			return border_size_;
		}
		void set_border_size(const unsigned border_size)
		{
			border_size_ = border_size;
		}

		const widget* get_widget() const
		{
			return widget_;
		}
		widget* get_widget()
		{
			return widget_;
		}

		void set_widget(widget* widget)
		{
			widget_ = widget;
		}

	private:
		/** The flags for the border and cell setup. */
		unsigned flags_;

		/**
		 * The size of the border, the actual configuration of the border
		 * is determined by the flags.
		 */
		unsigned border_size_;

		/**
		 * Pointer to the widget.
		 *
		 * Once the widget is assigned to the grid we own the widget and are
		 * responsible for it's destruction.
		 */
		widget* widget_;

		/** Returns the space needed for the border. */
		point border_space() const;

	}; // class child

public:
	/** Iterator for the child items. */
	class iterator
	{

	public:
		iterator(std::vector<child>::iterator itor) : itor_(itor)
		{
		}

		iterator operator++()
		{
			return iterator(++itor_);
		}
		iterator operator--()
		{
			return iterator(--itor_);
		}
		widget* operator->()
		{
			return itor_->get_widget();
		}
		widget* operator*()
		{
			return itor_->get_widget();
		}

		bool operator==(const iterator& i) const
		{
			return i.itor_ == this->itor_;
		}

		bool operator!=(const iterator& i) const
		{
			return i.itor_ != this->itor_;
		}

	private:
		std::vector<child>::iterator itor_;
	};

	iterator begin()
	{
		return iterator(children_.begin());
	}
	iterator end()
	{
		return iterator(children_.end());
	}

private:
	/** The number of grid rows. */
	unsigned rows_;

	/** The number of grid columns. */
	unsigned cols_;

	/***** ***** ***** ***** size caching ***** ***** ***** *****/

	/** The row heights in the grid. */
	mutable std::vector<unsigned> row_height_;

	/** The column widths in the grid. */
	mutable std::vector<unsigned> col_width_;

	/** The grow factor for all rows. */
	std::vector<unsigned> row_grow_factor_;

	/** The grow factor for all columns. */
	std::vector<unsigned> col_grow_factor_;

	/**
	 * The child items.
	 *
	 * All children are stored in a 1D vector and the formula to access a cell
	 * is: rows_ * col + row. All other vectors use the same access formula.
	 */
	std::vector<child> children_;

	/**
	 * Gets the grid child in the specified cell.
	 *
	 * @param row                 The row of the cell.
	 * @param col                 The column of the cell.
	 *
	 * @returns                   A const reference to the specified grid child.
	 */
	const grid::child& get_child(const unsigned row, const unsigned col) const
	{
		return children_[rows_ * col + row];
	}

	/**
	 * Gets the grid child in the specified cell.
	 *
	 * @param row                 The row of the cell.
	 * @param col                 The column of the cell.
	 *
	 * @returns                   A const reference to the specified grid child.
	 */
	grid::child& get_child(const unsigned row, const unsigned col)
	{
		return children_[rows_ * col + row];
	}

	/**
	 * Gets the grid child containing the specified widget.
	 *
	 * @param w                   The widget to search for.
	 *
	 * @returns                   A pointer to the relevant grid child, or nullptr
	 *                            if no grid cell 'owns' the given widget.
	 */
	grid::child* get_child(widget* w);

	/** Layouts the children in the grid. */
	void layout(const point& origin);

	/** See @ref widget::impl_draw_children. */
	virtual void impl_draw_children(surface& frame_buffer,
									int x_offset,
									int y_offset) override;
};

/**
 * Sets the single child in a grid.
 *
 * The function initializes the grid to 1 x 1 and adds the widget with the grow
 * to client flags.
 *
 * @param grid                    The grid to add the child to.
 * @param widget                  The widget to add as child to the grid.
 */
void set_single_child(grid& grid, widget* widget);

} // namespace gui2
