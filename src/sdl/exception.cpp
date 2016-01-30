/*
   Copyright (C) 2014 - 2016 by Mark de Wever <koraq@xs4all.nl>
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
#include "gpu.hpp"

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
#ifdef SDL_GPU
static std::string create_gpu_error(const std::string &op,
									const bool fetch_error_msg)
{
	if (fetch_error_msg) {
		return op + " Error »" + GPU_PopErrorCode().details + "«.\n";
	} else {
		return op;
	}
}
#endif

texception::texception(const std::string& operation, const bool use_sdl_error)
	: game::error(create_error(operation, use_sdl_error))
{
}

#ifdef SDL_GPU
tgpu_exception::tgpu_exception(const std::string &op,
							   const bool fetch_error_msg)
	: game::error(create_gpu_error(op, fetch_error_msg))
{
}
#endif

} // namespace sdl
