/* $Id$ */
/*
   Copyright (C) 2005 - 2008 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file util.cpp
//! String-routines - Templates for lexical_cast & lexical_cast_default.

#include "util.hpp"
#include <cstdlib>

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

