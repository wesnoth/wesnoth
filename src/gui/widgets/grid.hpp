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

namespace gui2 {

//! Base container class which needs to size children
class tgrid : public virtual twidget
{

public:
	// ***** ***** FLAGS ***** *****
	static const unsigned VERTICAL_RESIZE_GROW           = 1 << 1;
	static const unsigned VERTICAL_GROW_SEND_TO_CLIENT   = 1 << 2;

	static const unsigned VERTICAL_ALIGN_TOP             = 1 << 4;   
	static const unsigned VERTICAL_ALIGN_CENTER          = 1 << 5;   
	static const unsigned VERTICAL_ALIGN_BOTTOM          = 1 << 6;   
	static const unsigned VERTICAL_ALIGN_LANGUAGE        = 1 << 7;   


	static const unsigned HORIZONTAL_RESIZE_GROW         = 1 << 16;
	static const unsigned HORIZONTAL_GROW_SEND_TO_CLIENT = 1 << 17;

	static const unsigned HORIZONTAL_ALIGN_TOP           = 1 << 18;   
	static const unsigned HORIZONTAL_ALIGN_CENTER        = 1 << 19;   
	static const unsigned HORIZONTAL_ALIGN_BOTTOM        = 1 << 20;   
	static const unsigned HORIZONTAL_ALIGN_LANGUAGE      = 1 << 21;   


	static const unsigned BORDER_TOP                     = 1 << 24;
	static const unsigned BORDER_BOTTOM                  = 1 << 25;
	static const unsigned BORDER_LEFT                    = 1 << 26;
	static const unsigned BORDER_RIGHT                   = 1 << 27;

	
	tgrid(const unsigned rows, const unsigned cols, 
		const unsigned default_flags, const unsigned default_border_size);

	virtual ~tgrid();

	void add_child(twidget* widget, const unsigned row, 
		const unsigned col, const unsigned flags, const unsigned border_size);
	
	void add_child(twidget* widget, const unsigned row, const unsigned col)
		{ add_child(widget, row, col, default_flags_, default_border_size_); }

	void add_child(twidget* widget, const unsigned row, const unsigned col, const unsigned flags)
		{ add_child(widget, row, col, flags, default_border_size_); }
	 
	
	void set_rows(const unsigned rows);
	unsigned int get_rows() const { return rows_; }

	void set_cols(const unsigned cols);
	unsigned int get_cols() const { return cols_; }

	void remove_child(const unsigned row, const unsigned col);
	void removed_child(const std::string& id, const bool find_all = false);

	//! Inherited
	tpoint get_best_size();

	//! Inherited
	void set_best_size(const tpoint& origin);

	//! Gets the widget at the wanted coordinates.
	//! Override base class.
	twidget* get_widget(const tpoint& coordinate);

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
			dirty_(true),
			clip_()

			// Fixme make a class wo we can store some properties in the cache 
			// regarding size etc.
			{}
	
		const std::string& id() const { return id_; }
		void set_id(const std::string& id) { id_ = id; }

		unsigned get_flags() const { return flags_; }
		void set_flags(const unsigned flags) { flags_ = flags; dirty_ = true; }

		unsigned get_border_size() const { return border_size_; }
		void set_border_size(const unsigned border_size) 
			{  border_size_ = border_size; dirty_ = true; }

		twidget* widget() { return widget_; }
		void set_widget(twidget* widget) { widget_ = widget; dirty_ = true; }

		//! Gets the best size for the cell, not const since we modify the internal
		//! state, might use mutable later (if really needed).
		tpoint get_best_size();

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
		tpoint best_size_;

		//! Tracks the dirty state of the cell regarding best_size_.
		bool dirty_;

		//! The clipping area for the widget. This is also the size of 
		//! the container.
		SDL_Rect clip_;

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
	unsigned rows_;
	unsigned cols_;

	const unsigned default_flags_;

	const unsigned default_border_size_;

	


	std::vector<tchild> children_;
	tchild& child(const unsigned row, const unsigned col)
		{ return children_[rows_ * col + row]; }

};

//! Visible container to hold children.
class tpanel : public tgrid, public tcontrol
{

public:
	tpanel() : 
		tgrid(0, 0, 0, 0),
		tcontrol()
		{}

private:

}; 

} // namespace gui2

#endif
