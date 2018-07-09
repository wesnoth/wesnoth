/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

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

	std::string str() const {
		return stream_.str();
	}

	// Implicit x-value conversion to string
	operator std::string() const
#if HAVE_REF_QUALIFIERS
		&&
#endif
	{
		return stream_.str();
	}

private:
	std::ostringstream stream_;
};
