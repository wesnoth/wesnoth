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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/image.hpp"

#include "../../image.hpp" // We want the file in src/
#include "gui/auxiliary/widget_definition/image.hpp"
#include "gui/auxiliary/window_builder/image.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/detail/register.hpp"
#include "gui/widgets/settings.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

REGISTER_WIDGET(image)

tpoint timage::calculate_best_size() const
{
	surface image(image::get_image(image::locator(label())));

	if(!image) {
		DBG_GUI_L << LOG_HEADER << " empty image return default.\n";
		return get_config_default_size();
	}

	const tpoint minimum = get_config_default_size();
	const tpoint maximum = get_config_maximum_size();

	tpoint result = tpoint(image->w, image->h);

	if(minimum.x > 0 && result.x < minimum.x) {
		DBG_GUI_L << LOG_HEADER << " increase width to minimum.\n";
		result.x = minimum.x;
	} else if(maximum.x > 0 && result.x > maximum.x) {
		DBG_GUI_L << LOG_HEADER << " decrease width to maximum.\n";
		result.x = maximum.x;
	}

	if(minimum.y > 0 && result.y < minimum.y) {
		DBG_GUI_L << LOG_HEADER << " increase height to minimum.\n";
		result.y = minimum.y;
	} else if(maximum.y > 0 && result.y > maximum.y) {
		DBG_GUI_L << LOG_HEADER << " decrease height to maximum.\n";
		result.y = maximum.y;
	}

	DBG_GUI_L << LOG_HEADER << " result " << result << ".\n";
	return result;
}

void timage::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool timage::get_active() const
{
	return true;
}

unsigned timage::get_state() const
{
	return ENABLED;
}

bool timage::disable_click_dismiss() const
{
	return false;
}

const std::string& timage::get_control_type() const
{
	static const std::string type = "image";
	return type;
}

} // namespace gui2
