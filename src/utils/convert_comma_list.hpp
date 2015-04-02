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

#ifndef CONVERT_COMMA_LIST_HPP_
#define CONVERT_COMMA_LIST_HPP_

#include "global.hpp"

#include "config.hpp"
#include "config_class.tpp"
#include "serialization/string_utils.hpp"

namespace convert {

template<class T>
class comma_list : public attribute_value_converter<T> {
public:
	comma_list() {}
	~comma_list() {}

	T operator()(const aval & a) {
		std::vector<std::string> v = utils::split(a.str());
		return T(v.begin(), v.end());
	}

	aval operator()(const T & t) {
		aval a;
		a = utils::join<T>(t);
		return a;
	}
};

} // end namespace convert

#endif
