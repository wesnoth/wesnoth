/*
	Copyright (C) 2008 - 2025
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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/spacer.hpp"

#include "gui/core/register_widget.hpp"


namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(spacer)

spacer::spacer(const implementation::builder_spacer& builder, const std::string& w, const std::string& h)
	: styled_widget(builder, type())
	, width_(w)
	, height_(h)
{
}

bool spacer::fills_available_space()
{
	return (!width_.has_formula() && width_() == 0) && (!height_.has_formula() && height_() == 0);
}

void spacer::request_reduce_width(const unsigned maximum_width)
{
	// Do nothing unless this widget fills all available space (has non-size size).
	if(fills_available_space()) {
		styled_widget::request_reduce_width(maximum_width);
	}
}

void spacer::request_reduce_height(const unsigned maximum_height)
{
	// Do nothing unless this widget fills all available space (has non-size size).
	if(fills_available_space()) {
		styled_widget::request_reduce_height(maximum_height);
	}
}

point spacer::calculate_best_size() const
{
	const wfl::map_formula_callable& size = get_screen_size_variables();

	unsigned width = width_(size);
	unsigned height = height_(size);

	point best_size;

	if(width || height) {
		best_size = point(width, height);
	}

	if(best_size != point()) {
		return best_size;
	}

	return styled_widget::calculate_best_size();
}

void spacer::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool spacer::get_active() const
{
	return true;
}

unsigned spacer::get_state() const
{
	return 0;
}

bool spacer::disable_click_dismiss() const
{
	return false;
}

bool spacer::impl_draw_background()
{
	/* DO NOTHING */
	return true;
}

// }---------- DEFINITION ---------{

spacer_definition::spacer_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing spacer " << id;

	load_resolutions<resolution>(cfg);
}

spacer_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_spacer::builder_spacer(const config& cfg)
	: builder_styled_widget(cfg), width_(cfg["width"]), height_(cfg["height"])
{
}

std::unique_ptr<widget> builder_spacer::build() const
{
	auto widget = std::make_unique<spacer>(*this, width_, height_);

	DBG_GUI_G << "Window builder: placed spacer '" << id
			  << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
