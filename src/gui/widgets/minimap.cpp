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

#include "gui/widgets/minimap.hpp"

#include "map.hpp"
#include "map_exception.hpp"
#include "../../minimap.hpp"

#define ERR_C LOG_STREAM_INDENT(err, config)

namespace gui2 {

void tminimap::draw_background(surface& /*frame_buffer*/)
{
	assert(false); // FIXME implement.
}

void tminimap::set_borders(const unsigned left,
		const unsigned right, const unsigned top, const unsigned bottom)
{
	left_border_ = left;
	right_border_ = right;
	top_border_ = top;
	bottom_border_ = bottom;

	set_dirty();
}

void tminimap::draw_map(surface& surface)
{
	assert(terrain_);

	if(map_data_.empty()) {
		return;
	}

	try {
		const gamemap map(*terrain_, map_data_);

		SDL_Rect rect = get_rect();
		rect.x += left_border_;
		rect.y += top_border_;
		rect.w -= left_border_ + right_border_;
		rect.h -= top_border_ + bottom_border_;
		assert(rect.w > 0 && rect.h > 0);

		const ::surface surf = image::getMinimap(rect.w, rect.h, map, NULL);

		blit_surface(surf, NULL, surface, &rect);

	} catch (incorrect_map_format_exception& e) {
		ERR_C << "Error while loading the map: " << e.msg_ << '\n';
	}
}

} // namespace gui2

