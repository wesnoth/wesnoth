/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef CONVERT_HPP_
#define CONVERT_HPP_

#include "config.hpp"
#include "config_class.tpp"
#include "utils/boost_function_guarded.hpp"

#include <boost/optional.hpp>

namespace convert {

typedef config::attribute_value aval;

class to_int : public attribute_value_converter<int> {
public:
	to_int() : def_() {}
	to_int(int def) : def_(def) {}
	~to_int() {}

	int operator()(const aval & a) { return (def_ ? a.to_int(*def_) : a.to_int()); }
	aval operator()(const int & t) { aval a; a = t; return a; }
private:
	boost::optional<int> def_;
};

class to_unsigned : public attribute_value_converter<unsigned int> {
public:
	to_unsigned() : def_() {}
	to_unsigned(unsigned int def) : def_(def) {}
	~to_unsigned() {}

	unsigned int operator()(const aval & a) { return (def_ ? a.to_unsigned(*def_) : a.to_unsigned()); }
	aval operator()(const unsigned int & t) { aval a; a = t; return a; }
private:
	boost::optional<unsigned int> def_;
};

class to_bool : public attribute_value_converter<bool> {
public:
	to_bool() : def_() {}
	to_bool(bool def) : def_(def) {}
	~to_bool() {}

	bool operator()(const aval & a) { return (def_ ? a.to_bool(*def_) : a.to_bool()); }
	aval operator()(const bool & t) { aval a; a = t; return a; }
private:
	boost::optional<bool> def_;
};

class str : public attribute_value_converter<std::string> {
public:
	str() {}
	~str() {}

	std::string operator()(const aval & a) { return a.str(); }
	aval operator()(const std::string & s) { aval a; a = s; return a; }
};

/// Define an adhoc config attribute_value_converter from a pair of functions.
template<typename T>
class pair : public attribute_value_converter<T> {
public:
	pair(boost::function1<T, const aval &> from, boost::function1<aval,const T &> to) : from_(from), to_(to) {}
	~pair() {}

	T operator()(const aval & a) { return from_(a); }
	aval operator()(const T & t) { return to_(t); }
private:
	boost::function1<T, const aval &> from_;
	boost::function1<aval, const T &> to_;
};

} // end namespace convert

#endif
