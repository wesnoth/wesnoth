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

	if(def){
		return in_ranges<int>(cfg[attribute].to_int(*def), utils::parse_ranges_int(filter[attribute].str()));
	}
	return in_ranges<int>(cfg[attribute].to_int(), utils::parse_ranges_int(filter[attribute].str()));
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
		if(def){
			return in_ranges<int>(-cfg[opposite].to_int(*def), utils::parse_ranges_int(filter[attribute].str()));
		} else if(cfg.has_attribute(opposite)){
			return in_ranges<int>(-cfg[opposite].to_int(), utils::parse_ranges_int(filter[attribute].str()));
		}
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

	if(def){
		return in_ranges<double>(cfg[attribute].to_double(*def), utils::parse_ranges_real(filter[attribute].str()));
	}
	return in_ranges<double>(cfg[attribute].to_double(), utils::parse_ranges_real(filter[attribute].str()));
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

bool utils::config_filters::matches_ability_filter(const config & cfg, const std::string& tag_name, const config & filter)
{

	if(!filter["affect_adjacent"].empty()){
		bool adjacent = cfg.has_child("affect_adjacent");
		if(filter["affect_adjacent"].to_bool() != adjacent){
			return false;
		}
	}

	if(!bool_matches_if_present(filter, cfg, "affect_self", true))
		return false;

	if(!bool_or_empty(filter, cfg, "affect_allies"))
		return false;

	if(!bool_matches_if_present(filter, cfg, "affect_enemies", false))
		return false;

	if(!bool_matches_if_present(filter, cfg, "cumulative", false))
		return false;

	const std::vector<std::string> filter_type = utils::split(filter["tag_name"]);
	if ( !filter_type.empty() && std::find(filter_type.begin(), filter_type.end(), tag_name) == filter_type.end() )
		return false;

	if(!string_matches_if_present(filter, cfg, "overwrite_specials", "none"))
		return false;

	if(!string_matches_if_present(filter, cfg, "id", ""))
		return false;

	if(!string_matches_if_present(filter, cfg, "apply_to", "self"))
		return false;

	if(!string_matches_if_present(filter, cfg, "active_on", "both"))
		return false;

	//for damage_type only
	if(!string_matches_if_present(filter, cfg, "replacement_type", ""))
		return false;

	if(!string_matches_if_present(filter, cfg, "alternative_type", ""))
		return false;

	//for plague only
	if(!string_matches_if_present(filter, cfg, "type", ""))
		return false;

	//the value attribute can, depending on the type of ability checked,
	//have a different default value or even not have one at all, hence the need to check the different cases,
	//for example if the ability is "drains" then if the value sought is 50, the filter will match if value=50 or is not explicitly encoded.
	//Conversely, if the type is "damage" or "dummy" but no 'value' attribute is present
	//then no match will be possible regardless of whether the value is sought,
	//or because the default value of [damage] is the base value of the attack which is not fixed,
	//either because the 'dummy' abilities are not hardcoded and have no default value
	if(!filter["value"].empty()){
		if(tag_name == "drains"){
			if(!int_matches_if_present(filter, cfg, "value", 50)){
				return false;
			}
		} else if(tag_name == "berserk"){
			if(!int_matches_if_present(filter, cfg, "value", 1)){
				return false;
			}
		} else if(tag_name == "heal_on_hit" || tag_name == "heals" || tag_name == "regenerate" || tag_name == "leadership"){
			if(!int_matches_if_present(filter, cfg, "value" , 0)){
				return false;
			}
		} else {
			if(!int_matches_if_present(filter, cfg, "value")){
				return false;
			}
		}
	}

	if(!int_matches_if_present_or_negative(filter, cfg, "add", "sub"))
		return false;

	if(!int_matches_if_present_or_negative(filter, cfg, "sub", "add"))
		return false;

	if(!double_matches_if_present(filter, cfg, "multiply"))
		return false;

	if(!double_matches_if_present(filter, cfg, "divide"))
		return false;


	//the wml_filter is used in cases where the attribute we are looking for is not
	//previously listed or to check the contents of the sub_tags ([filter_adjacent],[filter_self],[filter_opponent] etc.
	//If the checked set does not exactly match the content of the capability, the function returns a false response.
	auto fwml = filter.optional_child("filter_wml");
	if (fwml){
		if(!cfg.matches(*fwml)){
			return false;
		}
	}

	// Passed all tests.
	return true;
}

bool utils::config_filters::common_matches_filter(const config & cfg, const std::string& tag_name, const config & filter)
{
	// Handle the basic filter.
	bool matches = matches_ability_filter(cfg, tag_name, filter);

	// Handle [and], [or], and [not] with in-order precedence
	for (const config::any_child condition : filter.all_children_range() )
	{
		// Handle [and]
		if ( condition.key == "and" )
			matches = matches && common_matches_filter(cfg, tag_name, condition.cfg);

		// Handle [or]
		else if ( condition.key == "or" )
			matches = matches || common_matches_filter(cfg, tag_name, condition.cfg);

		// Handle [not]
		else if ( condition.key == "not" )
			matches = matches && !common_matches_filter(cfg, tag_name, condition.cfg);
	}

	return matches;
}
