/*
   Copyright (C) 2009 - 2011 by Chris Hopman <cjhopman@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SHARED_STRING_HPP_INCLUDED
#define SHARED_STRING_HPP_INCLUDED

#include "shared_object.hpp"
#include "tstring.hpp"

struct shared_string : public shared_object<std::string> {
	typedef shared_object<std::string> super;

	template <typename T>
	shared_string(const T& o) : super(o) { }
	const char* c_str() const {
		return get().c_str();
	}

	operator std::string() const {
		return super::operator std::string();
	}

	operator t_string() const {
		return super::operator std::string();
	}

	bool empty() const {
		return get().empty();
	}

	size_t find(const std::string& str, size_t pos = 0) const {
		return get().find(str, pos);
	}

	std::string substr(size_t pos = 0, size_t n = std::string::npos) const {
		return get().substr(pos, n);
	}

	size_t size() const {
		return get().size();
	}

	std::string::const_iterator begin() const {
		return get().begin();
	}

	std::string::const_iterator end() const {
		return get().end();
	}
};
#endif
