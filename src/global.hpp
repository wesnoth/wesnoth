/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GLOBAL_HPP_INCLUDED
#define GLOBAL_HPP_INCLUDED

#ifdef _MSC_VER

#undef snprintf
#define snprintf _snprintf

// Disable warning about source encoding not in current code page.
#pragma warning(disable: 4819)

// Disable warning about deprecated functions.
#pragma warning(disable: 4996)

//disable some MSVC warnings which are useless according to mordante
#pragma warning(disable: 4244)
#pragma warning(disable: 4345)
#pragma warning(disable: 4250)
#pragma warning(disable: 4355)
#pragma warning(disable: 4351)

#endif //_MSC_VER

/**
 * Enable C++11 support in some parts of the code.
 *
 * These parts \em must  also work without C++11, since Wesnoth still uses C++98
 * as the official C++ version.
 *
 * @note Older versions of GCC don't define the proper version for
 * @c __cplusplus,  but have their own test macro. That test is omitted since
 * the amount of support for these compilers depends a lot on the exact
 * compiler version. If you want to enable it for these compilers simply define
 * the macro manually.
 */
#if (__cplusplus >= 201103L)
#define HAVE_CXX11
#endif

#ifdef HAVE_CXX11
#define FINAL final
#define OVERRIDE override
#else
#define FINAL
#define OVERRIDE
#endif

#ifdef NDEBUG
/*
 * Wesnoth uses asserts to avoid undefined behaviour. For example, to make sure
 * pointers are not NULL before deferring them, or collections are not empty
 * before accessing their elements. Therefore Wesnoth should not be compiled
 * with assertions disabled.
 */
#error "Compilation with NDEBUG defined isn't supported, Wesnoth depends on asserts."
#endif

#endif //GLOBAL_HPP_INCLUDED
