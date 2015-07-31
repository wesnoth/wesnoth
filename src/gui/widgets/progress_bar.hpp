/*
   Copyright (C) 2010 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_PROGRESS_BAR_HPP_INCLUDED
#define GUI_WIDGETS_PROGRESS_BAR_HPP_INCLUDED

#include "gui/widgets/control.hpp"

namespace gui2
{

class tprogress_bar : public tcontrol
{
public:
	tprogress_bar() : tcontrol(COUNT), percentage_(static_cast<unsigned>(-1))
	{
		// Force canvas update
		set_percentage(0);
	}

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref tcontrol::set_active. */
	virtual void set_active(const bool active) OVERRIDE;

	/** See @ref tcontrol::get_active. */
	virtual bool get_active() const OVERRIDE;

	/** See @ref tcontrol::get_state. */
	virtual unsigned get_state() const OVERRIDE;

	/** See @ref twidget::disable_click_dismiss. */
	bool disable_click_dismiss() const OVERRIDE;


	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_percentage(unsigned percentage);
	unsigned get_percentage() const
	{
		return percentage_;
	}

private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum tstate {
		ENABLED,
		COUNT
	};

	/** The percentage done. */
	unsigned percentage_;

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const OVERRIDE;
};

} // namespace gui2

#endif
