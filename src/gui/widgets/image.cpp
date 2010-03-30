/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/image.hpp"

#include "../../image.hpp"
#include "gui/auxiliary/widget_definition/image.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/settings.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2 {

REGISTER_WIDGET(image)

tpoint timage::calculate_best_size() const
{
	surface image(image::get_image(image::locator(label())));

	tpoint result(0, 0);
	if(image) {
		result = tpoint(image->w, image->h);
	}

	DBG_GUI_L << LOG_HEADER
		<< " empty image " << !image
		<< " result " << result
		<< ".\n";
	return result;
}

const std::string& timage::get_control_type() const
{
	static const std::string type = "image";
	return type;
}

} // namespace gui2

