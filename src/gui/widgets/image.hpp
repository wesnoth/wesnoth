/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/styled_widget.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

/** An image. */
class image : public styled_widget
{
public:
	image() : styled_widget(COUNT)
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
		return get_label();
	}

	virtual bool can_mouse_focus() const override { return !tooltip().empty(); }

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

private:
	/** See @ref widget::calculate_best_size. */
	virtual point calculate_best_size() const override;

public:
	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref styled_widget::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

	/** See @ref widget::disable_click_dismiss. */
	bool disable_click_dismiss() const override;

private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum state_t {
		ENABLED,
		COUNT
	};

	/** See @ref styled_widget::get_control_type. */
	virtual const std::string& get_control_type() const override;
};

// }---------- DEFINITION ---------{

struct image_definition : public styled_widget_definition
{
	explicit image_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_image : public builder_styled_widget
{
	explicit builder_image(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
