/*
	Copyright (C) 2011 - 2024
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
 * Implementation of key.hpp.
 */

#include "serialization/schema/key.hpp"

#include "config.hpp"

namespace schema_validation
{

wml_key::wml_key(const config& cfg)
	: name_(cfg["name"].str())
	, type_(cfg["type"].str())
	, default_()
	, mandatory_(false)
	, fuzzy_(name_.find_first_of("*?") != std::string::npos)
{
	if(cfg.has_attribute("mandatory")) {
		mandatory_ = cfg["mandatory"].to_bool();
	} else {
		if(cfg.has_attribute("default")) {
			default_ = cfg["default"].str();
		}
	}
}

void wml_key::print(std::ostream& os, int level) const
{
	std::string s;
	for(int j = 0; j < level; j++) {
		s.append(" ");
	}

	os << s << "[key]\n" << s << "    name=\"" << name_ << "\"\n" << s << "    type=\"" << type_ << "\"\n";

	if(is_mandatory()) {
		os << s << "    mandatory=\"true\"\n";
	} else {
		os << s << "    default=" << default_ << "\n";
	}

	// TODO: Other attributes

	os << s << "[/key]\n";
}

} // namespace schema_validation
