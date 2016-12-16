/*
Copyright (C) 2003 - 2016 the Battle for Wesnoth Project http://www.wesnoth.org/

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.

See the COPYING file for more details.
*/

#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 5

#include <ctime>

namespace util {
	inline std::string put_time(struct tm* time, const char* fmt) {
		char buf[256];
		if(strftime(buf, 256, fmt, time)) {
			return buf;
		}
		return "<badtime>";
	}
}

#else

#include <iomanip>

namespace util {
	using std::put_time;
}

#endif