/*
	Copyright (C) 2008 - 2021
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

#include <optional>
#include <string>

namespace string_enums
{
/**
 * The base template for associating string values with enum values.
 * Implementing classes should not set custom int values for their enum.
 * The number of enum values must match the number of elements in the @a values array.
 * The values the @a values array must be unique.
 */
template<typename T>
struct enum_base : public T
{
	/**
	 * Uses the int value of the provided enum to get the associated index of the @a values array in the implementing class.
	 *
	 * @param key The enum value to get the equivalent string for.
	 * @return The string value associated to the enum value.
	 */
	static std::string get_string(typename T::type key)
	{
		return std::string{T::values[static_cast<int>(key)]};
	}

	/**
	 * Convert a string into its enum equivalent.
	 *
	 * @param value The string value to convert.
	 * @return The equivalent enum or std::nullopt.
	 */
	static std::optional<typename T::type> get_enum(const std::string value)
	{
		for(unsigned int i = 0; i < T::values.size(); i++) {
			if(value == T::values[i]) {
				return static_cast<typename T::type>(i);
			}
		}
		return std::nullopt;
	}

	/**
	 * @return The size of the implementing class's @a values array.
	 */
	static constexpr std::size_t size()
	{
		return T::values.size();
	}
};
} // namespace string_enums
