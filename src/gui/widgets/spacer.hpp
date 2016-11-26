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

#include "gui/widgets/styled_widget.hpp"

#include "gui/auxiliary/typed_formula.hpp"
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
class spacer : public styled_widget
{
public:
	spacer() : styled_widget(0), best_size_(0, 0)
	{
	}

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

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_best_size(const point& best_size)
	{
		best_size_ = best_size;
	}

private:
	/** When we're used as a fixed size item, this holds the best size. */
	point best_size_;

	/** See @ref widget::impl_draw_background. */
	virtual void impl_draw_background(surface& frame_buffer,
									  int x_offset,
									  int y_offset) override;

	/** See @ref styled_widget::get_control_type. */
	virtual const std::string& get_control_type() const override;
};

// }---------- DEFINITION ---------{

struct spacer_definition : public styled_widget_definition
{
	explicit spacer_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_spacer : public builder_styled_widget
{
	explicit builder_spacer(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;

private:
	typed_formula<unsigned> width_;
	typed_formula<unsigned> height_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
