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

#include "gui/widgets/image.hpp"

#include "../../image.hpp"
#include "log.hpp"

namespace gui2 {

tpoint timage::get_best_size() const
{
	surface image(get_image(image::locator(label())));

	if(image) {
		return tpoint(image->w, image->h);
	}

	return tpoint(0, 0);
	
}

} // namespace gui2

