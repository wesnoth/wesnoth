/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2010 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FORMULA_STRING_UTILS_HPP_INCLUDED
#define FORMULA_STRING_UTILS_HPP_INCLUDED

#include "serialization/string_utils.hpp"

namespace utils {

/**
 * Function which will interpolate variables, starting with '$' in the string
 * 'str' with the equivalent symbols in the given symbol table. If 'symbols'
 * is NULL, then game event variables will be used instead.
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
