#ifndef WESNOTH_LUA_CONFIG_H_INCLUDED
#define WESNOTH_LUA_CONFIG_H_INCLUDED

/*
   Copyright (C) 2016 Gregory A Lundberg
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/*
**  Wesnoth Lua Configuration
**
**  If at all possible, Wesnoth should use the stock, unmodified Lua source
**  kit.
**
**  This file contains the cross-platform compile-time changes. Platform
**  maintainers should consult wesnothluaconfig.md for guidance on building
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

/*
**  Wesnoth-specific modifications.
*/

/*  We can NOT use strcoll on Windows!
 *
 *  All strings in Wesnoth are UTF-8 encoded. On Windows, strcoll assumes
 *  strings are UTF-16LE encoded; using strcoll will cause the strings to
 *  collate in a different order than on all other targets. This can cause
 *  OOS errors during networked multi-player games.
 */

#include <string.h>
#define strcoll(a,b) strcmp(a,b)

#endif
