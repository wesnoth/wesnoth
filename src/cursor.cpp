/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "picture.hpp"
#include "preferences/preferences.hpp"
#include "sdl/utils.hpp"

#include <boost/logic/tribool.hpp>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>

#include <array>
#include <memory>

namespace cursor
{
namespace
{
struct cursor_data
{
	std::string image_bw;
	std::string image_color;

	int hot_x{0};
	int hot_y{0};

	boost::tribool is_color{boost::indeterminate};

	using cursor_ptr = std::unique_ptr<SDL_Cursor, void(*)(SDL_Cursor*)>;
	cursor_ptr cursor{nullptr, SDL_FreeCursor};
};

//
// Array with each available cursor type.
// macOS needs 16x16 b&w cursors. TODO: is that still the case?
//
auto available_cursors = std::array
{
#ifdef __APPLE__
	cursor_data{ "normal.png",          "normal.png",      0,  0  },
	cursor_data{ "wait-alt.png",        "wait.png",        0,  0  },
	cursor_data{ "ibeam.png",           "ibeam.png",       14, 14 },
	cursor_data{ "move.png",            "move.png",        0,  0  },
	cursor_data{ "attack.png",          "attack.png",      0,  0  },
	cursor_data{ "select.png",          "select.png",      0,  0  },
	cursor_data{ "move_drag_alt.png",   "move_drag.png",   2,  20 },
	cursor_data{ "attack_drag_alt.png", "attack_drag.png", 3,  22 },
	cursor_data{ "no_cursor.png",       "",                0,  0  }
#else
	cursor_data{ "normal.png",          "normal.png",      0,  0  },
	cursor_data{ "wait.png",            "wait.png",        0,  0  },
	cursor_data{ "ibeam.png",           "ibeam.png",       14, 14 },
	cursor_data{ "move.png",            "move.png",        0,  0  },
	cursor_data{ "attack.png",          "attack.png",      0,  0  },
	cursor_data{ "select.png",          "select.png",      0,  0  },
	cursor_data{ "move_drag.png",       "move_drag.png",   2,  20 },
	cursor_data{ "attack_drag.png",     "attack_drag.png", 3,  22 },
	cursor_data{ "no_cursor.png",       "",                0,  0  }
#endif
};

static_assert(available_cursors.size() == cursor::NUM_CURSORS);

cursor::CURSOR_TYPE current_cursor = cursor::NORMAL;
bool have_focus = true;

bool use_color_cursors()
{
	return game_config::editor == false && prefs::get().use_color_cursors();
}

SDL_Cursor* create_cursor(const surface& surf)
{
	if(surf == nullptr) {
		return nullptr;
	}

	// The width must be a multiple of 8 (SDL requirement)

#ifdef __APPLE__
	std::size_t cursor_width = 16;
#else
	std::size_t cursor_width = surf->w;
	if((cursor_width % 8) != 0) {
		cursor_width += 8 - (cursor_width % 8);
	}
#endif

	std::vector<uint8_t> data((cursor_width * surf->h) / 8, 0);
	std::vector<uint8_t> mask(data.size(), 0);

	// See https://wiki.libsdl.org/SDL_CreateCursor for documentation
	// on the format that data has to be in to pass to SDL_CreateCursor
	const_surface_lock lock(surf);
	const utils::span pixels = lock.pixel_span();

	for(int y = 0; y != surf->h; ++y) {
		for(int x = 0; x != surf->w; ++x) {
			if(static_cast<std::size_t>(x) < cursor_width) {
				uint8_t r, g, b, a;
				SDL_GetRGBA(pixels[y * surf->w + x], surf->format, &r, &g, &b, &a);

				const std::size_t index = y * cursor_width + x;
				const std::size_t shift = 7 - (index % 8);

				const uint8_t trans = (a < 128 ? 0 : 1) << shift;
				const uint8_t black = (trans == 0 || (r + g + b) / 3 > 128 ? 0 : 1) << shift;

				data[index / 8] |= black;
				mask[index / 8] |= trans;
			}
		}
	}

	return SDL_CreateCursor(&data[0], &mask[0], cursor_width, surf->h, 0, 0);
}

SDL_Cursor* get_cursor(cursor::CURSOR_TYPE type)
{
	const bool use_color = use_color_cursors();
	cursor_data& data = available_cursors[type];

	if(data.cursor == nullptr || boost::indeterminate(data.is_color) || data.is_color != use_color) {
		static const std::string color_prefix = "cursors/";
		static const std::string bw_prefix = "cursors-bw/";

		if(use_color) {
			surface surf = image::get_surface(color_prefix + data.image_color);
			data.cursor.reset(SDL_CreateColorCursor(surf, data.hot_x, data.hot_y));
		} else {
			surface surf = image::get_surface(bw_prefix + data.image_bw);
			data.cursor.reset(create_cursor(surf));
		}

		data.is_color = use_color;
	}

	return data.cursor.get();
}

} // end anon namespace

manager::manager()
{
	set();
}

manager::~manager()
{
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

	SDL_SetCursor(get_cursor(current_cursor));
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
