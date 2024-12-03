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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "formula/string_utils.hpp"
#include "variable.hpp"

#include "config.hpp"
#include "log.hpp"
#include "gettext.hpp"

#include <algorithm>
#include <array>
#include <utility>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)

static bool two_dots(char a, char b) { return a == '.' && b == '.'; }

namespace utils {

	namespace detail {
		std::string(* evaluate_formula)(const std::string& formula) = nullptr;
	}

template <typename T>
class string_map_variable_set : public variable_set
{
public:
	string_map_variable_set(const std::map<std::string,T>& map) : map_(map) {}

	virtual config::attribute_value get_variable_const(const std::string &key) const
	{
		config::attribute_value val;
		const auto itor = map_.find(key);
		if (itor != map_.end())
			val = itor->second;
		return val;
	}
	virtual variable_access_const get_variable_access_read(const std::string& varname) const
	{
		temp_.reset(new config);
		for(const auto& p : map_) {
			temp_->insert(p.first, p.second);
		}
		return variable_access_const(varname, *temp_);
	}
private:
	const std::map<std::string,T>& map_;
	mutable std::shared_ptr<config> temp_; // only used if get_variable_access_read called
};
}

static std::string do_interpolation(const std::string &str, const variable_set& set)
{
	std::string res = str;
	// This needs to be able to store negative numbers to check for the while's condition
	// (which is only false when the previous '$' was at index 0)
	int rfind_dollars_sign_from = res.size();
	while(rfind_dollars_sign_from >= 0) {
		// Going in a backwards order allows nested variable-retrieval, e.g. in arrays.
		// For example, "I am $creatures[$i].user_description!"
		const std::string::size_type var_begin_loc = res.rfind('$', rfind_dollars_sign_from);

		// If there are no '$' left then we're done.
		if(var_begin_loc == std::string::npos) {
			break;
		}

		// For the next iteration of the loop, search for more '$'
		// (not from the same place because sometimes the '$' is not replaced)
		rfind_dollars_sign_from = static_cast<int>(var_begin_loc) - 1;


		const std::string::iterator var_begin = res.begin() + var_begin_loc;

		// The '$' is not part of the variable name.
		const std::string::iterator var_name_begin = var_begin + 1;
		std::string::iterator var_end = var_name_begin;

		if(var_name_begin == res.end()) {
			// Any '$' at the end of a string is just a '$'
			continue;
		} else if(*var_name_begin == '(') {
			// The $( ... ) syntax invokes a formula
			int paren_nesting_level = 0;
			bool in_string = false,
				in_comment = false;
			do {
				switch(*var_end) {
				case '(':
					if(!in_string && !in_comment) {
						++paren_nesting_level;
					}
					break;
				case ')':
					if(!in_string && !in_comment) {
						--paren_nesting_level;
					}
					break;
				case '#':
					if(!in_string) {
						in_comment = !in_comment;
					}
					break;
				case '\'':
					if(!in_comment) {
						in_string = !in_string;
					}
					break;
				// TODO: support escape sequences when/if they are allowed in FormulaAI strings
				}
			} while(++var_end != res.end() && paren_nesting_level > 0);
			if(utils::detail::evaluate_formula == nullptr) {
				WRN_NG << "Formula substitution ignored (and removed) because WFL engine is not present in the server.";
				res.replace(var_begin, var_end, "");
				continue;
			}
			if(paren_nesting_level > 0) {
				ERR_NG << "Formula in WML string cannot be evaluated due to "
					<< "a missing closing parenthesis:\n\t--> \""
					<< std::string(var_begin, var_end) << "\"";
				res.replace(var_begin, var_end, "");
				continue;
			}
			res.replace(var_begin, var_end, utils::detail::evaluate_formula(std::string(var_begin+2, var_end-1)));
			continue;
		}

		// Find the maximum extent of the variable name (it may be shortened later).
		for(int bracket_nesting_level = 0; var_end != res.end(); ++var_end) {
			const char c = *var_end;
			if(c == '[') {
				++bracket_nesting_level;
			}
			else if(c == ']') {
				if(--bracket_nesting_level < 0) {
					break;
				}
			}
			// isascii() breaks on mingw with -std=c++0x
			else if (!(((c) & ~0x7f) == 0)/*isascii(c)*/ || (!isalnum(c) && c != '.' && c != '_')) {
				break;
			}
		}

		// Two dots in a row cannot be part of a valid variable name.
		// That matters for random=, e.g. $x..$y
		var_end = std::adjacent_find(var_name_begin, var_end, two_dots);
		// the default value is specified after ''?'
		const std::string::iterator default_start = var_end < res.end() && *var_end == '?' ? var_end + 1 : res.end();

		// If the last character is '.', then it can't be a sub-variable.
		// It's probably meant to be a period instead. Don't include it.
		// Would need to do it repetitively if there are multiple '.'s at the end,
		// but don't actually need to do so because the previous check for adjacent '.'s would catch that.
		// For example, "My score is $score." or "My score is $score..."
		if(*(var_end-1) == '.'
		// However, "$array[$i]" by itself does not name a variable,
		// so if "$array[$i]." is encountered, then best to include the '.',
		// so that it more closely follows the syntax of a variable (if only to get rid of all of it).
		// (If it's the script writer's error, they'll have to fix it in either case.)
		// For example in "$array[$i].$field_name", if field_name does not exist as a variable,
		// then the result of the expansion should be "", not "." (which it would be if this exception did not exist).
		&& *(var_end-2) != ']') {
			--var_end;
		}

		const std::string var_name(var_name_begin, var_end);
		if(default_start == res.end()) {
			if(var_end != res.end() && *var_end == '|') {
				// It's been used to end this variable name; now it has no more effect.
				// This can allow use of things like "$$composite_var_name|.x"
				// (Yes, that's a WML 'pointer' of sorts. They are sometimes useful.)
				// If there should still be a '|' there afterwards to affect other variable names (unlikely),
				// just put another '|' there, one matching each '$', e.g. "$$var_containing_var_name||blah"
				++var_end;
			}


			if (var_name.empty()) {
				// Allow for a way to have $s in a string.
				// $| will be replaced by $.
				res.replace(var_begin, var_end, "$");
			}
			else {
				// The variable is replaced with its value.
				res.replace(var_begin, var_end,
					set.get_variable_const(var_name));
			}
		}
		else {
			var_end = default_start;
			while(var_end != res.end() && *var_end != '|') {
				++var_end;
			}
			const std::string::iterator default_end = var_end;
			const config::attribute_value& val = set.get_variable_const(var_name);
			if(var_end == res.end()) {
				res.replace(var_begin, default_start - 1, val);
			}
			else if(!val.empty()) {
				res.replace(var_begin, var_end + 1, val);
			}
			else {
				res.replace(var_begin, var_end + 1, std::string(default_start, default_end));
			}
		}
	}

	return res;
}

namespace utils {

std::string interpolate_variables_into_string(const std::string &str, const string_map * const symbols)
{
	auto set = string_map_variable_set<t_string>(*symbols);
	return do_interpolation(str, set);
}

std::string interpolate_variables_into_string(const std::string &str, const std::map<std::string,std::string> * const symbols)
{
	auto set = string_map_variable_set<std::string>(*symbols);
	return do_interpolation(str, set);
}

std::string interpolate_variables_into_string(const std::string &str, const variable_set& variables)
{
	return do_interpolation(str, variables);
}

t_string interpolate_variables_into_tstring(const t_string &tstr, const variable_set& variables)
{
	if(!tstr.str().empty()) {
		std::string interp = utils::interpolate_variables_into_string(tstr.str(), variables);
		if(tstr.str() != interp) {
			return t_string(interp);
		}
	}
	return tstr;
}

std::string format_conjunct_list(const t_string& empty, const std::vector<t_string>& elems) {
	switch(elems.size()) {
	case 0: return empty;
	case 1: return elems[0];
		// TRANSLATORS: Formats a two-element conjunctive list.
	case 2: return VGETTEXT("conjunct pair^$first and $second", {{"first", elems[0]}, {"second", elems[1]}});
	}
	// TRANSLATORS: Formats the first two elements of a conjunctive list.
	std::string prefix = VGETTEXT("conjunct start^$first, $second", {{"first", elems[0]}, {"second", elems[1]}});
	// For size=3 this loop is not entered
	for(std::size_t i = 2; i < elems.size() - 1; i++) {
		// TRANSLATORS: Formats successive elements of a conjunctive list.
		prefix = VGETTEXT("conjunct mid^$prefix, $next", {{"prefix", prefix}, {"next", elems[i]}});
	}
	// TRANSLATORS: Formats the final element of a conjunctive list.
	return VGETTEXT("conjunct end^$prefix, and $last", {{"prefix", prefix}, {"last", elems.back()}});
}

std::string format_disjunct_list(const t_string& empty, const std::vector<t_string>& elems) {
	switch(elems.size()) {
	case 0: return empty;
	case 1: return elems[0];
		// TRANSLATORS: Formats a two-element disjunctive list.
	case 2: return VGETTEXT("disjunct pair^$first or $second", {{"first", elems[0]}, {"second", elems[1]}});
	}
	// TRANSLATORS: Formats the first two elements of a disjunctive list.
	std::string prefix = VGETTEXT("disjunct start^$first, $second", {{"first", elems[0]}, {"second", elems[1]}});
	// For size=3 this loop is not entered
	for(std::size_t i = 2; i < elems.size() - 1; i++) {
		// TRANSLATORS: Formats successive elements of a disjunctive list.
		prefix = VGETTEXT("disjunct mid^$prefix, $next", {{"prefix", prefix}, {"next", elems[i]}});
	}
	// TRANSLATORS: Formats the final element of a disjunctive list.
	return VGETTEXT("disjunct end^$prefix, or $last", {{"prefix", prefix}, {"last", elems.back()}});
}

}

std::string vgettext_impl(const char *domain
		, const char *msgid
		, const utils::string_map& symbols)
{
	const std::string orig(translation::dsgettext(domain, msgid));
	const std::string msg = utils::interpolate_variables_into_string(orig, &symbols);
	return msg;
}

std::string vngettext_impl(const char* domain,
		const char* singular,
		const char* plural,
		int count,
		const utils::string_map& symbols)
{
	const std::string orig(translation::dsngettext(domain, singular, plural, count));
	const std::string msg = utils::interpolate_variables_into_string(orig, &symbols);
	return msg;
}

[[nodiscard]] std::size_t edit_distance_approx(std::string_view str_1, std::string_view str_2) noexcept
{
	// First, trim prefixes
	auto s1_first = str_1.begin();
	auto s2_first = str_2.begin();

	while(s1_first != str_1.end() && s2_first != str_2.end() && *s1_first == *s2_first) {
		++s1_first;
		++s2_first;
	}

	// Then, trim suffixes
	auto s1_size = static_cast<std::size_t>(str_1.end() - s1_first);
	auto s2_size = static_cast<std::size_t>(str_2.end() - s2_first);

	while(s1_size != 0 && s2_size != 0 && s1_first[s1_size - 1] == s2_first[s2_size - 1]) {
		--s1_size;
		--s2_size;
	}

	if(s1_size == 0) {
		return s2_size;
	}

	if(s2_size == 0) {
		return s1_size;
	}

	// Limit the relevant characters to no more than 15
	s1_size = std::min(s1_size, std::size_t{15});
	s2_size = std::min(s2_size, std::size_t{15});

	if(s1_size < s2_size) {
		std::swap(s1_first, s2_first);
		std::swap(s1_size, s2_size);
	}

	// This is an 'optimal string alignment distance' algorithm
	// (https://en.wikipedia.org/wiki/Damerau%E2%80%93Levenshtein_distance#Optimal_string_alignment_distance)
	// with some optimizations. Two variables are used to track the previous row instead of using another array.
	// `up` handles deletion, `row[j]` handles insertion, and `upper_left` handles substitution.

	// This is a single row of the matrix
	std::array<std::size_t, 16> row{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

	for(std::size_t i = 0; i != s1_size; ++i) {
		auto upper_left = i;
		row[0] = i + 1;

		for(std::size_t j = 0; j != s2_size; ++j) {
			const auto up = row[j + 1];
			const bool transposed = i > 0 && j > 0 && s1_first[i] == s2_first[j - 1] && s1_first[i - 1] == s2_first[j];

			if(s1_first[i] != s2_first[j] && !transposed) {
				row[j + 1] = std::min({up, row[j], upper_left}) + 1;
			} else {
				row[j + 1] = upper_left;
			}

			// When moving to the next element of a row, the previous `up` element is now the `upper_left`
			upper_left = up;
		}
	}

	return row[s2_size];
}
