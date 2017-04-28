/*
   Copyright (C) 2009 - 2017 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifdef DEBUG_LUA

#ifndef DEBUG_LUA_HPP_INCLUDED
#define DEBUG_LUA_HPP_INCLUDED

#include "lua/lualib.h"


void ds(lua_State *L, const bool verbose_table = true);

#endif

#endif
