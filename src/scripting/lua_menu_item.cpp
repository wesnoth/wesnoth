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

#include "config.hpp"
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
		luaL_argerror(L, n, "menu_item was deleted");
	}
	return *ptr;
}

lua_ptr<gui2::menu_item>& luaW_checkmenuitem_ptr(lua_State* L, int n)
{
	lua_ptr<gui2::menu_item>& lp =  *static_cast<lua_ptr<gui2::menu_item>*>(luaL_checkudata(L, n, menuitemKey));
	auto ptr = lp.get_ptr();
	if(!ptr) {
		luaL_argerror(L, n, "menu_item was deleted");
	}
	return lp;
}

static int impl_menuitem_get(lua_State *L)
{
    gui2::menu_item& item = luaW_checkmenuitem(L, 1);

    char const *m = luaL_checkstring(L, 2);
/*
	return_bool_attrib("checkbox", *(*ptr).checkbox);  // optional
	//return_tstring_attrib("details", *((**cfg).details));  // optional
	return_tstring_attrib("details", *(*ptr).details);  // optional
	//return_string_attrib("icon", (**cfg).icon);
	return_string_attrib("icon", (*ptr).icon);
	//return_string_attrib("image", *(**cfg).image);  // optional
	return_string_attrib("image", *(*ptr).image);  // optional
	//return_tstring_attrib("label", (**cfg).label);
*/
	LOG_LUA << "impl_menuitem_get(): get label";
	return_tstring_attrib("label", item.label);
	//return_tstring_attrib("tooltip", (**cfg).tooltip);
//	return_tstring_attrib("tooltip", (*ptr).tooltip);


	return 0;
}

static int impl_menuitem_set(lua_State *L)
{
	gui2::menu_item& item = luaW_checkmenuitem(L, 1);

	char const *m = luaL_checkstring(L, 2);

//	if(lua_isnil(L, 3)) {
	//	remove_config_attrib("checkbox", (**cfg));
//		remove_config_attrib("details", (**cfg));
//		remove_config_attrib("icon", (**cfg));
//		remove_config_attrib("image", (**cfg));
//		remove_config_attrib("label", (**cfg));
//		remove_config_attrib("tooltip", (**cfg));
//	} else {
//		modify_bool_attrib("checkbox",  (**cfg)["checkbox"] = value);
//		modify_bool_attrib("checkbox",  *(*ptr).checkbox = value);
	LOG_LUA << "impl_menuitem_set(): set details";
		modify_tstring_attrib("details", item.details = value);
//		modify_string_attrib("icon",  (*ptr).icon = value);
//		modify_string_attrib("image", *(*ptr).image = value);
	LOG_LUA << "impl_menuitem_set(): set label";
		//modify_tstring_attrib("label",  (**cfg).label = value);
		modify_tstring_attrib("label",  item.label = value);
//		modify_tstring_attrib("tooltip", (*ptr).tooltip = value);
//	}
	/*
	if(lua_isnil(L, 3)) {
		remove_config_attrib("checkbox", (**cfg));
		remove_config_attrib("details", (**cfg));
		remove_config_attrib("icon", (**cfg));
		remove_config_attrib("image", (**cfg));
		remove_config_attrib("label", (**cfg));
		remove_config_attrib("tooltip", (**cfg));
	} else {
		modify_bool_attrib("checkbox",  (**cfg)["checkbox"] = value);
		modify_tstring_attrib("details", (**cfg)["details"] = value);
		modify_string_attrib("icon",  (**cfg)["icon"]= value);
		modify_string_attrib("image", (**cfg)["image"] = value);
		modify_tstring_attrib("label",  (**cfg)["label"] = value);
		modify_tstring_attrib("tooltip", (**cfg)["tooltip"] = value);
	}
	*/

	std::string err_msg = "invalid modifiable property of menu item: ";
	err_msg += m;
	return luaL_argerror(L, 2, err_msg.c_str());
}

namespace lua_menuitem {
	std::string register_metatable(lua_State* L)
	{
		std::ostringstream cmd_out;

		// Create the menu_item metatable.
		cmd_out << "Adding menu_item metatable...\n";

		luaL_newmetatable(L, "menu_item");
		lua_pushcfunction(L, impl_menuitem_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_menuitem_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushstring(L, "menu_item");
		lua_setfield(L, -2, "__metatable");

		return cmd_out.str();
	}
}

