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
#include "gui/auxiliary/menu_item.hpp"
#include "scripting/lua_common.hpp"

#include <string>

namespace lua_menuitem {
	    std::string register_metatable(lua_State *L);
}

void luaW_pushmenuitem(lua_State* L, gui2::menu_item& m);

gui2::menu_item& luaW_checkmenuitem(lua_State* L, int n);

lua_ptr<gui2::menu_item>& luaW_checkmenuitem_ptr(lua_State* L, int n);

//  This should be going away
#define remove_config_attrib(name,cfg)\
do { \
    if (strcmp(m, (name)) == 0) { \
		cfg.remove_attribute(name); \
        return 0; \
    } \
} while(false)
