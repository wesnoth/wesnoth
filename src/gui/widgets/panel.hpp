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

#ifndef GUI_WIDGETS_PANEL_HPP_INCLUDED
#define GUI_WIDGETS_PANEL_HPP_INCLUDED

#include "gui/widgets/container.hpp"

namespace gui2 {

/**
 * Visible container to hold multiple widgets.
 *
 * This widget can draw items beyond the widgets it holds and in front of them.
 * A panel is always active so these functions return dummy values.
 */
class tpanel : public tcontainer_ 
{

public:

	/**
	 * Constructor.
	 *
	 * @param canvas_count        The canvas count for tcontrol.
	 */
	tpanel(const unsigned canvas_count = 2) : 
		tcontainer_(canvas_count)
	{
	}

	/** 
	 * Returns the client rect.
	 *
	 * The client rect is the area which is used for child items. The rest of
	 * the area of this widget is used for its own decoration.
	 *
	 * @returns                   The client rect.
	 */
	virtual SDL_Rect get_client_rect() const;

	/** Inherited from tcontrol. */
	bool get_active() const { return true; }

	/** Inherited from tcontrol. */
	unsigned get_state() const { return 0; }

#ifndef NEW_DRAW
	/** Inherited from tcontrol. */
	void draw(surface& surface,  const bool force = false,
	        const bool invalidate_background = false);
#else	
	/** Inherited from tcontrol. */
	void draw_background(surface& frame_buffer);

	/** Inherited from tcontrol. */
	void draw_foreground(surface& frame_buffer);
#endif

private:

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const 
		{ static const std::string type = "panel"; return type; }

	/** Inherited from tcontainer_. */
	tpoint border_space() const;

	/** Inherited from tcontainer_. */
	void set_self_active(const bool /*active*/) {}

}; 

} // namespace gui2

#endif


