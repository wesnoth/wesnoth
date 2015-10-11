/*
   Copyright (C) 2014 - 2015 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef LUA_GUI2_HPP_INCLUDED
#define LUA_GUI2_HPP_INCLUDED

struct lua_State;
class CVideo;
class lua_kernel_base;
class vconfig;

namespace lua_gui2 {

int intf_set_dialog_value(lua_State *L);
int intf_get_dialog_value(lua_State *L);
int intf_set_dialog_callback(lua_State *L);
int intf_set_dialog_markup(lua_State *L);
int intf_set_dialog_canvas(lua_State *L);
int intf_set_dialog_focus(lua_State *L);
int intf_set_dialog_active(lua_State *L);
int intf_set_dialog_visible(lua_State *L);
int intf_add_dialog_tree_node(lua_State *L);
int show_dialog(lua_State *L, CVideo & video);
int show_message_dialog(lua_State *L, CVideo & video);
int show_popup_dialog(lua_State *L, CVideo & video);
int show_lua_console(lua_State*L, CVideo & video, lua_kernel_base * lk);
int show_gamestate_inspector(CVideo & video, const vconfig & cfg);
int intf_remove_dialog_item(lua_State *L);

} // end namespace lua_gui2

#endif
