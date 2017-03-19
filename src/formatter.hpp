/*
   Copyright (C) 2007 - 2017 by David White <dave.net>
   Part of the Silver Tree Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by or later.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FORMATTER_HPP_INCLUDED
#define FORMATTER_HPP_INCLUDED

#include <sstream>
#include <utility>

/**
 * std::ostringstream wrapper.
 *
 * ostringstream's operator<< doesn't return a ostringstream&. It returns an
 * ostream& instead. This is unfortunate, because it means that you can't do
 * something like this: (ostringstream() << n).str() to convert an integer to a
 * string, all in one line instead you have to use this far more tedious
 * approach:
 *  ostringstream s;
 *  s << n;
 *  s.str();
 * This class corrects this shortcoming, allowing something like this:
 *  string result = (formatter() << "blah " << n << x << " blah").str();
 *
 * Actually, due to the ref qualified versions below, you can get away with this
 *
 *  string result = formatter() << "blah " << n << x << " blah";
 */
class formatter
{
public:
	formatter() :
		stream_()
	{
	}

	template<typename T>
	formatter& operator<<(const T & o)
#if HAVE_REF_QUALIFIERS
		&
#endif
	{
		stream_ << o;
		return *this;
	}

#if HAVE_REF_QUALIFIERS
	template <typename T>
	formatter && operator<<(const T & o) && {
		stream_ << o;
		return std::move(*this);
	}
#endif

	std::string str() {
		return stream_.str();
	}

	// Implicit x-value conversion to string
	operator std::string()
#if HAVE_REF_QUALIFIERS
		&&
#endif
	{
		return stream_.str();
	}

private:
	std::ostringstream stream_;
};

#endif
