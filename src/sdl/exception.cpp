/*
   Copyright (C) 2014 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "sdl/exception.hpp"

#include <SDL_error.h>

namespace sdl
{

static std::string create_error(const std::string& operation,
								const bool use_sdl_error)
{
	if(use_sdl_error) {
		return operation + " Error »" + SDL_GetError() + "«.\n";
	} else {
		return operation;
	}
}

exception::exception(const std::string& operation, const bool use_sdl_error)
	: game::error(create_error(operation, use_sdl_error))
{
}


} // namespace sdl
