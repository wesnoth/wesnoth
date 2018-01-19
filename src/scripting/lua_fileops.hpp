/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once
/**
 * This namespace contains the implementations for wesnoth's
 * safe fileops for lua.
 */

struct lua_State;

namespace lua_fileops {

int intf_have_file(lua_State*);
int intf_read_file(lua_State*);
int load_file(lua_State*);

} // end namespace lua_fileops
