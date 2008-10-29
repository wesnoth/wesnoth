/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#ifndef GUI_WIDGETS_GRID_HPP_INCLUDED
#define GUI_WIDGETS_GRID_HPP_INCLUDED

#include "gui/widgets/widget.hpp"
#include "gui/widgets/control.hpp"

#include <cassert>

namespace gui2 {

	/**
	 * Helper variable for the layout system.
	 * 
	 * @todo This is a kind of hack to make the layout engine work properly.
	 * Wrapping needs the cache disabled but the scrollbars need it enabled. 
	 * But the entire layout code needs a review and probably a rewrite...
	 */ 
	extern bool disable_cache;

/**
 * Base container class.
 *
 * This class holds a number of widgets and their wanted layout parameters. It
 * also layouts the items in the grid and hanldes their drawing.
 */
class tgrid : public virtual twidget
{
public:

	tgrid(const unsigned rows = 0, const unsigned cols = 0); 

	virtual ~tgrid();

	/***** ***** ***** ***** LAYOUT FLAGS ***** ***** ***** *****/
	static const unsigned VERTICAL_GROW_SEND_TO_CLIENT   = 1 << 0;
	static const unsigned VERTICAL_ALIGN_TOP             = 2 << 0;   
	static const unsigned VERTICAL_ALIGN_CENTER          = 3 << 0;   
	static const unsigned VERTICAL_ALIGN_BOTTOM          = 4 << 0;   
	static const unsigned VERTICAL_MASK                  = 7 << 0;   

	static const unsigned HORIZONTAL_GROW_SEND_TO_CLIENT = 1 << 3;
	static const unsigned HORIZONTAL_ALIGN_LEFT          = 2 << 3;   
	static const unsigned HORIZONTAL_ALIGN_CENTER        = 3 << 3;   
	static const unsigned HORIZONTAL_ALIGN_RIGHT         = 4 << 3;   
	static const unsigned HORIZONTAL_MASK                = 7 << 3;   

	static const unsigned BORDER_TOP                     = 1 << 6;
	static const unsigned BORDER_BOTTOM                  = 1 << 7;
	static const unsigned BORDER_LEFT                    = 1 << 8;
	static const unsigned BORDER_RIGHT                   = 1 << 9;

	/***** ***** ***** ***** ROW COLUMN MANIPULATION ***** ***** ***** *****/

	/**
	 * Addes a row to end of the grid.
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
		set_dirty(); 
	} 

	/**
	 * Sets the grow factor for a column.
	 *
	 * @todo refer to a page with the layout manipulation info.
	 *
	 * @param column              The column to modify.
	 * @param factor              The grow factor.
	 */
	void set_col_grow_factor(const unsigned col, const unsigned factor)
	{ 
		assert(col< col_grow_factor_.size());
		col_grow_factor_[col] = factor; 
		set_dirty(); 
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
	void set_child(twidget* widget, const unsigned row, 
		const unsigned col, const unsigned flags, const unsigned border_size);

	/**
	 * Exchangs a child in the grid.
	 *
	 * It replaced the child with a certain id with the new widget but doesn't
	 * touch the other settings of the child.
	 *
	 * @param id                  The id of the widget to free.
	 * @param widget              The widget to put in the grid.
	 * @parem recurse             Do we want to decent into the child grids.
	 *
	 * returns                    The widget which got removed (the parent of
	 *                            the widget is cleared). If no widget found
	 *                            and thus not replace NULL will returned.
	 */
	twidget* swap_child(
		const std::string& id, twidget* widget, const bool recurse);

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
		{ return child(row, col).widget(); }

	/** Returns the widget in the selected cell. */
	twidget* widget(const unsigned row, const unsigned col) 
		{ return child(row, col).widget(); }

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** Inherited from twidget. */
	tpoint get_minimum_size() const;

	/** Inherited from twidget. */
	tpoint get_best_size() const;

	/** Inherited from twidget. */
	tpoint get_best_size(const tpoint& maximum_size) const;

	/** Inherited from twidget. */
	tpoint get_maximum_size() const;

	/** Inherited from twidget. */
	bool can_wrap() const;

	/** Inherited from twidget. */
	bool set_width_constrain(const unsigned width);

	/** Inherited from twidget. */
	void clear_width_constrain();

	/** Inherited from twidget. */
	bool has_vertical_scrollbar() const;

	/** Inherited from twidget. */
	void draw(surface& surface,  const bool force = false,
	        const bool invalidate_background = false);

	/** Inherited from twidget. */
	twidget* find_widget(const tpoint& coordinate, const bool must_be_active);

	/** Inherited from twidget. */
	const twidget* find_widget(const tpoint& coordinate, 
			const bool must_be_active) const;

	/** Inherited from twidget.*/
	twidget* find_widget(const std::string& id, const bool must_be_active);

	/** Inherited from twidget.*/
	const twidget* find_widget(const std::string& id, 
			const bool must_be_active) const;

	/** Inherited from twidget.*/
	bool has_widget(const twidget* widget) const;

	/** Inherited from twidget. */
	void set_size(const SDL_Rect& rect);

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_rows(const unsigned rows);
	unsigned int get_rows() const { return rows_; }

	void set_cols(const unsigned cols);
	unsigned int get_cols() const { return cols_; }

	/**
	 * Wrapper to set_rows and set_cols.
	 *
	 * @param rows                Parameter to call set_rows with.
	 * @param cols                Parameter to call set_cols with.
	 */
	void set_rows_cols(const unsigned rows, const unsigned cols);

	/** Inherited from twidget. */
	void set_dirty(const bool dirty = true);

private:
	/** Child item of the grid. */
	class tchild 
	{
	public:
		tchild() : 
			id_(),
			flags_(0),
			border_size_(0),
			widget_(0),
			best_size_(0, 0),
			minimum_size_(0, 0),
			maximum_size_(0, 0),
			clip_()

			// Fixme make a class wo we can store some properties in the cache 
			// regarding size etc.
			{}
	
		/** Returns the best size for the cell. */
		tpoint get_best_size() const;

		/** Returns the best size for the cell. */
		tpoint get_best_size(const tpoint& maximum_size) const;

		/** Returns the minimum size for the cell. */
		tpoint get_minimum_size() const;

		/** Returns the maximum size for the cell. */
		tpoint get_maximum_size() const;

		/** Returns the can_wrap for the cell. */
		bool can_wrap() const { return widget_ ? widget_->can_wrap() : false; }

		/** Returns the set_width_constrain for the cell. */
		bool set_width_constrain(const unsigned width);

		/**
		 * Sets the size of the widget in the cell.
		 *
		 * @param orig            The origin (x, y) for the widget.
		 * @param size            The size for the widget.
		 */
		void set_size(tpoint orig, tpoint size);

		const std::string& id() const { return id_; }
		void set_id(const std::string& id) { id_ = id; }

		unsigned get_flags() const { return flags_; }
		void set_flags(const unsigned flags) { flags_ = flags; set_dirty(); }

		unsigned get_border_size() const { return border_size_; }
		void set_border_size(const unsigned border_size) 
			{  border_size_ = border_size; set_dirty(); }

		const twidget* widget() const { return widget_; }
		twidget* widget() { return widget_; }

		void set_widget(twidget* widget) { widget_ = widget; set_dirty(); }

	private:
		/** The id of the widget if it has a widget. */
		std::string id_;

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

		/**
		 * The best size for this cell, determined by the best size of the
		 * widget and the border_size_ and flags_.
		 */
		mutable tpoint best_size_;

		/** The minimum size for this cell, like best_size_. */
		mutable tpoint minimum_size_;

		/** The maximum size for this cell, like best_size_. */
		mutable tpoint maximum_size_;

		/** Returns the space needed for the border. */
		tpoint border_space() const;

		/**
		 * The clipping area for the widget. This is also the size of the
		 * container.
		 */
		SDL_Rect clip_;

		/**
		 * Sets the calculations to be dirty, this means all caches are simply
		 * cleared.
		 */
		void set_dirty()  // FIXME rename to clear cache??
		{ 
			best_size_ = tpoint(0, 0); 
			minimum_size_ = tpoint(0, 0); 
			maximum_size_ = tpoint(0, 0);
		}

	}; // class tchild

public:
	/** Iterator for the tchild items. */
	class iterator 
	{

	public:

		iterator(std::vector<tchild>::iterator itor) :
			itor_(itor) 
			{}

		iterator operator++() { return iterator(++itor_); }
		iterator operator--() { return iterator(--itor_); }
		twidget* operator->() { return itor_->widget(); }
		twidget* operator*() { return itor_->widget(); }

		bool operator!=(const iterator& i) const
			{ return i.itor_ != this->itor_; }

	private:
		std::vector<tchild>::iterator itor_;

	};

	iterator begin() { return iterator(children_.begin()); }
	iterator end() { return iterator(children_.end()); }

private:
	/** The number of grid rows. */
	unsigned rows_;

	/** The number of grid columns. */
	unsigned cols_;

	/***** ***** ***** ***** size caching ***** ***** ***** *****/

	/** Cache containing the best row heights. */
	mutable std::vector<unsigned> best_row_height_;

	/** Cache containing the best column widths. */
	mutable std::vector<unsigned> best_col_width_;

	/** Cache containing the best minimum heights. */
	mutable std::vector<unsigned> minimum_row_height_;

	/** Cache containing the minimum column widths. */
	mutable std::vector<unsigned> minimum_col_width_;

	/** 
	 * Clears the size caches. 
	 *
	 * @todo we need to evaluate how useful caching is in the first place since
	 * quite some functions invalidate the caches and most things are calculated
	 * only a few times. This means the caches might be overkill and adding
	 * complexity.
	 */
	void clear_cache();

	/** The row heights in the grid. */
	std::vector<unsigned> row_height_;

	/** The column widths in the grid. */
	std::vector<unsigned> col_width_;


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
		{ return children_[rows_ * col + row]; }
	tchild& child(const unsigned row, const unsigned col)
		{ clear_cache(); return children_[rows_ * col + row]; }

	/** Layouts the children in the grid. */
	void layout(const tpoint& origin);

	/** 
	 * Helper function to get the best or minimum size.
	 *
	 * @param id                     Name to use in debug output.
	 * @param width                  Reference to the vector width cache for the 
	 *                               size function of the caller.
	 * @param height                 Reference to the vector height cache for the 
	 *                               size function of the caller.
	 * @param size_proc              The function to call on the cells in order to
	 *                               get their sizes.
	 * @param size_proc_max          The function to call on the cells if there
	 *                               is a maximum set, this function will only
	 *                               be called if size_proc is NULL, this
	 *                               function should then also be valid.
	 * @param maximum_size           The maximum size value as parameter for
	 *                               size_proc_max.
	 *
	 * @return                       The wanted size.
	 */
	tpoint get_size(const std::string& id, std::vector<unsigned>& width, 
		std::vector<unsigned>& height, tpoint (tchild::*size_proc)() const,
		tpoint (tchild::*size_proc_max)(const tpoint&) const = NULL,
		const tpoint& maximum_size = tpoint(0, 0)) const;

	/**
	 * Gets the best height for a row.
	 *
	 * @param row                 The row to get the best height for.
	 * @param maximum_height      The wanted maximum height.
	 *
	 * @returns                   The best height for a row, if possible
	 *                            smaller as the maximum.
	 */
	unsigned get_best_row_height(const unsigned row, const unsigned maximum_height) const;
};


} // namespace gui2

#endif
