/*
Copyright (C) 2003 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.

See the COPYING file for more details.
*/

#include "utils/irdya_datetime.hpp"
#include "gettext.hpp"
#include "formula/string_utils.hpp"
#include "tstring.hpp"
#include <exception>

irdya_date irdya_date::read_date(const std::string& date)
{
	irdya_date date_result;
	// Currently only supports a year and an epoch.
	size_t year_start = date.find_first_not_of(' ');
	if(year_start == std::string::npos) {
		//throw std::invalid_argument("Irdya date is missing year");
		date_result.year = 0;
		return date_result;
	}
	size_t year_end = date.find_first_of(' ', year_start);
	if(year_end == std::string::npos) {
		year_end = date.size();
	}
	date_result.year = std::stoi(date.substr(year_start, year_end - year_start));
	size_t epoch_start = date.find_first_not_of(' ', year_end);
	if(epoch_start == std::string::npos) {
		date_result.epoch = EPOCH::WESNOTH;
	} else {
		size_t epoch_end = date.find_first_of(' ', epoch_start);
		date_result.epoch = EPOCH::string_to_enum(date.substr(epoch_start, epoch_end - epoch_start), EPOCH::WESNOTH);
	}
	return date_result;
}

std::string irdya_date::to_string() const
{
	utils::string_map args {{"year", std::to_string(year)}};
	switch(epoch.v) {
		case EPOCH::BEFORE_WESNOTH:
			// TRANSLATORS: "Before Wesnoth" - format for years prior to the founding of Wesnoth
			return VGETTEXT("$year BW", args);
		case EPOCH::WESNOTH:
			// TRANSLATORS: "Year of Wesnoth" - format for years after the founding of Wesnoth
			return VGETTEXT("$year YW", args);
		case EPOCH::BEFORE_FALL:
			// TRANSLATORS: "Before the Fall" -  format for years prior to the fall of Wesnoth
			return VGETTEXT("$year BF", args);
		case EPOCH::AFTER_FALL:
			// TRANSLATORS: "After the Fall" - format for years after the fall of Wesonth
			return VGETTEXT("$year AF", args);
	}
	return "";
}

bool operator<(const irdya_date& a, const irdya_date& b)
{
	if(!b.is_valid()) {
		return a.is_valid();
	}
	if(!a.is_valid()) {
		return false;
	}
	if(a.get_epoch().v < b.get_epoch().v) {
		return true;
	}
	if(a.get_epoch().v > b.get_epoch().v) {
		return false;
	}
	using EPOCH = irdya_date::EPOCH;
	// The BW and BF epochs count backward, much like BCE
	if((a.get_epoch() == EPOCH::BEFORE_WESNOTH || a.get_epoch() == EPOCH::BEFORE_FALL) && a.get_year() > b.get_year()) {
		return true;
	}
	if(a.get_year() < b.get_year()) {
		return true;
	}
	return false;
}

bool operator>(const irdya_date& a, const irdya_date& b)
{
	return b < a;
}

bool operator<=(const irdya_date& a, const irdya_date& b)
{
	return !(a > b);
}

bool operator>=(const irdya_date& a, const irdya_date& b)
{
	return !(a < b);
}

bool operator==(const irdya_date& a, const irdya_date& b)
{
	return a.get_year() == b.get_year() && a.get_epoch() == b.get_epoch();
}

bool operator!=(const irdya_date& a, const irdya_date& b)
{
	return !(a == b);
}

std::ostream& operator<<(std::ostream& s, const irdya_date& d)
{
	s << d.to_string();
	return s;
}
