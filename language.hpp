/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef LANGUAGE_HPP_INCLUDED
#define LANGUAGE_HPP_INCLUDED

#include "config.hpp"

#include <map>
#include <string>
#include <vector>

extern std::map<std::string,std::string> string_table;

std::vector<std::string> get_languages(config& cfg);
bool set_language(const std::string& locale, config& cfg);

std::string get_locale();

#endif
