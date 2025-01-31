/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
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

struct lua_State;
class lua_kernel_base;
class vconfig;
class game_data;
class game_state;

namespace lua_gui2 {

int intf_add_widget_definition(lua_State *L);
int show_message_dialog(lua_State *L);
int show_popup_dialog(lua_State *L);
int switch_theme(lua_State* L);
int show_menu(lua_State* L);
int show_story(lua_State* L);
int show_message_box(lua_State* L);
int show_lua_console(lua_State*L, lua_kernel_base * lk);
int show_gamestate_inspector(const std::string& name, const game_data& data, const game_state& state);
int intf_show_recruit_dialog(lua_State* L);
int intf_show_recall_dialog(lua_State* L);
int luaW_open(lua_State *L);

} // end namespace lua_gui2
