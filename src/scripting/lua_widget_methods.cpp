/*
	Copyright (C) 2020 - 2024
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
#include "gui/core/canvas.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/widgets/clickable_item.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/selectable_item.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_kernel_base.hpp"
#include "scripting/lua_ptr.hpp"
#include "scripting/lua_widget.hpp"
#include "scripting/lua_widget_methods.hpp"
#include "scripting/push_check.hpp"
#include "utils/scope_exit.hpp"
#include <functional>

#include <vector>


static lg::log_domain log_scripting_lua("scripting/lua");
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

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

	auto wp = std::make_unique<gui2::window>(def);

	if(!lua_isnoneornil(L, 2)) {
		lua_pushvalue(L, 2);
		luaW_pushwidget(L, *wp);
		lua_call(L, 1, 0);
	}

	int v = wp->show();

	if (!lua_isnoneornil(L, 3)) {
		lua_pushvalue(L, 3);
		luaW_pushwidget(L, *wp);
		lua_call(L, 1, 0);
	}
	luaW_clearwindowtable(L, wp.get());
	lua_pushinteger(L, v);
	return 1;
}

static gui2::widget* find_widget_impl(lua_State* L, gui2::widget* w, int i, bool readonly)
{
	assert(w);

	for(; !lua_isnoneornil(L, i); ++i)
	{
		if(gui2::listbox* list = dynamic_cast<gui2::listbox*>(w))
		{
			int v = lua_tointeger(L, i);
			if(v < 1) {
				throw std::invalid_argument("negative index");
			}
			int n = list->get_item_count();
			if(v > n) {
				if(readonly) {
					throw std::invalid_argument("index out of range");
				}
				gui2::widget_item dummy;
				for(; n < v; ++n) {
					list->add_row(dummy);
				}
			}
			w = list->get_row_grid(v - 1);
		} else if(gui2::multi_page* multi_page = dynamic_cast<gui2::multi_page*>(w)) {
			int v = lua_tointeger(L, i);
			if(v < 1) {
				throw std::invalid_argument("negative index");
			}
			int n = multi_page->get_page_count();
			if(v > n) {
				if(readonly) {
					throw std::invalid_argument("index out of range");
				}
				gui2::widget_item dummy;
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
					throw std::invalid_argument("negative index");
				}
				int n = tvn.count_children();
				if(v > n) {
					throw std::invalid_argument("index out of range");
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
					throw std::invalid_argument("negative index");
				}
				int n = tree_view_node->count_children();
				if(v > n) {
					throw std::invalid_argument("index out of range");
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
					throw std::invalid_argument("negative index");
				}
				int n = stacked_widget->get_layer_count();
				if(v > n) {
					throw std::invalid_argument("index out of range");
				}
				w = stacked_widget->get_layer_grid(v - 1);
			} else {
				std::string m = luaL_checkstring(L, i);
				w = stacked_widget->find(m, false);
			}
		} else {
			char const *m = lua_tostring(L, i);
			if(!m) {
				throw std::invalid_argument("expected a string");
			}
			w = w->find(m, false);
		}
		if(!w) {
			throw std::invalid_argument("widget not found");
		}
	}

	return w;
}

static int intf_find_widget(lua_State* L)
{
	gui2::widget* w = &luaW_checkwidget(L, 1);
	auto pw = find_widget_impl(L, w, 2, false);
	if(pw) {
		luaW_pushwidget(L, *pw);
		return 1;
	}
	return 0;
}


namespace
{
	int number_of_items(gui2::listbox& mp)
	{
		return mp.get_item_count();
	}
	int number_of_items(gui2::multi_page& mp)
	{
		return mp.get_page_count();
	}

	int number_of_items(gui2::tree_view_node& mp)
	{
		return mp.count_children();
	}

	int number_of_items(gui2::tree_view& mp)
	{
		return number_of_items(mp.get_root_node());
	}

	// converts a 1-based index given as lua paraemter to a 0-based index to be used in the c++ api.
	// and checks that it is in range
	template<typename TWidget>
	int check_index(lua_State* L, int arg, TWidget& w, bool for_insertion, utils::optional<int>& index)
	{
		int nitems = number_of_items(w);

		// index == nitems + 1 -> insert at the end.
		int max = for_insertion ? nitems + 1 : nitems;
		if(!index) {
			index = max;
		}

		if(*index <= 0 || *index > max) {
			luaL_argerror(L, arg, "widget child index out of range");
		}
		return *index - 1;
	}

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
 * - Arg 2: number (optional), index of the element to delete.
 * - Arg 3: number (optional), number of the elements to delete. (0 to delete all elements after index)
 */
static int intf_remove_dialog_item(lua_State* L)
{
	gui2::widget* w = &luaW_checkwidget(L, 1);
	utils::optional<int> pos = lua_check<utils::optional<int>>(L, 2);
	int number = lua_check<utils::optional<int>>(L, 3).value_or(1);

	if(gui2::listbox* list = dynamic_cast<gui2::listbox*>(w)) {
		int realpos = check_index(L, 2, *list, false, pos);
		list->remove_row(realpos, number);
	} else if(gui2::multi_page* multi_page = dynamic_cast<gui2::multi_page*>(w)) {
		int realpos = check_index(L, 2, *multi_page,false, pos);
		multi_page->remove_page(realpos, number);
	} else if(gui2::tree_view* tree_view = dynamic_cast<gui2::tree_view*>(w)) {
		int realpos = check_index(L, 2, *tree_view, false, pos);
		remove_treeview_node(tree_view->get_root_node(), realpos, number);
	} else if(gui2::tree_view_node* tree_view_node = dynamic_cast<gui2::tree_view_node*>(w)) {
		int realpos = check_index(L, 2, *tree_view_node, false, pos);
		remove_treeview_node(*tree_view_node, realpos, number);
	} else {
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");
	}

	return 0;
}

/**
 * Removes all entries from a list.
 * - Arg 1: widget
*/
static int intf_clear_items(lua_State* L)
{
	gui2::widget* w = &luaW_checkwidget(L, 1);

	if(auto* lb = dynamic_cast<gui2::listbox*>(w)) {
		lb->clear();
	} else if(auto* mp = dynamic_cast<gui2::multi_page*>(w)) {
		mp->clear();
	} else if(auto* tv = dynamic_cast<gui2::tree_view*>(w)) {
		tv->clear();
	} else if(auto* tvn = dynamic_cast<gui2::tree_view_node*>(w)) {
		tvn->clear();
	} else {
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");
	}

	return 0;
}

namespace { // helpers of intf_set_dialog_callback()
	void dialog_callback(lua_State* L, lua_ptr<gui2::widget>& wp, const std::string& id)
	{
		gui2::widget* w = wp.get_ptr();
		if(!w) {
			ERR_LUA << "widget was deleted";
			return;
		}
		gui2::window* wd = w->get_window();
		if(!wd) {
			ERR_LUA << "cannot find window in widget callback";
			return;
		}
		luaW_callwidgetcallback(L, w, wd, id);
	}
}//unnamed namespace for helpers of intf_set_dialog_callback()

/**
 * Sets a callback on a widget of the current dialog.
 * - Arg 1: widget.
 * - Arg 2: function.
 */
static int intf_set_dialog_callback(lua_State* L)
{
	lua_ptr<gui2::widget>& wp = luaW_checkwidget_ptr(L, 1);
	gui2::widget* w = wp.get_ptr();
	assert(w);
	gui2::window* wd = w->get_window();
	if(!wd) {
		throw std::invalid_argument("the widget has no window assigned");
	}
	if(!lua_isfunction(L, 2)) {
		return luaL_argerror(L, 2, "callback must be a function");
	}

	lua_pushvalue(L, 2);
	bool already_exists = luaW_setwidgetcallback(L, w, wd, "callback");
	if(already_exists) {
		return 0;
	}

	// TODO: i am not sure whether it is 100% safe to bind the lua_state here,
	//       (meaning whether it can happen that the lus state is destroyed)
	//       when a widgets callback is called.
	if(gui2::clickable_item* c = dynamic_cast<gui2::clickable_item*>(w)) {
		c->connect_click_handler(std::bind(&dialog_callback, L, wp, "callback"));
	} else if( dynamic_cast<gui2::selectable_item*>(w)) {
		connect_signal_notify_modified(*w, std::bind(&dialog_callback, L, wp, "callback"));
	} else if(dynamic_cast<gui2::integer_selector*>(w)) {
		connect_signal_notify_modified(*w, std::bind(&dialog_callback, L, wp, "callback"));
	} else if(dynamic_cast<gui2::listbox*>(w)) {
		connect_signal_notify_modified(*w, std::bind(&dialog_callback, L, wp, "callback"));
	} else if(dynamic_cast<gui2::tree_view*>(w)) {
		connect_signal_notify_modified(*w, std::bind(&dialog_callback, L, wp, "callback"));
	} else {
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");
	};

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
	c->queue_redraw();
	return 0;
}

/**
 * Sets a widget to have the focus
 * - Arg 1: widget.
 */
static int intf_set_dialog_focus(lua_State* L)
{
	gui2::widget* w = &luaW_checkwidget(L, 1);
	if(gui2::window* wd = w->get_window()) {
		wd->keyboard_capture(w);
	}
	return 0;
}


/**
 * Adds an item to a container widget that supports different types of items, for example a treeview.
 * - Arg 1: widget.
 * - Arg 2: string, the type (id of [node_definition]) of the new item.
 * - Arg 3: integer (optional), where to insert the new item.
 */
static int intf_add_item_of_type(lua_State* L)
{
	gui2::widget* w = &luaW_checkwidget(L, 1);
	gui2::widget* res = nullptr;
	const std::string node_type = luaL_checkstring(L, 2);
	utils::optional<int> insert_pos = lua_check<utils::optional<int>>(L, 3);
	static const gui2::widget_data data;

	if(gui2::tree_view_node* twn = dynamic_cast<gui2::tree_view_node*>(w)) {
		int realpos = check_index(L, 2, *twn, true, insert_pos);
		res = &twn->add_child(node_type, data, realpos);
	} else if(gui2::tree_view* tw = dynamic_cast<gui2::tree_view*>(w)) {
		int realpos = check_index(L, 2, *tw, true, insert_pos);
		res = &tw->get_root_node().add_child(node_type, data, realpos);
	} else if(gui2::multi_page* mp = dynamic_cast<gui2::multi_page*>(w)) {
		int realpos = check_index(L, 2, *mp, true, insert_pos);
		res = &mp->add_page(node_type, realpos, data);
	} else {
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");
	}
	if(res) {
		luaW_pushwidget(L, *res);
		lua_push(L, insert_pos.value());
		return 2;
	}
	return 0;
}
/**
 * Adds an item to a container widget, for example a listbox
 * - Arg 1: widget.
 * - Arg 2: integer (optional), where to insert the new item.
 */
static int intf_add_dialog_item(lua_State* L)
{
	gui2::widget* w = &luaW_checkwidget(L, 1);
	utils::optional<int> insert_pos = lua_check<utils::optional<int>>(L, 2);

	gui2::widget* res = nullptr;
	static const gui2::widget_data data;

	if(gui2::listbox* lb = dynamic_cast<gui2::listbox*>(w)) {
		int realpos = check_index(L, 2, *lb, true, insert_pos);
		res = &lb->add_row(data, realpos);
	} else {
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");
	}
	if(res) {
		luaW_pushwidget(L, *res);
		lua_push(L, insert_pos.value());
		return 2;
	}
	return 0;
}

/** Closes a window */
static int intf_dialog_close(lua_State* L)
{
	gui2::widget* w = &luaW_checkwidget(L, 1);
	if(gui2::window* wd = dynamic_cast<gui2::window*>(w)) {
		wd->close();
		return 0;
	} else {
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");
	}
}
namespace lua_widget {
int luaW_open(lua_State* L)
{
	auto& lk = lua_kernel_base::get_lua_kernel<lua_kernel_base>(L);
	lk.add_log("Adding widgets module...\n");
	static luaL_Reg const gui_callbacks[] = {
		//TODO: the naming is a bit arbitrary: widgets with different
		//      types of elements use add_node, widgets with only
		//      one type of element use add_element
		{ "add_item_of_type",   &intf_add_item_of_type },
		{ "add_item",           &intf_add_dialog_item },
		{ "focus",              &intf_set_dialog_focus },
		{ "set_canvas",         &intf_set_dialog_canvas },
		{ "set_callback",       &intf_set_dialog_callback },
		{ "remove_items_at",    &intf_remove_dialog_item },
		{ "clear_items",     	&intf_clear_items },
		{ "find",               &intf_find_widget },
		{ "close",              &intf_dialog_close },
		{ nullptr, nullptr },
	};
	lua_newtable(L);
	luaL_setfuncs(L, gui_callbacks, 0);
	return 1;
}
}
