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

static int impl_menuitem_get(lua_State *L)
{
    config **cfg = static_cast<config **>(lua_touserdata(L, 1));
    char const *m = luaL_checkstring(L, 2);

	// need to check if these return nil when empty

	return_bool_attrib("checkbox", (**cfg)["checkbox"].to_bool());
	return_tstring_attrib("details", (**cfg)["details"].str());
	return_string_attrib("icon", (**cfg)["icon"].str());
	return_string_attrib("image", (**cfg)["image"].str());
	return_tstring_attrib("label", (**cfg)["label"].str());
	return_tstring_attrib("tooltip", (**cfg)["tooltip"].str());

	return 0;
}

static int impl_menuitem_set(lua_State *L)
{
	config **cfg = static_cast<config **>(lua_touserdata(L, 1));
	char const *m = luaL_checkstring(L, 2);

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

	std::string err_msg = "unknown modifiable property of menu item: ";
	err_msg += m;
	return luaL_argerror(L, 2, err_msg.c_str());

}
		/* no, this does not go here.  We do this on a container widget, not one of it's rows!

static int impl_menuitem_len(lua_State *L)
{
	gui2::widget* w = &luaW_checkwidget(L, 1);
	if(gui2::options_button* ob = dynamic_cast<gui2::options_button*>(w)) {
		return ob->get_item_count();
	}
	return luaL_error(L, "unknown widget type");
}
*/

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
		/* no, this does not go here.  We do this on a container widget, not one of it's rows!
		lua_pushcfunction(L, impl_menuitem_len);
		lua_setfield(L, -2, "__len");
		*/
		lua_pushstring(L, "menu_item");
		lua_setfield(L, -2, "__metatable");

		return cmd_out.str();
	}
}

