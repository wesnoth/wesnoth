/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
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

#include "gettext.hpp"
#include "preferences/preferences.hpp"
#include "serialization/chrono.hpp"

#include <cassert>
#ifndef CPP20_CHRONO_SUPPORT
#include <ctime>
#endif

namespace utils
{
std::string format_time_summary(const std::chrono::system_clock::time_point& t)
{
#ifdef CPP20_CHRONO_SUPPORT
	const auto now = std::chrono::system_clock::now();
	std::string format_string;

	auto curr_time = std::chrono::year_month_weekday{std::chrono::floor<std::chrono::days>(now)};
	auto save_time = std::chrono::year_month_weekday{std::chrono::floor<std::chrono::days>(t)};

	if(curr_time == save_time) {
		// save is from today
		format_string = prefs::get().use_twelve_hour_clock_format()
			// TRANSLATORS: 12-hour time, eg '1:59 PM'
			? _("%I:%M %p")
			// TRANSLATORS: 24-hour time, eg '13:59'
			: _("%H:%M");

	} else if(curr_time.month() == save_time.month() && curr_time.index() == save_time.index()) {
		// save is from this week
		format_string = prefs::get().use_twelve_hour_clock_format()
			// TRANSLATORS: Day of week + 12-hour time, eg 'Sunday, 1:59 PM'
			? _("%A, %I:%M %p")
			// TRANSLATORS: Day of week + 24-hour time, eg 'Sunday, 13:59'
			: _("%A, %H:%M");

	} else if(curr_time.year() == save_time.year()) {
		// save is from current year
		// TRANSLATORS: Month + day of month, eg 'Nov 02'. Format for your locale.
		format_string = _("%b %d");

	} else {
		// save is from a different year
		// TRANSLATORS: Month + day of month + year, eg 'Nov 02 2021'. Format for your locale.
		format_string = _("%b %d %Y");
	}
#else
	const auto now = std::chrono::system_clock::now();

	auto as_time_t = std::chrono::system_clock::to_time_t(now);
	const std::tm* timeptr = std::localtime(&as_time_t);
	if(timeptr == nullptr) {
		return "";
	}

	const std::tm current_time = *timeptr;

	as_time_t = std::chrono::system_clock::to_time_t(t);
	timeptr = std::localtime(&as_time_t);
	if(timeptr == nullptr) {
		return "";
	}

	const std::tm save_time = *timeptr;

	std::string format_string;

	if(current_time.tm_year == save_time.tm_year) {
		const int days_apart = current_time.tm_yday - save_time.tm_yday;
		if(days_apart == 0) {
			// save is from today
			if(prefs::get().use_twelve_hour_clock_format() == false) {
				// TRANSLATORS: 24-hour time, eg '13:59'
				format_string = _("%H:%M");
			}
			else {
				// TRANSLATORS: 12-hour time, eg '1:59 PM'
				format_string = _("%I:%M %p");
			}
		} else if(days_apart > 0 && days_apart <= current_time.tm_wday) {
			// save is from this week
			if(prefs::get().use_twelve_hour_clock_format() == false) {
				// TRANSLATORS: Day of week + 24-hour time, eg 'Sunday, 13:59'
				format_string = _("%A, %H:%M");
			}
			else {
				// TRANSLATORS: Day of week + 12-hour time, eg 'Sunday, 1:59 PM'
				format_string = _("%A, %I:%M %p");
			}
		} else {
			// save is from current year
			// TRANSLATORS: Month + day of month, eg 'Nov 02'. Format for your locale.
			format_string = _("%b %d");
		}
	} else {
		// save is from a different year
		// TRANSLATORS: Month + day of month + year, eg 'Nov 02 2021'. Format for your locale.
		format_string = _("%b %d %Y");
	}
#endif
	// TODO: make sure this doesn't result in #1709 coming back
	assert(!format_string.empty());
	return chrono::format_local_timestamp(t, format_string);
}

} // namespace utils
