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
**  Wesnoth must use the stock, unmodified Lua source kit, to remain compatible
**  with using system copies of Lua.  Please avoid making incompatible changes
**  here.
*/



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

/*  Push std::exception::what() strings onto the Lua stack for use by
 *  luaW_pcall().
 *
 *  LUAI_THROW() and LUAI_TRY() are internal Lua macros that may not always
 *  exist and can't be defined for system copies of Lua, so don't rely on them
 *  for anything more important.
 */

#include <cassert>
#include <exception>

#include "lua_jailbreak_exception.hpp"

#define LUAI_THROW(L,c) throw(c)

#define LUAI_TRY(L,c,a) \
	try { \
		try { \
			a \
		} catch(const lua_jailbreak_exception &) { \
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
