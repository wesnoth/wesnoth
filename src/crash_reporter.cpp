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

#include "crash_reporter.hpp"

#include "backtrace.hpp"

#include <stdio.h>
#include <signal.h>
#include <stdlib.h> // for exit, abort
#include <string.h> // for memset

#include <exception>

static void abortHandler(int);
static void terminateHandler();

namespace crash_reporter {

void register_handlers()
{
	fprintf(stderr, "Registering crash handlers...\n");

	// Register assert, segfault, illegal op code, and floating point errors to the abort handler.
	/*
	signal( SIGABRT, abortHandler );
	signal( SIGSEGV, abortHandler );
	signal( SIGILL,  abortHandler );
	signal( SIGFPE,  abortHandler );
	signal( SIGBUS,  abortHandler );
	*/

	// based on http://stackoverflow.com/questions/18606468/segmentation-fault-in-sigaction-signal-handler
	struct sigaction act_h;

	memset(&act_h, 0, sizeof(act_h));
	sigemptyset(&act_h.sa_mask);
        act_h.sa_handler = &abortHandler;

	bool good = true;
	good = good and (0 == sigaction(SIGINT, &act_h, 0));
	good = good and (0 == sigaction(SIGABRT, &act_h, 0));
	good = good and (0 == sigaction(SIGSEGV, &act_h, 0));
	good = good and (0 == sigaction(SIGILL, &act_h, 0));
	good = good and (0 == sigaction(SIGFPE, &act_h, 0));
	good = good and (0 == sigaction(SIGBUS, &act_h, 0));

	if (!good) {
		fprintf(stderr, "Failure when registering handlers.\n");
	} else {
		good = good and (0 == sigaction(SIGSEGV, 0, &act_h));
		if (!good) {
			fprintf(stderr, "Failed to register SIGSEGV handler.\n");
		} else {
			good = good and (act_h.sa_handler != NULL);
			if (!good) {
				fprintf(stderr, "Failed to find SIGSEGV handler after registering it.\n");
			}
		}
	}

	if (good) {
		fprintf(stderr, "Success.\n");
	}

	fprintf(stderr, "\n");

	// Register terminate calls (mainly halting caused by exceptions) to the terminate handler
	std::set_terminate( terminateHandler );
}

} // namespace crash_reporter

// This code based on source: http://oroboro.com/stack-trace-on-crash/ , retrieved April 2, 2015.
void abortHandler( int signum )
{
	// associate each signal with a signal name string.
	const char* name = NULL;
	switch( signum )
	{
	case SIGABRT: name = "SIGABRT";  break;
	case SIGSEGV: name = "SIGSEGV";  break;
	case SIGBUS:  name = "SIGBUS";   break;
	case SIGILL:  name = "SIGILL";   break;
	case SIGFPE:  name = "SIGFPE";   break;
	}

	// Notify the user which signal was caught. We use printf, because this is the 
	// most basic output function. Once you get a crash, it is possible that more 
	// complex output systems like streams and the like may be corrupted. So we 
	// make the most basic call possible to the lowest level, most 
	// standard print function.
	if ( name )
		fprintf( stderr, "Caught signal %d (%s)\n", signum, name );
	else
		fprintf( stderr, "Caught signal %d\n", signum );
 
	// Dump a stack trace, using whatever method we can from backtrace.hpp
	printBacktrace();
 
	// If you caught one of the above signals, it is likely you just 
	// want to quit your program right now.
	exit( signum );
}

// This code based on this blog post: https://akrzemi1.wordpress.com/2011/10/05/using-stdterminate/ retrieved April 2, 2015
void terminateHandler()
{
	fprintf( stderr, "Terminate handler called.\n");
	if (std::uncaught_exception()) {
		fprintf( stderr, "An exception was thrown:\n");

		#ifdef HAVE_CXX11
		if( auto exc = std::current_exception() ) { 
			try {
				rethrow_exception( exc );
			} catch (std::exception & e) {
				fprintf( stderr, e.what() );
			} catch (std::string & e) {
				fprintf( stderr, "(actually a std::string was thrown):\n")
				fprintf( stderr, e.c_str() );
			} catch (const char * e) {
				fprintf( stderr, "(actually a char * was thrown):\n")
				fprintf( stderr, e );
			} catch (...) {
				fprintf( stderr, "Something which is not an exception or a string was thrown.\n");
			}
		}
		#endif
	} else {
		fprintf( stderr, "No exception on the stack.\n");
	}

	// Dump a backtrace, using whatever method we can from backtrace.hpp
	printBacktrace();
	abort();
}
