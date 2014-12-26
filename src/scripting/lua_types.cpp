/*
   Copyright (C) 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "lua_types.hpp"

/* Dummy pointer for getting unique keys for Lua's registry. */
static char const v_executeKey = 0;
static char const v_gettypeKey = 0;
static char const v_getraceKey = 0;
static char const v_getunitKey = 0;
static char const v_unitvarKey = 0;
static char const v_ustatusKey = 0;


luatypekey const executeKey = static_cast<void *>(const_cast<char *>(&v_executeKey));
luatypekey const gettypeKey = static_cast<void *>(const_cast<char *>(&v_gettypeKey));
luatypekey const getraceKey = static_cast<void *>(const_cast<char *>(&v_getraceKey));
luatypekey const getunitKey = static_cast<void *>(const_cast<char *>(&v_getunitKey));
luatypekey const unitvarKey = static_cast<void *>(const_cast<char *>(&v_unitvarKey));
luatypekey const ustatusKey = static_cast<void *>(const_cast<char *>(&v_ustatusKey));
