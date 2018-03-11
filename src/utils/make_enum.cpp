/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "utils/make_enum.hpp"
#include "wml_exception.hpp"
#include "game_config.hpp"

namespace make_enum_detail
{
	void debug_conversion_error(const std::string& temp, const bad_enum_cast & e)
	{
		if (!temp.empty() && game_config::debug) {
			FAIL( e.what() );
		}
	}
}
