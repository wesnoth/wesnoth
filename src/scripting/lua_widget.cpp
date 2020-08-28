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

#include "scripting/lua_widget.hpp"
#include "scripting/lua_widget_attributes.hpp"

#include "scripting/lua_common.hpp"
#include "scripting/lua_unit.hpp"
#include "scripting/lua_unit_type.hpp"
#include "scripting/lua_ptr.hpp"
#include "scripting/push_check.hpp"

#include "gui/widgets/widget.hpp"

#include "lua/lauxlib.h"
#include "lua/lua.h"                    // for lua_State, lua_settop, etc

#include <type_traits>

static const char widgetKey[] = "widget";

void luaW_pushwidget(lua_State* L, gui2::widget& w)
{
	new(L) lua_ptr<gui2::widget>(w);
	luaL_setmetatable(L, widgetKey);
}

gui2::widget& luaW_checkwidget(lua_State* L, int n)
{
	lua_ptr<gui2::widget>& lp =  *static_cast<lua_ptr<gui2::widget>*>(luaL_checkudata(L, n, widgetKey));
	auto ptr = lp.get_ptr();
	if(!ptr) {
		luaL_argerror(L, n, "widget was deleted");
	}
	return *ptr;
}


bool luaW_iswidget(lua_State* L, int index)
{
	return luaL_testudata(L, index, widgetKey) != nullptr;
}

static int impl_widget_collect(lua_State* L)
{
	lua_ptr<gui2::widget>* w = static_cast<lua_ptr<gui2::widget>*>(luaL_checkudata(L, 1, widgetKey));
	w->~lua_ptr<gui2::widget>();
	return 0;
}


namespace lua_widget {
	void register_metatable(lua_State* L)
	{

		luaL_newmetatable(L, widgetKey);
		lua_pushcfunction(L, lua_widget::impl_widget_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, lua_widget::impl_widget_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushcfunction(L, impl_widget_collect);
		lua_setfield(L, -2, "__gc");
		lua_pushstring(L, widgetKey);
		lua_setfield(L, -2, "__metatable");
	}
}
