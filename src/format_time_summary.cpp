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

#include "format_time_summary.hpp"

#include "assert.h"
#include "gettext.hpp"
#include "preferences/general.hpp"

namespace utils {

std::string format_time_summary(std::time_t t) {
	std::time_t curtime = std::time(nullptr);
	const std::tm* timeptr = std::localtime(&curtime);
	if(timeptr == nullptr) {
		return "";
	}

	const std::tm current_time = *timeptr;

	timeptr = std::localtime(&t);
	if(timeptr == nullptr) {
		return "";
	}

	const std::tm save_time = *timeptr;

	std::string format_string;

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
		format_string = _("%b %d %Y");
	}
	assert(!format_string.empty());

	return translation::strftime(format_string, &save_time);
}

}
