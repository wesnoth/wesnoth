/*
	Copyright (C) 2016 - 2024
	Gregory A Lundberg
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#ifndef WESNOTH_LUA_CONFIG_H_INCLUDED
#define WESNOTH_LUA_CONFIG_H_INCLUDED

/*
**  Wesnoth Lua Configuration
**
**  If at all possible, Wesnoth should use the stock, unmodified Lua source
**  kit.
**
**  This file contains the cross-platform compile-time changes. Platform
**  maintainers should consult wesnoth_lua_config.md for guidance on building
**  Lua for Wesnoth on their platform using their toolset.
**
**  Lua comes with a number of configuration options, such as backward
**  compatibility with earlier versions. In the original source kit, the
**  intention is these be set as compile-time options. For Wesnoth, place
**  them here to better ensure consistency.
**
**  In addition, there are specific non-standard changes which Wesnoth
**  requires. Those changes, if at all possible, should appear here. This
**  will reduce (hopefully, actually eliminate) any changes to the Lua
**  source kit. If that is not possible, be sure to describe the changes
**  needed, including a brief rationale and commit hashes, in
**  wesnoth_lua_config.md
*/



/*
**  Standard Lua options.
*/

/*  Wesnoth definitions for compatibility
 *  No longer needed in C++, included only for Lua runtime.
 */
#define LUA_COMPAT_5_2
#define LUA_COMPAT_5_1
#define LUA_COMPAT_FLOATSTRING



/*
**  Wesnoth-specific modifications.
*/

/*  We canNOT use strcoll on Windows!
 *
 *  All strings in Wesnoth are UTF-8 encoded. On Windows, strcoll assumes
 *  strings are UTF-16LE encoded; using strcoll will cause the strings to
 *  collate in a different order than on all other targets. This can cause
 *  OOS errors during networked multi-player games.
 */

#include <string.h>
#define strcoll(a,b) strcmp(a,b)

/*  We need to rethrow exceptions.
 *
 *  The stock Lua catch(...) consumes the exception. We need to re-throw
 *  it so. This allows the inner function (in C++ -> Lua -> C++) to pass
 *  back information about the exception, instead of reclassifying all
 *  exceptions to a single Lua status code.
 */

#include <cassert>
#include <exception>

#include "lua_jailbreak_exception.hpp"

#define LUAI_THROW(L,c) throw(c)

#define LUAI_TRY(L,c,a) \
	try { \
		try { \
			a \
		} catch(const lua_jailbreak_exception &e) { \
			e.store(); \
			throw; \
		} catch(const std::exception &e) { \
			lua_pushstring(L, e.what()); \
			luaG_errormsg(L); \
			throw; \
		} catch (const lua_longjmp *) { \
			/*this exception is used internaly by lua exceptions*/ \
			throw; \
		} catch(...) { \
			assert(false && "Lua is swallowing an un-named exception... this indicates a programmer error, please derive all exceptions from either std::exception, or lua_jailbreak_exception (and not with multiple inheritance pathways to either or this exception handler will not work!)"); \
			throw; \
		} \
	} catch(...) { \
	if((c)->status == 0) \
		(c)->status = -1;\
	}

#define luai_jmpbuf     int  /* dummy variable */

#endif
