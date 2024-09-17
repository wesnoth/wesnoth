/*
	Copyright (C) 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "log.hpp"
#include "gui/widgets/options_button.hpp"
#include "scripting/lua_menu_item.hpp"
#include "scripting/push_check.hpp"

#include <string>

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

static const char menuitemKey[] = "menu_item";

void luaW_pushmenuitem(lua_State* L, gui2::menu_item& m)
{
	new(L) lua_ptr<gui2::menu_item>(m);
	luaL_setmetatable(L, menuitemKey);
}

gui2::menu_item& luaW_checkmenuitem(lua_State* L, int n)
{
	lua_ptr<gui2::menu_item>& lp =  *static_cast<lua_ptr<gui2::menu_item>*>(luaL_checkudata(L, n, menuitemKey));
	auto ptr = lp.get_ptr();
	if(!ptr) {
		luaL_argerror(L, n, "menu item was deleted");
	}
	return *ptr;
}

lua_ptr<gui2::menu_item>& luaW_checkmenuitem_ptr(lua_State* L, int n)
{
	lua_ptr<gui2::menu_item>& lp =  *static_cast<lua_ptr<gui2::menu_item>*>(luaL_checkudata(L, n, menuitemKey));
	auto ptr = lp.get_ptr();
	if(!ptr) {
		luaL_argerror(L, n, "menu item was deleted");
	}
	return lp;
}

static int impl_menuitem_get(lua_State *L)
{
    gui2::menu_item& item = luaW_checkmenuitem(L, 1);
    char const *m = luaL_checkstring(L, 2);

	return_bool_optional("checkbox", item.checkbox);
	return_tstring_optional("details", item.details);
	return_string_attrib("icon", item.icon);
	return_string_optional("image", item.image);
	return_tstring_attrib("label", item.label);
	return_tstring_attrib("tooltip", item.tooltip);

	return luaL_error(L, "invalid menu item attribute: %s", m);
}

static int impl_menuitem_set(lua_State *L)
{
	gui2::menu_item& item = luaW_checkmenuitem(L, 1);
	char const *m = luaL_checkstring(L, 2);

	if(lua_isnil(L, 3)) {
		clear_optional("checkbox", item.checkbox);
		clear_optional("details", item.details);
		clear_optional("image", item.image);
		if((strcmp(m, "icon") == 0) ||
		 	(strcmp(m, "label") == 0) ||
		 	(strcmp(m, "tooltip") == 0)) {
			return luaL_error(L, "nil is not a valid value for attribute: %s", m);
		}
	} else {
		modify_bool_attrib("checkbox", item.checkbox = value);
		modify_tstring_attrib("details", item.details = value);
		modify_string_attrib("icon", item.icon = value);
		modify_string_attrib("image", item.image = value);
		modify_tstring_attrib("label", item.label = value);
		modify_tstring_attrib("tooltip", item.tooltip = value);
	}

	std::string err_msg = "invalid modifiable property of menu item: ";
	err_msg += m;
	return luaL_argerror(L, 2, err_msg.c_str());
}

static int impl_menuitem_collect(lua_State* L)
{
        lua_ptr<gui2::menu_item>* m = static_cast<lua_ptr<gui2::menu_item>*>(luaL_checkudata(L, 1, menuitemKey));
        m->~lua_ptr<gui2::menu_item>();
        return 0;
}

namespace lua_menuitem {
	std::string register_metatable(lua_State* L)
	{
		std::ostringstream cmd_out;

		// Create the menu_item metatable.
		cmd_out << "Adding " << menuitemKey <<  " metatable...\n";

		luaL_newmetatable(L, menuitemKey);
		lua_pushcfunction(L, impl_menuitem_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_menuitem_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushcfunction(L, impl_menuitem_collect);
		lua_setfield(L, -2, "__gc");
		lua_pushstring(L, menuitemKey);
		lua_setfield(L, -2, "__metatable");

		return cmd_out.str();
	}
}
