/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Thomas Baumhauer <thomas.baumhauer@NOSPAMgmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef HASH_HPP_INCLUDED
#define HASH_HPP_INCLUDED

#include <string>

namespace util {

unsigned char* md5(const std::string& input);
int get_iteration_count(const std::string& hash);
std::string get_salt(const std::string& hash);
bool is_valid_hash(const std::string& hash);
std::string encode_hash(unsigned char* input);
std::string create_hash(const std::string& password, const std::string& salt, int iteration_count =10);
bool password_matches_hash(const std::string& password, const std::string& hash);

} // namespace util

#endif // HASH_HPP_INCLUDED
