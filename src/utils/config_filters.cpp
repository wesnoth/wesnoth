/*
	Copyright (C) 2003 - 2023
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include <algorithm>
#include <vector>

#include "utils/config_filters.hpp"

#include "serialization/string_utils.hpp" // for utils::split
#include "utils/math.hpp"                 // for in_ranges

bool utils::config_filters::bool_matches_if_present(const config& filter, const config& cfg, const std::string& attribute, bool def)
{
	if(filter[attribute].empty()) {
		return true;
	}

	return filter[attribute].to_bool() == cfg[attribute].to_bool(def);
}

bool utils::config_filters::string_matches_if_present(
	const config& filter, const config& cfg, const std::string& attribute, const std::string& def)
{
	if(filter[attribute].empty()) {
		return true;
	}

	const std::vector<std::string> filter_attribute = utils::split(filter[attribute]);
	return (
		std::find(filter_attribute.begin(), filter_attribute.end(), cfg[attribute].str(def)) != filter_attribute.end());
}

bool utils::config_filters::unsigned_matches_if_present(const config& filter, const config& cfg, const std::string& attribute)
{
	if(filter[attribute].empty()) {
		return true;
	}

	return in_ranges<int>(cfg[attribute].to_int(0), utils::parse_ranges_unsigned(filter[attribute].str()));
}

bool utils::config_filters::int_matches_if_present(const config& filter, const config& cfg, const std::string& attribute)
{
	if(filter[attribute].empty()) {
		return true;
	}

	return in_ranges<int>(cfg[attribute].to_int(0), utils::parse_ranges_int(filter[attribute].str()));
}

bool utils::config_filters::int_matches_if_present_or_negative(
	const config& filter, const config& cfg, const std::string& attribute, const std::string& opposite)
{
	if(int_matches_if_present(filter, cfg, attribute)) {
		return true;
	}

	// don't check for !cfg[opposite].empty(), as this assumes a default value of zero (similarly to
	// int_matches_if_present)
	if(cfg[attribute].empty()) {
		return in_ranges<int>(-cfg[opposite].to_int(0), utils::parse_ranges_int(filter[attribute].str()));
	}

	return false;
}

bool utils::config_filters::double_matches_if_present(const config& filter, const config& cfg, const std::string& attribute)
{
	if(filter[attribute].empty()) {
		return true;
	}

	return in_ranges<double>(cfg[attribute].to_double(1), utils::parse_ranges_real(filter[attribute].str()));
}
