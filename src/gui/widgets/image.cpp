/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Mark de Wever <koraq@xs4all.nl>
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

namespace gui2 {

tpoint timage::calculate_best_size() const
{
	surface image(get_image(image::locator(label())));

	tpoint result(0, 0);
	if(image) {
		result = tpoint(image->w, image->h);
	}

	DBG_G_L << "timage " << __func__ << ":"
		<< " empty image " << !image
		<< " result " << result
		<< ".\n";
	return result;
}

} // namespace gui2

