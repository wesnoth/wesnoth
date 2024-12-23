/*
	Copyright (C) 2005 - 2024
	by Guillaume Melquiond <guillaume.melquiond@gmail.com>
	Copyright (C) 2003 by David White <dave@whitevine.net>
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

// Need this to get the default GETTEXT_DOMAIN for VGETTEXT/VNGETTEXT
#include "gettext.hpp"

#include "serialization/string_utils.hpp"

#include <ctime>
#include <string_view>

class variable_set;

namespace utils {

	namespace detail {
		extern std::string(* evaluate_formula)(const std::string& formula);
	}

/**
 * Determines if a string might contain variables to interpolate.
 * This can allow one to skip future interpolations (plural -- if there is only
 * one interpolation, the savings are not worth this check). In this spirit,
 * precision is sacrificed in the name of efficiency; the check is quick and
 * allows false positives, but there are no false negatives. (A false negative
 * would lead to incorrect behavior, whereas a false positive leads to merely
 * inefficient behavior.) In practice, false positives should be uncommon enough
 * to not worry about.
 */
inline bool might_contain_variables(const std::string &str)
{ return str.find('$') != std::string::npos; }

/**
 * Function which will interpolate variables, starting with '$' in the string
 * 'str' with the equivalent symbols in the given symbol table. If 'symbols'
 * is nullptr, then game event variables will be used instead.
 */
std::string interpolate_variables_into_string(const std::string &str, const std::map<std::string, t_string> * const symbols);
std::string interpolate_variables_into_string(const std::string &str, const std::map<std::string,std::string> * const symbols);
std::string interpolate_variables_into_string(const std::string &str, const variable_set& variables);

/**
 * Function that does the same as the above, for t_stringS.
 * If a change was made, then the t_string's translation is done in this
 * function, rather than at use. This limitation is due to the use of
 * create-time interpolation, rather than use-time.
 */
t_string interpolate_variables_into_tstring(const t_string &str, const variable_set& variables);

/**
 * Format a conjunctive list.
 * @param empty The string to return for an empty list
 * @param elems The list of entries in the list
 * @return The elements of the list joined by "and".
 */
std::string format_conjunct_list(const t_string& empty, const std::vector<t_string>& elems);

/**
 * Format a disjunctive list.
 * @param empty The string to return for an empty list
 * @param elems The list of entries in the list
 * @return The elements of the list joined or "and".
 */
std::string format_disjunct_list(const t_string& empty, const std::vector<t_string>& elems);

}

/**
 * Implementation functions for the VGETTEXT and VNGETTEXT macros.
 *
 * DO NOT USE DIRECTLY unless you really know what you're doing.
 * See https://github.com/wesnoth/wesnoth/issues/2716 for more info.
 */

std::string vgettext_impl(const char* domain, const char* msgid, const utils::string_map& symbols);

std::string vngettext_impl(const char* domain,
		const char* singular,
		const char* plural,
		int count,
		const utils::string_map& symbols);

/**
 * Handy wrappers around interpolate_variables_into_string and gettext.
 *
 * These should cover most usecases. If you're not sure whether you want
 * these macros or their implementation functions, use these. The only time
 * you should need to use the implementation functions directly is to pass a
 * different textdomain than the current value of GETTEXT_DOMAIN.
 */

#define VGETTEXT(msgid, ...) \
	vgettext_impl(GETTEXT_DOMAIN, msgid, __VA_ARGS__)

#define VNGETTEXT(msgid, msgid_plural, count, ...) \
	vngettext_impl(GETTEXT_DOMAIN, msgid, msgid_plural, count, __VA_ARGS__)

/**
 * @brief Calculate the approximate edit distance of two strings.
 *
 * @param str_1 First string to compare.
 * @param str_2 Second string to compare.
 *
 * @returns A score indicating how different the two strings are--the lower the score, the more similar the strings are.
 *
 * @note To avoid dynamic allocation, this function limits the number of characters that participate in the comparison.
 */
[[nodiscard]] std::size_t edit_distance_approx(std::string_view str_1, std::string_view str_2) noexcept;
