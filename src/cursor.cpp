/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
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

#include "game_preferences.hpp"
#include "image.hpp"
#include "preferences_display.hpp"
#include "sdl/rect.hpp"
#include "video.hpp"

#include <iostream>

static bool use_color_cursors()
{
#ifdef __APPLE__
	// Color cursors on OS X are known to be unusable, so don't use them ever.
	// See bug #18112.
	return false;
#else
	return game_config::editor == false && preferences::use_color_cursors();
#endif
}

static SDL_Cursor* create_cursor(surface surf)
{
	const surface nsurf(make_neutral_surface(surf));
	if(nsurf == NULL) {
		return NULL;
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
	std::vector<Uint8> data((cursor_width*nsurf->h)/8,0);
	std::vector<Uint8> mask(data.size(),0);

	// See http://sdldoc.csn.ul.ie/sdlcreatecursor.php for documentation
	// on the format that data has to be in to pass to SDL_CreateCursor
	const_surface_lock lock(nsurf);
	const Uint32* const pixels = lock.pixels();
	for(int y = 0; y != nsurf->h; ++y) {
		for(int x = 0; x != nsurf->w; ++x) {

			if (static_cast<size_t>(x) < cursor_width) {
				Uint8 r,g,b,a;
				SDL_GetRGBA(pixels[y*nsurf->w + x],nsurf->format,&r,&g,&b,&a);

				const size_t index = y*cursor_width + x;
				const size_t shift = 7 - (index % 8);

				const Uint8 trans = (a < 128 ? 0 : 1) << shift;
				const Uint8 black = (trans == 0 || (r+g + b) / 3 > 128 ? 0 : 1) << shift;

				data[index/8] |= black;
				mask[index/8] |= trans;
			}
		}
	}

	return SDL_CreateCursor(&data[0],&mask[0],cursor_width,nsurf->h,0,0);
}

namespace {

SDL_Cursor* cache[cursor::NUM_CURSORS] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

// This array must have members corresponding to cursor::CURSOR_TYPE enum members
// Apple need 16x16 b&w cursors
#ifdef __APPLE__
const std::string bw_images[cursor::NUM_CURSORS] = { "normal.png", "wait-alt.png", "move.png", "attack.png", "select.png", "move_drag_alt.png" , "attack_drag_alt.png", "no_cursor.png"};
#else
const std::string bw_images[cursor::NUM_CURSORS] = { "normal.png", "wait.png", "move.png", "attack.png", "select.png", "move_drag.png", "attack_drag.png", "no_cursor.png"};
#endif

const std::string color_images[cursor::NUM_CURSORS] = { "normal.png", "wait.png", "move.png", "attack.png", "select.png", "move_drag.png", "attack_drag.png", ""};

// Position of the hotspot of the cursor, from the normal topleft
const int shift_x[cursor::NUM_CURSORS] = {0, 0, 0, 0, 0, 2, 3, 0};
const int shift_y[cursor::NUM_CURSORS] = {0, 0, 0, 0, 0, 20, 22, 0};

cursor::CURSOR_TYPE current_cursor = cursor::NORMAL;

int cursor_x = -1, cursor_y = -1;
surface cursor_buf = NULL;
bool have_focus = true;
bool color_ready = false;

}

static SDL_Cursor* get_cursor(cursor::CURSOR_TYPE type)
{
	if(cache[type] == NULL) {
		static const std::string prefix = "cursors-bw/";
		const surface surf(image::get_image(prefix + bw_images[type]));
		cache[type] = create_cursor(surf);
	}

	return cache[type];
}

static void clear_cache()
{
	for(size_t n = 0; n != cursor::NUM_CURSORS; ++n) {
		if(cache[n] != NULL) {
			SDL_FreeCursor(cache[n]);
			cache[n] = NULL;
		}
	}

	if(cursor_buf != NULL) {
		cursor_buf = NULL;
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

	const CURSOR_TYPE new_cursor = use_color_cursors() && color_ready ? cursor::NO_CURSOR : current_cursor;

	SDL_Cursor * cursor_image = get_cursor(new_cursor);

	// Causes problem on Mac:
	//if (cursor_image != NULL && cursor_image != SDL_GetCursor())
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
		color_ready = false;
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

void draw(surface screen)
{
	if(use_color_cursors() == false) {
		return;
	}

	if(current_cursor == NUM_CURSORS) {
		current_cursor = NORMAL;
	}

	if(have_focus == false) {
		cursor_buf = NULL;
		return;
	}

	if (!color_ready) {
		// Display start to draw cursor
		// so it can now display color cursor
		color_ready = true;
		// Reset the cursor to be sure that we hide the b&w
		set();
	}

	/** @todo FIXME: don't parse the file path every time */
	const surface surf(image::get_image("cursors/" + color_images[current_cursor]));
	if(surf == NULL) {
		// Fall back to b&w cursors
		std::cerr << "could not load color cursors. Falling back to hardware cursors\n";
		preferences::set_color_cursors(false);
		return;
	}

	if(cursor_buf != NULL && (cursor_buf->w != surf->w || cursor_buf->h != surf->h)) {
		cursor_buf = NULL;
	}

	if(cursor_buf == NULL) {
		cursor_buf = create_compatible_surface(surf);
		if(cursor_buf == NULL) {
			std::cerr << "Could not allocate surface for mouse cursor\n";
			return;
		}
	}

	int new_cursor_x, new_cursor_y;
	SDL_GetMouseState(&new_cursor_x,&new_cursor_y);
	const bool must_update = new_cursor_x != cursor_x || new_cursor_y != cursor_y;
	cursor_x = new_cursor_x;
	cursor_y = new_cursor_y;

	// Save the screen area where the cursor is being drawn onto the back buffer
	SDL_Rect area = sdl::create_rect(cursor_x - shift_x[current_cursor]
			, cursor_y - shift_y[current_cursor]
			, surf->w
			, surf->h);
	sdl_copy_portion(screen,&area,cursor_buf,NULL);

	// Blit the surface
	sdl_blit(surf,NULL,screen,&area);

	if(must_update) {
		update_rect(area);
	}
}

void undraw(surface screen)
{
	if(use_color_cursors() == false) {
		return;
	}

	if(cursor_buf == NULL) {
		return;
	}

	SDL_Rect area = sdl::create_rect(cursor_x - shift_x[current_cursor]
			, cursor_y - shift_y[current_cursor]
			, cursor_buf->w
			, cursor_buf->h);
	sdl_blit(cursor_buf,NULL,screen,&area);
	update_rect(area);
}

} // end namespace cursor

