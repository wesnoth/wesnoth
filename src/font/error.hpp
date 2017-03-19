/*
   Copyright (C) 2016 - 2017 by Chris Beck<render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FONT_ERROR_HPP
#define FONT_ERROR_HPP

#include "exceptions.hpp"
#include <string>

namespace font {

struct error : public game::error {
	error(const std::string& str = "Font initialization failed") : game::error(str) {}
};

} // end namespace font

#endif
