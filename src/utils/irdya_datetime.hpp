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

#pragma once

#include "utils/make_enum.hpp"
#include <vector>
#include <string>

class irdya_date {
public:
	MAKE_ENUM(EPOCH,
		(BEFORE_WESNOTH, "BW")
		(WESNOTH, "YW")
		(BEFORE_FALL, "BF")
		(AFTER_FALL, "AF")
		);
private:
	EPOCH epoch;
	unsigned int year = 0, month, day;
	// TODO: Decide how many months and days there are!
public:
	static irdya_date read_date(const std::string& date);
	irdya_date() = default;
	irdya_date(EPOCH epoch, unsigned year) : epoch(epoch), year(year) {}

	EPOCH get_epoch() const {return epoch;}
	unsigned int get_year() const {return year;}
	bool is_valid() const {
		// There is no year 0, so use that to represent an "invalid" date
		return year != 0;
	}
	// Outputs a locale-dependent string describing the year
	std::string to_string() const;
};

bool operator<(const irdya_date& a, const irdya_date& b);
bool operator<=(const irdya_date& a, const irdya_date& b);
bool operator>(const irdya_date& a, const irdya_date& b);
bool operator>=(const irdya_date& a, const irdya_date& b);
bool operator==(const irdya_date& a, const irdya_date& b);
bool operator!=(const irdya_date& a, const irdya_date& b);
