/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
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

