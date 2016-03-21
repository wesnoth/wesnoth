/*
   Copyright (C) 2010 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

/**
 * A widget to draw upon.
 *
 * This widget has a fixed size like the spacer, but allows the user to
 * manual draw items. The widget is display only.
 */
class tdrawing : public tcontrol
{
public:
	tdrawing() : tcontrol(COUNT), best_size_(0, 0)
	{
	}

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

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_best_size(const tpoint& best_size)
	{
		best_size_ = best_size;
	}

private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in
	 * settings.hpp.
	 */
	enum tstate {
		ENABLED,
		COUNT
	};

	/** When we're used as a fixed size item, this holds the best size. */
	tpoint best_size_;

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const OVERRIDE;
};

// }---------- DEFINITION ---------{

struct tdrawing_definition : public tcontrol_definition
{
	explicit tdrawing_definition(const config& cfg);

	struct tresolution : public tresolution_definition_
	{
		explicit tresolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct tbuilder_drawing : public tbuilder_control
{
	explicit tbuilder_drawing(const config& cfg);

	using tbuilder_control::build;

	twidget* build() const;

	/** The width of the widget. */
	tformula<unsigned> width;

	/** The height of the widget. */
	tformula<unsigned> height;

	/** Config containing what to draw on the widgets canvas. */
	config draw;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
