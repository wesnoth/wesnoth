/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef LUA_GUI2_DEFINED
#define LUA_GUI2_DEFINED

struct lua_State;

int intf_set_dialog_value(lua_State *L);
int intf_get_dialog_value(lua_State *L);
int intf_set_dialog_callback(lua_State *L);
int intf_set_dialog_markup(lua_State *L);
int intf_set_dialog_canvas(lua_State *L);
int intf_set_dialog_active(lua_State *L);
int intf_show_dialog(lua_State *L);

#endif
