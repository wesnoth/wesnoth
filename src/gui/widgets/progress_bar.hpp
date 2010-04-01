/* $Id$ */
/*
   Copyright (C) 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_PROGRESS_BAR_HPP_INCLUDED
#define GUI_WIDGETS_PROGRESS_BAR_HPP_INCLUDED

#include "gui/widgets/control.hpp"

namespace gui2 {

class tprogress_bar
	: public tcontrol
{
public:

	tprogress_bar()
		: tcontrol(COUNT)
		, percentage_(0)
	{
	}

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** Inherited from tcontrol. */
	void set_active(const bool /*active*/) {}

	/** Inherited from tcontrol. */
	bool get_active() const { return true; }

	/** Inherited from tcontrol. */
	unsigned get_state() const { return ENABLED; }

	/** Inherited from tcontrol. */
	bool disable_click_dismiss() const { return false; }


	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_precentage(const unsigned percentage);
	unsigned get_percentage() const { return percentage_; }

private:

	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum tstate { ENABLED, COUNT };

	/** The percentage done. */
	unsigned percentage_;

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const;
};

} // namespace gui2

#endif

