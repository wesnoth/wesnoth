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

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

/** An image. */
class timage : public tcontrol
{
public:
	timage() : tcontrol(COUNT), best_size_(0, 0)
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

	virtual bool can_mouse_focus() const override { return !tooltip().empty(); }

	void set_best_size(const tpoint& best_size)
	{
		best_size_ = best_size;
	}

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

private:
	/** See @ref twidget::calculate_best_size. */
	virtual tpoint calculate_best_size() const override;

public:
	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref tcontrol::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref tcontrol::get_active. */
	virtual bool get_active() const override;

	/** See @ref tcontrol::get_state. */
	virtual unsigned get_state() const override;

	/** See @ref twidget::disable_click_dismiss. */
	bool disable_click_dismiss() const override;

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

	/** When we're used as a fixed size item, this holds the best size. */
	tpoint best_size_;

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const override;
};

// }---------- DEFINITION ---------{

struct timage_definition : public tcontrol_definition
{
	explicit timage_definition(const config& cfg);

	struct tresolution : public tresolution_definition_
	{
		explicit tresolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct tbuilder_image : public tbuilder_control
{
	explicit tbuilder_image(const config& cfg);

	using tbuilder_control::build;

	/** The width of the widget. */
	tformula<unsigned> width;

	/** The height of the widget. */
	tformula<unsigned> height;

	twidget* build() const;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
