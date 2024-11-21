/*
	Copyright (C) 2008 - 2024
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

#include "gui/widgets/vertical_scrollbar.hpp"
#include "gui/core/register_widget.hpp"
#include "wml_exception.hpp"


namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(vertical_scrollbar)

vertical_scrollbar::vertical_scrollbar(const implementation::builder_vertical_scrollbar& builder)
	: scrollbar_base(builder, type())
{
}

unsigned vertical_scrollbar::minimum_positioner_length() const
{
	const auto conf = cast_config_to<vertical_scrollbar_definition>();
	assert(conf);
	return conf->minimum_positioner_length;
}

unsigned vertical_scrollbar::maximum_positioner_length() const
{
	const auto conf = cast_config_to<vertical_scrollbar_definition>();
	assert(conf);
	return conf->maximum_positioner_length;
}

unsigned vertical_scrollbar::offset_before() const
{
	const auto conf = cast_config_to<vertical_scrollbar_definition>();
	assert(conf);
	return conf->top_offset;
}

unsigned vertical_scrollbar::offset_after() const
{
	const auto conf = cast_config_to<vertical_scrollbar_definition>();
	assert(conf);
	return conf->bottom_offset;
}

bool vertical_scrollbar::on_positioner(const point& coordinate) const
{
	rect positioner_rect(
		0, get_positioner_offset(), get_width(), get_positioner_length()
	);

	// Note we assume the positioner is over the entire height of the widget.
	return positioner_rect.contains(coordinate);
}

int vertical_scrollbar::on_bar(const point& coordinate) const
{
	// Not on the widget, leave.
	if(static_cast<std::size_t>(coordinate.x) > get_width()
	   || static_cast<std::size_t>(coordinate.y) > get_height()) {
		return 0;
	}

	// we also assume the bar is over the entire width of the widget.
	if(static_cast<std::size_t>(coordinate.y) < get_positioner_offset()) {
		return -1;
	} else if(static_cast<std::size_t>(coordinate.y) > get_positioner_offset()
												  + get_positioner_length()) {
		return 1;
	} else {
		return 0;
	}
}

bool vertical_scrollbar::in_orthogonal_range(const point& coordinate) const
{
	return static_cast<std::size_t>(coordinate.y) < get_height();
}

// }---------- DEFINITION ---------{

vertical_scrollbar_definition::vertical_scrollbar_definition(
		const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing vertical scrollbar " << id;

	load_resolutions<resolution>(cfg);
}

vertical_scrollbar_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
	, minimum_positioner_length(cfg["minimum_positioner_length"].to_unsigned())
	, maximum_positioner_length(cfg["maximum_positioner_length"].to_unsigned())
	, top_offset(cfg["top_offset"].to_unsigned())
	, bottom_offset(cfg["bottom_offset"].to_unsigned())
{
	VALIDATE(minimum_positioner_length,
			 missing_mandatory_wml_key("resolution",
									   "minimum_positioner_length"));

	// Note the order should be the same as the enum state_t in scrollbar.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("scrollbar_definition][resolution", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("scrollbar_definition][resolution", "state_disabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_pressed", missing_mandatory_wml_tag("scrollbar_definition][resolution", "state_pressed")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_focused", missing_mandatory_wml_tag("scrollbar_definition][resolution", "state_focused")));
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_vertical_scrollbar::builder_vertical_scrollbar(const config& cfg)
	: builder_styled_widget(cfg)
{
}

std::unique_ptr<widget> builder_vertical_scrollbar::build() const
{
	auto widget = std::make_unique<vertical_scrollbar>(*this);

	widget->finalize_setup();

	DBG_GUI_G << "Window builder:"
			  << " placed vertical scrollbar '" << id << "' with definition '"
			  << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
