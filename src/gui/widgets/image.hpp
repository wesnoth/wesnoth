/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_IMAGE_HPP_INCLUDED
#define GUI_WIDGETS_IMAGE_HPP_INCLUDED

#include "gui/widgets/control.hpp"

namespace gui2
{

/** An image. */
class timage : public tcontrol
{
public:
	timage() : tcontrol(COUNT)
	{
	}

	/**
	 * Wrapper for set_label.
	 *
	 * Some people considered this function missing and confusing so added
	 * this forward version.
	 *
	 * @param label               The filename image to show.
	 */
	void set_image(const t_string& label)
	{
		set_label(label);
	}

	/**
	 * Wrapper for label.
	 *
	 * Some people considered this function missing and confusing so added
	 * this forward version.
	 *
	 * @returns                   The filename of the image shown.
	 */
	t_string get_image() const
	{
		return label();
	}

	virtual bool can_mouse_focus() const OVERRIDE { return false; }

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

private:
	/** See @ref twidget::calculate_best_size. */
	virtual tpoint calculate_best_size() const OVERRIDE;

public:
	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref tcontrol::set_active. */
	virtual void set_active(const bool active) OVERRIDE;

	/** See @ref tcontrol::get_active. */
	virtual bool get_active() const OVERRIDE;

	/** See @ref tcontrol::get_state. */
	virtual unsigned get_state() const OVERRIDE;

	/** See @ref twidget::disable_click_dismiss. */
	bool disable_click_dismiss() const OVERRIDE;

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

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const OVERRIDE;
};

} // namespace gui2

#endif
