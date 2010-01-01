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

}

/** Handy wrappers around interpolate_variables_into_string and gettext. */
std::string vgettext(const char*, const utils::string_map&);
std::string vngettext(const char*, const char*, int, const utils::string_map&);

#endif
