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

#pragma once

#include "config.hpp"

/**
 * Utility functions for implementing [filter], [filter_ability], [filter_weapon], etc.
 *
 * For example, a filter of `x=1` puts a requirement on the value of `x` but accepts any value of `y`.
 *
 * Both `int` and `double` assume a default value of zero, so a filter that accepts zero will match an
 * unset value as well as a present value.
 */
namespace utils::config_filters
{
/**
 * Checks whether the filter matches the value of @a cfg[@a attribute]. If @a cfg doesn't have that
 * attribute, assume that an unset value is equivalent to @a def.
 *
 * Always returns true if the filter puts no restriction on the value of @a cfg[@a attribute].
 */
bool bool_matches_if_present(const config& filter, const config& cfg, const std::string& attribute, bool def);

/**
 * Checks whether the filter matches the value of @a cfg[@a attribute]. If @a cfg doesn't have that
 * attribute, assume that an unset value is equivalent to @a def if exist, else value false is returned.
 *
 * Always returns true if the filter puts no restriction on the value of @a cfg[@a attribute].
 */
bool double_matches_if_present(const config& filter, const config& cfg, const std::string& attribute, utils::optional<double> def = utils::nullopt);
bool int_matches_if_present(const config& filter, const config& cfg, const std::string& attribute, utils::optional<int> def = utils::nullopt);

/**
 * Supports filters using "add" and "sub" attributes, for example a filter `add=1` matching a cfg containing either
 * `add=1` or `sub=-1`; this assumes that code elsewhere has already checked that cfg contains at most one of those
 * keys.
 *
 * This only checks for the presence of @a attribute and @a opposite in the filter, so the caller should call this function a second
 * time, with @a attribute and @a opposite reversed and if none of these attribute is here value false is returned.
 *
 * The function is named "negative" in case we later want to add a "reciprocal" for the "multiply"/"divide" pair.
 */
bool int_matches_if_present_or_negative(
	const config& filter, const config& cfg, const std::string& attribute, const std::string& opposite, utils::optional<int> def = utils::nullopt);

bool string_matches_if_present(
	const config& filter, const config& cfg, const std::string& attribute, const std::string& def);

/**
 * filter[attribute] and cfg[attribute] are assumed to be comma-separated lists.
 * If the filter is present, each item in filter[attribute] must match an item in cfg[attribute]
 * for the function to return true.
 */
bool set_includes_if_present(const config& filter, const config& cfg, const std::string& attribute);

bool bool_or_empty(const config& filter, const config& cfg, const std::string& attribute);

} // namespace utils::config_filters
