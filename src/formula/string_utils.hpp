/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2016 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FORMULA_STRING_UTILS_HPP_INCLUDED
#define FORMULA_STRING_UTILS_HPP_INCLUDED

#include "serialization/string_utils.hpp"
#include <boost/assign.hpp>

class variable_set;

namespace utils {

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
std::string interpolate_variables_into_string(const std::string &str, const string_map * const symbols);
std::string interpolate_variables_into_string(const std::string &str, const variable_set& variables);

/**
 * Function that does the same as the above, for t_stringS.
 * If a change was made, then the t_string's translation is done in this
 * function, rather than at use. This limitation is due to the use of
 * create-time interpolation, rather than use-time.
 */
t_string interpolate_variables_into_tstring(const t_string &str, const variable_set& variables);

}
/// An alias for boost::assign::map_list_of<std::string, std::string>
inline boost::assign_detail::generic_list< std::pair
        <
            boost::assign_detail::assign_decay<std::string>::type,
            boost::assign_detail::assign_decay<std::string>::type
        > >
string_map_of(const std::string& k, const std::string& v)
{
	return boost::assign::map_list_of<std::string, std::string>(k, v);
}

/** Handy wrappers around interpolate_variables_into_string and gettext. */
std::string vgettext(const char* msgid, const utils::string_map& symbols);
std::string vgettext(const char* domain
		, const char* msgid
		, const utils::string_map& symbols);

std::string vngettext(const char*, const char*, int, const utils::string_map&);

/**
 * @todo Convert all functions.
 *
 * All function in this file should have an overloaded version with a domain
 * and probably convert all callers to use the macro instead of directly calling
 * the function.
 */

#ifdef GETTEXT_DOMAIN
#define	VGETTEXT(msgid, symbols) vgettext(GETTEXT_DOMAIN, msgid, symbols)
#else
#define	VGETTEXT(msgid, symbols) vgettext(msgid, symbols)
#endif

#endif
