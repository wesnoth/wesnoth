/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "drawing_queue.hpp"

#include "display.hpp"
#include "sdl/surface.hpp"
#include "video.hpp"

namespace
{
enum {
	// You may adjust the following when needed:

	// maximum border. 3 should be safe even if a larger border is in use somewhere
	MAX_BORDER = 3,

	// store x, y, and layer in one 32 bit integer
	// 4 most significant bits == layer group   => 16
	BITS_FOR_LAYER_GROUP = 4,

	// 10 second most significant bits == y     => 1024
	BITS_FOR_Y = 10,

	// 1 third most significant bit == x parity => 2
	BITS_FOR_X_PARITY = 1,

	// 8 fourth most significant bits == layer  => 256
	BITS_FOR_LAYER = 8,

	// 9 least significant bits == x / 2        => 512 (really 1024 for x)
	BITS_FOR_X_OVER_2 = 9
};

} // end anon namespace

drawing_queue::buffer_key::layer_group_array drawing_queue::buffer_key::layer_groups {{
	LAYER_TERRAIN_BG,
	LAYER_UNIT_FIRST,
	LAYER_UNIT_MOVE_DEFAULT,
	// Make sure the movement doesn't show above fog and reachmap.
	LAYER_REACHMAP
}};

drawing_queue::buffer_key::buffer_key(const map_location &loc, layer layer)
	: key_(0)
{
	// Start with the index of last group entry...
	unsigned int group_i = layer_groups.size() - 1;

	// ...and works backwards until the group containing the specified layer is found.
	while(layer < layer_groups[group_i]) {
		--group_i;
	}

	enum {
		SHIFT_LAYER          = BITS_FOR_X_OVER_2,
		SHIFT_X_PARITY       = BITS_FOR_LAYER    + SHIFT_LAYER,
		SHIFT_Y              = BITS_FOR_X_PARITY + SHIFT_X_PARITY,
		SHIFT_LAYER_GROUP    = BITS_FOR_Y        + SHIFT_Y
	};

	static_assert(SHIFT_LAYER_GROUP + BITS_FOR_LAYER_GROUP == sizeof(key_) * 8, "Bit field too small");

	/* The parity of x must be more significant than the layer but less significant than y.
	 * Thus basically every row is split in two: First the row containing all the odd x
	 * then the row containing all the even x. Since thus the least significant bit of x is
	 * not required for x ordering anymore it can be shifted out to the right.
	 */
	const unsigned int x_parity = static_cast<unsigned int>(loc.x) & 1;

	key_  = (group_i  << SHIFT_LAYER_GROUP) | (static_cast<unsigned int>(loc.y + MAX_BORDER) << SHIFT_Y);
	key_ |= (x_parity << SHIFT_X_PARITY);
	key_ |= (static_cast<unsigned int>(layer) << SHIFT_LAYER) | static_cast<unsigned int>(loc.x + MAX_BORDER) / 2;
}

void drawing_queue::render_buffer()
{
	// std::list::sort() is a stable sort
	buffer_.sort();

	display* disp = display::get_singleton();

	SDL_Rect clip_rect = disp->map_area();
	surface& screen = disp->get_screen_surface();

	clip_rect_setter set_clip_rect(screen, &clip_rect);

	/* Info regarding the rendering algorithm.
	 *
	 * In order to render a hex properly it needs to be rendered per row. On
	 * this row several layers need to be drawn at the same time. Mainly the
	 * unit and the background terrain. This is needed since both can spill
	 * in the next hex. The foreground terrain needs to be drawn before to
	 * avoid decapitation a unit.
	 *
	 * This ended in the following priority order:
	 * layergroup > location > layer > 'blit_helper' > surface
	 */
	for(const blit_helper& blit : buffer_) {
		for(const surface& surf : blit.surfaces()) {
			// Note that dstrect can be changed by sdl_blit
			// and so a new instance should be initialized
			// to pass to each call to sdl_blit.
			SDL_Rect dstrect {blit.x(), blit.y(), 0, 0};
			SDL_Rect srcrect = blit.clip();
			SDL_Rect* srcrectArg = (srcrect.x | srcrect.y | srcrect.w | srcrect.h) ? &srcrect : nullptr;

			sdl_blit(surf, srcrectArg, screen, &dstrect);
			// NOTE: the screen part should already be marked as 'to update'
		}
	}

	// Clear the buffer.
	buffer_.clear();
}
