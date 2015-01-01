/*
   Copyright (C) 2013 - 2015 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "strftime.hpp"

#include "gettext.hpp"
#include "language.hpp"

#include <string>

namespace {

const char *wday_abbr[] = {N_("Sun"), N_("Mon"), N_("Tue"), N_("Wed"),
	N_("Thu"), N_("Fri"), N_("Sat")};
const char *wday_full[] = {N_("Sunday"), N_("Monday"), N_("Tuesday"),
	N_("Wednesday"), N_("Thursday"), N_("Friday"), N_("Saturday")};
const char *mon_abbr[] = {N_("Jan"), N_("Feb"), N_("Mar"), N_("Apr"),
	N_("abbrev^May"), N_("Jun"), N_("Jul"), N_("Aug"), N_("Sep"), N_("Oct"),
	N_("Nov"), N_("Dec")};
const char *mon_full[] = {N_("January"), N_("February"), N_("March"),
	N_("April"), N_("May"), N_("June"), N_("July"), N_("August"),
	N_("September"), N_("October"), N_("November"), N_("December")};

}

static std::string reformat(const std::string& format, const std::tm* time,
	bool locale_correct, bool ampm_supported)
{
	if (locale_correct && ampm_supported) {
		return format;
	}

	std::string new_format;

	for (std::string::const_iterator it = format.begin(); it != format.end();
		++it) {

		if (*it == '%') {
			++it;

			bool unrecognized = false;

			if (!locale_correct) {
				switch (*it) {
				case 'a': // abbreviated weekday name
					new_format += (time->tm_wday < 0 || time->tm_wday > 6) ?
						"?" : translation::sgettext(wday_abbr[time->tm_wday]);
					continue;
				case 'A': // full weekday name
					new_format += (time->tm_wday < 0 || time->tm_wday > 6) ?
						"?" : translation::sgettext(wday_full[time->tm_wday]);
					continue;
				case 'b': // abbreviated month name
				case 'h':
					new_format += (time->tm_mon < 0 || time->tm_mon > 11) ?
						"?" : translation::sgettext(mon_abbr[time->tm_mon]);
					continue;
				case 'B': // full month name
					new_format += (time->tm_mon < 0 || time->tm_mon > 11) ?
						"?" : translation::sgettext(mon_full[time->tm_mon]);
					continue;
				case 'c': // date and time
					new_format += reformat(_("%a %b %e %H:%M:%S %Y"), time,
						locale_correct, ampm_supported);
					continue;
				case '+': // date and time
					new_format += reformat(_("%a %d %b %Y %H:%M:%S %z"),
						time, locale_correct, ampm_supported);
					continue;
				default:
					unrecognized = true;
				}
			}

			if (!ampm_supported || !locale_correct) {
				switch (*it) {
				case 'p': // AM or PM designation
					new_format += (time->tm_hour < 12 ? _("AM") : _("PM"));
					continue;
				case 'P': // am or pm designation
					new_format += (time->tm_hour < 12 ? _("am") : _("pm"));
					continue;
				default:
					unrecognized = true;
				}
			}

			// Unrecognized format specifiers should be left
			// for std::strftime to deal with them.
			if (unrecognized) {
				new_format += "%";
			}
		}

		new_format += *it;
	}

	return new_format;
}

static bool locale_supports_ampm(const std::tm* time)
{
	const unsigned buffer_size = 16;
	char time_buffer[buffer_size] = {0};

	size_t ret = std::strftime(time_buffer, buffer_size, "%p", time);

	if (ret == 0) {
		return false;
	}

	return true;
}

namespace util {

size_t strftime(char* str, size_t count, const std::string& format,
	const std::tm* time)
{
	bool ampm_supported = locale_supports_ampm(time);
	const std::string f = reformat(format, time, time_locale_correct(),
		ampm_supported);

	return std::strftime(str, count, f.c_str(), time);
}

} // end namespace util
