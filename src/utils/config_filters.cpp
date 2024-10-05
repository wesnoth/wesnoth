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
#include <set>
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

bool utils::config_filters::set_includes_if_present(const config& filter, const config& cfg, const std::string& attribute)
{
	if(!filter.has_attribute(attribute)) {
		return true;
	}
	//This function compares two sets in such a way that if
	//no member of the ability set matches the filter element,
	//it returns a false value but if the attribute is not present in ability
	//then there is no point in wasting resources on building the set from the filter and the value is returned immediately.
	if(!cfg.has_attribute(attribute)) {
		return false;
	}

	const std::set<std::string> filter_attribute = utils::split_set(filter[attribute].str());
	const std::set<std::string> cfg_attribute = utils::split_set(cfg[attribute].str());
	for(const std::string& fil_at : filter_attribute) {
		if (cfg_attribute.count(fil_at) == 0){
			return false;
		}
	}
	return true;
}

bool utils::config_filters::int_matches_if_present(const config& filter, const config& cfg, const std::string& attribute, utils::optional<int> def)
{
	if(!filter.has_attribute(attribute)) {
		return true;
	}
	//This function can be called in cases where no default value is supposed to exist,
	//if this is the case and the checked attribute does not exist then no value can match.
	if(!cfg.has_attribute(attribute) && !def) {
		return false;
	}

	//if filter attribute is "default" check if cfg attribute equals to def.
	if(filter[attribute] == "default" && def){
		return (cfg[attribute].to_int(*def) == *def);
	}
	int value_def = def ? (*def) : 0;
	return in_ranges<int>(cfg[attribute].to_int(value_def), utils::parse_ranges_int(filter[attribute].str()));
}

bool utils::config_filters::int_matches_if_present_or_negative(
	const config& filter, const config& cfg, const std::string& attribute, const std::string& opposite, utils::optional<int> def)
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
		//if filter attribute is "default" check if cfg attribute equals to def.
		if(filter[attribute] == "default" && def){
			return (cfg[attribute].to_int(*def) == *def);
		}
		int value_def = def ? (*def) : 0;
		return in_ranges<int>(-cfg[opposite].to_int(value_def), utils::parse_ranges_int(filter[attribute].str()));
	}

	return false;
}

bool utils::config_filters::double_matches_if_present(const config& filter, const config& cfg, const std::string& attribute, utils::optional<double> def)
{
	if(!filter.has_attribute(attribute)) {
		return true;
	}
	//there is no attribute returning a decimal value using which has a default value but the variable exists in case this changes,
	//otherwise in case the attribute is missing from the ability no value can match.
	if(!cfg.has_attribute(attribute) && !def) {
		return false;
	}

	//if filter attribute is "default" check if cfg attribute equals to def.
	if(filter[attribute] == "default" && def){
		return (cfg[attribute].to_int(*def) == *def);
	}
	double value_def = def ? (*def) : 1;
	return in_ranges<double>(cfg[attribute].to_double(value_def), utils::parse_ranges_real(filter[attribute].str()));
}

bool utils::config_filters::bool_or_empty(const config& filter, const config& cfg, const std::string& attribute)
{
	//Here, no numeric value by default since it returns a Boolean value,
	//except that this function is called in cases where a third value exists in addition to true/false.
	//If the attribute is not present in the ability this induces a different behavior than if it takes a true or false value
	//and this presence must be verified before checking its value.
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
