/*
	Copyright (C) 2011 - 2025
	by Sytyi Nick <nsytyi@gmail.com>
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

#include "config.hpp"
#include "utils/optional_fwd.hpp"
#include "utils/variant.hpp"

struct is_translatable
{
	bool empty;
	is_translatable(bool b) : empty(b) {}
	bool operator()(const std::string& str) const
	{
		return str.empty() ? empty : false;
	}
	bool operator()(const t_string&) const
	{
		return true;
	}
	bool operator()(const utils::monostate&) const
	{
		return true;
	}
	template<typename T>
	bool operator()(const T&) const
	{
		return false;
	}
};

namespace schema_validation
{

std::shared_ptr<wml_type> wml_type::from_config(const config& cfg)
{
	utils::optional<config::const_child_itors> composite_range;
	std::shared_ptr<wml_type> type;
	if(cfg.has_child("union")) {
		type = std::make_shared<wml_type_union>(cfg["name"]);
		composite_range.emplace(cfg.mandatory_child("union").child_range("type"));
	} else if(cfg.has_child("intersection")) {
		type = std::make_shared<wml_type_intersection>(cfg["name"]);
		composite_range.emplace(cfg.mandatory_child("intersection").child_range("type"));
	} else if(cfg.has_child("list")) {
		const config& list_cfg = cfg.mandatory_child("list");
		int list_min = list_cfg["min"].to_int();
		int list_max = list_cfg["max"].str() == "infinite" ? -1 : list_cfg["max"].to_int(-1);
		if(list_max < 0) list_max = std::numeric_limits<int>::max();
		type = std::make_shared<wml_type_list>(cfg["name"], list_cfg["split"].str("\\s*,\\s*"), list_min, list_max);
		composite_range.emplace(list_cfg.child_range("element"));
	} else if(cfg.has_attribute("value")) {
		auto t = std::make_shared<wml_type_simple>(cfg["name"], cfg["value"]);
		if(cfg["allow_translatable"].to_bool()) t->allow_translatable();
		type = t;
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

bool wml_type_simple::matches(const config_attribute_value& value, const map&) const
{
	if(!allow_translatable_ && value.apply_visitor(is_translatable(false))) return false;
	boost::smatch sub;
	return boost::regex_match(value.str(), sub, pattern_);
}

bool wml_type_alias::matches(const config_attribute_value& value, const map& type_map) const
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

bool wml_type_union::matches(const config_attribute_value& value, const map& type_map) const
{
	for(const auto& type : subtypes_) {
		if(type->matches(value, type_map)) {
			return true;
		}
	}
	return false;
}

bool wml_type_intersection::matches(const config_attribute_value& value, const map& type_map) const
{
	for(const auto& type : subtypes_) {
		if(!type->matches(value, type_map)) {
			return false;
		}
	}
	return true;
}

bool wml_type_list::matches(const config_attribute_value& value_attr, const map& type_map) const
{
	auto value = value_attr.str();
	boost::sregex_token_iterator it(value.begin(), value.end(), split_, -1), end;
	int n = 0;
	bool result = std::all_of(it, end, [this, &type_map, &n](const boost::ssub_match& match){
		// Not sure if this is necessary?
		if(!match.matched) return true;
		n++;
		config_attribute_value elem;
		elem = std::string(match.first, match.second);
		return this->wml_type_union::matches(elem, type_map);
	});
	return result && n >= min_ && n <= max_;
}

bool wml_type_tstring::matches(const config_attribute_value& value, const map&) const
{
	return value.apply_visitor(is_translatable(true));
}

} // namespace schema_validation
