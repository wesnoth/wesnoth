/*
   Copyright (C) 2005 - 2013 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  String-routines - Templates for lexical_cast & lexical_cast_default.
 */

#include "util.hpp"

#include "gettext.hpp"
#include "preferences.hpp"
#include "strftime.hpp"

#include <cstdlib>
template<>
size_t lexical_cast<size_t, const std::string&>(const std::string& a)
{
	char* endptr;
	size_t res = strtoul(a.c_str(), &endptr, 10);

	if (a.empty() || *endptr != '\0') {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}

#ifndef MSVC_DO_UNIT_TESTS
template<>
size_t lexical_cast<size_t, const char*>(const char* a)
{
	char* endptr;
	size_t res = strtoul(a, &endptr, 10);

	if (*a == '\0' || *endptr != '\0') {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}
#endif
template<>
size_t lexical_cast_default<size_t, const std::string&>(const std::string& a, size_t def)
{
	if(a.empty()) {
		return def;
	}

	char* endptr;
	size_t res = strtoul(a.c_str(), &endptr, 10);

	if (*endptr != '\0') {
		return def;
	} else {
		return res;
	}
}
template<>
size_t lexical_cast_default<size_t, const char*>(const char* a, size_t def)
{
	if(*a == '\0') {
		return def;
	}

	char* endptr;
	size_t res = strtoul(a, &endptr, 10);

	if (*endptr != '\0') {
		return def;
	} else {
		return res;
	}
}
template<>
long lexical_cast<long, const std::string&>(const std::string& a)
{
	char* endptr;
	long res = strtol(a.c_str(), &endptr, 10);

	if (a.empty() || *endptr != '\0') {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}

template<>
long lexical_cast<long, const char*>(const char* a)
{
	char* endptr;
	long res = strtol(a, &endptr, 10);

	if (*a == '\0' || *endptr != '\0') {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}
template<>
long lexical_cast_default<long, const std::string&>(const std::string& a, long def)
{
	if(a.empty()) {
		return def;
	}

	char* endptr;
	long res = strtol(a.c_str(), &endptr, 10);

	if (*endptr != '\0') {
		return def;
	} else {
		return res;
	}
}
template<>
long lexical_cast_default<long, const char*>(const char* a, long def)
{
	if(*a == '\0') {
		return def;
	}

	char* endptr;
	long res = strtol(a, &endptr, 10);

	if (*endptr != '\0') {
		return def;
	} else {
		return res;
	}
}
template<>
int lexical_cast<int, const std::string&>(const std::string& a)
{
	char* endptr;
	int res = strtol(a.c_str(), &endptr, 10);

	if (a.empty() || *endptr != '\0') {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}

#ifndef MSVC_DO_UNIT_TESTS
template<>
int lexical_cast<int, const char*>(const char* a)
{
	char* endptr;
	int res = strtol(a, &endptr, 10);

	if (*a == '\0' || *endptr != '\0') {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}
#endif

template<>
int lexical_cast_default<int, const std::string&>(const std::string& a, int def)
{
	if(a.empty()) {
		return def;
	}

	char* endptr;
	int res = strtol(a.c_str(), &endptr, 10);

	if (*endptr != '\0') {
		return def;
	} else {
		return res;
	}
}

template<>
int lexical_cast_default<int, const char*>(const char* a, int def)
{
	if(*a == '\0') {
		return def;
	}

	char* endptr;
	int res = strtol(a, &endptr, 10);

	if (*endptr != '\0') {
		return def;
	} else {
		return res;
	}
}

template<>
double lexical_cast<double, const std::string&>(const std::string& a)
{
	char* endptr;
	double res = strtod(a.c_str(), &endptr);

	if (a.empty() || *endptr != '\0') {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}

template<>
double lexical_cast<double, const char*>(const char* a)
{
	char* endptr;
	double res = strtod(a, &endptr);

	if (*a == '\0' || *endptr != '\0') {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}

template<>
double lexical_cast_default<double, const std::string&>(const std::string& a, double def)
{
	char* endptr;
	double res = strtod(a.c_str(), &endptr);

	if (a.empty() || *endptr != '\0') {
		return def;;
	} else {
		return res;
	}
}

template<>
double lexical_cast_default<double, const char*>(const char* a, double def)
{
	char* endptr;
	double res = strtod(a, &endptr);

	if (*a == '\0' || *endptr != '\0') {
		return def;
	} else {
		return res;
	}
}

template<>
float lexical_cast<float, const std::string&>(const std::string& a)
{
	char* endptr;
	float res = static_cast<float>(strtod(a.c_str(), &endptr));

	if (a.empty() || *endptr != '\0') {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}

template<>
float lexical_cast<float, const char*>(const char* a)
{
	char* endptr;
	float res = static_cast<float>(strtod(a, &endptr));

	if (*a == '\0' || *endptr != '\0') {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}
template<>
float lexical_cast_default<float, const std::string&>(const std::string& a, float def)
{
	char* endptr;
	float res = static_cast<float>(strtod(a.c_str(), &endptr));

	if (a.empty() || *endptr != '\0') {
		return def;;
	} else {
		return res;
	}
}

template<>
float lexical_cast_default<float, const char*>(const char* a, float def)
{
	char* endptr;
	float res = static_cast<float>(strtod(a, &endptr));

	if (*a == '\0' || *endptr != '\0') {
		return def;
	} else {
		return res;
	}
}

namespace util {

std::string format_time_summary(time_t t) {
	time_t curtime = time(NULL);
	const struct tm* timeptr = localtime(&curtime);
	if(timeptr == NULL) {
		return "";
	}

	const struct tm current_time = *timeptr;

	timeptr = localtime(&t);
	if(timeptr == NULL) {
		return "";
	}

	const struct tm save_time = *timeptr;

	const char* format_string = NULL;

	if(current_time.tm_year == save_time.tm_year) {
		const int days_apart = current_time.tm_yday - save_time.tm_yday;
		if(days_apart == 0) {
			// save is from today
			if(preferences::use_twelve_hour_clock_format() == false) {
				format_string = _("%H:%M");
			}
			else {
				format_string = _("%I:%M %p");
			}
		} else if(days_apart > 0 && days_apart <= current_time.tm_wday) {
			// save is from this week
			if(preferences::use_twelve_hour_clock_format() == false) {
				format_string = _("%A, %H:%M");
			}
			else {
				format_string = _("%A, %I:%M %p");
			}
		} else {
			// save is from current year
			format_string = _("%b %d");
		}
	} else {
		// save is from a different year
		format_string = _("%b %d %y");
	}
	assert(format_string);

	char buf[40];
	const size_t res = util::strftime(buf,sizeof(buf),format_string,&save_time);
	if(res == 0) {
		buf[0] = 0;
	}

	return buf;
}

}
