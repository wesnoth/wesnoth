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
	//! @param canvas_count  The canvas count for tcontrol.
	tpanel(const unsigned canvas_count = 2) : 
		tcontrol(canvas_count),
		grid_()
	{
		grid_.set_parent(this);
	}

	/** Inherited from tcontrol, copied from container.hpp. */
	twidget* find_widget(const tpoint& coordinate, const bool must_be_active) 
		{ return grid_.find_widget(coordinate, must_be_active); }

	/** Inherited from tcontrol, copied from container.hpp. */
	const twidget* find_widget(const tpoint& coordinate, 
			const bool must_be_active) const
		{ return grid_.find_widget(coordinate, must_be_active); }

	/** Inherited from tcontrol, copied from container.hpp.*/
	twidget* find_widget(const std::string& id, const bool must_be_active)
	{ 
		twidget* result = tcontrol::find_widget(id, must_be_active);
		return result ? result : grid_.find_widget(id, must_be_active); 
	}

	/** Inherited from tcontrol, copied from container.hpp.*/
	const twidget* find_widget(const std::string& id, 
			const bool must_be_active) const
	{ 
		const twidget* result = tcontrol::find_widget(id, must_be_active);
		return result ? result : grid_.find_widget(id, must_be_active); 
	}
	
	// Inherited from twidget.
	bool dirty() const { return twidget::dirty() || grid_.dirty(); }

	//! A panel is always active atm so ignore the request.
	void set_active(const bool /*active*/) {}
	bool get_active() const { return true; }
	unsigned get_state() const { return 0; }

	//! Inherited from tcontrol.
	tpoint get_minimum_size() const;
	tpoint get_best_size() const;

	// get_maximum_size is inherited from tcontrol.

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

	void add_child(twidget* widget, const unsigned row, 
		const unsigned col, const unsigned flags, const unsigned border_size)
		{ grid_.add_child(widget, row, col, flags, border_size); }
	
	void set_row_grow_factor(const unsigned row, const unsigned factor) 
		{ grid_.set_row_grow_factor(row, factor); }

	void set_col_grow_factor(const unsigned col, const unsigned factor)
		{ grid_.set_col_grow_factor(col, factor); }

private:
	tgrid grid_;

	//! Inherited from tcontrol.
	const std::string& get_control_type() const 
		{ static const std::string type = "panel"; return type; }

	//! Returns the space used by the border.
	tpoint border_space() const;
}; 

} // namespace gui2

#endif


