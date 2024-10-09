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
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/multimenu_button.hpp"
#include "gui/widgets/options_button.hpp"
#include "scripting/lua_attributes.hpp"
#include "scripting/lua_menu_item.hpp"
#include "scripting/push_check.hpp"

#include <string>

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

/**
 * Implementation for a lua reference to a unit_type.
 */

// Registry key
static const char MenuItem[] = "menu item";

#define MENU_ITEM_GETTER(name, type) LATTR_GETTER(name, type, gui2::menu_item, mi)
#define MENU_ITEM_SETTER(name, type) LATTR_SETTER(name, type, gui2::menu_item, mi)
luaW_Registry menuItemReg{MenuItem};

template<> struct lua_object_traits<gui2::menu_item> {
    inline static auto metatable = MenuItem;
//	inline static const gui2::menu_item& get(lua_State* L, int n) {
    inline static gui2::menu_item& get(lua_State* L, int n) {
        return luaW_checkmenuitem(L, n);
    }
};

/* Perhaps an error macro something like:
 *
 * if item.get_parent() { luaL_error(L, "BLAHBLAH %s for %s, luaL_checkstring(L, 2),
 * 	 mi.get_parent().get_control_type()); }
 *
 * 	 but what to do when parent=nullptr?
 *
 * */

MENU_ITEM_GETTER("checkbox", utils::optional<bool>) {
    if(dynamic_cast<gui2::menu_button*>(mi.get_parent())) {
        luaL_error(L, "invalid menu item attribute: %s for menu_button", luaL_checkstring(L, 2));
    }
	return mi.checkbox;
}

MENU_ITEM_GETTER("details", utils::optional<t_string>) {
	return mi.details;
}

MENU_ITEM_GETTER("icon", std::string) {
	if(dynamic_cast<gui2::multimenu_button*>(mi.get_parent())) {
		luaL_error(L, "invalid menu item attribute: %s for multimenu_button", luaL_checkstring(L, 2));
	}
    return mi.icon;
}

MENU_ITEM_GETTER("image", utils::optional<std::string>) {
	// these may change, for now don't mess up my test cases
	if(dynamic_cast<gui2::menu_button*>(mi.get_parent())) {
		luaL_error(L, "invalid menu item attribute: %s for menu_button", luaL_checkstring(L, 2));
	}
	if(dynamic_cast<gui2::multimenu_button*>(mi.get_parent())) {
		luaL_error(L, "invalid menu item attribute: %s for multimenu_button", luaL_checkstring(L, 2));
	}
	return mi.image;
}

MENU_ITEM_GETTER("label", t_string) {
    return mi.label;
}

MENU_ITEM_GETTER("tooltip", t_string) {
    return mi.tooltip;
}

MENU_ITEM_GETTER("value", std::string) {
    return mi.value;
}

MENU_ITEM_SETTER("checkbox", utils::optional<bool>) {
	if(lua_isnil(L, 3)) {
		if(dynamic_cast<gui2::multimenu_button*>(mi.get_parent())) {
			luaL_error(L, "multimenu_button menu item must contain checkbox, cannot set to nil");
		}
	}

	if(dynamic_cast<gui2::menu_button*>(mi.get_parent())) {
		luaL_error(L, "invalid menu item attribute: %s for menu_button", luaL_checkstring(L, 2));
	}

	if(gui2::multimenu_button* mmb = dynamic_cast<gui2::multimenu_button*>(mi.get_parent())) {
    	mi.checkbox = luaW_toboolean(L, 3);
		mmb->update_label();
	} else {
		mi.checkbox = value;
	}
}

MENU_ITEM_SETTER("details", utils::optional<t_string>) {
	mi.details = value;
}

MENU_ITEM_SETTER("icon", std::string) {
	if(dynamic_cast<gui2::multimenu_button*>(mi.get_parent())) {
		luaL_error(L, "invalid menu item attribute: %s for multimenu_button", luaL_checkstring(L, 2));
	}
	mi.icon = value;
}

MENU_ITEM_SETTER("image", utils::optional<std::string>) {
	// these may change, for now don't mess up my test cases
	if(dynamic_cast<gui2::menu_button*>(mi.get_parent())) {
		luaL_error(L, "invalid menu item attribute: %s for menu_button", luaL_checkstring(L, 2));
	}
	if(dynamic_cast<gui2::multimenu_button*>(mi.get_parent())) {
		luaL_error(L, "invalid menu item attribute: %s for multimenu_button", luaL_checkstring(L, 2));
	}
	mi.image = value;
}

MENU_ITEM_SETTER("label", t_string) {
	mi.label = value;
}

MENU_ITEM_SETTER("tooltip", t_string) {
	mi.tooltip = value;
}

MENU_ITEM_SETTER("value", std::string) {
	mi.value = value;
}

/**
 * Gets some data on a menu item (__index metamethod).
 * - Arg 1:
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_menu_item_get(lua_State *L)
{
    return menuItemReg.get(L);
}

/**
 * Sets some data on a menu item (__newindex metamethod).
 * - Arg 1:
 * - Arg 2: string containing the name of the property.
 * - Arg 3: something containing the attribute.
 */
static int impl_menu_item_set(lua_State *L)
{
    return menuItemReg.set(L);
}

/**
 * Prints valid attributes on a menu item (__dir metamethod).
 * - Arg 1:
 * - Arg 2: string containing the name of the property.
 * - Ret 1: a list of attributes.
 */
static int impl_menu_item_dir(lua_State *L)
{
    return menuItemReg.dir(L);
}

void luaW_pushmenuitem(lua_State* L, gui2::menu_item& m)
{
	new(L) lua_ptr<gui2::menu_item>(m);
	luaL_setmetatable(L, MenuItem);
}

gui2::menu_item& luaW_checkmenuitem(lua_State* L, int n)
{
	lua_ptr<gui2::menu_item>& lp =  *static_cast<lua_ptr<gui2::menu_item>*>(luaL_checkudata(L, n, MenuItem));
	auto ptr = lp.get_ptr();
	if(!ptr) {
		luaL_argerror(L, n, "menu item was deleted");
	}
	return *ptr;
}

lua_ptr<gui2::menu_item>& luaW_checkmenuitem_ptr(lua_State* L, int n)
{
	lua_ptr<gui2::menu_item>& lp =  *static_cast<lua_ptr<gui2::menu_item>*>(luaL_checkudata(L, n, MenuItem));
	auto ptr = lp.get_ptr();
	if(!ptr) {
		luaL_argerror(L, n, "menu item was deleted");
	}
	return lp;
}

static int impl_menuitem_collect(lua_State* L)
{
        lua_ptr<gui2::menu_item>* m = static_cast<lua_ptr<gui2::menu_item>*>(luaL_checkudata(L, 1, MenuItem));
        m->~lua_ptr<gui2::menu_item>();
        return 0;
}

namespace lua_menuitem {
	std::string register_metatable(lua_State* L)
	{
		std::ostringstream cmd_out;

		// Create the menu_item metatable.
		cmd_out << "Adding " << MenuItem <<  " metatable...\n";

		luaL_newmetatable(L, MenuItem);
		lua_pushcfunction(L, impl_menu_item_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_menu_item_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushcfunction(L, impl_menuitem_collect);
		lua_setfield(L, -2, "__gc");
		lua_pushcfunction(L, impl_menu_item_dir);
		lua_setfield(L, -2, "__dir");
		lua_pushstring(L, MenuItem);
		lua_setfield(L, -2, "__metatable");

		return cmd_out.str();
	}
}
