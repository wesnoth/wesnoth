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

#ifndef GUI_WIDGETS_SPACER_HPP_INCLUDED
#define GUI_WIDGETS_SPACER_HPP_INCLUDED

#include "gui/widgets/control.hpp"

#include "gui/auxiliary/formula.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

/**
 * An empty widget.
 *
 * Since every grid cell needs a widget this is a blank widget. This widget can
 * also be used to 'force' sizes.
 *
 * Since we're a kind of dummy class we're always active, our drawing does
 * nothing.
 */
class tspacer : public tcontrol
{
public:
	tspacer() : tcontrol(0), best_size_(0, 0)
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
	/** When we're used as a fixed size item, this holds the best size. */
	tpoint best_size_;

	/** See @ref twidget::impl_draw_background. */
	virtual void impl_draw_background(surface& frame_buffer,
									  int x_offset,
									  int y_offset) OVERRIDE;

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const OVERRIDE;
};

// }---------- DEFINITION ---------{

struct tspacer_definition : public tcontrol_definition
{
	explicit tspacer_definition(const config& cfg);

	struct tresolution : public tresolution_definition_
	{
		explicit tresolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct tbuilder_spacer : public tbuilder_control
{
	explicit tbuilder_spacer(const config& cfg);

	using tbuilder_control::build;

	twidget* build() const;

private:
	tformula<unsigned> width_;
	tformula<unsigned> height_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
