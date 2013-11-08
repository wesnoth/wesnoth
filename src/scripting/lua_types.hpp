/*
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

typedef void* luatype;

// i dont want to cast to void* each time ....
// a drawback is, that these are now normal static variables wich are initialised at initialisation time (so you shoudn't use these at/before initialisation time).



extern luatype const dlgclbkKey;
extern luatype const executeKey;
extern luatype const getsideKey;
extern luatype const gettextKey;
extern luatype const gettypeKey;
extern luatype const getraceKey;
extern luatype const getunitKey;
extern luatype const tstringKey;
extern luatype const unitvarKey;
extern luatype const ustatusKey;
extern luatype const vconfigKey;