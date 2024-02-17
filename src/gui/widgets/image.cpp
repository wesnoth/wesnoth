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

#include "gui/widgets/image.hpp"

#include "picture.hpp" // We want the file in src/

#include "gui/core/widget_definition.hpp"
#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "wml_exception.hpp"
#include "gettext.hpp"


#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(image)

image::image(const implementation::builder_image& builder)
	: styled_widget(builder, type())
{
}

point image::calculate_best_size() const
{
	point image_size = ::image::get_size(::image::locator{get_label()});

	if(image_size.x == 0 || image_size.y == 0) {
		DBG_GUI_L << LOG_HEADER << " empty image return default.";
		return get_config_default_size();
	}

	const point minimum = get_config_default_size();
	const point maximum = get_config_maximum_size();

	point result {image_size.x, image_size.y};

	if(minimum.x > 0 && result.x < minimum.x) {
		DBG_GUI_L << LOG_HEADER << " increase width to minimum.";
		result.x = minimum.x;
	} else if(maximum.x > 0 && result.x > maximum.x) {
		DBG_GUI_L << LOG_HEADER << " decrease width to maximum.";
		result.x = maximum.x;
	}

	if(minimum.y > 0 && result.y < minimum.y) {
		DBG_GUI_L << LOG_HEADER << " increase height to minimum.";
		result.y = minimum.y;
	} else if(maximum.y > 0 && result.y > maximum.y) {
		DBG_GUI_L << LOG_HEADER << " decrease height to maximum.";
		result.y = maximum.y;
	}

	DBG_GUI_L << LOG_HEADER << " result " << result << ".";
	return result;
}

void image::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool image::get_active() const
{
	return true;
}

unsigned image::get_state() const
{
	return ENABLED;
}

bool image::disable_click_dismiss() const
{
	return false;
}

// }---------- DEFINITION ---------{

image_definition::image_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing image " << id;

	load_resolutions<resolution>(cfg);
}

image_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	// Note the order should be the same as the enum state_t in image.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", _("Missing required state for image control")));
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_image::builder_image(const config& cfg) : builder_styled_widget(cfg)
{
}

std::unique_ptr<widget> builder_image::build() const
{
	auto widget = std::make_unique<image>(*this);

	DBG_GUI_G << "Window builder: placed image '" << id << "' with definition '"
			  << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
