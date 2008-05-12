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
 * widget .
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

	/** Inherited from twidget. */
	twidget* get_widget(const tpoint& coordinate) 
		{ return grid_.get_widget(coordinate); }

	/** Inherited from twidget.*/
	twidget* get_widget_by_id(const std::string& id)
	{ 
		twidget* result = twidget::get_widget_by_id(id);
		return result ? result : grid_.get_widget_by_id(id); 
	}

	/** Inherited from twidget.*/
	bool has_widget(const twidget* widget) const 
		{ return grid_.has_widget(widget); }

	/** Inherited from tcontrol. */
	tpoint get_minimum_size() const { return grid_.get_minimum_size(); }

	/** Inherited from tcontrol. */
	tpoint get_best_size() const { return grid_.get_best_size(); }

	/** Inherited from tcontrol. */
	void set_size(const SDL_Rect& rect) 
	{	
		tcontrol::set_size(rect);
		grid_.set_size(rect);
	}

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

protected:
	const tgrid& grid() const { return grid_; }
	tgrid& grid() { return grid_; }
		
private:

	tgrid grid_;
};

} // namespace gui2

#endif


