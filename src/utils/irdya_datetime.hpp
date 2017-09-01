/*
	Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include <ostream>
#include <string>
#include <vector>

class irdya_date
{
public:
	MAKE_ENUM(EPOCH, (BEFORE_WESNOTH, "BW")(WESNOTH, "YW")(BEFORE_FALL, "BF")(AFTER_FALL, "AF"))

	irdya_date() = default;

	irdya_date(EPOCH epoch, unsigned year)
		: epoch(epoch)
		, year(year)
	{
	}

	static irdya_date read_date(const std::string& date);

	EPOCH get_epoch() const
	{
		return epoch;
	}

	unsigned int get_year() const
	{
		return year;
	}

	/* There is no year 0, so use that to represent an "invalid" date. */
	bool is_valid() const
	{
		return year != 0;
	}

	/* Outputs a locale-dependent string describing the year. */
	std::string to_string() const;

private:
	EPOCH epoch;

	// TODO: Decide how many months and days there are!
	unsigned int year = 0, month = 0, day = 0;
};

bool operator<(const irdya_date& a, const irdya_date& b);
bool operator<=(const irdya_date& a, const irdya_date& b);
bool operator>(const irdya_date& a, const irdya_date& b);
bool operator>=(const irdya_date& a, const irdya_date& b);
bool operator==(const irdya_date& a, const irdya_date& b);
bool operator!=(const irdya_date& a, const irdya_date& b);

std::ostream& operator<<(std::ostream& s, const irdya_date& d);
