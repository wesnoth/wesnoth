/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_GRID_HPP_INCLUDED
#define GUI_WIDGETS_GRID_HPP_INCLUDED

#include "gui/widgets/widget.hpp"

namespace gui2
{

/**
 * Base container class.
 *
 * This class holds a number of widgets and their wanted layout parameters. It
 * also layouts the items in the grid and handles their drawing.
 */
class tgrid : public twidget
{
	friend class tdebug_layout_graph;
	friend struct tgrid_implementation;

public:
	explicit tgrid(const unsigned rows = 0, const unsigned cols = 0);

	virtual ~tgrid();

	/***** ***** ***** ***** LAYOUT FLAGS ***** ***** ***** *****/
	static const unsigned VERTICAL_SHIFT = 0;
	static const unsigned VERTICAL_GROW_SEND_TO_CLIENT = 1 << VERTICAL_SHIFT;
	static const unsigned VERTICAL_ALIGN_TOP = 2 << VERTICAL_SHIFT;
	static const unsigned VERTICAL_ALIGN_CENTER = 3 << VERTICAL_SHIFT;
	static const unsigned VERTICAL_ALIGN_BOTTOM = 4 << VERTICAL_SHIFT;
	static const unsigned VERTICAL_MASK = 7 << VERTICAL_SHIFT;

	static const unsigned HORIZONTAL_SHIFT = 3;
	static const unsigned HORIZONTAL_GROW_SEND_TO_CLIENT = 1
														   << HORIZONTAL_SHIFT;
	static const unsigned HORIZONTAL_ALIGN_LEFT = 2 << HORIZONTAL_SHIFT;
	static const unsigned HORIZONTAL_ALIGN_CENTER = 3 << HORIZONTAL_SHIFT;
	static const unsigned HORIZONTAL_ALIGN_RIGHT = 4 << HORIZONTAL_SHIFT;
	static const unsigned HORIZONTAL_MASK = 7 << HORIZONTAL_SHIFT;

	static const unsigned BORDER_TOP = 1 << 6;
	static const unsigned BORDER_BOTTOM = 1 << 7;
	static const unsigned BORDER_LEFT = 1 << 8;
	static const unsigned BORDER_RIGHT = 1 << 9;
	static const unsigned BORDER_ALL = BORDER_TOP | BORDER_BOTTOM | BORDER_LEFT
									   | BORDER_RIGHT;

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
	 * @param col                 The columnof the cell.
	 * @param flags               The flags for the widget in the cell.
	 * @param border_size         The size of the border for the cell, how the
	 *                            border is applied depends on the flags.
	 */
	void set_child(twidget* widget,
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
	 * @param widget              The widget to put in the grid.
	 * @param recurse             Do we want to decent into the child grids.
	 * @param new_parent          The new parent for the swapped out widget.
	 *
	 * returns                    The widget which got removed (the parent of
	 *                            the widget is cleared). If no widget found
	 *                            and thus not replace nullptr will returned.
	 */
	twidget* swap_child(const std::string& id,
						twidget* widget,
						const bool recurse,
						twidget* new_parent = nullptr);

	/**
	 * Removes and frees a widget in a cell.
	 *
	 * @param row                 The row of the cell.
	 * @param col                 The columnof the cell.
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
	 * If a child inherits from tcontrol or is a tgrid it will call
	 * set_active() for the child otherwise it ignores the widget.
	 *
	 * @param active              Parameter for set_active.
	 */
	void set_active(const bool active);


	/** Returns the widget in the selected cell. */
	const twidget* widget(const unsigned row, const unsigned col) const
	{
		return child(row, col).widget();
	}

	/** Returns the widget in the selected cell. */
	twidget* widget(const unsigned row, const unsigned col)
	{
		return child(row, col).widget();
	}

	virtual bool can_mouse_focus() const override { return false; }
	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

	/** See @ref twidget::layout_initialise. */
	virtual void layout_initialise(const bool full_initialisation) override;

	/**
	 * Tries to reduce the width of a container.
	 *
	 * See @ref layout_algorithm for more information.
	 *
	 * @param maximum_width       The wanted maximum width.
	 */
	void reduce_width(const unsigned maximum_width);

	/** See @ref twidget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned maximum_width) override;

	/** See @ref twidget::demand_reduce_width. */
	virtual void demand_reduce_width(const unsigned maximum_width) override;

	/**
	 * Tries to reduce the height of a container.
	 *
	 * See @ref layout_algorithm for more information.
	 *
	 * @param maximum_height      The wanted maximum height.
	 */
	void reduce_height(const unsigned maximum_height);

	/** See @ref twidget::request_reduce_height. */
	virtual void request_reduce_height(const unsigned maximum_height) override;

	/** See @ref twidget::demand_reduce_height. */
	virtual void demand_reduce_height(const unsigned maximum_height) override;

	/**
	 * Recalculates the best size.
	 *
	 * This is used for scrollbar containers when they try to update their
	 * contents size before falling back to the 'global' invalidate_layout.
	 *
	 * @returns                   The newly calculated size.
	 */
	tpoint recalculate_best_size();

private:
	/** See @ref twidget::calculate_best_size. */
	virtual tpoint calculate_best_size() const override;

public:
	/** See @ref twidget::can_wrap. */
	virtual bool can_wrap() const override;

public:
	/** See @ref twidget::place. */
	virtual void place(const tpoint& origin, const tpoint& size) override;

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref twidget::set_origin. */
	virtual void set_origin(const tpoint& origin) override;

	/** See @ref twidget::set_visible_rectangle. */
	virtual void set_visible_rectangle(const SDL_Rect& rectangle) override;

	/** See @ref twidget::layout_children. */
	virtual void layout_children() override;

	/** See @ref twidget::child_populate_dirty_list. */
	virtual void
	child_populate_dirty_list(twindow& caller,
							  const std::vector<twidget*>& call_stack) override;

	/** See @ref twidget::find_at. */
	virtual twidget* find_at(const tpoint& coordinate,
							 const bool must_be_active) override;

	/** See @ref twidget::find_at. */
	virtual const twidget* find_at(const tpoint& coordinate,
								   const bool must_be_active) const override;

	/** See @ref twidget::find. */
	twidget* find(const std::string& id, const bool must_be_active) override;

	/** See @ref twidget::find. */
	const twidget* find(const std::string& id,
						const bool must_be_active) const override;

	/** See @ref twidget::has_widget. */
	virtual bool has_widget(const twidget& widget) const override;

	/** See @ref twidget::disable_click_dismiss. */
	bool disable_click_dismiss() const override;

	/** See @ref twidget::create_walker. */
	virtual iterator::twalker_* create_walker() override;

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
	class tchild
	{
		friend struct tgrid_implementation;

	public:
		tchild() : flags_(0), border_size_(0), widget_(nullptr)

		// Fixme make a class where we can store some properties in the cache
		// regarding size etc.
		{
		}

		/** Returns the best size for the cell. */
		tpoint get_best_size() const;

		/**
		 * Places the widget in the cell.
		 *
		 * @param origin          The origin (x, y) for the widget.
		 * @param size            The size for the widget.
		 */
		void place(tpoint origin, tpoint size);

		/** Forwards @ref tgrid::layout_initialise to the cell. */
		void layout_initialise(const bool full_initialisation);

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

		const twidget* widget() const
		{
			return widget_;
		}
		twidget* widget()
		{
			return widget_;
		}

		void set_widget(twidget* widget)
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
		twidget* widget_;

		/** Returns the space needed for the border. */
		tpoint border_space() const;

	}; // class tchild

public:
	/** Iterator for the tchild items. */
	class iterator
	{

	public:
		iterator(std::vector<tchild>::iterator itor) : itor_(itor)
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
		twidget* operator->()
		{
			return itor_->widget();
		}
		twidget* operator*()
		{
			return itor_->widget();
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
		std::vector<tchild>::iterator itor_;
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
	std::vector<tchild> children_;
	const tchild& child(const unsigned row, const unsigned col) const
	{
		return children_[rows_ * col + row];
	}
	tchild& child(const unsigned row, const unsigned col)
	{
		return children_[rows_ * col + row];
	}

	/** Layouts the children in the grid. */
	void layout(const tpoint& origin);

	/** See @ref twidget::impl_draw_children. */
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
void set_single_child(tgrid& grid, twidget* widget);

} // namespace gui2

#endif
