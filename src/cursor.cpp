/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#include "image.hpp"
#include "preferences/game.hpp"
#include "sdl/utils.hpp"

#include <boost/logic/tribool.hpp>

#include <array>
#include <memory>

namespace cursor
{
namespace
{
using cursor_ptr_t = std::unique_ptr<SDL_Cursor, std::function<void(SDL_Cursor*)>>;

struct cursor_data
{
	cursor_ptr_t cursor;

	boost::tribool is_color;

	std::string image_bw;
	std::string image_color;

	int hot_x;
	int hot_y;
};

//
// Array with each available cursor type.
// macOS needs 16x16 b&w cursors. TODO: is that still the case?
//
std::array<cursor_data, cursor::NUM_CURSORS> available_cursors {{
#ifdef __APPLE__
	{ nullptr, boost::indeterminate, "normal.png",          "normal.png",      0, 0  },
	{ nullptr, boost::indeterminate, "wait-alt.png",        "wait.png",        0, 0  },
	{ nullptr, boost::indeterminate, "move.png",            "move.png",        0, 0  },
	{ nullptr, boost::indeterminate, "attack.png",          "attack.png",      0, 0  },
	{ nullptr, boost::indeterminate, "select.png",          "select.png",      0, 0  },
	{ nullptr, boost::indeterminate, "move_drag_alt.png",   "move_drag.png",   2, 20 },
	{ nullptr, boost::indeterminate, "attack_drag_alt.png", "attack_drag.png", 3, 22 },
	{ nullptr, boost::indeterminate, "no_cursor.png",       "",                0, 0  }
#else
	{ nullptr, boost::indeterminate, "normal.png",      "normal.png",      0, 0  },
	{ nullptr, boost::indeterminate, "wait.png",        "wait.png",        0, 0  },
	{ nullptr, boost::indeterminate, "move.png",        "move.png",        0, 0  },
	{ nullptr, boost::indeterminate, "attack.png",      "attack.png",      0, 0  },
	{ nullptr, boost::indeterminate, "select.png",      "select.png",      0, 0  },
	{ nullptr, boost::indeterminate, "move_drag.png",   "move_drag.png",   2, 20 },
	{ nullptr, boost::indeterminate, "attack_drag.png", "attack_drag.png", 3, 22 },
	{ nullptr, boost::indeterminate, "no_cursor.png",   "",                0, 0  }

#endif
}};

cursor::CURSOR_TYPE current_cursor = cursor::NORMAL;

bool have_focus = true;

bool use_color_cursors()
{
	return game_config::editor == false && preferences::use_color_cursors();
}

SDL_Cursor* create_cursor(surface surf)
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
	if((cursor_width % 8) != 0) {
		cursor_width += 8 - (cursor_width % 8);
	}
#endif

	std::vector<uint8_t> data((cursor_width * nsurf->h) / 8, 0);
	std::vector<uint8_t> mask(data.size(), 0);

	// See https://wiki.libsdl.org/SDL_CreateCursor for documentation
	// on the format that data has to be in to pass to SDL_CreateCursor
	const_surface_lock lock(nsurf);
	const uint32_t* const pixels = lock.pixels();

	for(int y = 0; y != nsurf->h; ++y) {
		for(int x = 0; x != nsurf->w; ++x) {
			if(static_cast<size_t>(x) < cursor_width) {
				uint8_t r, g, b, a;
				SDL_GetRGBA(pixels[y * nsurf->w + x], nsurf->format, &r, &g, &b, &a);

				const size_t index = y * cursor_width + x;
				const size_t shift = 7 - (index % 8);

				const uint8_t trans = (a < 128 ? 0 : 1) << shift;
				const uint8_t black = (trans == 0 || (r + g + b) / 3 > 128 ? 0 : 1) << shift;

				data[index / 8] |= black;
				mask[index / 8] |= trans;
			}
		}
	}

	return SDL_CreateCursor(&data[0], &mask[0], cursor_width, nsurf->h, 0, 0);
}

SDL_Cursor* get_cursor(cursor::CURSOR_TYPE type)
{
	const bool use_color = use_color_cursors();
	cursor_data& data = available_cursors[type];

	if(data.cursor == nullptr || boost::indeterminate(data.is_color) || data.is_color != use_color) {
		static const std::string color_prefix = "cursors/";
		static const std::string bw_prefix = "cursors-bw/";

		if(use_color) {
			const surface surf(image::get_image(color_prefix + data.image_color));

			// Construct a temporary ptr to provide a new deleter.
			cursor_ptr_t temp(SDL_CreateColorCursor(surf, data.hot_x, data.hot_y), SDL_FreeCursor);
			data.cursor = std::move(temp);
		} else {
			const surface surf(image::get_image(bw_prefix + data.image_bw));

			// Construct a temporary ptr to provide a new deleter.
			cursor_ptr_t temp(create_cursor(surf), SDL_FreeCursor);
			data.cursor = std::move(temp);
		}

		data.is_color = use_color;
	}

	return data.cursor.get();
}

} // end anon namespace

manager::manager()
{
	SDL_ShowCursor(SDL_ENABLE);
	set();
}

manager::~manager()
{
	SDL_ShowCursor(SDL_ENABLE);
}

void set(CURSOR_TYPE type)
{
	// Change only if it's a valid cursor
	if(type != NUM_CURSORS) {
		current_cursor = type;
	} else if(current_cursor == NUM_CURSORS) {
		// Except if the current one is also invalid.
		// In this case, change to a valid one.
		current_cursor = NORMAL;
	}

	SDL_Cursor* cursor_image = get_cursor(current_cursor);

	// Causes problem on Mac:
	// if (cursor_image != nullptr && cursor_image != SDL_GetCursor())
	SDL_SetCursor(cursor_image);

	SDL_ShowCursor(SDL_ENABLE);
}

void set_dragging(bool drag)
{
	switch(current_cursor) {
	case MOVE:
		if(drag) cursor::set(MOVE_DRAG);
		break;
	case ATTACK:
		if(drag) cursor::set(ATTACK_DRAG);
		break;
	case MOVE_DRAG:
		if(!drag) cursor::set(MOVE);
		break;
	case ATTACK_DRAG:
		if(!drag) cursor::set(ATTACK);
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

	if(!focus) {
		set();
	}
}

setter::setter(CURSOR_TYPE type)
	: old_(current_cursor)
{
	set(type);
}

setter::~setter()
{
	set(old_);
}

} // end namespace cursor
