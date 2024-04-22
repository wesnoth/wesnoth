/*
	Copyright (C) 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "utils/charconv.hpp"
#include <cctype>
#include <cstdlib>
#include <cassert>
#include <string>

#ifdef USE_FALLBACK_CHARCONV

namespace
{
	template<typename T>
	T string_to_floating_point(const char* str, char** str_end) = delete;
	template<>
	float string_to_floating_point(const char* str, char** str_end)
	{
		return std::strtof(str, str_end);
	}
	template<>
	double string_to_floating_point(const char* str, char** str_end)
	{
		return std::strtod(str, str_end);
	}
	template<>
	long double string_to_floating_point(const char* str, char** str_end)
	{
		return std::strtold(str, str_end);
	}
}
namespace utils::charconv
{
	template<typename T>
	std::enable_if_t<std::is_same_v<T, double> || std::is_same_v<T, float> || std::is_same_v<T, long double>, from_chars_result> from_chars(const char* first, const char* last, T& value, chars_format fmt)
	{
		// We don't need the other formats.
		assert(fmt = chars_format::general);

		// Make a copy to make sure its null terminated.
		std::string buffer = std::string(first, last);
		// from_chars doesnt support leading whitespace or plus signs.
		if(buffer.empty() || std::isspace(buffer.front()) || buffer.front() == '+' ) {
			return { first, std::errc::invalid_argument };
		}


		locale_t l = newlocale(LC_ALL, "C", static_cast<locale_t>(0));
		locale_t l_prev = uselocale(l);

		//initilize this to silence a warning.
		char* str_end = nullptr;
		value = string_to_floating_point<T>( buffer.data(), &str_end );

		uselocale(l_prev);
		freelocale(l);

		if(str_end == buffer.data()) {
			return { first, std::errc::invalid_argument };
		}
		return { first + (str_end - buffer.data()), std::errc() };
	}
	template from_chars_result from_chars(const char* first, const char* last, float& value, chars_format fmt);
	template from_chars_result from_chars(const char* first, const char* last, double& value, chars_format fmt);
	template from_chars_result from_chars(const char* first, const char* last, long double& value, chars_format fmt);
}

#endif
