/*
	Copyright (C) 2009 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 * @file
 * New lexcical_cast header.
 *
 * For debugging you can include this header _in_ a namespace (to honor ODR)
 * and have a set of functions that throws exceptions instead of doing the
 * real job. This is done for the unit tests but should normally not be done.
 */

#ifdef LEXICAL_CAST_DEBUG
#undef LEXICAL_CAST_HPP_INCLUDED
#endif

#ifndef LEXICAL_CAST_HPP_INCLUDED
#define LEXICAL_CAST_HPP_INCLUDED

#ifdef LEXICAL_CAST_DEBUG

#undef DEBUG_THROW
/**
 * Throws an exception for debugging.
 *
 * @param id                      The unique name to identify the function.
 *                                @note this name is a user defined string and
 *                                should not be modified once used!
 */
#define DEBUG_THROW(id) throw id;
#else

#include "utils/charconv.hpp"

#include "utils/optional_fwd.hpp"

#include <cstdlib>
#include <limits>
#include <string>
#include <sstream>
#include <type_traits>

#define DEBUG_THROW(id)
#endif

/**
 * @namespace implementation
 * Contains the implementation details for lexical_cast and shouldn't be used
 * directly.
 */
namespace implementation {

	template<
		  typename To
		, typename From
		, typename ToEnable = void
		, typename FromEnable = void
	>
	struct lexical_caster;

} // namespace implementation

/**
 * Lexical cast converts one type to another.
 *
 * @tparam To                     The type to convert to.
 * @tparam From                   The type to convert from.
 *
 * @param value                   The value to convert.
 *
 * @returns                       The converted value.
 *
 * @throw                         bad_lexical_cast if the cast was unsuccessful.
 */
template<typename To, typename From>
inline To lexical_cast(From value)
{
	return implementation::lexical_caster<To, From>().operator()(value, utils::nullopt);
}

/**
 * Lexical cast converts one type to another with a fallback.
 *
 * @tparam To                     The type to convert to.
 * @tparam From                   The type to convert from.
 *
 * @param value                   The value to convert.
 * @param fallback                The fallback value to return if the cast fails.
 *
 * @returns                       The converted value.
 */
template<typename To, typename From>
inline To lexical_cast_default(From value, To fallback = To())
{
	return implementation::lexical_caster<To, From>().operator()(value, fallback);
}

/** Thrown when a lexical_cast fails. */
struct bad_lexical_cast : std::exception
{
	const char* what() const noexcept
	{
		return "bad_lexical_cast";
	}
};

namespace implementation {

/**
 * Base class for the conversion.
 *
 * Since functions can't be partially specialized we use a class, which can be
 * partially specialized for the conversion.
 *
 * @tparam To                     The type to convert to.
 * @tparam From                   The type to convert from.
 * @tparam ToEnable               Filter to enable the To type.
 * @tparam FromEnable             Filter to enable the From type.
 */
template<
	  typename To
	, typename From
	, typename ToEnable
	, typename FromEnable
>
struct lexical_caster
{
	To operator()(From value, utils::optional<To> fallback) const
	{
		DEBUG_THROW("generic");

		To result = To();
		std::stringstream sstr;

		if(!(sstr << value && sstr >> result)) {
			if(fallback) { return *fallback; }

			throw bad_lexical_cast();
		} else {
			return result;
		}
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning strings from an integral type or a pointer to an
 * integral type.
 */
template <typename From>
struct lexical_caster<
	  std::string
	, From
	, void
	, std::enable_if_t<std::is_arithmetic_v<From>>
>
{
	std::string operator()(From value, utils::optional<std::string>) const
	{
		DEBUG_THROW("specialized - To std::string - From arithmetic");
		if constexpr (std::is_same_v<bool, From>) {
			return value ? "1" : "0";
		} else {
			return utils::charconv_buffer<From>(value).to_string();
		}
	}
};



/**
 * Specialized conversion class.
 *
 * @note is specialized to silence C4804 from MSVC.
 */
template <>
struct lexical_caster<
	  bool
	, std::string_view
	, void
	, void
>
{
	bool operator()(std::string_view str, utils::optional<bool> fallback) const
	{
		DEBUG_THROW("specialized - To bool - From string");
		utils::trim_for_from_chars(str);
		if(str == "1") {
			return true;
		} else if(str == "0") {
			return false;
		} else if (fallback) {
			return *fallback;
		} else {
			throw bad_lexical_cast();
		}
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning arithmetic from a string_view, also used by std::string and (const) char*
 */
template <typename To>
struct lexical_caster<
	  To
	, std::string_view
	, std::enable_if_t<std::is_arithmetic_v<To>>
	, void
>
{
	To operator()(std::string_view str, utils::optional<To> fallback) const
	{
		DEBUG_THROW("specialized - To arithmetic - From string");

		To res = To();

		utils::trim_for_from_chars(str);

		auto [ptr, ec] = utils::charconv::from_chars(str.data(), str.data() + str.size(), res);
		if(ec == std::errc()) {
			return res;
		} else if(fallback){
			return *fallback;
		} else {
			throw bad_lexical_cast();
		}
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning arithmetic from a std::string
 */
template <typename To>
struct lexical_caster<
	  To
	, std::string
	, std::enable_if_t<std::is_arithmetic_v<To>>
	, void
>
{
	To operator()(const std::string& value, utils::optional<To> fallback) const
	{
		// Dont DEBUG_THROW. the test shodul test what actual implementaiton is used in the end, not which specialazion that jsut forwards  to another
		if(fallback) {
			return lexical_cast_default<To>(std::string_view(value), *fallback);
		} else {
			return lexical_cast<To>(std::string_view(value));
		}
	}
};


/**
 * Specialized conversion class.
 *
 * Specialized for returning arithmetic from a (const) char*.
 */
template <class To, class From>
struct lexical_caster<
	  To
	, From
	, std::enable_if_t<std::is_arithmetic_v<To> >
	, std::enable_if_t<std::is_same_v<From, const char*> || std::is_same_v<From, char*> >
>
{
	To operator()(const std::string& value, utils::optional<To> fallback) const
	{
		// Dont DEBUG_THROW. the test shodul test what actual implementaiton is used in the end, not which specialazion that jsut forwards  to another
		if(fallback) {
			return lexical_cast_default<To>(std::string_view(value), *fallback);
		} else {
			return lexical_cast<To>(std::string_view(value));
		}
	}
};

} // namespace implementation

#endif
