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

#ifndef __GUI_WIDGETS_GRID_HPP_INCLUDED__
#define __GUI_WIDGETS_GRID_HPP_INCLUDED__

#include "gui/widgets/widget.hpp"
#include "gui/widgets/control.hpp"

#include <cassert>

namespace gui2 {

//! Base container class which needs to size children
class tgrid : public virtual twidget
{

public:
	// ***** ***** FLAGS ***** *****
	static const unsigned VERTICAL_GROW_SEND_TO_CLIENT   = 1 << 0;

	static const unsigned VERTICAL_ALIGN_TOP             = 3 << 1;   
	static const unsigned VERTICAL_ALIGN_CENTER          = 2 << 1;   
	static const unsigned VERTICAL_ALIGN_BOTTOM          = 1 << 1;   

	static const unsigned HORIZONTAL_GROW_SEND_TO_CLIENT = 1 << 3;

	static const unsigned HORIZONTAL_ALIGN_LEFT          = 3 << 4;   
	static const unsigned HORIZONTAL_ALIGN_CENTER        = 2 << 4;   
	static const unsigned HORIZONTAL_ALIGN_RIGHT         = 1 << 4;   

	static const unsigned BORDER_TOP                     = 1 << 6;
	static const unsigned BORDER_BOTTOM                  = 1 << 7;
	static const unsigned BORDER_LEFT                    = 1 << 8;
	static const unsigned BORDER_RIGHT                   = 1 << 9;

	
	tgrid(const unsigned rows = 0, const unsigned cols = 0); 

	virtual ~tgrid();

	void add_child(twidget* widget, const unsigned row, 
		const unsigned col, const unsigned flags, const unsigned border_size);
	
	void set_rows(const unsigned rows);
	unsigned int get_rows() const { return rows_; }

	/**
	 * Addes a row to end of the grid.
	 *
	 * @param count               Number of rows to add, should be > 0.
	 *
	 * @returns                   The row number of the first row added.
	 */
	unsigned add_row(const unsigned count = 1);

	void set_cols(const unsigned cols);
	unsigned int get_cols() const { return cols_; }

	void set_rows_cols(const unsigned rows, const unsigned cols);

	void set_row_grow_factor(const unsigned row, const unsigned factor)
	{ 
		assert(row < row_grow_factor_.size()); 
		row_grow_factor_[row] = factor; 
		set_dirty(); 
	} 

	void set_col_grow_factor(const unsigned col, const unsigned factor)
	{ 
		assert(col< col_grow_factor_.size());
		col_grow_factor_[col] = factor; 
		set_dirty(); 
	}

	void remove_child(const unsigned row, const unsigned col);
	void remove_child(const std::string& id, const bool find_all = false);

	/** Inherited from twidget. */
	bool has_vertical_scrollbar() const;

	//! Inherited from twidget.
	tpoint get_minimum_size() const;
	tpoint get_best_size() const;
	tpoint get_maximum_size() const;

	//! Inherited from twidget.
	void set_size(const SDL_Rect& rect);

	//! Gets the widget at the wanted coordinates.
	//! Override base class.
	twidget* get_widget(const tpoint& coordinate);

	//! Gets a widget with the wanted id.
	//! Override base class.
	twidget* get_widget_by_id(const std::string& id);

	/** Inherited from twidget.*/
	bool has_widget(const twidget* widget) const;

	//! Inherited from twidget.
	void draw(surface& surface);

	/** Returns the widget in the selected cell. */
	const twidget* widget(const unsigned row, const unsigned col) const
		{ return child(row, col).widget(); }

	/** Returns the widget in the selected cell. */
	twidget* widget(const unsigned row, const unsigned col) 
		{ return child(row, col).widget(); }

private:
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

		//! Returns the best size for the cell.
		tpoint get_best_size() const;

		//! Returns the minimum size for the cell.
		tpoint get_minimum_size() const;

		//! Returns the maximum size for the cell.
		tpoint get_maximum_size() const;

		void set_size(tpoint orig, tpoint size);

	private:
		//! The id of the widget if it has a widget.
		std::string id_;

		//! The flags for the border and cell setup.
		unsigned flags_;

		//! The size of the border, the actual configuration of the border
		//! is determined by the flags.
		unsigned border_size_;

		//! Pointer to the widget. FIXME who owns the widget....
		twidget* widget_;

		//! The best size for this cell, determined by the best size
		//! of the widget and the border_size_ and flags_.
		mutable tpoint best_size_;

		//! The minimum size for this cell, like best_size_.
		mutable tpoint minimum_size_;

		//! The maximum size for this cell, like best_size_.
		mutable tpoint maximum_size_;

		//! Returns the space needed for the border.
		tpoint border_space() const;

		//! The clipping area for the widget. This is also the size of 
		//! the container.
		SDL_Rect clip_;

		//! Sets the calculations to be dirty, this means all caches
		//! are simply cleared.
		void set_dirty()  // FIXME rename to clear cache??
		{ 
			best_size_ = tpoint(0, 0); 
			minimum_size_ = tpoint(0, 0); 
			maximum_size_ = tpoint(0, 0);
		}

	}; // class tchild

public:
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
	//! The number of rows / columns.
	unsigned rows_;
	unsigned cols_;

	//! The optimal row heights / col widths.
	mutable std::vector<unsigned> best_row_height_;
	mutable std::vector<unsigned> best_col_width_;

	//! The minimal row heights / col widths.
	mutable std::vector<unsigned> minimum_row_height_;
	mutable std::vector<unsigned> minimum_col_width_;

	//! The row heights / col widths currently used.
	std::vector<unsigned> row_height_;
	std::vector<unsigned> col_width_;

	//! The resize factors for rows / cols.
	std::vector<unsigned> row_grow_factor_;
	std::vector<unsigned> col_grow_factor_;

	//! Contains all cells.
	std::vector<tchild> children_;
	const tchild& child(const unsigned row, const unsigned col) const
		{ return children_[rows_ * col + row]; }
	tchild& child(const unsigned row, const unsigned col)
		{ clear_cache(); return children_[rows_ * col + row]; }

	void clear_cache();

	void layout(const tpoint& origin);

	//! Helper function to get the best or minimum size.
	tpoint get_size(const std::string& id, std::vector<unsigned>& width, 
		std::vector<unsigned>& height, tpoint (tchild::*size_proc)() const) const;
};

} // namespace gui2

#endif
