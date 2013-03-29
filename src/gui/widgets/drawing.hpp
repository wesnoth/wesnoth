/*
   Copyright (C) 2010 - 2013 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_DRAWING_HPP_INCLUDED
#define GUI_WIDGETS_DRAWING_HPP_INCLUDED

#include "gui/widgets/control.hpp"

namespace gui2 {

/**
 * A widget to draw upon.
 *
 * This widget has a fixed size like the spacer, but allows the user to
 * manual draw items. The widget is display only.
 */
class tdrawing
	: public tcontrol
{
public:
	tdrawing()
		: tcontrol(COUNT)
		, best_size_(0, 0)
	{
	}

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

private:
	/** See @ref twidget::calculate_best_size. */
	virtual tpoint calculate_best_size() const OVERRIDE;
public:

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** Inherited from tcontrol. */
	void set_active(const bool) {}

	/** Inherited from tcontrol. */
	bool get_active() const { return true; }

	/** Inherited from tcontrol. */
	unsigned get_state() const { return 0; }

	/** Inherited from tcontrol. */
	bool disable_click_dismiss() const { return false; }

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_best_size(const tpoint& best_size) { best_size_ = best_size; }

private:

	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in
	 * settings.hpp.
	 */
	enum tstate { ENABLED, COUNT };

	/** When we're used as a fixed size item, this holds the best size. */
	tpoint best_size_;

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const;
};


} // namespace gui2

#endif


