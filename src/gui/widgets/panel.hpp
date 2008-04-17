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

#ifndef __GUI_WIDGETS_PANEL_HPP_INCLUDED__
#define __GUI_WIDGETS_PANEL_HPP_INCLUDED__

#include "gui/widgets/grid.hpp"

namespace gui2 {

//! Visible container to hold children.
class tpanel : public tcontrol
{

public:
	tpanel() : 
		tcontrol(0),
		grid_(0, 0, 0, 0)
		{
			grid_.set_parent(this);
		}

	
	// Inherited from twidget.
	twidget* get_widget(const tpoint& coordinate) { return grid_.get_widget(coordinate); }

	// Inherited from twidget.
	twidget* get_widget_by_id(const std::string& id) { return grid_.get_widget_by_id(id); }

	// Inherited from twidget.
	bool dirty() const { return twidget::dirty() || grid_.dirty(); }


	//***** **** wrappers to the grid **** ****
	tgrid::iterator begin() { return grid_.begin(); }
	tgrid::iterator end() { return grid_.end(); }

	void set_client_size(const SDL_Rect& rect) { grid_.set_size(rect); }

	void set_rows(const unsigned rows) { grid_.set_rows(rows); }
	unsigned int get_rows() const { return grid_.get_rows(); }

	void set_cols(const unsigned cols) { grid_.set_cols(cols); }
	unsigned int get_cols() const { return grid_.get_cols(); }

	void set_rows_cols(const unsigned rows, const unsigned cols)
		{ grid_.set_rows_cols(rows, cols); }
#if 0
	// FIXME if these are really not needed remove them.
	void add_child(twidget* widget, const unsigned row, const unsigned col)
		{ grid_.add_child(widget, row, col); }

	void add_child(twidget* widget, const unsigned row, const unsigned col, const unsigned flags)
		{ grid_.add_child(widget, row, col, flags); }
#endif
	void add_child(twidget* widget, const unsigned row, 
		const unsigned col, const unsigned flags, const unsigned border_size)
		{ grid_.add_child(widget, row, col, flags, border_size); }
	
	void set_row_scaling(const unsigned row, const unsigned scale) 
		{ grid_.set_row_scaling(row, scale); }

	void set_col_scaling(const unsigned col, const unsigned scale)
		{ grid_.set_col_scaling(col, scale); }

	//! Inherited from twidget.
	//FIXME we also need to load our own config
	void draw(surface& surface) { grid_.draw(surface); }

	//! Inherited from twidget.
	//FIXME we also need to load our own config
	void load_config() { grid_.load_config(); }

private:
	tgrid grid_;

}; 

} // namespace gui2

#endif


