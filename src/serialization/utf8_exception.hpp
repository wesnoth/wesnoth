/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <exception>

namespace utf8 {
	/**
	 * Thrown by operations encountering invalid UTF-8 data.
	 *
	 * Also used for invalid UTF-16 and UCS-4 data.
	 *
	 * @todo FIXME: This clearly needs a better name for that reason.
	 */
	class invalid_utf8_exception : public std::exception {};
}
