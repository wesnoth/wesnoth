/*
Copyright (C) 2003 - 2017 the Battle for Wesnoth Project http://www.wesnoth.org/

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.

See the COPYING file for more details.
*/

#include "global.hpp"

// The version of libstdc++ shipped with GCC 4.x does not have put_time in the <iomanip> header
// Thus if GCC is the compiler being used, we can simply check the compiler version.
// However, if clang is being used, this won't work.
// Instead, we check for the presence of the <experimental/any> header.
// This was introduced in GCC 5.1, so it's not a perfect check, but it appears to be the best available.
// (Boost also uses the same check internally.)
#if !HAVE_PUT_TIME

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
