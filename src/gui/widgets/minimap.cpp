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

#include "gui/widgets/minimap.hpp"

#include "gui/auxiliary/log.hpp"
#include "map.hpp"
#include "map_exception.hpp"
#include "../../minimap.hpp"

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM_INDENT(err, log_config)

namespace gui2 {

void tminimap::set_borders(const unsigned left,
		const unsigned right, const unsigned top, const unsigned bottom)
{
	left_border_ = left;
	right_border_ = right;
	top_border_ = top;
	bottom_border_ = bottom;

	set_dirty();
}

void tminimap::impl_draw_background(surface& frame_buffer)
{
	assert(terrain_);

	DBG_GUI_D << "tminimap " << __func__ << ": "
			<< " id " << id()
			<< " size " << get_rect()
			<< ".\n";

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

		blit_surface(surf, NULL, frame_buffer, &rect);

	} catch (incorrect_map_format_exception& e) {
		ERR_CF << "Error while loading the map: " << e.msg_ << '\n';
	}
}

} // namespace gui2

