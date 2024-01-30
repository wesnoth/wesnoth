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

#include "scripting/lua_widget.hpp"
#include "scripting/lua_widget_attributes.hpp"

#include "log.hpp"
#include "gui/widgets/widget.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_unit.hpp"
#include "scripting/lua_unit_type.hpp"
#include "scripting/lua_ptr.hpp"
#include "scripting/push_check.hpp"


#include "lua/wrapper_lauxlib.h"

#include <type_traits>

static const char widgetKey[] = "widget";
static char widgetdataKey[] = "widgetdata";

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

lua_ptr<gui2::widget>& luaW_checkwidget_ptr(lua_State* L, int n)
{
	lua_ptr<gui2::widget>& lp =  *static_cast<lua_ptr<gui2::widget>*>(luaL_checkudata(L, n, widgetKey));
	auto ptr = lp.get_ptr();
	if(!ptr) {
		luaL_argerror(L, n, "widget was deleted");
	}
	return lp;
}


bool luaW_iswidget(lua_State* L, int index)
{
	return luaL_testudata(L, index, widgetKey) != nullptr;
}


static void luaW_pushwidgettablecontainer(lua_State* L)
{
	lua_pushlightuserdata(L, &widgetdataKey[0]);
	lua_rawget(L, LUA_REGISTRYINDEX);
	if(lua_isnoneornil(L, -1)) {
		lua_pop(L, 1);
		lua_createtable(L, 0, 0);
		lua_pushlightuserdata(L, &widgetdataKey[0]);
		lua_pushvalue(L , -2);
		lua_rawset(L, LUA_REGISTRYINDEX);
	}
}

void luaW_pushwindowtable(lua_State* L,  gui2::window* owner)
{
	luaW_pushwidgettablecontainer(L);
	lua_pushlightuserdata(L, owner);
	lua_rawget(L, -2);
	if(lua_isnoneornil(L, -1))
	{
		//stack: windowstable, nil
		lua_pop(L, 1);
		//stack: windowstable
		lua_createtable(L, 1, 0);
		//stack: windowstable, {}
		lua_pushlightuserdata(L, owner);
		//stack: windowtable, {}, wg_id
		lua_pushvalue(L, -2);
		//stack: windowtable, {}, wg_id, {}
		lua_rawset(L, -4);
		//stack: windowtable, {}
	}
	lua_remove(L, lua_absindex(L, -2));
}

void luaW_clearwindowtable(lua_State* L, gui2::window* owner)
{
	luaW_pushwidgettablecontainer(L);
	lua_pushlightuserdata(L, owner);
	lua_pushnil(L);
	lua_rawset(L, -3);
	lua_pop(L, 1);
}


void luaW_pushwidgettable(lua_State* L, gui2::widget* wg, gui2::window* owner)
{
	luaW_pushwindowtable(L, owner);
	lua_pushlightuserdata(L, wg);
	lua_rawget(L, -2);
	if(lua_isnoneornil(L, -1))
	{
		//stack: windowtable, nil
		lua_pop(L, 1);
		//stack: windowtable
		lua_createtable(L, 1, 0);
		//stack: windowtable, {}
		luaW_pushwidget(L, *wg);
		//stack: windowtable, {}, wg
		lua_rawseti(L, -2, 1);
		//stack: windowtable, { wg},
		lua_pushlightuserdata(L, wg);
		//stack: windowtable, { wg}, wg_id
		lua_pushvalue(L, -2);
		//stack: windowtable, { wg}, wg_id, {wg}
		lua_rawset(L, -4);
		//stack: windowtable, { wg}
	}
	lua_remove(L, lua_absindex(L, -2));
}


bool luaW_setwidgetcallback(lua_State* L, gui2::widget* wg, gui2::window* owner, std::string_view name)
{
	//stack: function
	luaW_pushwidgettable(L, wg, owner);
	//stack: function, {}
	lua_push(L, name);
	//stack: function, {}, name
	lua_rawget(L, -2);
	// function, old_function
	bool existed_already = !lua_isnoneornil(L, -1);
	lua_pop(L, 1);
	// function,
	lua_push(L, name);
	//stack: function, {}, name
	lua_rotate(L, lua_absindex(L, -3), -1);
	//stack: {}, name, function
	lua_rawset(L, -3);
	//stack: {name = function}
	lua_pop(L, 1);
	return existed_already;
}

void luaW_getwidgetcallback(lua_State* L, gui2::widget* wg, gui2::window* owner, std::string_view name)
{
	luaW_pushwidgettable(L, wg, owner);
	//stack: {name = function},
	lua_push(L, name);
	//stack: {name = function}, name
	lua_rawget(L, -2);
	//stack: {name = function}, function
	lua_remove(L, lua_absindex(L, -2));
	//stack: function
}

void luaW_callwidgetcallback(lua_State* L, gui2::widget* wg, gui2::window* owner, std::string_view name)
{
	luaW_getwidgetcallback(L, wg, owner, name);
	assert(lua_isfunction(L, -1));
	luaW_pushwidget(L, *wg);
	lua_call(L, 1, 0);
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
		lua_pushcfunction(L, lua_widget::impl_widget_dir);
		lua_setfield(L, -2, "__dir");
		lua_pushcfunction(L, impl_widget_collect);
		lua_setfield(L, -2, "__gc");
		lua_pushstring(L, widgetKey);
		lua_setfield(L, -2, "__metatable");
	}
}
