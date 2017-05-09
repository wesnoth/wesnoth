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

#pragma once

/**
 * @file
 * Contains a basic exception class for SDL operations.
 */

#include "exceptions.hpp"

namespace sdl
{

struct exception : public game::error
{
	/**
	 * Constructor.
	 *
	 * @param operation           The operation that has failed.
	 * @param use_sdl_error       If set to @c true the @p operation is
	 *                            appended with the SDL error message. Else the
	 *                            @p operation is the error message for the
	 *                            exception.
	 */
	exception(const std::string& operation, const bool use_sdl_error);
};


} // namespace sdl
