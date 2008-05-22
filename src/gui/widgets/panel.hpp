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

#include "gui/widgets/container.hpp"

namespace gui2 {

//! Visible container to hold children.
class tpanel : public tcontainer_ 
{

public:
	//! Constructor.
	//!
	//! @param canvas_count  The canvas count for tcontrol.
	tpanel(const unsigned canvas_count = 2) : 
		tcontainer_(canvas_count)
	{
	}

	twidget* find_widget(const tpoint& coordinate, const bool must_be_active) 
		{ return tcontainer_::find_widget(coordinate, must_be_active); }

	const twidget* find_widget(const tpoint& coordinate, 
			const bool must_be_active) const
		{ return tcontainer_::find_widget(coordinate, must_be_active); }

	twidget* find_widget(const std::string& id, const bool must_be_active)
		{ return tcontainer_::find_widget(id, must_be_active); }

	const twidget* find_widget(const std::string& id, 
			const bool must_be_active) const
		{ return tcontainer_::find_widget(id, must_be_active); }

	bool has_vertical_scrollbar() const { return false; }

	//! A panel is always active atm so ignore the request.
	void set_active(const bool /*active*/) {}
	bool get_active() const { return true; }
	unsigned get_state() const { return 0; }

	//! Inherited from tcontrol.
	void draw(surface& surface);

	SDL_Rect get_client_rect() const;

private:

	//! Inherited from tcontrol.
	const std::string& get_control_type() const 
		{ static const std::string type = "panel"; return type; }

	/** Inherited from tcontainer_. */
	tpoint border_space() const;
}; 

} // namespace gui2

#endif


