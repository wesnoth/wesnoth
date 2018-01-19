/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2018 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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
 * Routines related to configuration-files / WML.
 */

#include "config_attribute_value.hpp"

#include "lexical_cast.hpp"
#include "log.hpp"
#include "utils/const_clone.hpp"
#include "utils/functional.hpp"

#include <cstdlib>
#include <cstring>
#include <deque>
#include <istream>

#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/variant.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define DBG_CF LOG_STREAM(debug, log_config)

// Special string values.
const std::string config_attribute_value::s_yes("yes");
const std::string config_attribute_value::s_no("no");
const std::string config_attribute_value::s_true("true");
const std::string config_attribute_value::s_false("false");

/** Default implementation, but defined out-of-line for efficiency reasons. */
config_attribute_value::config_attribute_value()
	: value_()
{
}

/** Default implementation, but defined out-of-line for efficiency reasons. */
config_attribute_value::~config_attribute_value()
{
}

/** Default implementation, but defined out-of-line for efficiency reasons. */
config_attribute_value::config_attribute_value(const config_attribute_value& that)
	: value_(that.value_)
{
}

/** Default implementation, but defined out-of-line for efficiency reasons. */
config_attribute_value& config_attribute_value::operator=(const config_attribute_value& that)
{
	value_ = that.value_;
	return *this;
}

config_attribute_value& config_attribute_value::operator=(bool v)
{
	value_ = yes_no(v);
	return *this;
}

config_attribute_value& config_attribute_value::operator=(int v)
{
	value_ = v;
	return *this;
}

config_attribute_value& config_attribute_value::operator=(long long v)
{
	if(v > 0) {
		// We can store this unsigned.
		return *this = static_cast<unsigned long long>(v);
	}

	if(v >= INT_MIN) {
		// We can store this as an int.
		return *this = static_cast<int>(v);
	}

	// Getting to this point should be rare. (Currently, getting here means
	// something like there was so much draining in a campaign that the
	// total damage taken is not only negative, but so negative that an
	// int cannot hold the value.) So rare that it is not worth precise
	// treatment; just use a double.
	value_ = static_cast<double>(v);
	return *this;
}

config_attribute_value& config_attribute_value::operator=(unsigned long long v)
{
	// Use int for smaller numbers.
	if(v <= INT_MAX) {
		return *this = static_cast<int>(v);
	}

	value_ = v;
	return *this;
}

config_attribute_value& config_attribute_value::operator=(double v)
{
	// Try to store integers in other types.
	if(v > 0.0) {
		// Convert to unsigned and pass this off to that assignment operator.
		unsigned long long ull = static_cast<unsigned long long>(v);
		if(static_cast<double>(ull) == v) {
			return *this = ull;
		}
	} else {
		// Convert to integer and pass this off to that assignment operator.
		int i = static_cast<int>(v);
		if(static_cast<double>(i) == v) {
			return *this = i;
		}
	}

	// If we get here, this does in fact get stored as a double.
	value_ = v;
	return *this;
}

namespace
{
/**
 * Attempts to convert @a source to the template type.
 * This is to avoid "overzealous reinterpretations of certain WML strings as
 * numeric types" (c.f. bug #19201).
 * @returns true if the conversion was successful and the source string
 *          can be reobtained by streaming the result.
 */
template<typename To>
bool from_string_verify(const std::string& source, To& res)
{
	// Check 1: convertable to the target type.
	std::istringstream in_str(source);
	if(!(in_str >> res)) {
		return false;
	}

	// Check 2: convertable back to the same string.
	std::ostringstream out_str;
	out_str << res;
	return out_str.str() == source;
}
} // end anon namespace

config_attribute_value& config_attribute_value::operator=(const std::string& v)
{
	// Handle some special strings.
	if(v.empty()) {
		value_ = v;
		return *this;
	}

	if(v == s_yes) {
		value_ = yes_no(true);
		return *this;
	}

	if(v == s_no) {
		value_ = yes_no(false);
		return *this;
	}

	if(v == s_true) {
		value_ = true_false(true);
		return *this;
	}

	if(v == s_false) {
		value_ = true_false(false);
		return *this;
	}

	// Attempt to convert to a number.
	char* eptr;
	double d = strtod(v.c_str(), &eptr);
	if(*eptr == '\0') {
		// Possibly a number. See what type it should be stored in.
		// (All conversions will be from the string since the largest integer
		// type could have more precision than a double.)
		if(d > 0.0) {
			// The largest type for positive integers is unsigned long long.
			unsigned long long ull = 0;
			if(from_string_verify<unsigned long long>(v, ull)) {
				return *this = ull;
			}
		} else {
			// The largest (variant) type for negative integers is int.
			int i = 0;
			if(from_string_verify<int>(v, i)) {
				return *this = i;
			}
		}

		// This does not look like an integer, so it should be a double.
		// However, make sure it can convert back to the same string (in
		// case this is a string that just looks like a numeric value).
		std::ostringstream tester;
		tester << d;
		if(tester.str() == v) {
			value_ = d;
			return *this;
		}
	}

	// No conversion possible. Store the string.
	value_ = v;
	return *this;
}

config_attribute_value& config_attribute_value::operator=(const t_string& v)
{
	if(!v.translatable()) {
		return *this = v.str();
	}

	value_ = v;
	return *this;
}

bool config_attribute_value::to_bool(bool def) const
{
	if(const yes_no* p = boost::get<const yes_no>(&value_))
		return *p;
	if(const true_false* p = boost::get<const true_false>(&value_))
		return *p;

	// No other types are ever recognized as boolean.
	return def;
}

namespace
{
/// Visitor for converting a variant to a numeric type (T).
template<typename T>
class attribute_numeric_visitor : public boost::static_visitor<T>
{
public:
	// Constructor stores the default value.
	attribute_numeric_visitor(T def) : def_(def) {}

	T operator()(boost::blank const &) const { return def_; }
	T operator()(bool)                 const { return def_; }
	T operator()(int i)                const { return static_cast<T>(i); }
	T operator()(unsigned long long u) const { return static_cast<T>(u); }
	T operator()(double d)             const { return static_cast<T>(d); }
	T operator()(const std::string& s) const { return lexical_cast_default<T>(s, def_); }
	T operator()(t_string const &)     const { return def_; }

private:
	const T def_;
};
} // end anon namespace

int config_attribute_value::to_int(int def) const
{
	return apply_visitor(attribute_numeric_visitor<int>(def));
}

long long config_attribute_value::to_long_long(long long def) const
{
	return apply_visitor(attribute_numeric_visitor<long long>(def));
}

unsigned config_attribute_value::to_unsigned(unsigned def) const
{
	return apply_visitor(attribute_numeric_visitor<unsigned>(def));
}

size_t config_attribute_value::to_size_t(size_t def) const
{
	return apply_visitor(attribute_numeric_visitor<size_t>(def));
}

time_t config_attribute_value::to_time_t(time_t def) const
{
	return apply_visitor(attribute_numeric_visitor<time_t>(def));
}

double config_attribute_value::to_double(double def) const
{
	return apply_visitor(attribute_numeric_visitor<double>(def));
}

/// Visitor for converting a variant to a string.
class config_attribute_value::string_visitor : public boost::static_visitor<std::string>
{
	const std::string default_;

public:
	string_visitor(const std::string& fallback) : default_(fallback) {}

	std::string operator()(const boost::blank &) const { return default_; }
	std::string operator()(const yes_no & b)     const { return b.str(); }
	std::string operator()(const true_false & b) const { return b.str(); }
	std::string operator()(int i)                const { return lexical_cast<std::string>(i); }
	std::string operator()(unsigned long long u) const { return lexical_cast<std::string>(u); }
	std::string operator()(double d)             const { return lexical_cast<std::string>(d); }
	std::string operator()(const std::string& s) const { return s; }
	std::string operator()(t_string const &s)    const { return s.str(); }
};

std::string config_attribute_value::str(const std::string& fallback) const
{
	return apply_visitor(string_visitor(fallback));
}

t_string config_attribute_value::t_str() const
{
	if(const t_string* p = boost::get<const t_string>(&value_)) {
		return *p;
	}

	return str();
}

/**
 * Tests for an attribute that was never set.
 */
bool config_attribute_value::blank() const
{
	return boost::get<const boost::blank>(&value_) != nullptr;
}

/**
 * Tests for an attribute that either was never set or was set to "".
 */
bool config_attribute_value::empty() const
{
	if(boost::get<const boost::blank>(&value_)) {
		return true;
	}

	if(const std::string* p = boost::get<const std::string>(&value_)) {
		return p->empty();
	}

	return false;
}

/// Visitor handling equality checks.
class config_attribute_value::equality_visitor : public boost::static_visitor<bool>
{
public:
	// Most generic: not equal.
	template<typename T, typename U>
	bool operator()(const T&, const U&) const
	{
		return false;
	}

	// Same types are comparable and might be equal.
	template<typename T>
	bool operator()(const T& lhs, const T& rhs) const
	{
		return lhs == rhs;
	}

	// Boolean values can be compared.
	bool operator()(const true_false& lhs, const yes_no& rhs) const
	{
		return bool(lhs) == bool(rhs);
	}

	bool operator()(const yes_no& lhs, const true_false& rhs) const
	{
		return bool(lhs) == bool(rhs);
	}
};

/**
 * Checks for equality of the attribute values when viewed as strings.
 * Exception: Boolean synonyms can be equal ("yes" == "true").
 * Note: Blanks have no string representation, so do not equal "" (an empty string).
 */
bool config_attribute_value::operator==(const config_attribute_value& other) const
{
	return boost::apply_visitor(equality_visitor(), value_, other.value_);
}

/**
 * Checks for equality of the attribute values when viewed as strings.
 * Exception: Boolean synonyms can be equal ("yes" == "true").
 * Note: Blanks have no string representation, so do not equal "" (an empty string).
 * Also note that translatable string are never equal to non translatable strings.
 */
bool config_attribute_value::equals(const std::string& str) const
{
	config_attribute_value v;
	v = str;
	return *this == v;
	// if c["a"] = "1" then this solution would have resulted in c["a"] == "1" beeing false
	// because a["a"] is '1' and not '"1"'.
	// return boost::apply_visitor(std::bind( equality_visitor(), _1, std::cref(str) ), value_);
	// that's why we don't use it.
}

std::ostream& operator<<(std::ostream& os, const config_attribute_value& v)
{
	// Simple implementation, but defined out-of-line because of the templating
	// involved.
	return os << v.value_;
}
