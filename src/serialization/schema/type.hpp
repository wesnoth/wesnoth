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
 * This file contains object "type", which is used to store
 * information about types while annotation parsing.
 */

#pragma once

#include <boost/regex.hpp>

class config;
class config_attribute_value;

namespace schema_validation
{

/**
 * Stores information about a schema type.
 * This is an abstract base class for several variants of schema type.
 */
class wml_type {
protected:
	std::string name_;
public:
	wml_type() = delete;
	explicit wml_type(const std::string& name) : name_(name) {}
	using ptr = std::shared_ptr<wml_type>;
	using map = std::map<std::string, ptr>;
	virtual bool matches(const config_attribute_value& value, const map& type_map) const = 0;
	static std::shared_ptr<wml_type> from_config(const config& cfg);
	virtual ~wml_type() {}
};

/**
 * Stores information about a schema type.
 * This type represents a simple pattern match.
 */
class wml_type_simple : public wml_type {
	boost::regex pattern_;
	bool allow_translatable_ = false;
public:
	wml_type_simple(const std::string& name, const std::string& pattern) : wml_type(name), pattern_(pattern) {}
	bool matches(const config_attribute_value& value, const map& type_map) const override;
	void allow_translatable() {allow_translatable_ =  true;}
};

/**
 * Stores information about a schema type.
 * This type represents a name alias for another type.
 */
class wml_type_alias : public wml_type {
	mutable std::shared_ptr<wml_type> cached_;
	std::string link_;
public:
	wml_type_alias(const std::string& name, const std::string& link) : wml_type(name), link_(link) {}
	bool matches(const config_attribute_value& value, const map& type_map) const override;
};

/**
 * Stores information about a schema type.
 * This is an abstract base class for composite types.
 */
class wml_type_composite : public wml_type {
protected:
	std::vector<std::shared_ptr<wml_type>> subtypes_;
public:
	explicit wml_type_composite(const std::string& name) : wml_type(name) {}
	void add_type(std::shared_ptr<wml_type> type)
	{
		subtypes_.push_back(type);
	}
};

/**
 * Stores information about a schema type.
 * Represents a union type, which matches if any of its subtypes match.
 */
class wml_type_union : public wml_type_composite {
public:
	explicit wml_type_union(const std::string& name) : wml_type_composite(name) {}
	bool matches(const config_attribute_value& value, const map& type_map) const override;
};

/**
 * Stores information about a schema type.
 * Represents an intersection type, which matches if all of its subtypes match.
 */
class wml_type_intersection : public wml_type_composite {
public:
	explicit wml_type_intersection(const std::string& name) : wml_type_composite(name) {}
	bool matches(const config_attribute_value& value, const map& type_map) const override;
};

/**
 * Stores information about a schema type.
 * Represents a list type, where each list element is itself a union.
 */
class wml_type_list : public wml_type_union {
	boost::regex split_;
	int min_ = 0, max_ = -1;
public:
	wml_type_list(const std::string& name, const std::string& pattern, int min, int max)
		: wml_type_union(name)
		, split_(pattern)
		, min_(min)
		, max_(max)
	{}
	bool matches(const config_attribute_value& value, const map& type_map) const override;
};

class wml_type_tstring : public wml_type {
public:
	wml_type_tstring() : wml_type("t_string") {}
	bool matches(const config_attribute_value& value, const map& type_map) const override;
};

}
