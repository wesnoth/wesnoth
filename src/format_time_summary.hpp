/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FORMAT_TIME_SUMMARY_H_INCLUDED
#define FORMAT_TIME_SUMMARY_H_INCLUDED

#include <ctime>
#include <string>

namespace util {
	std::string format_time_summary(time_t t);
}

#endif
