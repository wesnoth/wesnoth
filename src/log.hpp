/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef LOG_HPP_INCLUDED
#define LOG_HPP_INCLUDED

#define LOG_DATA

#ifdef LOG_DATA

#include <iostream>
#include <string>

#include "SDL.h"

struct scope_logger
{
	scope_logger(const std::string& str) : ticks_(SDL_GetTicks()), str_(str) {
		do_indent();
		std::cerr << "BEGIN: " << str_ << "\n";
		do_indent();
		++indent;
	}

	~scope_logger() {
		const int ticks = SDL_GetTicks() - ticks_;
		--indent;
		do_indent();
		do_indent();
		std::cerr << "END: " << str_ << " (took " << ticks << "ms)\n";
	}

	void do_indent()
	{
		for(int i = 0; i != indent; ++i)
			std::cerr << "  ";
	}

private:
	int ticks_;
	std::string str_;
	static int indent;
};

#define log_data0(a) std::cerr << a << "\n";
#define log_data1(a,b) std::cerr << a << " info: " << b << "\n";
#define log_data2(a,b,c) std::cerr << a << " info: " << b << ", " << c << "\n";

#define log_scope(a) scope_logger scope_logging_object__(a);

#else
#define log_data0(a)
#define log_data1(a,b)
#define log_data2(a,b,c)

#define log_scope(a)
#endif

#endif
