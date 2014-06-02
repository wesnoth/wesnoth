/*
   Copyright (C) 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SDL_RECT_HPP_INCLUDED
#define SDL_RECT_HPP_INCLUDED

/**
 * @file
 * Constains the SDL_Rect helper code.
 */

#include <SDL_version.h>
#include "utils.hpp"

#if SDL_VERSION_ATLEAST(2, 0, 0)
#include <SDL_rect.h>
#else
#include <SDL_video.h>
#endif



namespace sdl
{

extern const SDL_Rect empty_rect;

/**
 * Creates an empty SDL_Rect.
 *
 * Since SDL_Rect doesn't have a constructor it's not possible to create it as
 * a temporary for a function parameter. This functions overcomes this limit.
 */
SDL_Rect create_rect(const int x, const int y, const int w, const int h);

bool point_in_rect(int x, int y, const SDL_Rect& rect);
bool rects_overlap(const SDL_Rect& rect1, const SDL_Rect& rect2);
SDL_Rect intersect_rects(SDL_Rect const &rect1, SDL_Rect const &rect2);
SDL_Rect union_rects(const SDL_Rect &rect1, const SDL_Rect &rect2);

void fill_rect_alpha(SDL_Rect &rect, Uint32 color, Uint8 alpha, surface target);

void draw_rectangle(int x, int y, int w, int h, Uint32 color, surface tg);

void draw_solid_tinted_rectangle(int x, int y, int w, int h,
								 int r, int g, int b,
								 double alpha, surface target);
} // namespace sdl

bool operator==(const SDL_Rect& a, const SDL_Rect& b);
bool operator!=(const SDL_Rect& a, const SDL_Rect& b);

#endif
