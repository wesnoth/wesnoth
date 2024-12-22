/*
	Copyright (C) 2005 - 2024
	by Philippe Plantier <ayin@anathas.org>
	Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

#include "font/constants.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class t_string;

namespace utils {

using string_map = std::map<std::string, t_string>;

const std::vector<std::string> res_order = {"blade", "pierce", "impact", "fire", "cold", "arcane"};

struct res_compare {
	/** Returns whether a < b, considering res_order. */
	bool operator()(const std::string& a, const std::string& b) const {
		for(const std::string& r : res_order) {
			if (b == r)	// this means b <= a, so a < b is false
				return false;
			if (a == r)
				return true;
		}
		return a < b;	// fallback only reached when neither a nor b occur in res_order
	}
};

using string_map_res = std::map<std::string, t_string, res_compare>;

bool isnewline(const char c);
bool portable_isspace(const char c);
bool notspace(char c);

enum {
	REMOVE_EMPTY = 0x01, /** REMOVE_EMPTY: remove empty elements. */
	STRIP_SPACES = 0x02  /** STRIP_SPACES: strips leading and trailing blank spaces. */
};

void trim(std::string_view& s);

template<typename F>
void split_foreach_impl(std::string_view s, char sep, const F& f)
{
	if(s.empty()) {
		return;
	}
	while(true)
	{
		std::size_t partend = s.find(sep);
		if(partend == std::string_view::npos) {
			break;
		}
		f(s.substr(0, partend));
		s.remove_prefix(partend + 1);
	}
	f(s);
}

template<typename F>
void split_foreach(std::string_view s, char sep, const int flags, const F& f)
{
	split_foreach_impl(s, sep, [&](std::string_view item) {
		if(flags & STRIP_SPACES) {
			trim(item);
		}
		if(!(flags & REMOVE_EMPTY) || !item.empty()) {
			f(item);
		}
	});
}

/** Splits a (comma-)separated string into a vector of pieces. */
std::vector<std::string> split(std::string_view val, const char c = ',', const int flags = REMOVE_EMPTY | STRIP_SPACES);
std::set<std::string> split_set(std::string_view val, const char c = ',', const int flags = REMOVE_EMPTY | STRIP_SPACES);

std::vector<std::string_view> split_view(std::string_view val, const char c = ',', const int flags = REMOVE_EMPTY | STRIP_SPACES);

/**
 * This function is identical to split(), except it does not split when it otherwise would if the
 * previous character was identical to the parameter 'quote' (i.e. it does not split quoted commas).
 * This method was added to make it possible to quote user input, particularly so commas in user input
 * would not cause visual problems in menus.
 *
 * @todo Why not change split()? That would change the methods post condition.
 */
std::vector<std::string> quoted_split(const std::string& val, char c= ',', int flags = REMOVE_EMPTY | STRIP_SPACES, char quote = '\\');

/**
 * Splits a string based on two separators into a map.
 *
 * Major: the separator between elements of the map
 * Minor: the separator between keys and values in one element
 *
 * For example, the string 'a:b,c:d,e:f' would be parsed into:
 *  a => b
 *  c => d
 *  e => f
 */
std::map<std::string, std::string> map_split(
	const std::string& val,
	char major = ',',
	char minor = ':',
	int flags = REMOVE_EMPTY | STRIP_SPACES,
	const std::string& default_value = "");

/**
 * Splits a string based either on a separator, except then the text appears within specified parenthesis.
 *
 * If the separator is "0" (default), it splits a string into an odd number of parts:
 * - The part before the first '(',
 * - the part between the first '('
 * - and the matching right ')', etc ...
 * and the remainder of the string.
 *
 * Note that one can use the same character for both the left and right parenthesis, which usually makes
 * the most sense for this function.
 *
 * Note that this will find the first matching char in the left string and match against the corresponding
 * char in the right string. A correctly processed string should return a vector with an odd number of
 * elements. Empty elements are never removed as they are placeholders, hence REMOVE EMPTY only works for
 * the separator split.
 *
 * INPUT:   ("a(b)c{d}e(f{g})h", 0, "({", ")}")
 * RETURNS: {"a", "b", "c", "d", "e", "f{g}", "h"}
 */
std::vector< std::string > parenthetical_split(
	std::string_view val,
	const char separator = 0,
	std::string_view left = "(",
	std::string_view right = ")",
	const int flags = REMOVE_EMPTY | STRIP_SPACES);

/**
 * Similar to parenthetical_split, but also expands embedded square brackets.
 *
 * Notes:
 * - The Separator must be specified and number of entries in each square bracket must match in each section.
 * - Leading zeros are preserved if specified between square brackets.
 * - An asterisk as in [a*n] indicates to expand 'a' n times
 *
 * This is useful for expanding animation WML code.
 *
 * Examples:
 *
 * INPUT:   ("a[1~3](1,[5,6,7]),b[8,9]", ",")
 * RETURNS: {"a1(1,5)", "a2(1,6)", "a3(1,7)", "b8", "b9"}
 *
 * INPUT:   ("abc[07~10]")
 * RETURNS: {"abc07", "abc08", "abc09", "abc10"}
 *
 * INPUT:   ("a[1,2]b[3~4]:c[5,6]")
 * RETURNS: {"a1b3:c5", "a2b4:c6"}
 *
 * INPUT:   ("abc[3~1].png")
 * RETURNS: {"abc3.png", "abc2.png", "abc1.png"}
 *
 * INPUT:   ("abc[3,1].png")
 * RETURNS: {"abc3.png", "abc1.png"}
 *
 * INPUT:   ("abc[de,xyz]")
 * RETURNS: {"abcde", "abcxyz"}
 *
 * INPUT:   ("abc[1*3]")
 * RETURNS: {"abc1", "abc1", "abc1"}
 */
std::vector<std::string> square_parenthetical_split(
	const std::string& val,
	const char separator = ',',
	const std::string& left = "([",
	const std::string& right = ")]",
	const int flags = REMOVE_EMPTY | STRIP_SPACES);

/**
 * Generates a new string joining container items in a list.
 *
 * @param v A container with elements.
 * @param s List delimiter.
 */
template <typename T>
std::string join(const T& v, const std::string& s = ",")
{
	std::stringstream str;

	for(typename T::const_iterator i = v.begin(); i != v.end(); ++i) {
		str << *i;
		if(std::next(i) != v.end()) {
			str << s;
		}
	}

	return str.str();
}

template <typename T>
std::string join_map(
	const T& v,
	const std::string& major = ",",
	const std::string& minor = ":")
{
	std::stringstream str;

	for(typename T::const_iterator i = v.begin(); i != v.end(); ++i) {
		str << i->first << minor << i->second;
		if(std::next(i) != v.end()) {
			str << major;
		}
	}

	return str.str();
}

/**
 * Generates a new string containing a bullet list.
 *
 * List items are preceded by the indentation blanks, a bullet string and
 * another blank; all but the last item are followed by a newline.
 *
 * @param v A container with elements.
 * @param indent Number of indentation blanks.
 * @param bullet The leading bullet string.
 */
template<typename T>
std::string bullet_list(const T& v, std::size_t indent = 4, const std::string& bullet = font::unicode_bullet)
{
	std::ostringstream str;

	for(typename T::const_iterator i = v.begin(); i != v.end(); ++i) {
		if(i != v.begin()) {
			str << '\n';
		}

		str << std::string(indent, ' ') << bullet << ' ' << *i;
	}

	return str.str();
}

/**
 * Indent a block of text.
 *
 * Only lines with content are changed; empty lines are left intact. However,
 * if @a string is an empty string itself, the indentation unit with the
 * specified @a indent_size will be returned instead.
 *
 * @param string      Text to indent.
 * @param indent_size Number of indentation units to use.
 */
std::string indent(const std::string& string, std::size_t indent_size = 4);

/**
 * Recognises the following patterns, and returns a {min, max} pair.
 *
 * * "1" returns {1, 1}
 * * "1-3" returns {1, 3}
 * * "1-infinity" returns {1, maximum int}
 * * "-1" returns {-1, -1}
 * * "-3--1" returns {-3, -1}
 *
 * Note that:
 *
 * * "3-1" returns {3, 3} and does not log an error
 * * "-1--3" returns {-1, -1} and does not log an error
 * * Although "-infinity--1", "2-infinity" and "-infinity-infinity" are all supported,
 * * ranges that can't match a reasonable number, e.g. "-infinity" or "infinity..infinity", may be treated as errors.
 */
std::pair<int, int> parse_range(std::string_view str);

/**
 * Handles a comma-separated list of inputs to parse_range, in a context that does not expect
 * negative values. Will return an empty list if any of the ranges have a minimum that's below
 * zero.
 */
std::vector<std::pair<int, int>> parse_ranges_unsigned(const std::string& str);

/**
 * Handles a comma-separated list of inputs to parse_range.
 */
std::vector<std::pair<int, int>> parse_ranges_int(const std::string& str);

/**
 * Recognises similar patterns to parse_range, and returns a {min, max} pair.
 *
 * For this function, "infinity" results in std::numeric_limits<double>::infinity.
 */
std::pair<double, double> parse_range_real(std::string_view str);

std::vector<std::pair<double, double>> parse_ranges_real(const std::string& str);

int apply_modifier(const int number, const std::string &amount, const int minimum = 0);

/** Add a "+" or replace the "-" par Unicode minus */
inline std::string print_modifier(const std::string &mod)
{
	return mod[0] == '-' ? (font::unicode_minus + std::string(mod.begin() + 1, mod.end())) : ("+" + mod);
}

/** Prepends a configurable set of characters with a backslash */
std::string escape(std::string_view str, const char *special_chars);

/**
 * Prepend all special characters with a backslash.
 *
 * Special characters are:
 * #@{}+-,\*=
 */
inline std::string escape(std::string_view str)
{
	return escape(str, "#@{}+-,\\*=");
}

/** Remove all escape characters (backslash) */
std::string unescape(std::string_view str);

/** Percent-escape characters in a UTF-8 string intended to be part of a URL. */
std::string urlencode(std::string_view str);

/** Surround the string 'str' with double quotes. */
inline std::string quote(std::string_view str)
{
	return '"' + std::string(str) + '"';
}

/** Convert no, false, off, 0, 0.0 to false, empty to def, and others to true */
bool string_bool(const std::string& str,bool def=false);

/** Converts a bool value to 'true' or 'false' */
std::string bool_string(const bool value);

/** Convert into a signed value (using the Unicode "−" and +0 convention */
std::string signed_value(int val);

/** Sign with Unicode "−" if negative */
std::string half_signed_value(int val);

/** Convert into a percentage (using the Unicode "−" and +0% convention */
inline std::string signed_percent(int val) {return signed_value(val) + "%";}

/**
 * Convert into a string with an SI-postfix.
 *
 * If the unit is to be translatable,
 * a t_string should be passed as the third argument.
 * _("unit_byte^B") is suggested as standard.
 *
 * There are no default values because they would not be translatable.
 */
std::string si_string(double input, bool base2, const std::string& unit);

/**
 * Try to complete the last word of 'text' with the 'wordlist'.
 *
 * @param[in, out] text  The parameter's usage is:
 *                       - Input: Text where we try to complete the last word
 *                         of.
 *                       - Output: Text with completed last word.
 * @param[in, out] wordlist
 *                        The parameter's usage is:
 *                        - Inout: A vector of strings to complete against.
 *                        - Output: A vector of strings that matched 'text'.
 *
 * @retval true           iff text is just one word (no spaces)
 */
bool word_completion(std::string& text, std::vector<std::string>& wordlist);

/** Check if a message contains a word. */
bool word_match(const std::string& message, const std::string& word);

/**
 * Match using '*' as any number of characters (including none),
 * '+' as one or more characters, and '?' as any one character.
 */
bool wildcard_string_match(const std::string& str, const std::string& match);

/**
 * Converts '*' to '%' and optionally escapes '_'.
 *
 * @param str The original string.
 * @param underscores Whether to escape underscore characters as well.
 */
void to_sql_wildcards(std::string& str, bool underscores = true);

/**
 * Check if the username contains only valid characters.
 *
 * (all alpha-numeric characters plus underscore and hyphen)
 */
bool isvalid_username(const std::string& login);

/**
 * Check if the username pattern contains only valid characters.
 *
 * (all alpha-numeric characters plus underscore, hyphen,
 * question mark and asterisk)
 */
bool isvalid_wildcard(const std::string& login);

/**
 * Truncates a string to a given utf-8 character count and then appends an ellipsis.
 */
void ellipsis_truncate(std::string& str, const std::size_t size);

} // end namespace utils
