
/*
   Copyright (C) 2007 by David White <dave.net>
   Part of the Silver Tree Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 or later.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef FORMATTER_HPP_INCLUDED
#define FORMATTER_HPP_INCLUDED

#include <sstream>

class formatter
{
public:
	template<typename T>
	formatter& operator<<(const T& o) {
		stream_ << o;
		return *this;
	}

	const std::string str() {
		return stream_.str();
	}

	const char* c_str() {
		return str().c_str();
	}

	operator std::string() {
		return stream_.str();
	}
private:
	std::ostringstream stream_;
};

#endif
