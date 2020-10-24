/*
   Copyright (C) 2011 - 2018 by Sytyi Nick <nsytyi@gmail.com>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Implementation of type.hpp.
 */

#include "serialization/schema/type.hpp"
#include "boost/optional.hpp"

#include "config.hpp"

namespace schema_validation
{

/*WIKI
 * @begin{parent}{name="wml_schema/tag/"}
 * @begin{tag}{name="key"}{min=0}{max=-1}
 * @begin{table}{config}
 *     name & string & &              The name of key. $
 *     type & string & &              The type of key value. $
 *     default & string & &        The default value of the key. $
 *     mandatory & string & &   Shows if key is mandatory $
 * @end{table}
 * @end{tag}{name="key"}
 * @end{parent}{name="wml_schema/tag/"}
 */
std::shared_ptr<wml_type> wml_type::from_config(const config& cfg)
{
	boost::optional<config::const_child_itors> composite_range;
	std::shared_ptr<wml_type> type;
	if(cfg.has_child("union")) {
		type = std::make_shared<wml_type_union>(cfg["name"]);
		composite_range.emplace(cfg.child("union").child_range("type"));
	} else if(cfg.has_child("intersection")) {
		type = std::make_shared<wml_type_intersection>(cfg["name"]);
		composite_range.emplace(cfg.child("intersection").child_range("type"));
	} else if(cfg.has_child("list")) {
		const config& list_cfg = cfg.child("list");
		int list_min = list_cfg["min"].to_int();
		int list_max = list_cfg["max"].str() == "infinite" ? -1 : list_cfg["max"].to_int(-1);
		if(list_max < 0) list_max = INT_MAX;
		type = std::make_shared<wml_type_list>(cfg["name"], list_cfg["split"].str("\\s*,\\s*"), list_min, list_max);
		composite_range.emplace(list_cfg.child_range("element"));
	} else if(cfg.has_attribute("value")) {
		type = std::make_shared<wml_type_simple>(cfg["name"], cfg["value"]);
	} else if(cfg.has_attribute("link")) {
		type = std::make_shared<wml_type_alias>(cfg["name"], cfg["link"]);
	}
	if(composite_range) {
		auto composite_type = std::dynamic_pointer_cast<wml_type_composite>(type);
		for(const config& elem : *composite_range) {
			composite_type->add_type(wml_type::from_config(elem));
		}
	}
	return type;
}

bool wml_type_simple::matches(const std::string& value, const map&) const
{
	std::smatch sub;
	return std::regex_match(value, sub, pattern_);
}

bool wml_type_alias::matches(const std::string& value, const map& type_map) const
{
	if(!cached_) {
		auto it = type_map.find(link_);
		if(it == type_map.end()) {
			// TODO: Error message about the invalid type?
			return false;
		}
		cached_ = it->second;
	}
	return cached_->matches(value, type_map);
}

bool wml_type_union::matches(const std::string& value, const map& type_map) const
{
	for(const auto& type : subtypes_) {
		if(type->matches(value, type_map)) {
			return true;
		}
	}
	return false;
}

bool wml_type_intersection::matches(const std::string& value, const map& type_map) const
{
	for(const auto& type : subtypes_) {
		if(!type->matches(value, type_map)) {
			return false;
		}
	}
	return true;
}

bool wml_type_list::matches(const std::string& value, const map& type_map) const
{
	std::sregex_token_iterator it(value.begin(), value.end(), split_, -1), end;
	int n = 0;
	bool result = std::all_of(it, end, [this, &type_map, &n](const boost::ssub_match& match){
		// Not sure if this is necessary?
		if(!match.matched) return true;
		n++;
		return this->wml_type_union::matches(std::string(match.first, match.second), type_map);
	});
	return result && n >= min_ && n <= max_;
}

} // namespace schema_validation
