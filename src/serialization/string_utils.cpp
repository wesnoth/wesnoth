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

#include <cctype>

#include "serialization/string_utils.hpp"

namespace game_events {
std::string const &get_variable_const(std::string const &varname);
}

namespace {

bool not_id(char c)
{
	return !isdigit(c) && !isalpha(c) && c != '.' && c != '_';
}

void do_interpolation(std::string &res, size_t npos, utils::string_map const *m)
{
	const std::string::iterator i = std::find(res.begin() + npos, res.end(), '$');
	if (i == res.end() || i + 1 == res.end())
		return;

	npos = i - res.begin();

	const std::string::iterator end = std::find_if(i + 1, res.end(), not_id);

	const std::string key(i + 1, end);
	res.erase(i, end);

	if (m != NULL) {
		const utils::string_map::const_iterator itor = m->find(key);
		if (itor != m->end())
			res.insert(npos,itor->second);
	} else
		res.insert(npos, game_events::get_variable_const(key));

	do_interpolation(res,npos,m);
}

}

namespace utils {

bool isnewline(char c)
{
	return c == '\r' || c == '\n';
}

//make sure that we can use Mac, DOS, or Unix style text files on any system
//and they will work, by making sure the definition of whitespace is consistent
bool portable_isspace(char c)
{
	// returns true only on ASCII spaces
	if ((unsigned char)c >= 128)
		return false;
	return isnewline(c) || isspace(c);
}

//make sure we regard '\r' and '\n' as a space, since Mac, Unix, and DOS
//all consider these differently.
bool notspace(char c)
{
	return !portable_isspace(c);
}

std::string &strip(std::string &str)
{
	//if all the string contains is whitespace, then the whitespace may
	//have meaning, so don't strip it
	std::string::iterator it = std::find_if(str.begin(), str.end(), notspace);
	if (it == str.end())
		return str;

	str.erase(str.begin(), it);
	str.erase(std::find_if(str.rbegin(), str.rend(), notspace).base(), str.end());

	return str;
}

std::vector< std::string > split(std::string const &val, char c, int flags)
{
	std::vector< std::string > res;

	std::string::const_iterator i1 = val.begin();
	std::string::const_iterator i2 = val.begin();

	while (i2 != val.end()) {
		if (*i2 == c) {
			std::string new_val(i1, i2);
			if (flags & STRIP_SPACES)
				strip(new_val);
			if (!(flags & REMOVE_EMPTY) || !new_val.empty())
				res.push_back(new_val);
			++i2;
			if (flags & STRIP_SPACES) {
				while (i2 != val.end() && *i2 == ' ')
					++i2;
			}

			i1 = i2;
		} else {
			++i2;
		}
	}

	std::string new_val(i1, i2);
	if (flags & STRIP_SPACES)
		strip(new_val);
	if (!(flags & REMOVE_EMPTY) || !new_val.empty())
		res.push_back(new_val);

	return res;
}

std::string interpolate_variables_into_string(std::string const &str, string_map const *symbols)
{
	std::string res = str;
	do_interpolation(res, 0, symbols);

	//remove any pipes in the string, as they are used simply to seperate variables
	res.erase(std::remove(res.begin(),res.end(),'|'),res.end());

	return res;
}



}
