/*
   Copyright (C) 2008 by David White <dave@whitevine.net>
                 2008 - 2014 by Richard Kettering <kettering.richard@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


#ifndef ASSERTS_HPP_INCLUDED
#define ASSERTS_HPP_INCLUDED

#include <cstdlib>
#include <iostream>
#include <csignal>

#ifdef _MSC_VER
#define BREAKPOINT() __debugbreak()
#define WES_HALT() do { BREAKPOINT(); exit(1); } while (false)

#elif defined(__GNUG__) && (defined(__i386__) || defined(__x86_64__)) \
  && !defined(__native_client__)
#define BREAKPOINT() asm("int3")
#define WES_HALT() do { BREAKPOINT(); abort(); } while (false)

#elif defined(SIGTRAP)
#define BREAKPOINT() do{ ::std::raise(SIGTRAP); } while (0)
#define WES_HALT() do { BREAKPOINT(); abort(); } while (false)

#else
#define BREAKPOINT()
#define WES_HALT() abort()
#endif

#define ERROR_LOG(a) do { \
	std::cerr << __FILE__ << ":" << __LINE__ << " ASSSERTION FAILED: " << a << std::endl; \
	WES_HALT(); \
	} while (false)

//for custom logging.  Example usage:
//ASSERT_LOG(x != y, "x not equal to y. Value of x: " << x << ", y: " << y);
#define ASSERT_LOG(a,b) if (!(a)) { ERROR_LOG(b); } else (void)0

#define FATAL_ERROR ERROR_LOG("FATAL ERROR")

/**
 * Marker for code that should be unreachable.
 *
 * This can be used to avoid compiler warnings and to detect logic errors in the code.
 */
#define UNREACHABLE_CODE ERROR_LOG("REACHED UNREACHABLE CODE")

//helper macro for the simple operator cases defined below
#define ASSERT_OP(a,op,b) ASSERT_LOG((a) op (b), #a " " #op " " #b " (" << (a) << " " #op " " << (b) << ")")

//various asserts of standard "equality" tests, such as "equals", "not equals", "greater than", etc.
//Example usage ASSERT_GE(x, y);
//on failure this will cerr "assertion failed: x >= y (value_of_x >= value_of_y)"
#define ASSERT_EQ(a,b) ASSERT_OP(a,==,b)
#define ASSERT_NE(a,b) ASSERT_OP(a,!=,b)
#define ASSERT_GE(a,b) ASSERT_OP(a,>=,b)
#define ASSERT_LE(a,b) ASSERT_OP(a,<=,b)
#define ASSERT_GT(a,b) ASSERT_OP(a,>,b)
#define ASSERT_LT(a,b) ASSERT_OP(a,<,b)


#endif

