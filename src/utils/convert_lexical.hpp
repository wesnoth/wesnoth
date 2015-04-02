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

#ifndef CONVERT_LEXICAL_HPP_
#define CONVERT_LEXICAL_HPP_

#include "config.hpp"
#include "config_class.tpp"
#include "global.hpp"
#include "util.hpp"

#include <boost/optional.hpp>

namespace convert {

template<typename T>
class lexical : public attribute_value_converter<T> {
public:
	lexical() {}
	lexical(T def) : def_(def) {}
	~lexical() {}

	T operator()(const aval & a) {
		if (def_) return lexical_cast_default<T>(a.str(), *def_);
		else return lexical_cast<T>(a.str());
	}

	aval operator()(const T & t) {
		aval a;
		a = lexical_cast<std::string>(t);
		return a;
	}
private:
	boost::optional<T> def_;
};

} // end namespace convert

#endif
