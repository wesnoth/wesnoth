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

#include "lua_gui2.hpp"

#include "gui/auxiliary/canvas.hpp"     // for tcanvas
#include "gui/auxiliary/window_builder.hpp"  // for twindow_builder, etc
#include "gui/dialogs/gamestate_inspector.hpp"
#include "gui/dialogs/lua_interpreter.hpp"
#include "gui/widgets/clickable.hpp"    // for tclickable_
#include "gui/widgets/control.hpp"      // for tcontrol
#include "gui/widgets/multi_page.hpp"   // for tmulti_page
#include "gui/widgets/progress_bar.hpp"  // for tprogress_bar
#include "gui/widgets/selectable.hpp"   // for tselectable_
#include "gui/widgets/slider.hpp"       // for tslider
#include "gui/widgets/text_box.hpp"     // for ttext_box
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/widget.hpp"       // for twidget
#include "gui/widgets/window.hpp"       // for twindow

#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif

#include "config.hpp"
#include "log.hpp"
#include "scripting/lua_api.hpp"        // for luaW_toboolean, etc
#include "scripting/lua_common.hpp"
#include "scripting/lua_types.hpp"      // for getunitKey, dlgclbkKey, etc
#include "serialization/string_utils.hpp"
#include "tstring.hpp"
#include "video.hpp"

#include <boost/bind.hpp>

#include <map>
#include <utility>
#include <vector>

#include "lua/lauxlib.h"                // for luaL_checkinteger, etc
#include "lua/lua.h"                    // for lua_setfield, etc

static lg::log_domain log_scripting_lua("scripting/lua");
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

static const char * dlgclbkKey = "dialog callback";

namespace {
	struct scoped_dialog
	{
		lua_State *L;
		scoped_dialog *prev;
		static scoped_dialog *current;
		gui2::twindow *window;
		typedef std::map<gui2::twidget *, int> callback_map;
		callback_map callbacks;

		scoped_dialog(lua_State *l, gui2::twindow *w);
		~scoped_dialog();
	private:
		scoped_dialog(const scoped_dialog &); // not implemented; not allowed.
	};

	scoped_dialog *scoped_dialog::current = NULL;

	scoped_dialog::scoped_dialog(lua_State *l, gui2::twindow *w)
		: L(l), prev(current), window(w), callbacks()
	{
		lua_pushstring(L
				, dlgclbkKey);
		lua_createtable(L, 1, 0);
		lua_pushvalue(L, -2);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_rawseti(L, -2, 1);
		lua_rawset(L, LUA_REGISTRYINDEX);
		current = this;
	}

	scoped_dialog::~scoped_dialog()
	{
		delete window;
		current = prev;
		lua_pushstring(L
				, dlgclbkKey);
		lua_pushvalue(L, -1);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_rawgeti(L, -1, 1);
		lua_remove(L, -2);
		lua_rawset(L, LUA_REGISTRYINDEX);
	}
}//unnamed namespace for scoped_dialog

static gui2::twidget *find_widget(lua_State *L, int i, bool readonly)
{
	if (!scoped_dialog::current) {
		luaL_error(L, "no visible dialog");
		error_call_destructors_1:
		luaL_argerror(L, i, "out of bounds");
		error_call_destructors_2:
		luaL_typerror(L, i, "string");
		error_call_destructors_3:
		luaL_argerror(L, i, "widget not found");
		return NULL;
	}

	gui2::twidget *w = scoped_dialog::current->window;
	for (; !lua_isnoneornil(L, i); ++i)
	{
#ifdef GUI2_EXPERIMENTAL_LISTBOX
		if (gui2::tlist *l = dynamic_cast<gui2::tlist *>(w))
#else
		if (gui2::tlistbox *l = dynamic_cast<gui2::tlistbox *>(w))
#endif
		{
			int v = lua_tointeger(L, i);
			if (v < 1)
				goto error_call_destructors_1;
			int n = l->get_item_count();
			if (v > n) {
				if (readonly)
					goto error_call_destructors_1;
				utils::string_map dummy;
				for (; n < v; ++n)
					l->add_row(dummy);
			}
			w = l->get_row_grid(v - 1);
		}
		else if (gui2::tmulti_page *l = dynamic_cast<gui2::tmulti_page *>(w))
		{
			int v = lua_tointeger(L, i);
			if (v < 1)
				goto error_call_destructors_1;
			int n = l->get_page_count();
			if (v > n) {
				if (readonly)
					goto error_call_destructors_1;
				utils::string_map dummy;
				for (; n < v; ++n)
					l->add_page(dummy);
			}
			w = &l->page_grid(v - 1);
		}
		else if (gui2::ttree_view *tv = dynamic_cast<gui2::ttree_view *>(w))
		{
			gui2::ttree_view_node& tvn = tv->get_root_node();
			if(lua_isnumber(L, i))
			{
				int v = lua_tointeger(L, i);
				if (v < 1)
					goto error_call_destructors_1;
				int n = tvn.size();
				if (v > n) {
					goto error_call_destructors_1;
				}
				w = &tvn.get_child_at(v - 1);

			}
			else
			{
				std::string m = luaL_checkstring(L, i);
				w = tvn.find(m, false);
			}
		}
		else if (gui2::ttree_view_node *tvn = dynamic_cast<gui2::ttree_view_node *>(w))
		{
			if(lua_isnumber(L, i))
			{
				int v = lua_tointeger(L, i);
				if (v < 1)
					goto error_call_destructors_1;
				int n = tvn->size();
				if (v > n) {
					goto error_call_destructors_1;
				}
				w = &tvn->get_child_at(v - 1);

			}
			else
			{
				std::string m = luaL_checkstring(L, i);
				w = tvn->find(m, false);
			}
		}
		else
		{
			char const *m = lua_tostring(L, i);
			if (!m) goto error_call_destructors_2;
			w = w->find(m, false);
		}
		if (!w) goto error_call_destructors_3;
	}

	return w;
}

namespace lua_gui2 {

/**
 * Displays a window.
 * - Arg 1: WML table describing the window.
 * - Arg 2: function called at pre-show.
 * - Arg 3: function called at post-show.
 * - Ret 1: integer.
 */
int show_dialog(lua_State *L, CVideo & video)
{
	config def_cfg = luaW_checkconfig(L, 1);

	gui2::twindow_builder::tresolution def(def_cfg);
	scoped_dialog w(L, gui2::build(video, &def));

	if (!lua_isnoneornil(L, 2)) {
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

/**
 * Sets the value of a widget on the current dialog.
 * - Arg 1: scalar.
 * - Args 2..n: path of strings and integers.
 */
int intf_set_dialog_value(lua_State *L)
{
	gui2::twidget *w = find_widget(L, 2, false);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	if (gui2::tlist *l = dynamic_cast<gui2::tlist *>(w))
#else
	if (gui2::tlistbox *l = dynamic_cast<gui2::tlistbox *>(w))
#endif
	{
		int v = luaL_checkinteger(L, 1);
		int n = l->get_item_count();
		if (1 <= v && v <= n)
			l->select_row(v - 1);
		else
			return luaL_argerror(L, 1, "out of bounds");
	}
	else if (gui2::tmulti_page *l = dynamic_cast<gui2::tmulti_page *>(w))
	{
		int v = luaL_checkinteger(L, 1);
		int n = l->get_page_count();
		if (1 <= v && v <= n)
			l->select_page(v - 1);
		else
			return luaL_argerror(L, 1, "out of bounds");
	}
	else if (gui2::tselectable_ *s = dynamic_cast<gui2::tselectable_ *>(w))
	{
		s->set_value(luaW_toboolean(L, 1));
	}
	else if (gui2::ttext_box *t = dynamic_cast<gui2::ttext_box *>(w))
	{
		const t_string& text = luaW_checktstring(L, 1);
		t->set_value(text.str());
	}
	else if (gui2::tslider *s = dynamic_cast<gui2::tslider *>(w))
	{
		const int v = luaL_checkinteger(L, 1);
		const int m = s->get_minimum_value();
		const int n = s->get_maximum_value();
		if (m <= v && v <= n)
			s->set_value(v);
		else
			return luaL_argerror(L, 1, "out of bounds");
	}
	else if (gui2::tprogress_bar *p = dynamic_cast<gui2::tprogress_bar *>(w))
	{
		const int v = luaL_checkinteger(L, 1);
		if (0 <= v && v <= 100)
			p->set_percentage(v);
		else
			return luaL_argerror(L, 1, "out of bounds");
	}
	else
	{
		t_string v = luaW_checktstring(L, 1);
		gui2::tcontrol *c = dynamic_cast<gui2::tcontrol *>(w);
		if (!c) return luaL_argerror(L, lua_gettop(L), "unsupported widget");
		c->set_label(v);
	}

	return 0;
}

/**
 * Gets the value of a widget on the current dialog.
 * - Args 1..n: path of strings and integers.
 * - Ret 1: scalar.
 */
int intf_get_dialog_value(lua_State *L)
{
	gui2::twidget *w = find_widget(L, 1, true);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	if (gui2::tlist *l = dynamic_cast<gui2::tlist *>(w))
#else
	if (gui2::tlistbox *l = dynamic_cast<gui2::tlistbox *>(w))
#endif
	{
		lua_pushinteger(L, l->get_selected_row() + 1);
	} else if (gui2::tmulti_page *l = dynamic_cast<gui2::tmulti_page *>(w)) {
		lua_pushinteger(L, l->get_selected_page() + 1);
	} else if (gui2::tselectable_ *s = dynamic_cast<gui2::tselectable_ *>(w)) {
		lua_pushboolean(L, s->get_value());
	} else if (gui2::ttext_box *t = dynamic_cast<gui2::ttext_box *>(w)) {
		lua_pushstring(L, t->get_value().c_str());
	} else if (gui2::tslider *s = dynamic_cast<gui2::tslider *>(w)) {
		lua_pushinteger(L, s->get_value());
	} else if (gui2::tprogress_bar *p = dynamic_cast<gui2::tprogress_bar *>(w)) {
		lua_pushinteger(L, p->get_percentage());
	} else if (gui2::ttree_view *tv = dynamic_cast<gui2::ttree_view *>(w)) {
		std::vector<int> path = tv->selected_item()->describe_path();
		lua_createtable(L, path.size(), 0);
		for(size_t i =0; i < path.size(); ++i) {
			lua_pushinteger(L, path[i] + 1);
			lua_rawseti(L, -2, i + 1);
		}
	} else
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");

	return 1;
}
namespace
{
	void remove_treeview_node(gui2::ttree_view_node& node, size_t pos, int number)
	{
		//Not tested yet.
		gui2::ttree_view& tv = node.tree_view();
		if(pos >= node.size()) {
			return;
		}
		if(number <= 0 || number + pos > node.size()) {
			number = node.size() - pos;
		}
		for (int i = 0; i < number; ++i) {
			tv.remove_node(&node.get_child_at(pos));
		}
	}
}
/**
 * Removes an entry from a list.
 * - Arg 1: number, index of the element to delete.
 * - Arg 2: number, number of teh elements to delete. (0 to delete all elements after index)
 * - Args 2..n: path of strings and integers.
 */
int intf_remove_dialog_item(lua_State *L)
{
	int pos = luaL_checkinteger(L, 1) - 1;
	int number = luaL_checkinteger(L, 2);
	gui2::twidget *w = find_widget(L, 3, true);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	if (gui2::tlist *l = dynamic_cast<gui2::tlist *>(w))
#else
	if (gui2::tlistbox *l = dynamic_cast<gui2::tlistbox *>(w))
#endif
	{
		l->remove_row(pos, number);
	} else if (gui2::tmulti_page *l = dynamic_cast<gui2::tmulti_page *>(w)) {
		l->remove_page(pos, number);
	} else if (gui2::ttree_view *tv = dynamic_cast<gui2::ttree_view *>(w)) {
		remove_treeview_node(tv->get_root_node(), pos, number);
	} else if (gui2::ttree_view_node *tvn = dynamic_cast<gui2::ttree_view_node *>(w)) {
		remove_treeview_node(*tvn, pos, number);
	} else
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");

	return 1;
}

namespace { // helpers of intf_set_dialog_callback()
	void dialog_callback(gui2::twidget& w)
	{
		int cb;
		{
			scoped_dialog::callback_map &m = scoped_dialog::current->callbacks;
			scoped_dialog::callback_map::const_iterator i = m.find(&w);
			if (i == m.end()) return;
			cb = i->second;
		}
		lua_State *L = scoped_dialog::current->L;
		lua_pushstring(L
				, dlgclbkKey);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_rawgeti(L, -1, cb);
		lua_remove(L, -2);
		lua_call(L, 0, 0);
	}

	/** Helper struct for intf_set_dialog_callback. */
	struct tdialog_callback_wrapper
	{
		void forward(gui2::twidget* widget)
		{
			assert(widget);
			dialog_callback(*widget);
		}
	};
}//unnamed namespace for helpers of intf_set_dialog_callback()

/**
 * Sets a callback on a widget of the current dialog.
 * - Arg 1: function.
 * - Args 2..n: path of strings and integers.
 */
int intf_set_dialog_callback(lua_State *L)
{
	gui2::twidget *w = find_widget(L, 2, true);

	scoped_dialog::callback_map &m = scoped_dialog::current->callbacks;
	scoped_dialog::callback_map::iterator i = m.find(w);
	if (i != m.end())
	{
		lua_pushstring(L
				, dlgclbkKey);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_pushnil(L);
		lua_rawseti(L, -2, i->second);
		lua_pop(L, 1);
		m.erase(i);
	}

	if (lua_isnil(L, 1)) return 0;

	if (gui2::tclickable_ *c = dynamic_cast<gui2::tclickable_ *>(w)) {
		static tdialog_callback_wrapper wrapper;
		c->connect_click_handler(boost::bind(
									  &tdialog_callback_wrapper::forward
									, wrapper
									, w));
	} else if (gui2::tselectable_ *s = dynamic_cast<gui2::tselectable_ *>(w)) {
		s->set_callback_state_change(&dialog_callback);
	}
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	else if (gui2::tlist *l = dynamic_cast<gui2::tlist *>(w)) {
		static tdialog_callback_wrapper wrapper;
		connect_signal_notify_modified(*l
				, boost::bind(
					  &tdialog_callback_wrapper::forward
					, wrapper
					, w));
	}
#else
	else if (gui2::tlistbox *l = dynamic_cast<gui2::tlistbox *>(w)) {
		l->set_callback_value_change(&dialog_callback);
	}
#endif
	else if (gui2::ttree_view *tv = dynamic_cast<gui2::ttree_view *>(w)) {
		tv->set_selection_change_callback(&dialog_callback);
	}
	else
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");

	lua_pushstring(L
			, dlgclbkKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	int n = lua_rawlen(L, -1) + 1;
	m[w] = n;
	lua_pushvalue(L, 1);
	lua_rawseti(L, -2, n);
	lua_pop(L, 1);

	return 0;
}

/**
 * Enables/disables Pango markup on the label of a widget of the current dialog.
 * - Arg 1: boolean.
 * - Args 2..n: path of strings and integers.
 */
int intf_set_dialog_markup(lua_State *L)
{
	bool b = luaW_toboolean(L, 1);
	gui2::twidget *w = find_widget(L, 2, true);
	gui2::tcontrol *c = dynamic_cast<gui2::tcontrol *>(w);
	if (!c) return luaL_argerror(L, lua_gettop(L), "unsupported widget");

	c->set_use_markup(b);
	return 0;
}

/**
 * Sets a canvas on a widget of the current dialog.
 * - Arg 1: integer.
 * - Arg 2: WML table.
 * - Args 3..n: path of strings and integers.
 */
int intf_set_dialog_canvas(lua_State *L)
{
	int i = luaL_checkinteger(L, 1);
	gui2::twidget *w = find_widget(L, 3, true);
	gui2::tcontrol *c = dynamic_cast<gui2::tcontrol *>(w);
	if (!c) return luaL_argerror(L, lua_gettop(L), "unsupported widget");

	std::vector<gui2::tcanvas> &cv = c->canvas();
	if (i < 1 || unsigned(i) > cv.size())
		return luaL_argerror(L, 1, "out of bounds");

	config cfg = luaW_checkconfig(L, 2);
	cv[i - 1].set_cfg(cfg);
	return 0;
}

/**
 * Sets a widget to have the focus
 * - Args 1..n: path of strings and integers.
 */
int intf_set_dialog_focus(lua_State *L)
{
	gui2::twidget *w = find_widget(L, 1, true);
	scoped_dialog::current->window->keyboard_capture(w);
	return 0;
}

/**
 * Sets a widget's state to active or inactive
 * - Arg 1: boolean.
 * - Args 2..n: path of strings and integers.
 */
int intf_set_dialog_active(lua_State *L)
{
	const bool b = luaW_toboolean(L, 1);
	gui2::twidget *w = find_widget(L, 2, true);
	gui2::tcontrol *c = dynamic_cast<gui2::tcontrol *>(w);
	if (!c) return luaL_argerror(L, lua_gettop(L), "unsupported widget");

	c->set_active(b);
	return 0;
}

int show_lua_console(lua_State * /*L*/, CVideo & video, lua_kernel_base * lk)
{
	gui2::tlua_interpreter::display(video, lk);
	return 0;
}

int show_gamestate_inspector(CVideo & video, const vconfig & cfg)
{
	gui2::tgamestate_inspector inspect_dialog(cfg);
	inspect_dialog.show(video);
	return 0;
}

/**
 * Sets a widget's state to active or inactive
 * - Arg 1: string, the type (id of [node_definition]) of teh new node.
 * - Arg 3: integer, where to insert the new node.
 * - Args 3..n: path of strings and integers.
 */

int intf_add_dialog_tree_node(lua_State *L)
{
	const std::string node_type = luaL_checkstring(L, 1);
	const int insert_pos = luaL_checkinteger(L, 2);
	static const std::map<std::string, string_map> data;
	gui2::twidget *w = find_widget(L, 3, false);
	gui2::ttree_view_node *twn = dynamic_cast<gui2::ttree_view_node *>(w);
	if (!twn) {
		if(gui2::ttree_view* tw = dynamic_cast<gui2::ttree_view *>(w)) {
			twn = &tw->get_root_node();
		}
		else {
			return luaL_argerror(L, lua_gettop(L), "unsupported widget");
		}
	}
	twn->add_child(node_type, data, insert_pos);
	return 0;
}

} // end namespace lua_gui2
