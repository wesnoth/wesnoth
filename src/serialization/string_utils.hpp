/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef SERIALIZATION_STRING_UTILS_HPP_INCLUDED
#define SERIALIZATION_STRING_UTILS_HPP_INCLUDED

#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace utils {

bool isnewline(char c);
bool portable_isspace(char c);
bool notspace(char c);

// REMOVE_EMPTY : remove empty elements
// STRIP_SPACES : strips leading and trailing blank spaces
enum { REMOVE_EMPTY = 0x01, STRIP_SPACES = 0x02 };
std::vector< std::string > split(std::string const &val, char c = ',', int flags = REMOVE_EMPTY | STRIP_SPACES);
std::string join(std::vector< std::string > const &v, char c = ',');
std::vector< std::string > quoted_split(std::string const &val, char c= ',',
                                        int flags = REMOVE_EMPTY | STRIP_SPACES, char quote = '\\');
std::pair< int, int > parse_range(std::string const &str);
bool notspace(char c);
std::string &escape(std::string &str);
std::string &unescape(std::string &str);
std::string &strip(std::string &str);
bool has_value(std::string const &values, std::string const &val);

typedef std::map< std::string, std::string > string_map;
// function which will interpolate variables, starting with '$' in the string 'str' with
// the equivalent symbols in the given symbol table. If 'symbols' is NULL, then game event
// variables will be used instead
std::string interpolate_variables_into_string(std::string const &str, string_map const *symbols = NULL);

}

#endif
