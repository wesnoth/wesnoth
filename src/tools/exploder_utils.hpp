/* $Id$ */
/*
   Copyright (C) 2004 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EXPLODER_UTILS_HPP_INCLUDED
#define EXPLODER_UTILS_HPP_INCLUDED

#include "../sdl_utils.hpp"
#include <string>

struct exploder_failure
{
public:
	exploder_failure(const std::string& message) :
	       message(message) {}

	std::string message;
};

std::string get_mask_dir();
std::string get_exploder_dir();

void masked_overwrite_surface(SDL_Surface* dest, SDL_Surface* src, SDL_Surface* mask, int x, int y);
bool image_empty(SDL_Surface* surf);
void save_image(SDL_Surface *surf, const std::string &filename);
	
#endif
