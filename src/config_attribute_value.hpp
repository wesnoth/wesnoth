/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
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
 * Definitions for the interface to Wesnoth Markup Language (WML).
 *
 * This module defines the interface to Wesnoth Markup Language (WML).  WML is
 * a simple hierarchical text-based file format.  The format is defined in
 * Wiki, under BuildingScenariosWML
 *
 * All configuration files are stored in this format, and data is sent across
 * the network in this format.  It is thus used extensively throughout the
 * game.
 */

#pragma once

#include "tstring.hpp"
#include "utils/variant.hpp"

#include <chrono>
#include <climits>
#include <ctime>
#include <iosfwd>
#include <string>
#include <vector>
#include <type_traits>

/**
 * Variant for storing WML attributes.
 * The most efficient type is used when assigning a value. For instance,
 * strings "yes", "no", "true", "false" will be detected and stored as boolean.
 * @note The blank variant is only used when querying missing attributes.
 *       It is not stored in config objects.
 */
class config_attribute_value
{
	/**
	 * A wrapper for bool to get the correct streaming ("true"/"false").
	 * Most visitors can simply treat this as bool.
	 */
public:
	class true_false
	{
		bool value_;
	public:
		explicit true_false(bool value = false) : value_(value) {}
		operator bool() const { return value_; }

		const std::string & str() const
		{
			return value_ ? config_attribute_value::s_true : config_attribute_value::s_false;
		}
	};
	friend std::ostream& operator<<(std::ostream &os, const true_false &v) { return os << v.str(); }

	/**
	 * A wrapper for bool to get the correct streaming ("yes"/"no").
	 * Most visitors can simply treat this as bool.
	 */
	class yes_no
	{
		bool value_;
	public:
		explicit yes_no(bool value = false) : value_(value) {}
		operator bool() const { return value_; }

		const std::string & str() const
		{
			return value_ ? config_attribute_value::s_yes : config_attribute_value::s_no;
		}
	};
	friend std::ostream& operator<<(std::ostream &os, const yes_no &v) { return os << v.str(); }
private:
	/** Visitor for checking equality. */
	class equality_visitor;
	/** Visitor for converting a variant to a string. */
	class string_visitor;

	// Data will be stored in a variant, allowing for the possibility of
	// boolean, numeric, and translatable data in addition to basic string
	// data. For most purposes, int is the preferred type for numeric data
	// as it is fast (often natural word size). While it is desirable to
	// use few types (to keep the overhead low), we do have use cases for
	// fractions (double) and huge numbers (up to the larger of LLONG_MAX
	// and SIZE_MAX).
	typedef utils::variant<utils::monostate,
		true_false, yes_no,
		int, unsigned long long, double,
		std::string, t_string
	> value_type;
	/**
	 * The stored value will always use the first type from the variant
	 * definition that can represent it and that can be streamed to the
	 * correct string representation (if applicable).
	 * This is enforced upon assignment.
	 */
	value_type value_;

public:
	// Numeric assignments:
	config_attribute_value& operator=(bool v);
	config_attribute_value& operator=(int v);
	config_attribute_value& operator=(long v) { return operator=(static_cast<long long>(v)); }
	config_attribute_value& operator=(long long v);
	config_attribute_value& operator=(unsigned v) { return operator=(static_cast<unsigned long long>(v)); }
	config_attribute_value& operator=(unsigned long v) { return operator=(static_cast<unsigned long long>(v)); }
	config_attribute_value& operator=(unsigned long long v);
	config_attribute_value& operator=(double v);

	// String assignments:
	config_attribute_value& operator=(const char *v) { return operator=(std::string(v)); }
	config_attribute_value& operator=(std::string&& v);
	config_attribute_value& operator=(const std::string &v);
	config_attribute_value& operator=(const std::string_view &v);
	config_attribute_value& operator=(const t_string &v);

	//TODO: should this be a normal constructor?
	template<typename T>
	static config_attribute_value create(const T val)
	{
		config_attribute_value res;
		res = val;
		return res;
	}

	template<typename... Args>
	config_attribute_value& operator=(const std::chrono::duration<Args...>& v)
	{
		return this->operator=(v.count());
	}

	/** Calls @ref operator=(const std::string&) if @a v is not empty. */
	void write_if_not_empty(const std::string& v);
	void write_if_not_empty(const t_string& v);

	// Extracting as a specific type:
	bool to_bool(bool def = false) const;
	int to_int(int def = 0) const;
	long long to_long_long(long long def = 0) const;
	unsigned to_unsigned(unsigned def = 0) const;
	std::size_t to_size_t(std::size_t def = 0) const;
	std::time_t to_time_t(std::time_t def = 0) const;
	double to_double(double def = 0.) const;
	std::string str(const std::string& fallback = "") const;
	t_string t_str() const;

	bool to(const bool def) const { return to_bool(def); }
	int to(int def) const { return to_int(def); }
	unsigned to(unsigned def) const { return to_unsigned(def); }
	double to(double def) const { return to_double(def); }
	std::string to(const std::string& def) const { return str(def); }

	// Implicit conversions:
	operator std::string() const { return str(); }
	operator t_string() const { return t_str(); }

	/** Tests for an attribute that was never set. */
	bool blank() const;
	/** Tests for an attribute that either was never set or was set to "". */
	bool empty() const;

	// Comparisons:
	bool operator==(const config_attribute_value &other) const;
	bool operator!=(const config_attribute_value &other) const
	{
		return !operator==(other);
	}

	bool operator==(bool comp) const
	{
		const bool has_bool =
			utils::holds_alternative<yes_no>(value_) ||
			utils::holds_alternative<true_false>(value_);
		return has_bool && to_bool() == comp;
	}

	template<typename T>
	bool operator==(const T& comp) const
	{
		if constexpr(std::is_convertible_v<T, std::string>) {
			config_attribute_value v;
			v = comp;
			return *this == v;
		} else {
			return utils::holds_alternative<T>(value_) && this->to(T{}) == comp;
		}
	}

	template<typename T>
	bool friend operator!=(const config_attribute_value& val, const T& str)
	{
		return !val.operator==(str);
	}

	template<typename T>
	bool friend operator!=(const T &str, const config_attribute_value& val)
	{
		return !val.operator==(str);
	}

	// Streaming:
	friend std::ostream& operator<<(std::ostream& os, const config_attribute_value& v);

	/**
	 * Visitor support:
	 * Applies a visitor to the underlying variant.
	 * (See the documentation for Boost.Variant.)
	 */
	template <typename V>
	auto apply_visitor(const V & visitor) const
	{
		return utils::visit(visitor, value_);
	}

private:
	// Special strings.
	static const std::string s_yes, s_no;
	static const std::string s_true, s_false;
};

#ifndef USING_BOOST_VARIANT
/** Specialize operator<< for monostate. Boost already does this, but the STL does not. */
inline std::ostream& operator<<(std::ostream& os, const std::monostate&) { return os; }
#endif

namespace utils
{
	std::vector<std::string> split(const config_attribute_value& val);
}
