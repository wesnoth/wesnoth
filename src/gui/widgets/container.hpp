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

#ifndef __GUI_WIDGETS_CONTAINER_HPP_INCLUDED__
#define __GUI_WIDGETS_CONTAINER_HPP_INCLUDED__

#include "gui/widgets/grid.hpp"
#include "gui/widgets/control.hpp"

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
public:
	tcontainer_(const unsigned canvas_count) :
		tcontrol(canvas_count)
	{
		grid_.set_parent(this);
	}

	/** Inherited from twidget. */
	bool dirty() const { return twidget::dirty() || grid_.dirty(); }

	/** Inherited from tcontrol. */
	twidget* find_widget(const tpoint& coordinate, const bool must_be_active) 
		{ return grid_.find_widget(coordinate, must_be_active); }

	/** Inherited from tcontrol. */
	const twidget* find_widget(const tpoint& coordinate, 
			const bool must_be_active) const
		{ return grid_.find_widget(coordinate, must_be_active); }

	/** Inherited from tcontrol.*/
	twidget* find_widget(const std::string& id, const bool must_be_active)
	{ 
		twidget* result = tcontrol::find_widget(id, must_be_active);
		return result ? result : grid_.find_widget(id, must_be_active); 
	}

	/** Inherited from tcontrol.*/
	const twidget* find_widget(const std::string& id, const bool must_be_active) const
	{ 
		const twidget* result = tcontrol::find_widget(id, must_be_active);
		return result ? result : grid_.find_widget(id, must_be_active); 
	}

	/** Inherited from twidget.*/
	bool has_widget(const twidget* widget) const 
		{ return grid_.has_widget(widget); }

	/** 
	 * Inherited from twidget. 
	 * 
	 * Since we can't define a good default behaviour we force the inheriting
	 * classes to define this function. So inheriting classes act as one widget
	 * others as a collection of multiple objects.
	 */
	bool has_vertical_scrollbar() const = 0;

	/** Inherited from tcontrol. */
	tpoint get_minimum_size() const;

	/** Inherited from tcontrol. */
	tpoint get_best_size() const;

	/** Inherited from tcontrol. */
	void set_size(const SDL_Rect& rect) 
	{	
		tcontrol::set_size(rect);
		set_client_size(get_client_rect());
	}

	/** FIXME see whether needed to be exported. */
	void set_client_size(const SDL_Rect& rect) { grid_.set_size(rect); }

	/** Inherited from tcontrol. */
	void draw(surface& surface);
	
	/***** **** wrappers to the grid **** ****/

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

	void add_child(twidget* widget, const unsigned row, 
		const unsigned col, const unsigned flags, const unsigned border_size)
		{ grid_.add_child(widget, row, col, flags, border_size); }

	void set_row_grow_factor(const unsigned row, const unsigned factor) 
		{ grid_.set_row_grow_factor(row, factor); }

	void set_col_grow_factor(const unsigned col, const unsigned factor)
		{ grid_.set_col_grow_factor(col, factor); }

	virtual SDL_Rect get_client_rect() const { return get_rect(); }
protected:
	const tgrid& grid() const { return grid_; }
	tgrid& grid() { return grid_; }
		
private:

	tgrid grid_;

	//! Returns the space used by the border.
	virtual tpoint border_space() const { return tpoint(0, 0); }
};

} // namespace gui2

#endif


