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

#pragma once

#include <boost/version.hpp>

#include <array>
#include <assert.h>
#include <cctype>
#include <string_view>
#include <string>
#include <stdexcept>
// The gcc implemenetation of from_chars is in some versions just a temporaty solution that calls
// strtod that's why we prefer the boost version if available.
#if BOOST_VERSION >= 108500 && __has_include(<boost/charconv.hpp>)
	#define USE_BOOST_CHARCONV
#elif defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE >= 11
	#define USE_STD_CHARCONV
#elif defined(_MSC_VER) && _MSC_VER >= 1924
	#define USE_STD_CHARCONV
#elif defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 14000
	#define USE_FALLBACK_CHARCONV
#else
	#error No charconv implementation found.
#endif

#ifdef USE_BOOST_CHARCONV
#include <boost/charconv.hpp>
namespace charconv_impl = boost::charconv;
#else
#include <charconv>
namespace charconv_impl = std;
#endif


namespace utils::charconv
{
	using chars_format      = charconv_impl::chars_format;
	using from_chars_result = charconv_impl::from_chars_result;
	using to_chars_result   = charconv_impl::to_chars_result;

	template<typename... T>
	to_chars_result to_chars(char* first, char* last, T&&... value )
	{
		return charconv_impl::to_chars(first, last, value...);
	}

	template<typename T>
	std::enable_if_t<std::is_integral_v<T>, from_chars_result> from_chars(const char* first, const char* last, T& value, int base = 10 )
	{
		return charconv_impl::from_chars(first, last, value, base);
	}

#ifndef USE_FALLBACK_CHARCONV
	template<typename T>
	std::enable_if_t<std::is_floating_point_v<T>, from_chars_result> from_chars(const char* first, const char* last, T& value, chars_format fmt = chars_format::general )
	{
		return charconv_impl::from_chars(first, last, value, fmt);
	}
#else
	template<typename T>
	std::enable_if_t<std::is_same_v<T, double> || std::is_same_v<T, float> || std::is_same_v<T, long double>, from_chars_result> from_chars(const char* first, const char* last, T& value, chars_format fmt = chars_format::general );
#endif

	// the maximum size of a string that to_chars produces for type T, with the default chars_format
	template<class T>
	constexpr size_t buffer_size = 50;
}

namespace utils
{
	// from_chars doesnt support leading whitespaces, call this before from_chars if you want to accept leading whitespaces.
	inline void trim_for_from_chars(std::string_view& v)
	{
		while(!v.empty() && std::isspace(v.front())) {
			v.remove_prefix(1);
		}
		if(v.size() >= 2 && v[0] == '+' && v[1] != '-' ) {
			v.remove_prefix(1);
		}
	}


	// converts a number to a char buffer without allocations.
	template<typename TNum>
	struct charconv_buffer
	{
		std::array<char, utils::charconv::buffer_size<TNum>> buffer;
		size_t size;

		charconv_buffer()
			: size(0)
		{
		}

		charconv_buffer(TNum num)
			: size(0)
		{
			set_value(num);
		}

		void set_value(TNum num)
		{
			auto [ptr, ec] = utils::charconv::to_chars(buffer.data(), buffer.data() + buffer.size(), num);
			if(ec != std::errc()) {
				// Shouldnt happen by definition of utils::charconv::buffer_size<TNum>
				assert(!"Error in charconv_buffer, buffer not large enough");
				size = 0;
			} else {
				size = ptr - buffer.data();
			}
			//TODO: should we make this null-terminated?
		}

		std::string_view get_view() const
		{
			return std::string_view(buffer.data(), size);
		}

		std::string to_string() const
		{
			return std::string(buffer.data(), size);
		}
	};

	/// Same interface as std::stod and meant as a drop in replacement, except:
	/// - It takes a std::string_view
	/// - It is locale independent
	inline double stod(std::string_view str) {
		trim_for_from_chars(str);
		double res;
		auto [ptr, ec] = utils::charconv::from_chars(str.data(), str.data() + str.size(), res);
		if(ec == std::errc::invalid_argument) {
			throw std::invalid_argument("");
		} else if(ec == std::errc::result_out_of_range) {
			throw std::out_of_range("");
		}
		return res;
	}
	/// Same interface as std::stoi and meant as a drop in replacement, except:
	/// - It takes a std::string_view
	inline int stoi(std::string_view str) {
		trim_for_from_chars(str);
		int res;
		auto [ptr, ec] = utils::charconv::from_chars(str.data(), str.data() + str.size(), res);
		if(ec == std::errc::invalid_argument) {
			throw std::invalid_argument("");
		} else if(ec == std::errc::result_out_of_range) {
			throw std::out_of_range("");
		}
		return res;
	}
}
