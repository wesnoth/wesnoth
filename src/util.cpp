/* $Id$ */
/*
   Copyright (C) 2005 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "util.hpp"
#include <cstdlib>

// Remove tags from begining of string (it creates new string, so it's safe to call)
std::string del_tags(std::string name){
	std::stringstream str;
	bool colour_def = false;
	bool not_name = true;
	std::string::const_iterator it;

	for (it = name.begin(); it != name.end(); it++){
		// On the first analphabet character we stop react on specials characters
		if (not_name && isalpha(*it)){
			not_name = false;
			str 	<< *it;
			continue;
		}
		// Start of RGB definition block, so stop react on numbers
		if (not_name && *it == '<'){
			colour_def = true;
			continue;
		}
		// Ending of RGB block
		if (*it == '>'){
			colour_def = false;
			continue;
		}
		// Number outside colour block
		if (not_name && !colour_def && isdigit(*it)){
			not_name = false;
		} 
						
		str << *it;
	}
	return str.str();
}

template<>
int lexical_cast<int, const std::string&>(const std::string& a)
{
	char* endptr;
	int res = strtol(a.c_str(), &endptr, 10);

	if (a.empty() || *endptr != '\0') {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}

template<>
int lexical_cast<int, const char*>(const char* a)
{
	char* endptr;
	int res = strtol(a, &endptr, 10);

	if (*a == '\0' || *endptr != '\0') {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}

template<>
int lexical_cast_default<int, const std::string&>(const std::string& a, int def)
{
	if(a.empty()) {
		return def;
	}

	char* endptr;
	int res = strtol(a.c_str(), &endptr, 10);

	if (*endptr != '\0') {
		return def;
	} else {
		return res;
	}
}

template<>
int lexical_cast_default<int, const char*>(const char* a, int def)
{
	if(*a == '\0') {
		return def;
	}

	char* endptr;
	int res = strtol(a, &endptr, 10);

	if (*endptr != '\0') {
		return def;
	} else {
		return res;
	}
}

template<>
double lexical_cast_default<double, const std::string&>(const std::string& a, double def)
{
	if(a.empty()) {
		return def;
	}

	char* endptr;
	double res = strtod(a.c_str(), &endptr);

	if (*endptr != '\0') {
		return def;
	} else {
		return res;
	}
}

