/*
   Copyright (C) 2015 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

// Comment out this line to avoid using boost demangle on non-msvc compilers.
//#define USE_BOOST_CORE_DEMANGLE

#include <cstdio>

#ifdef _MSC_VER

// This code based on stackwalker routine from Anura, github.com/anura-engine/anura/, retrieved April 2, 2015

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "StackWalker.h"

class StderrStackWalker : public StackWalker
{
public:
	StderrStackWalker() : StackWalker(RetrieveVerbose) {}
protected:
	virtual void OnOutput(LPCSTR szText)
	{
		//SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s\n", szText);
		//StackWalker::OnOutput(szText);
		fprintf( stderr, "%s\n", szText);
	}
};

void printBacktrace() {
	StderrStackWalker sw; 
	sw.ShowCallstack();
}






#else // _MSC_VER = false






// This code based on stack trace code provided by Ignacio R. Morelle

#include <csignal>
#include <exception>
#include <iostream>
#include <sstream>

#include <execinfo.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef USE_BOOST_CORE_DEMANGLE
#include <boost/core/demangle.hpp>
#else
#include <cxxabi.h>
#endif

#ifndef HAVE_CXX11
#define nullptr NULL
#endif

const char* const indent = "    ";
const int max_frames = 10;

void backtrace_to_stderr(char** trace, int frames, bool demangle = true)
{
	if (!trace) {
		return;
	}

	if (!demangle) {
		for (int k = 0; k < frames; ++k) {
			std::cerr << indent << trace[k] << '\n';
		}

		return;
	}

	//
	// See <http://stackoverflow.com/a/2526298>.
	//
	for (int k = 0; k < frames; ++k) {
		char* mangled = nullptr;
		char* off_begin = nullptr;
		char* off_end = nullptr;

		// Find parentheses and +address offset surrounding mangled name.
		for (char* p = trace[k]; *p; ++p) {
			switch (*p)
			{
				case '(':
					mangled = p;
					break;
				case '+':
					off_begin = p;
					break;
				case ')':
					off_end = p;
					break;
			}
		}

		if (mangled && off_begin && off_end && mangled < off_begin) {
			*mangled++ = *off_begin++ = *off_end++ = '\0';
#ifdef USE_BOOST_CORE_DEMANGLE
			std::cerr << indent << trace[k] << ": "
					  << boost::core::demangle(mangled)
					  << "+" << off_begin << off_end << '\n';
#else // USE_BOOST_CORE_DEMANGLE
			int status = -1;
			char* demangled = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
			std::cerr << indent << trace[k] << ": "
					  << (status == 0 ? demangled : mangled)
					  << "+" << off_begin << off_end << '\n';
			free(demangled);
#endif // USE_BOOST_CORE_DEMANGLE
		}
	}
}

#ifndef HAVE_CXX11
#undef nullptr
#endif

void printBacktrace()
{
	void* buf[max_frames];
	const int frames = backtrace(buf, max_frames);

	std::cerr << "*** Backtrace (last " << frames << " frames):\n";

	char** const trace = backtrace_symbols(buf, frames);
	backtrace_to_stderr(trace, frames);
	free(trace);
}

#endif // _MSC_VER
