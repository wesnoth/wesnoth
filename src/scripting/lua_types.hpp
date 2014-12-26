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

#ifndef SCRIPTING_LUA_TYPES_HPP
#define SCRIPTING_LUA_TYPES_HPP

typedef void* luatypekey;

// i dont want to cast to void* each time ....
// a drawback is, that these are now normal static variables wich are initialised at initialisation time (so you shoudn't use these at/before initialisation time).
extern luatypekey const executeKey;
extern luatypekey const getsideKey;
extern luatypekey const gettypeKey;
extern luatypekey const getraceKey;
extern luatypekey const getunitKey;
extern luatypekey const unitvarKey;
extern luatypekey const ustatusKey;
extern luatypekey const currentscriptKey;

#endif
