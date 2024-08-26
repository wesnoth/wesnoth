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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/drawing.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/register_widget.hpp"

#include "gettext.hpp"
#include "wml_exception.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(drawing)

drawing::drawing(const implementation::builder_drawing& builder)
	: styled_widget(builder, type())
	, best_size_(0, 0)
{
}

point drawing::calculate_best_size() const
{
	return best_size_ != point() ? best_size_
									  : styled_widget::calculate_best_size();
}

void drawing::request_reduce_width(const unsigned maximum_width)
{
	if(best_size_ != point()) {
		// This drawing is of fixed size, do nothing.
	} else {
		styled_widget::request_reduce_width(maximum_width);
	}
}

void drawing::request_reduce_height(const unsigned maximum_height)
{
	if(best_size_ != point()) {
		// This drawing is of fixed size, do nothing.
	} else {
		styled_widget::request_reduce_height(maximum_height);
	}
}

void drawing::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool drawing::get_active() const
{
	return true;
}

unsigned drawing::get_state() const
{
	return 0;
}

bool drawing::disable_click_dismiss() const
{
	return false;
}

// }---------- DEFINITION ---------{

drawing_definition::drawing_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing drawing " << id;

	load_resolutions<resolution>(cfg);
}

drawing_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	/*
	 * Note the order should be the same as the enum state_t in drawing.hpp.
	 * Normally the [draw] section is in the config, but for this widget the
	 * original draw section is ignored, so send a dummy.
	 */
	static const config dummy("draw");
	state.emplace_back(dummy);
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_drawing::builder_drawing(const config& cfg)
	: builder_styled_widget(cfg)
	, width(cfg["width"])
	, height(cfg["height"])
	, draw(VALIDATE_WML_CHILD(cfg, "draw", missing_mandatory_wml_tag("drawing", "draw")))
{
}

std::unique_ptr<widget> builder_drawing::build() const
{
	auto widget = std::make_unique<drawing>(*this);

	const wfl::map_formula_callable& size = get_screen_size_variables();

	const unsigned w = width(size);
	const unsigned h = height(size);

	if(w || h) {
		widget->set_best_size(point(w, h));
	}

	widget->set_drawing_data(draw);

	DBG_GUI_G << "Window builder: placed drawing '" << id
			  << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
