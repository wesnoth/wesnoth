/*
	Copyright (C) 2003 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "utils/general.hpp"

#if defined(__clang__) || defined(__GNUG__)
#include <cxxabi.h>
#endif

namespace utils
{
std::string get_unknown_exception_type()
{
#if defined(__clang__) || defined(__GNUG__)
	std::string to_demangle = __cxxabiv1::__cxa_current_exception_type()->name();
	int status = 0;
	char* buff = __cxxabiv1::__cxa_demangle(to_demangle.c_str(), nullptr, nullptr, &status);
	if(status == 0)
	{
		std::string demangled = buff;
		std::free(buff);
		return demangled;
	}
	else
	{
		return to_demangle;
	}
#else
	return "";
#endif
}
}
