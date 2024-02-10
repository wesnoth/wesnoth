/*
	Copyright (C) 2003 - 2024
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
	if(!filter.has_attribute(attribute)) {
		return true;
	}

	return filter[attribute].to_bool() == cfg[attribute].to_bool(def);
}

bool utils::config_filters::string_matches_if_present(
	const config& filter, const config& cfg, const std::string& attribute, const std::string& def)
{
	if(!filter.has_attribute(attribute)) {
		return true;
	}

	const std::vector<std::string> filter_attribute = utils::split(filter[attribute]);
	return (
		std::find(filter_attribute.begin(), filter_attribute.end(), cfg[attribute].str(def)) != filter_attribute.end());
}

bool utils::config_filters::unsigned_matches_if_present(const config& filter, const config& cfg, const std::string& attribute)
{
	if(!filter.has_attribute(attribute)) {
		return true;
	}
	if(!cfg.has_attribute(attribute)) {
		return false;
	}

	return in_ranges<int>(cfg[attribute].to_int(0), utils::parse_ranges_unsigned(filter[attribute].str()));
}

bool utils::config_filters::int_matches_if_present(const config& filter, const config& cfg, const std::string& attribute, std::optional<int> def)
{
	if(!filter.has_attribute(attribute)) {
		return true;
	}
	if(!cfg.has_attribute(attribute) && !def) {
		return false;
	}

	int value_def = def ? (*def) : 0;
	return in_ranges<int>(cfg[attribute].to_int(value_def), utils::parse_ranges_int(filter[attribute].str()));
}

bool utils::config_filters::int_matches_if_present_or_negative(
	const config& filter, const config& cfg, const std::string& attribute, const std::string& opposite, std::optional<int> def)
{
	if(int_matches_if_present(filter, cfg, attribute, def)) {
		return true;
	}

	// Check if cfg[opposite].empty() and have optional def.
	// If def don't exist return false.
	if(!cfg.has_attribute(attribute)) {
		if(!cfg.has_attribute(opposite) && !def) {
			return false;
		}
		int value_def = def ? (*def) : 0;
		return in_ranges<int>(-cfg[opposite].to_int(value_def), utils::parse_ranges_int(filter[attribute].str()));
	}

	return false;
}

bool utils::config_filters::double_matches_if_present(const config& filter, const config& cfg, const std::string& attribute, std::optional<double> def)
{
	if(!filter.has_attribute(attribute)) {
		return true;
	}
	if(!cfg.has_attribute(attribute) && !def) {
		return false;
	}

	double value_def = def ? (*def) : 1;
	return in_ranges<double>(cfg[attribute].to_double(value_def), utils::parse_ranges_real(filter[attribute].str()));
}

bool utils::config_filters::bool_or_empty(const config& filter, const config& cfg, const std::string& attribute)
{
	if(!filter.has_attribute(attribute)) {
		return true;
	}

	std::set<std::string> filter_attribute = utils::split_set(filter[attribute].str());
	bool is_matches = false;
	if(cfg.has_attribute(attribute)){
		if(filter_attribute.count("yes") != 0 || filter_attribute.count("true") != 0){
			is_matches = cfg[attribute].to_bool();
		}
		if(!is_matches && (filter_attribute.count("no") != 0 || filter_attribute.count("false") != 0)){
			is_matches = !cfg[attribute].to_bool();
		}
	} else {
		if(filter_attribute.count("none") != 0){
			is_matches = true;
		}
	}
	return is_matches;
}
