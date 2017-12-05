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

/**
 * @file
 * Support for different cursors-shapes.
 */

#include "cursor.hpp"

#include "preferences/game.hpp"
#include "image.hpp"
#include "preferences/display.hpp"
#include "sdl/rect.hpp"

#include <iostream>
#include <boost/logic/tribool.hpp>
using boost::logic::tribool;
using boost::logic::indeterminate;

static bool use_color_cursors()
{
	return game_config::editor == false && preferences::use_color_cursors();
}

static SDL_Cursor* create_cursor(surface surf)
{
	const surface nsurf(make_neutral_surface(surf));
	if(nsurf == nullptr) {
		return nullptr;
	}

	// The width must be a multiple of 8 (SDL requirement)

#ifdef __APPLE__
	size_t cursor_width = 16;
#else
	size_t cursor_width = nsurf->w;
	if((cursor_width%8) != 0) {
		cursor_width += 8 - (cursor_width%8);
	}
#endif
	std::vector<uint8_t> data((cursor_width*nsurf->h)/8,0);
	std::vector<uint8_t> mask(data.size(),0);

	// See http://sdldoc.csn.ul.ie/sdlcreatecursor.php for documentation
	// on the format that data has to be in to pass to SDL_CreateCursor
	const_surface_lock lock(nsurf);
	const uint32_t* const pixels = lock.pixels();
	for(int y = 0; y != nsurf->h; ++y) {
		for(int x = 0; x != nsurf->w; ++x) {

			if (static_cast<size_t>(x) < cursor_width) {
				uint8_t r,g,b,a;
				SDL_GetRGBA(pixels[y*nsurf->w + x],nsurf->format,&r,&g,&b,&a);

				const size_t index = y*cursor_width + x;
				const size_t shift = 7 - (index % 8);

				const uint8_t trans = (a < 128 ? 0 : 1) << shift;
				const uint8_t black = (trans == 0 || (r+g + b) / 3 > 128 ? 0 : 1) << shift;

				data[index/8] |= black;
				mask[index/8] |= trans;
			}
		}
	}

	return SDL_CreateCursor(&data[0],&mask[0],cursor_width,nsurf->h,0,0);
}

namespace {

SDL_Cursor* cache[cursor::NUM_CURSORS] { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
tribool cache_color[cursor::NUM_CURSORS] {
	indeterminate, indeterminate, indeterminate, indeterminate,
	indeterminate, indeterminate, indeterminate, indeterminate,
};

// This array must have members corresponding to cursor::CURSOR_TYPE enum members
// Apple need 16x16 b&w cursors
#ifdef __APPLE__
const std::string bw_images[cursor::NUM_CURSORS] { "normal.png", "wait-alt.png", "move.png", "attack.png", "select.png", "move_drag_alt.png" , "attack_drag_alt.png", "no_cursor.png"};
#else
const std::string bw_images[cursor::NUM_CURSORS] { "normal.png", "wait.png", "move.png", "attack.png", "select.png", "move_drag.png", "attack_drag.png", "no_cursor.png"};
#endif

const std::string color_images[cursor::NUM_CURSORS] { "normal.png", "wait.png", "move.png", "attack.png", "select.png", "move_drag.png", "attack_drag.png", ""};

// Position of the hotspot of the cursor, from the normal topleft
// These are only for the color cursors
const int shift_x[cursor::NUM_CURSORS] {0, 0, 0, 0, 0, 2, 3, 0};
const int shift_y[cursor::NUM_CURSORS] {0, 0, 0, 0, 0, 20, 22, 0};

cursor::CURSOR_TYPE current_cursor = cursor::NORMAL;

bool have_focus = true;

}

static SDL_Cursor* get_cursor(cursor::CURSOR_TYPE type)
{
	bool is_color = use_color_cursors();
	if(cache[type] == nullptr || indeterminate(cache_color[type]) || cache_color[type] != is_color) {
		const std::string prefix = is_color ? "cursors/" : "cursors-bw/";
		const surface surf(image::get_image(prefix + (is_color ? color_images : bw_images)[type]));
		if (is_color) {
			cache[type] = SDL_CreateColorCursor(surf.get(), shift_x[type], shift_y[type]);
		} else {
			cache[type] = create_cursor(surf);
		}
		cache_color[type] = is_color;
	}

	return cache[type];
}

static void clear_cache()
{
	for(size_t n = 0; n != cursor::NUM_CURSORS; ++n) {
		if(cache[n] != nullptr) {
			SDL_FreeCursor(cache[n]);
			cache[n] = nullptr;
		}
	}
}

namespace cursor
{

manager::manager()
{
	SDL_ShowCursor(SDL_ENABLE);
	set();
}

manager::~manager()
{
	clear_cache();
	SDL_ShowCursor(SDL_ENABLE);
}

void set(CURSOR_TYPE type)
{
	// Change only if it's a valid cursor
	if (type != NUM_CURSORS) {
		current_cursor = type;
	} else if (current_cursor == NUM_CURSORS) {
		// Except if the current one is also invalid.
		// In this case, change to a valid one.
		current_cursor = NORMAL;
	}

	SDL_Cursor * cursor_image = get_cursor(current_cursor);

	// Causes problem on Mac:
	//if (cursor_image != nullptr && cursor_image != SDL_GetCursor())
	SDL_SetCursor(cursor_image);

	SDL_ShowCursor(SDL_ENABLE);
}

void set_dragging(bool drag)
{
	switch(current_cursor) {
		case MOVE:
			if (drag) cursor::set(MOVE_DRAG);
			break;
		case ATTACK:
			if (drag) cursor::set(ATTACK_DRAG);
			break;
		case MOVE_DRAG:
			if (!drag) cursor::set(MOVE);
			break;
		case ATTACK_DRAG:
			if (!drag) cursor::set(ATTACK);
			break;
		default:
			break;
	}
}

CURSOR_TYPE get()
{
	return current_cursor;
}

void set_focus(bool focus)
{
	have_focus = focus;
	if (focus==false) {
		set();
	}
}

setter::setter(CURSOR_TYPE type) : old_(current_cursor)
{
	set(type);
}

setter::~setter()
{
	set(old_);
}


} // end namespace cursor

