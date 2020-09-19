/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

struct lua_State;
class lua_kernel_base;
class vconfig;
class game_data;
class game_state;

namespace lua_widget {

int impl_widget_get(lua_State* L);
int impl_widget_set(lua_State* L);

} // end namespace lua_gui2
