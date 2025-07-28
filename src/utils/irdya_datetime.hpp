/*
	Copyright (C) 2017 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "utils/wesnoth_epoch.hpp"

#include <ostream>
#include <string>

namespace utils
{

/**
 * Calendar for handling and comparing dates using the common epoches of the storyline.
 */
class irdya_date
{
public:
	irdya_date() = default;

	irdya_date(wesnoth_epoch::type epoch, unsigned year)
		: epoch(epoch)
		, year(year)
	{
	}

	static irdya_date read_date(const std::string& date);

	wesnoth_epoch::type get_epoch() const
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
	wesnoth_epoch::type epoch;

	unsigned int year = 0;

	// TODO: Decide how many months and days there are!
	//unsigned int month = 0;
	//unsigned int day = 0;
};

bool operator<(const irdya_date& a, const irdya_date& b);
bool operator<=(const irdya_date& a, const irdya_date& b);
bool operator>(const irdya_date& a, const irdya_date& b);
bool operator>=(const irdya_date& a, const irdya_date& b);
bool operator==(const irdya_date& a, const irdya_date& b);
bool operator!=(const irdya_date& a, const irdya_date& b);

std::ostream& operator<<(std::ostream& s, const irdya_date& d);

}
