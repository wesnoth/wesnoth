/*
	Copyright (C) 2008 - 2025
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

#include <array>
#include "utils/optional_fwd.hpp"
#include <string>
#include <string_view>
#include <tuple>

namespace string_enums
{
/**
 * The base template for associating string values with enum values.
 * Implementing classes should not set custom int values for their enum.
 * The number of enum values must match the number of elements in the @a values array.
 * The values the @a values array must be unique.
 */
template<typename Definition>
struct enum_base : public Definition
{
	using enum_type = typename Definition::type;

	// check that all implementations of this are scoped enums
	// TODO: C++23 std::is_scoped_enum
	static_assert(std::is_enum_v<enum_type> && !std::is_convertible_v<enum_type, int>, "Enum is not a scoped enum");

	/**
	 * Converts a enum to its string equivalent.
	 *
	 * @param key        The enum value to get the equivalent string for.
	 * @return           The string value associated with the enum value.
	 */
	static std::string get_string(enum_type key)
	{
		return std::string{Definition::values[static_cast<int>(key)]};
	}

	/**
	 * Converts a string into its enum equivalent.
	 *
	 * @param value      The string value to convert.
	 * @return           The equivalent enum or utils::nullopt.
	 */
	static constexpr utils::optional<enum_type> get_enum(const std::string_view value)
	{
		for(unsigned int i = 0; i < size(); i++) {
			if(value == Definition::values[i]) {
				return static_cast<enum_type>(i);
			}
		}
		return utils::nullopt;
	}

	/**
	 * Converts an int into its enum equivalent.
	 *
	 * @param value      The string value to convert.
	 * @return           The equivalent enum or utils::nullopt.
	 */
	static constexpr utils::optional<enum_type> get_enum(unsigned long value)
	{
		if(value < size()) {
			return static_cast<enum_type>(value);
		} else {
			return utils::nullopt;
		}
	}

	/**
	 * @return The size of the implementing class's @a values array.
	 */
	static constexpr std::size_t size()
	{
		return Definition::values.size();
	}

	/** Provide a alias template for an array of matching size. */
	template<typename T>
	using sized_array = std::array<T, size()>;
};

#ifndef __MINGW64__
#define ENUM_AND_ARRAY(...)                                                                                            \
	enum class type { __VA_ARGS__ };                                                                                   \
	static constexpr std::array values{__VA_ARGS__};
#else
#define ENUM_AND_ARRAY(...)                                                                                            \
	enum class type { __VA_ARGS__ };                                                                                   \
	static constexpr std::array<std::string_view, std::tuple_size_v<decltype(std::make_tuple(__VA_ARGS__))>>           \
		values{__VA_ARGS__};
#endif

} // namespace string_enums
