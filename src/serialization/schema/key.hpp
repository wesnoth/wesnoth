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
 * This file contains object "key", which is used to store
 * information about keys while annotation parsing.
 */

#pragma once

#include <iosfwd>
#include <string>

class config;

namespace schema_validation
{

/**
 * wml_key is used to save the information about one key.
 * Key has next info: name, type, default value or key is mandatory.
 */
class wml_key
{
public:
	wml_key()
		: name_("")
		, type_("")
		, default_("\"\"")
		, mandatory_(false)
		, fuzzy_(false)
	{
	}

	wml_key(const std::string& name, const std::string& type, const std::string& def = "\"\"")
		: name_(name)
		, type_(type)
		, default_(def)
		, mandatory_(def.empty())
		, fuzzy_(name.find_first_of("*?") != std::string::npos)
	{
	}

	wml_key(const config&);

	const std::string& get_name() const
	{
		return name_;
	}

	const std::string& get_type() const
	{
		return type_;
	}

	const std::string& get_default() const
	{
		return default_;
	}

	bool is_mandatory() const
	{
		return mandatory_;
	}

	bool is_fuzzy() const {
		return fuzzy_;
	}

	void set_name(const std::string& name)
	{
		name_ = name;
	}

	void set_type(const std::string& type)
	{
		type_ = type;
	}

	void set_default(const std::string& def)
	{
		default_ = def;
		if(def.empty()) {
			mandatory_ = true;
		}
	}

	void set_mandatory(bool mandatory)
	{
		mandatory_ = mandatory;
	}

	void set_fuzzy(bool f)
	{
		fuzzy_ = f;
	}

	/** is used to print key info
	 * the format is next
	 *  [key]
	 *      name="name"
	 *      type="type"
	 *      default="default"
	 *      mandatory="true/false"
	 *  [/key]
	 */
	void print(std::ostream& os, int level) const;

	/** Compares keys by name. Used in std::sort, i.e. */
	bool operator<(const wml_key& k) const
	{
		return (get_name() < k.get_name());
	}

private:
	/** Name of key. */
	std::string name_;

	/** Type of key. */
	std::string type_;

	/** Default value. */
	std::string default_;

	/** Shows, if key is a mandatory key. */
	bool mandatory_;

	/** Whether the key is a fuzzy match. */
	bool fuzzy_;
};
}
