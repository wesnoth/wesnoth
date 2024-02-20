/*
	Copyright (C) 2009 - 2024
	by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

#include "scripting/lua_ptr.hpp"
#include "serialization/string_utils.hpp"

namespace gui2 {
   class widget;
   class window;
}
struct lua_State;

namespace lua_widget {
	void register_metatable(lua_State* L);
}

void luaW_pushwidget(lua_State* L, gui2::widget& w);
bool luaW_iswidget(lua_State* L, int idx);
gui2::widget& luaW_checkwidget(lua_State* L, int idx);
lua_ptr<gui2::widget>& luaW_checkwidget_ptr(lua_State* L, int idx);



/** [-0, +1, -] */
void luaW_pushwindowtable(lua_State* L,  gui2::window* owner);
/** [-0, +0, -] */
void luaW_clearwindowtable(lua_State* L, gui2::window* owner);
/** [-0, +1, -] */
void luaW_pushwidgettable(lua_State* L, gui2::widget* wg, gui2::window* owner);
/** returns true if a callback already existed. [-1, +0, -] */
bool luaW_setwidgetcallback(lua_State* L, gui2::widget* wg, gui2::window* owner, std::string_view name);
/** pushed ther callback function onoto the stack [-0, +1, -] */
void luaW_getwidgetcallback(lua_State* L, gui2::widget* wg, gui2::window* owner, std::string_view name);
/** callas a widgets callback [-0, +0, e] */
void luaW_callwidgetcallback(lua_State* L, gui2::widget* wg, gui2::window* owner, std::string_view name);
