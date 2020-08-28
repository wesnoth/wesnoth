/*
   Copyright (C) 2009 - 2018 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

#include <string>

#include "units/attack_type.hpp"

namespace gui2 {
   class widget;
}
struct lua_State;

namespace lua_widget {
	void register_metatable(lua_State* L);
}

void luaW_pushwidget(lua_State* L, gui2::widget& w);
bool luaW_iswidget(lua_State* L, int idx);
gui2::widget& luaW_checkwidget(lua_State* L, int idx);
gui2::widget* luaW_checkwidget_ptr(lua_State* L, int idx);
