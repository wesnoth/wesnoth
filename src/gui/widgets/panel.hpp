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
	//! Constructor.
	//!
	//! @param load_conf     When a class inherits from a panel that
	//!                      config should be loaded, so set this to false.
	//! @param canvas_count  The canvas count for tcontrol.
	tpanel(const bool load_conf = true, const unsigned canvas_count = 2) : 
		tcontrol(canvas_count),
		grid_(0, 0, 0, 0)
	{
		if(load_conf) load_config();
		grid_.set_parent(this);
	}
	
	// Inherited from twidget.
	twidget* get_widget(const tpoint& coordinate) { return grid_.get_widget(coordinate); }

	// Inherited from twidget.
	twidget* get_widget_by_id(const std::string& id) { return grid_.get_widget_by_id(id); }

	// Inherited from twidget.
	bool dirty() const { return twidget::dirty() || grid_.dirty(); }

	//! A panel is always active atm so ignore the request.
	void set_active(const bool /*active*/) {}
	bool get_active() const { return true; }
	unsigned get_state() const { return 0; }

	//! Inherited from tcontrol.
	void draw(surface& surface);

	//! Inherited from tcontrol.
	void set_size(const SDL_Rect& rect);

	//***** **** wrappers to the grid **** ****
	tgrid::iterator begin() { return grid_.begin(); }
	tgrid::iterator end() { return grid_.end(); }

	SDL_Rect get_client_rect() const;

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

private:
	tgrid grid_;

	//! Inherited from tcontrol.
	const std::string& get_control_type() const 
		{ static const std::string type = "panel"; return type; }
}; 

} // namespace gui2

#endif


