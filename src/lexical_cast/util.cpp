/* $Id$ */
/*
   Copyright (C) 2005 - 2009 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file util.cpp
 *  String-routines - Templates for lexical_cast & lexical_cast_default.
 */

// g++ -O2 -o util util.cpp && ./util
#define LOG
//#define THROW

#include "./util.hpp"

#include <cstdlib>

template<>
int lexical_cast<int, const std::string&>(const std::string& a)
{
#ifdef LOG
	std::cerr << "Lexical cast specialized int - string\n";
#endif
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
#ifdef LOG
	std::cerr << "Lexical cast specialized int - char*\n";
#endif
	char* endptr;
	int res = strtol(a, &endptr, 10);

	if (*a == '\0' || *endptr != '\0') {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}

int main()
{
#ifdef THROW
	try {
		std::cerr << NEW_lexical_cast<std::string>(1) << '\n';
	} catch(const std::string& type) {
		std::cerr << type << '\n';
	}

	return 0;
#endif

#ifndef LOG
	for(size_t i = 0; i != 1000000; ++i) {
#if 0
		lexical_cast<std::string>(1);
#else
		NEW_lexical_cast<std::string>(1);
#endif
	}

#else

	std::string foo = "1";
	const std::string& bar = "1";
	char foobar[] = "a";
	foobar[0] = '1';

	std::cerr << "Original\n";
	std::cerr << lexical_cast<int>(foo) << '\n';
	std::cerr << lexical_cast<int>(bar) << '\n';
	std::cerr << lexical_cast<int>("1") << '\n';
	std::cerr << lexical_cast<int>(foobar) << '\n';
	std::cerr << lexical_cast<short>(foo) << '\n';
	std::cerr << lexical_cast<short>(bar) << '\n';
	std::cerr << lexical_cast<short>("1") << '\n';
	std::cerr << lexical_cast<short>(foobar) << '\n';
	std::cerr << lexical_cast<unsigned>(foo) << '\n';
	std::cerr << lexical_cast<unsigned>(bar) << '\n';
	std::cerr << lexical_cast<unsigned>("1") << '\n';
	std::cerr << lexical_cast<unsigned>(foobar) << '\n';
	std::cerr << lexical_cast<std::string>(1) << '\n';
	std::cerr << lexical_cast<std::string>(1u) << '\n';

	std::cerr << "\nNew\n";
	std::cerr << NEW_lexical_cast<int>(foo) << '\n';
	std::cerr << NEW_lexical_cast<int>(bar) << '\n';
	std::cerr << NEW_lexical_cast<int>("1") << '\n';
	std::cerr << NEW_lexical_cast<int>(foobar) << '\n';
	std::cerr << NEW_lexical_cast<short>(foo) << '\n';
	std::cerr << NEW_lexical_cast<short>(bar) << '\n';
	std::cerr << NEW_lexical_cast<short>("1") << '\n';
	std::cerr << NEW_lexical_cast<short>(foobar) << '\n';
	std::cerr << NEW_lexical_cast<unsigned>(foo) << '\n';
	std::cerr << NEW_lexical_cast<unsigned>(bar) << '\n';
	std::cerr << NEW_lexical_cast<unsigned>("1") << '\n';
	std::cerr << NEW_lexical_cast<unsigned>(foobar) << '\n';
	std::cerr << NEW_lexical_cast<std::string>(1) << '\n';
	std::cerr << NEW_lexical_cast<std::string>(1u) << '\n';
#endif
}
