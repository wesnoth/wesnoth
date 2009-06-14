/* $Id$ */
/*
   Copyright (C) 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_SCROLLBAR_PANEL_HPP_INCLUDED
#define GUI_WIDGETS_SCROLLBAR_PANEL_HPP_INCLUDED

#include "gui/widgets/scrollbar_container.hpp"

namespace gui2 {

/**
 * Visible container to hold multiple widgets.
 *
 * This widget can draw items beyond the widgets it holds and in front of
 * them. A panel is always active so these functions return dummy values.
 */
class tscrollbar_panel
	: public tscrollbar_container
{

	friend struct tbuilder_scrollbar_panel;
public:

	/**
	 * Constructor.
	 *
	 * @param canvas_count        The canvas count for tcontrol.
	 */
	tscrollbar_panel(const unsigned canvas_count = 2) :
		tscrollbar_container(canvas_count)
	{
	}

	/** Inherited from tcontrol. */
	bool get_active() const { return true; }

	/** Inherited from tcontrol. */
	unsigned get_state() const { return 0; }

private:

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const
		{ static const std::string type = "scrollbar_panel"; return type; }

	/** Inherited from tcontainer_. */
	void set_self_active(const bool /*active*/) {}

};

} // namespace gui2

#endif

