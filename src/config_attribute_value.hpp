/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "global.hpp"
#include "utils/type_trait_aliases.hpp"

#include <climits>
#include <ctime>
#include <iosfwd>
#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <type_traits>
#include <memory>

#include <boost/exception/exception.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>
#include <boost/range/iterator_range.hpp>

#include "tstring.hpp"

#ifdef HAVE_CXX14
#	ifdef __clang__ // Check this first, because clang also defines __GNUC__
#		ifdef __apple_build_version__ // Apple clang
#			if (__clang_major__ == 5 && __clang_minor__ >= 1) || __clang_major__ > 5 // Apple clang 5.1+
#				define USE_HETEROGENOUS_LOOKUPS
#			endif
#		else // Non-Apple clang
#			if (__clang_major__ == 3 && __clang_minor__ >= 4) || __clang_major__ > 3 // clang 3.4+
#				define USE_HETEROGENOUS_LOOKUPS
#			endif
#		endif
#	elif defined(__GNUC__) && __GNUC__ >= 5 // GCC 5.0+
#		define USE_HETEROGENOUS_LOOKUPS
#	endif
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1900 // MSVC 2015
#	define USE_HETEROGENOUS_LOOKUPS
#endif

#ifdef USE_HETEROGENOUS_LOOKUPS
#if BOOST_VERSION > 106100
#include <boost/utility/string_view.hpp>
using config_key_type = boost::string_view;
#else
#include <boost/utility/string_ref.hpp>
using config_key_type = boost::string_ref;
#endif
#else
using config_key_type = const std::string &;
#endif

class enum_tag;

/**
 * Variant for storing WML attributes.
 * The most efficient type is used when assigning a value. For instance,
 * strings "yes", "no", "true", "false" will be detected and stored as boolean.
 * @note The blank variant is only used when querying missing attributes.
 *       It is not stored in config objects.
 */
class config_attribute_value
{
	/// A wrapper for bool to get the correct streaming ("true"/"false").
	/// Most visitors can simply treat this as bool.
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

	/// A wrapper for bool to get the correct streaming ("yes"/"no").
	/// Most visitors can simply treat this as bool.
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
	/// Visitor for checking equality.
	class equality_visitor;
	/// Visitor for converting a variant to a string.
	class string_visitor;

	// Data will be stored in a variant, allowing for the possibility of
	// boolean, numeric, and translatable data in addition to basic string
	// data. For most purposes, int is the preferred type for numeric data
	// as it is fast (often natural word size). While it is desirable to
	// use few types (to keep the overhead low), we do have use cases for
	// fractions (double) and huge numbers (up to the larger of LLONG_MAX
	// and SIZE_MAX).
	typedef boost::variant<boost::blank,
		true_false, yes_no,
		int, unsigned long long, double,
		std::string, t_string
	> value_type;
	/// The stored value will always use the first type from the variant
	/// definition that can represent it and that can be streamed to the
	/// correct string representation (if applicable).
	/// This is enforced upon assignment.
	value_type value_;

public:
	/// Default implementation, but defined out-of-line for efficiency reasons.
	config_attribute_value();
	/// Default implementation, but defined out-of-line for efficiency reasons.
	~config_attribute_value();
	/// Default implementation, but defined out-of-line for efficiency reasons.
	config_attribute_value(const config_attribute_value &);
	/// Default implementation, but defined out-of-line for efficiency reasons.
	config_attribute_value &operator=(const config_attribute_value &);

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
	config_attribute_value& operator=(const std::string &v);
	config_attribute_value& operator=(const t_string &v);
	template<typename T>
	utils::enable_if_t<std::is_base_of<enum_tag, T>::value, config_attribute_value&> operator=(const T &v)
	{
		return operator=(T::enum_to_string(v));
	}

	/** Calls @ref operator=(const std::string&) if @a v is not empty. */
	void write_if_not_empty(const std::string& v);

	// Extracting as a specific type:
	bool to_bool(bool def = false) const;
	int to_int(int def = 0) const;
	long long to_long_long(long long def = 0) const;
	unsigned to_unsigned(unsigned def = 0) const;
	size_t to_size_t(size_t def = 0) const;
	time_t to_time_t(time_t def = 0) const;
	double to_double(double def = 0.) const;
	std::string str(const std::string& fallback = "") const;
	t_string t_str() const;
	/**
		@tparam T a type created with MAKE_ENUM macro
		NOTE: since T::VALUE constants is not of type T but of the underlying enum type you must specify the template parameter explicitly
		TODO: Fix this in c++11 using constexpr types.
	*/
	template<typename T>
	utils::enable_if_t<std::is_base_of<enum_tag, T>::value, T> to_enum(const T &v) const
	{
		return T::string_to_enum(this->str(), v);
	}

	// Implicit conversions:
	operator int() const { return to_int(); }
	operator std::string() const { return str(); }
	operator t_string() const { return t_str(); }

	/// Tests for an attribute that was never set.
	bool blank() const;
	/// Tests for an attribute that either was never set or was set to "".
	bool empty() const;


	// Comparisons:
	bool operator==(const config_attribute_value &other) const;
	bool operator!=(const config_attribute_value &other) const
	{
		return !operator==(other);
	}

	bool equals(const std::string& str) const;
	// These function prevent t_string creation in case of c["a"] == "b" comparisons.
	// The templates are needed to prevent using these function in case of c["a"] == 0 comparisons.
	template<typename T>
	utils::enable_if_t<std::is_same<const std::string, utils::add_const_t<T>>::value, bool>
		friend operator==(const config_attribute_value &val, const T &str)
	{
		return val.equals(str);
	}

	template<typename T>
	utils::enable_if_t<std::is_same<const char*, T>::value, bool>
		friend operator==(const config_attribute_value& val, T str)
	{
		return val.equals(std::string(str));
	}

	template<typename T>
	bool friend operator==(const T& str, const config_attribute_value& val)
	{
		return val == str;
	}

	template<typename T>
	bool friend operator!=(const config_attribute_value& val, const T& str)
	{
		return !(val == str);
	}

	template<typename T>
	bool friend operator!=(const T &str, const config_attribute_value& val)
	{
		return !(val == str);
	}

	// Streaming:
	friend std::ostream& operator<<(std::ostream& os, const config_attribute_value& v);

	// Visitor support:
	/// Applies a visitor to the underlying variant.
	/// (See the documentation for Boost.Variant.)
	template <typename V>
	typename V::result_type apply_visitor(const V & visitor) const
	{
		return boost::apply_visitor(visitor, value_);
	}

private:
	// Special strings.
	static const std::string s_yes, s_no;
	static const std::string s_true, s_false;
};
