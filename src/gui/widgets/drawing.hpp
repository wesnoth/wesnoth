/*
	Copyright (C) 2010 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "gui/widgets/styled_widget.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

class config;

namespace gui2
{
namespace implementation
{
	struct builder_drawing;
}
// ------------ WIDGET -----------{

/**
 * @ingroup GUIWidgetWML
 *
 * A drawing is widget with a fixed size and gives access to the canvas of the widget in the window instance.
 * It has a fixed size like the spacer, but allows the user to manually draw items.
 * This widget is display only.
 *
 * The widget normally has no event interaction so only one state is defined:
 * * state_enabled - the drawing is enabled. The state is a dummy since the things really drawn are placed in the window instance.
 *
 * If either the width or the height is not zero the drawing functions as a fixed size drawing.
 * Key          |Type                                    |Default  |Description
 * -------------|----------------------------------------|---------|-----------
 * width        | @ref guivartype_f_unsigned "f_unsigned"|0        |The width of the drawing.
 * height       | @ref guivartype_f_unsigned "f_unsigned"|0        |The height of the drawing.
 * draw         | @ref guivartype_config "config"        |mandatory|The config containing the drawing.
 *
 * The variables available are the same as for the window resolution see @ref builder_window::window_resolution for the list of items.
 */
class drawing : public styled_widget
{
public:
	explicit drawing(const implementation::builder_drawing& builder);

	canvas& get_drawing_canvas()
	{
		return get_canvas(0);
	}

	void set_drawing_data(const ::config& cfg)
	{
		get_drawing_canvas().set_cfg(cfg);
	}

	void append_drawing_data(const ::config& cfg)
	{
		get_drawing_canvas().append_cfg(cfg);
	}

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

	/** See @ref widget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned maximum_width) override;

	/** See @ref widget::request_reduce_height. */
	virtual void request_reduce_height(const unsigned maximum_height) override;

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
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in
	 * settings.hpp.
	 */
	enum state_t {
		ENABLED,
	};

	/** When we're used as a fixed size item, this holds the best size. */
	point best_size_;

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;
};

// }---------- DEFINITION ---------{

struct drawing_definition : public styled_widget_definition
{
	explicit drawing_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_drawing : public builder_styled_widget
{
	explicit builder_drawing(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

	/** The width of the widget. */
	typed_formula<unsigned> width;

	/** The height of the widget. */
	typed_formula<unsigned> height;

	/** Config containing what to draw on the widgets canvas. */
	config draw;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
