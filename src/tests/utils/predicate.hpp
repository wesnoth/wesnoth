/*
   Copyright (C) 2008 - 2016 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TESTS_UTILS_FAKE_DISPLAY_HPP_INCLUDED
#define TESTS_UTILS_FAKE_DISPLAY_HPP_INCLUDED

#include <cstdarg>

namespace test_utils {

	/**
	 * Used to check if first parameter matches one of given values
	 * used with BOOST_CHECK_PREDICATE
	 **/
	template<class T>
	bool one_of(const T& val, int va_number, ...)
	{
		T param;
		va_list vl;
		va_start(vl, va_number);

		bool ret = false;

		for (int i = 0; i < va_number; ++i)
		{
			param = va_arg(vl, T);
			if (param == val)
			{
				ret = true;
				break;
			}
		}
		va_end(vl);
		return ret;
	}


}
#endif
