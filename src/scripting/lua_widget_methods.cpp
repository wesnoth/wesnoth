/*
   Copyright (C) 2020  the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


#include "config.hpp"
#include "gui/core/canvas.hpp"
#include "gui/core/gui_definition.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/widgets/clickable_item.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/multimenu_button.hpp"
#include "gui/widgets/progress_bar.hpp"
#include "gui/widgets/selectable_item.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_cpp_function.hpp"
#include "scripting/lua_kernel_base.hpp"
#include "scripting/lua_ptr.hpp"
#include "scripting/lua_widget.hpp"
#include "scripting/lua_widget_methods.hpp"
#include "scripting/push_check.hpp"
#include "serialization/string_utils.hpp"
#include "tstring.hpp"
#include "utils/functional.hpp"

#include <type_traits>
#include <map>
#include <utility>
#include <vector>
#include <boost/optional.hpp>

#include "lua/lauxlib.h"
#include "lua/lua.h"

static lg::log_domain log_scripting_lua("scripting/lua");
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

static const char dlgclbkKey[] = "dialog callback";

namespace {
	struct scoped_dialog
	{
		lua_State* L;
		scoped_dialog* prev;
		static scoped_dialog* current;
		std::unique_ptr<gui2::window> window;
		typedef std::map<gui2::widget*, int> callback_map;
		callback_map callbacks;

		scoped_dialog(lua_State* l, gui2::window* w);
		~scoped_dialog();
	private:
		scoped_dialog(const scoped_dialog&) = delete;
	};

	scoped_dialog* scoped_dialog::current = nullptr;

	scoped_dialog::scoped_dialog(lua_State* l, gui2::window* w)
		: L(l), prev(current), window(w), callbacks()
	{
		lua_pushstring(L, dlgclbkKey);
		lua_createtable(L, 1, 0);
		lua_pushvalue(L, -2);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_rawseti(L, -2, 1);
		lua_rawset(L, LUA_REGISTRYINDEX);
		current = this;
	}

	scoped_dialog::~scoped_dialog()
	{
		current = prev;
		lua_pushstring(L, dlgclbkKey);
		//stack: dlgclbkKey
		lua_pushvalue(L, -1);
		//stack: dlgclbkKey, dlgclbkKey
		lua_rawget(L, LUA_REGISTRYINDEX);
		//stack: dlgclbkKey, registery[dlgclbkKey]
		lua_rawgeti(L, -1, 1);
		//stack: dlgclbkKey, registery[dlgclbkKey], registery[dlgclbkKey][1]
		lua_remove(L, -2);
		//stack: dlgclbkKey, registery[dlgclbkKey][1]
		lua_rawset(L, LUA_REGISTRYINDEX);
		//registery[dlgclbkKey] = registery[dlgclbkKey][1] //restorign the old value of registery[dlgclbkKey]
	}
}//unnamed namespace for scoped_dialog


/**
 * Displays a window.
 * - Arg 1: WML table describing the window.
 * - Arg 2: function called at pre-show.
 * - Arg 3: function called at post-show.
 * - Ret 1: integer.
 */
int intf_show_dialog(lua_State* L)
{
	config def_cfg = luaW_checkconfig(L, 1);

	gui2::builder_window::window_resolution def(def_cfg);
	scoped_dialog w(L, gui2::build(&def));

	if(!lua_isnoneornil(L, 2)) {
		lua_pushvalue(L, 2);
		lua_call(L, 0, 0);
	}

	int v = scoped_dialog::current->window->show(true, 0);

	if (!lua_isnoneornil(L, 3)) {
		lua_pushvalue(L, 3);
		lua_call(L, 0, 0);
	}

	lua_pushinteger(L, v);
	return 1;
}

static gui2::widget* find_widget_impl(lua_State* L, gui2::widget* w, int i, bool readonly)
{
	if(!w && !scoped_dialog::current) {
		luaL_error(L, "no visible dialog");
		error_oob_call_dtors:
		luaL_argerror(L, i, "out of bounds");
		error_not_str_call_dtors:
		luaW_type_error(L, i, "string");
		error_no_wgt_call_dtors:
		luaL_argerror(L, i, "widget not found");
		return nullptr;
	}
	if(!w) {
		w = scoped_dialog::current->window.get();
	}

	for(; !lua_isnoneornil(L, i); ++i)
	{
		if(gui2::listbox* list = dynamic_cast<gui2::listbox*>(w))
		{
			int v = lua_tointeger(L, i);
			if(v < 1) {
				goto error_oob_call_dtors;
			}
			int n = list->get_item_count();
			if(v > n) {
				if(readonly) {
					goto error_oob_call_dtors;
				}
				utils::string_map dummy;
				for(; n < v; ++n) {
					list->add_row(dummy);
				}
			}
			w = list->get_row_grid(v - 1);
		} else if(gui2::multi_page* multi_page = dynamic_cast<gui2::multi_page*>(w)) {
			int v = lua_tointeger(L, i);
			if(v < 1) {
				goto error_oob_call_dtors;
			}
			int n = multi_page->get_page_count();
			if(v > n) {
				if(readonly) {
					goto error_oob_call_dtors;
				}
				utils::string_map dummy;
				for(; n < v; ++n) {
					multi_page->add_page(dummy);
				}
			}
			w = &multi_page->page_grid(v - 1);
		} else if(gui2::tree_view* tree_view = dynamic_cast<gui2::tree_view*>(w)) {
			gui2::tree_view_node& tvn = tree_view->get_root_node();
			if(lua_isnumber(L, i)) {
				int v = lua_tointeger(L, i);
				if(v < 1) {
					goto error_oob_call_dtors;
				}
				int n = tvn.count_children();
				if(v > n) {
					goto error_oob_call_dtors;
				}
				w = &tvn.get_child_at(v - 1);

			} else {
				std::string m = luaL_checkstring(L, i);
				w = tvn.find(m, false);
			}
		} else if(gui2::tree_view_node* tree_view_node = dynamic_cast<gui2::tree_view_node*>(w)) {
			if(lua_isnumber(L, i)) {
				int v = lua_tointeger(L, i);
				if(v < 1) {
					goto error_oob_call_dtors;
				}
				int n = tree_view_node->count_children();
				if(v > n) {
					goto error_oob_call_dtors;
				}
				w = &tree_view_node->get_child_at(v - 1);

			} else {
				std::string m = luaL_checkstring(L, i);
				w = tree_view_node->find(m, false);
			}
		} else if(gui2::stacked_widget* stacked_widget = dynamic_cast<gui2::stacked_widget*>(w)) {
			if(lua_isnumber(L, i)) {
				int v = lua_tointeger(L, i);
				if(v < 1) {
					goto error_oob_call_dtors;
				}
				int n = stacked_widget->get_layer_count();
				if(v > n) {
					goto error_oob_call_dtors;
				}
				w = stacked_widget->get_layer_grid(v - 1);
			} else {
				std::string m = luaL_checkstring(L, i);
				w = stacked_widget->find(m, false);
			}
		} else {
			char const *m = lua_tostring(L, i);
			if(!m) {
				goto error_not_str_call_dtors;
			}
			w = w->find(m, false);
		}
		if(!w) {
			goto error_no_wgt_call_dtors;
		}
	}

	return w;
}

static int intf_find_widget(lua_State* L)
{
	int start = 1;
	gui2::widget* w = nullptr;
	if(luaW_iswidget(L, 1)) {
		start = 2;
		w = &luaW_checkwidget(L, 1);
	}
	auto pw = find_widget_impl(L, w, start, false);
	if(pw) {
		luaW_pushwidget(L, *pw);
		return 1;
	}
	return 0;
}


namespace
{
	void remove_treeview_node(gui2::tree_view_node& node, std::size_t pos, int number)
	{
		//Not tested yet.
		gui2::tree_view& tv = node.get_tree_view();
		if(pos >= node.count_children()) {
			return;
		}
		if(number <= 0 || number + pos > node.count_children()) {
			number = node.count_children() - pos;
		}
		for(int i = 0; i < number; ++i) {
			tv.remove_node(&node.get_child_at(pos));
		}
	}
}

/**
 * Removes an entry from a list.
 * - Arg 1: widget
 * - Arg 2: number, index of the element to delete.
 * - Arg 3: number, number of the elements to delete. (0 to delete all elements after index)
 */
static int intf_remove_dialog_item(lua_State* L)
{
	gui2::widget* w = &luaW_checkwidget(L, 1);
	int pos = luaL_checkinteger(L, 2) - 1;
	int number = luaL_checkinteger(L, 3);

	if(gui2::listbox* list = dynamic_cast<gui2::listbox*>(w))
	{
		list->remove_row(pos, number);
	} else if(gui2::multi_page* multi_page = dynamic_cast<gui2::multi_page*>(w)) {
		multi_page->remove_page(pos, number);
	} else if(gui2::tree_view* tree_view = dynamic_cast<gui2::tree_view*>(w)) {
		remove_treeview_node(tree_view->get_root_node(), pos, number);
	} else if(gui2::tree_view_node* tree_view_node = dynamic_cast<gui2::tree_view_node*>(w)) {
		remove_treeview_node(*tree_view_node, pos, number);
	} else {
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");
	}

	return 1;
}

namespace { // helpers of intf_set_dialog_callback()
	void dialog_callback(gui2::widget& w)
	{
		int cb;
		{
			scoped_dialog::callback_map &m = scoped_dialog::current->callbacks;
			scoped_dialog::callback_map::const_iterator i = m.find(&w);
			if(i == m.end()) {
				return;
			}
			cb = i->second;
		}
		lua_State *L = scoped_dialog::current->L;
		lua_pushstring(L, dlgclbkKey);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_rawgeti(L, -1, cb);
		lua_remove(L, -2);
		lua_call(L, 0, 0);
	}

	/** Helper struct for intf_set_dialog_callback. */
	struct dialog_callback_wrapper
	{
		void forward(gui2::widget* widget, bool& handled, bool& halt)
		{
			assert(widget);
			dialog_callback(*widget);
			handled = true;
			halt = true;
		}
	};
}//unnamed namespace for helpers of intf_set_dialog_callback()

/**
 * Sets a callback on a widget of the current dialog.
 * - Arg 1: widget.
 * - Arg 2: function.
 */
static int intf_set_dialog_callback(lua_State* L)
{
	gui2::widget* w = &luaW_checkwidget(L, 1);

	scoped_dialog::callback_map &m = scoped_dialog::current->callbacks;
	scoped_dialog::callback_map::iterator i = m.find(w);
	if(i != m.end()) {
		lua_pushstring(L, dlgclbkKey);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_pushnil(L);
		lua_rawseti(L, -2, i->second);
		lua_pop(L, 1);
		m.erase(i);
	}

	if(lua_isnil(L, 1)) {
		return 0;
	}

	if(gui2::clickable_item* c = dynamic_cast<gui2::clickable_item*>(w)) {
		static dialog_callback_wrapper wrapper;
		c->connect_click_handler(std::bind(&dialog_callback_wrapper::forward, wrapper, w, _3, _4));
	} else if(gui2::selectable_item* si = dynamic_cast<gui2::selectable_item*>(w)) {
		connect_signal_notify_modified(dynamic_cast<gui2::widget&>(*si), std::bind(dialog_callback, _1));
	} else if(gui2::integer_selector* is = dynamic_cast<gui2::integer_selector*>(w)) {
		connect_signal_notify_modified(dynamic_cast<gui2::widget&>(*is), std::bind(dialog_callback, _1));
	} else if(gui2::listbox* l = dynamic_cast<gui2::listbox*>(w)) {
		static dialog_callback_wrapper wrapper;
		connect_signal_notify_modified(*l, std::bind(&dialog_callback_wrapper::forward, wrapper, w, _3, _4));
	} else if(gui2::tree_view* tv = dynamic_cast<gui2::tree_view*>(w)) {
		connect_signal_notify_modified(*tv, std::bind(dialog_callback, _1));
	} else {
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");
	}

	lua_pushstring(L, dlgclbkKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	int n = lua_rawlen(L, -1) + 1;
	m[w] = n;
	lua_pushvalue(L, 2);
	lua_rawseti(L, -2, n);
	lua_pop(L, 1);

	return 0;
}


/**
 * Sets a canvas on a widget of the current dialog.
 * - Arg 1: widget.
 * - Arg 2: integer.
 * - Arg 3: WML table.
 */
static int intf_set_dialog_canvas(lua_State* L)
{
	gui2::widget* w = &luaW_checkwidget(L, 1);
	int i = luaL_checkinteger(L, 2);
	gui2::styled_widget* c = dynamic_cast<gui2::styled_widget*>(w);
	if(!c) {
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");
	}

	std::vector<gui2::canvas> &cv = c->get_canvases();
	if(i < 1 || static_cast<unsigned>(i) > cv.size()) {
		return luaL_argerror(L, 2, "out of bounds");
	}

	config cfg = luaW_checkconfig(L, 3);
	cv[i - 1].set_cfg(cfg);
	c->set_is_dirty(true);
	return 0;
}

/**
 * Sets a widget to have the focus
 * - Arg 1: widget.
 */
static int intf_set_dialog_focus(lua_State* L)
{
	gui2::widget* w = &luaW_checkwidget(L, 1);
	scoped_dialog::current->window->keyboard_capture(w);
	return 0;
}


/**
 * Sets a widget's state to active or inactive
 * - Arg 1: widget.
 * - Arg 2: string, the type (id of [node_definition]) of the new node.
 * - Arg 3: integer, where to insert the new node.
 */
static int intf_add_item_of_type(lua_State* L)
{
	gui2::widget* w = &luaW_checkwidget(L, 1);
	gui2::widget* res = nullptr;
	const std::string node_type = luaL_checkstring(L, 2);
	int insert_pos = -1;
	if(lua_isnumber(L, 3)) {
		insert_pos = luaL_checkinteger(L, 3);
	}
	static const std::map<std::string, string_map> data;

	if(gui2::tree_view_node* twn = dynamic_cast<gui2::tree_view_node*>(w)) {
		res = &twn->add_child(node_type, data, insert_pos);
	} else if(gui2::tree_view* tw = dynamic_cast<gui2::tree_view*>(w)) {
		res = &tw->get_root_node().add_child(node_type, data, insert_pos);
	} else if(gui2::multi_page* mp = dynamic_cast<gui2::multi_page*>(w)) {
		res = &mp->add_page(node_type, insert_pos, data);
	} else {
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");
	}
	if(res) {
		luaW_pushwidget(L, *res);
		lua_push(L, insert_pos);
		return 2;
	}
	return 0;
}
/**
 * Sets a widget's state to active or inactive
 * - Arg 1: widget.
 */
static int intf_add_dialog_item(lua_State* L)
{
	
	gui2::widget* w = &luaW_checkwidget(L, 1);
	gui2::widget* res = nullptr;
	static const std::map<std::string, string_map> data;

	if(gui2::listbox* lb = dynamic_cast<gui2::listbox*>(w)) {
		res = &lb->add_row(data);
	} else {
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");
	}
	if(res) {
		luaW_pushwidget(L, *res);
		return 1;
	}
	return 0;
}

namespace lua_widget {
int luaW_open(lua_State* L)
{
	auto& lk = lua_kernel_base::get_lua_kernel<lua_kernel_base>(L);
	lk.add_log("Adding widgets module...\n");
	static luaL_Reg const gui_callbacks[] = {
		//TODO: the naming is a bit arbitaty: widgets with differnt
		//      types of elements use add_node, eidgets with only
		//      one type of element use add_element
		{ "add_item_of_type",   &intf_add_item_of_type },
		{ "add_item",           &intf_add_dialog_item },
		{ "focus",              &intf_set_dialog_focus },
		{ "set_canvas",         &intf_set_dialog_canvas },
		{ "set_callback",       &intf_set_dialog_callback },
		{ "remove_items_at",     &intf_remove_dialog_item },
		{ "find",               &intf_find_widget },
		{ nullptr, nullptr },
	};
	lua_newtable(L);
	luaL_setfuncs(L, gui_callbacks, 0);
	return 1;
}
}
