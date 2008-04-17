/* $Id$ */
/*
   copyright (c) 2008 by mark de wever <koraq@xs4all.nl>
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

namespace gui2 {

//! Base container class which needs to size children
class tgrid : public virtual twidget
{

public:
	// ***** ***** FLAGS ***** *****
//	static const unsigned VERTICAL_RESIZE_GROW           = 1 << 1;
	static const unsigned VERTICAL_GROW_SEND_TO_CLIENT   = 1 << 2;

	static const unsigned VERTICAL_ALIGN_TOP             = 3 << 4;   
	static const unsigned VERTICAL_ALIGN_CENTER          = 2 << 4;   
	static const unsigned VERTICAL_ALIGN_BOTTOM          = 1 << 4;   
//	static const unsigned VERTICAL_ALIGN_LANGUAGE        = 0 << 4;   


//	static const unsigned HORIZONTAL_RESIZE_GROW         = 1 << 16;
	static const unsigned HORIZONTAL_GROW_SEND_TO_CLIENT = 1 << 17;

	static const unsigned HORIZONTAL_ALIGN_LEFT          = 3 << 18;   
	static const unsigned HORIZONTAL_ALIGN_CENTER        = 2 << 18;   
	static const unsigned HORIZONTAL_ALIGN_RIGHT         = 1 << 18;   
//	static const unsigned HORIZONTAL_ALIGN_LANGUAGE      = 0 << 18;   


	static const unsigned BORDER_TOP                     = 1 << 24;
	static const unsigned BORDER_BOTTOM                  = 1 << 25;
	static const unsigned BORDER_LEFT                    = 1 << 26;
	static const unsigned BORDER_RIGHT                   = 1 << 27;

	
	tgrid(const unsigned rows, const unsigned cols, 
		const unsigned default_flags, const unsigned default_border_size);

	virtual ~tgrid();

#if 0
	// FIXME if these are really not needed remove them.
	void add_child(twidget* widget, const unsigned row, const unsigned col, const unsigned flags)
		{ add_child(widget, row, col, flags, default_border_size_); }
	 
	void add_child(twidget* widget, const unsigned row, const unsigned col)
		{ add_child(widget, row, col, default_flags_, default_border_size_); }
#endif
	void add_child(twidget* widget, const unsigned row, 
		const unsigned col, const unsigned flags, const unsigned border_size);
	
	void set_rows(const unsigned rows);
	unsigned int get_rows() const { return rows_; }

	void set_cols(const unsigned cols);
	unsigned int get_cols() const { return cols_; }

	void set_rows_cols(const unsigned rows, const unsigned cols);

	void set_row_scaling(const unsigned row, const unsigned scale)
		{ row_scaling_[row] = scale; set_dirty(); } //FIXME add assert.

	void set_col_scaling(const unsigned col, const unsigned scale)
		{ col_scaling_[col] = scale; set_dirty(); } //FIXME add assert.

	void remove_child(const unsigned row, const unsigned col);
	void remove_child(const std::string& id, const bool find_all = false);

	//FIXME  add the option to set the proportional growth for each row and column

	//! Inherited from twidget.
	tpoint get_minimum_size() const { /*FIXME IMPLEMENT*/ return tpoint(0,0); } 
	tpoint get_best_size() const;
	tpoint get_maximum_size() const { /*FIXME IMPLEMENT*/ return tpoint(0,0); }

	//! Inherited from twidget.
	void set_size(const SDL_Rect& rect);

	//! Gets the widget at the wanted coordinates.
	//! Override base class.
	twidget* get_widget(const tpoint& coordinate);

	//! Gets a widget with the wanted id.
	//! Override base class.
	twidget* get_widget_by_id(const std::string& id);

	//! Inherited from twidget.
	void draw(surface& surface);

	//! Inherited from twidget.
	void load_config();

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

		twidget* widget() { return widget_; }
		void set_widget(twidget* widget) { widget_ = widget; set_dirty(); }

		//! Gets the best size for the cell, not const since we modify the internal
		//! state, might use mutable later (if really needed).
		tpoint get_best_size() const;

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

		//! The clipping area for the widget. This is also the size of 
		//! the container.
		SDL_Rect clip_;

		//! Sets the calculations to be dirty, this means all caches
		//! are simply cleared.
		void set_dirty()  // FIXME rename to clear cache??
		{ 
			best_size_ = tpoint(0, 0); 
			minimum_size_ = tpoint(0, 0); 
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

	//! Default flags for a grid cell.
	const unsigned default_flags_;

	//! Default border size for a grid cell.
	const unsigned default_border_size_;

	//! The optimal row heights / col widths.
	mutable std::vector<unsigned> best_row_height_; //FIXME implement
	mutable std::vector<unsigned> best_col_width_; //FIXME implement

	//! The minimal row heights / col widths.
	mutable std::vector<unsigned> minimum_row_height_; //FIXME implement
	mutable std::vector<unsigned> minimum_col_width_; //FIXME implement

	//! The row heights / col widths currently used.
	std::vector<unsigned> row_height_;
	std::vector<unsigned> col_width_;

	//! The resize factors for rows / cols.
	std::vector<unsigned> row_scaling_;
	std::vector<unsigned> col_scaling_;

	//! Contains all cells.
	std::vector<tchild> children_;
	const tchild& child(const unsigned row, const unsigned col) const
		{ return children_[rows_ * col + row]; }
	tchild& child(const unsigned row, const unsigned col)
		{ clear_cache(); return children_[rows_ * col + row]; }

	void clear_cache();

	void layout(const tpoint& origin);
};

} // namespace gui2

#endif
