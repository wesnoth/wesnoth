/*
   Copyright (C) 2014 - 2016 by Chris Beck <render787@gmail.com>
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

#include "gui/auxiliary/old_markup.hpp"
#include "gui/core/canvas.hpp"     // for tcanvas
#include "gui/core/window_builder.hpp"  // for twindow_builder, etc
#include "gui/dialogs/drop_down_list.hpp"
#include "gui/dialogs/gamestate_inspector.hpp"
#include "gui/dialogs/lua_interpreter.hpp"
#include "gui/dialogs/wml_message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/clickable.hpp"    // for tclickable_
#include "gui/widgets/control.hpp"      // for tcontrol
#include "gui/widgets/multi_page.hpp"   // for tmulti_page
#include "gui/widgets/progress_bar.hpp"  // for tprogress_bar
#include "gui/widgets/selectable.hpp"   // for tselectable_
#include "gui/widgets/slider.hpp"       // for tslider
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/text_box.hpp"     // for ttext_box
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/widget.hpp"       // for twidget
#include "gui/widgets/window.hpp"       // for twindow

#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif

#include "config.hpp"
#include "log.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_unit.hpp"
#include "scripting/lua_unit_type.hpp"
#include "scripting/push_check.hpp"
#include "serialization/string_utils.hpp"
#include "tstring.hpp"
#include "game_data.hpp"
#include "game_state.hpp"

#include "utils/functional.hpp"

#include <map>
#include <utility>
#include <vector>

#include "lua/lauxlib.h"                // for luaL_checkinteger, etc
#include "lua/lua.h"                    // for lua_setfield, etc

class CVideo;

static lg::log_domain log_scripting_lua("scripting/lua");
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

static const char dlgclbkKey[] = "dialog callback";

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

	scoped_dialog *scoped_dialog::current = nullptr;

	scoped_dialog::scoped_dialog(lua_State *l, gui2::twindow *w)
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
		delete window;
		current = prev;
		lua_pushstring(L, dlgclbkKey);
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
		luaW_type_error(L, i, "string");
		error_call_destructors_3:
		luaL_argerror(L, i, "widget not found");
		return nullptr;
	}

	gui2::twidget *w = scoped_dialog::current->window;
	for (; !lua_isnoneornil(L, i); ++i)
	{
#ifdef GUI2_EXPERIMENTAL_LISTBOX
		if (gui2::tlist *list = dynamic_cast<gui2::tlist *>(w))
#else
		if (gui2::tlistbox *list = dynamic_cast<gui2::tlistbox *>(w))
#endif
		{
			int v = lua_tointeger(L, i);
			if (v < 1)
				goto error_call_destructors_1;
			int n = list->get_item_count();
			if (v > n) {
				if (readonly)
					goto error_call_destructors_1;
				utils::string_map dummy;
				for (; n < v; ++n)
					list->add_row(dummy);
			}
			w = list->get_row_grid(v - 1);
		}
		else if (gui2::tmulti_page *multi_page = dynamic_cast<gui2::tmulti_page *>(w))
		{
			int v = lua_tointeger(L, i);
			if (v < 1)
				goto error_call_destructors_1;
			int n = multi_page->get_page_count();
			if (v > n) {
				if (readonly)
					goto error_call_destructors_1;
				utils::string_map dummy;
				for (; n < v; ++n)
					multi_page->add_page(dummy);
			}
			w = &multi_page->page_grid(v - 1);
		}
		else if (gui2::ttree_view *tree_view = dynamic_cast<gui2::ttree_view *>(w))
		{
			gui2::ttree_view_node& tvn = tree_view->get_root_node();
			if(lua_isnumber(L, i))
			{
				int v = lua_tointeger(L, i);
				if (v < 1)
					goto error_call_destructors_1;
				int n = tvn.count_children();
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
		else if (gui2::ttree_view_node *tree_view_node = dynamic_cast<gui2::ttree_view_node *>(w))
		{
			if(lua_isnumber(L, i))
			{
				int v = lua_tointeger(L, i);
				if (v < 1)
					goto error_call_destructors_1;
				int n = tree_view_node->count_children();
				if (v > n) {
					goto error_call_destructors_1;
				}
				w = &tree_view_node->get_child_at(v - 1);

			}
			else
			{
				std::string m = luaL_checkstring(L, i);
				w = tree_view_node->find(m, false);
			}
		}
		else if(gui2::tstacked_widget* stacked_widget = dynamic_cast<gui2::tstacked_widget*>(w)) {
			if(lua_isnumber(L, i)) {
				int v = lua_tointeger(L, i);
				if(v < 1) {
					goto error_call_destructors_1;
				}
				int n = stacked_widget->get_layer_count();
				if(v > n) {
					goto error_call_destructors_1;
				}
				w = stacked_widget->get_layer_grid(v - 1);
			} else {
				std::string m = luaL_checkstring(L, i);
				w = stacked_widget->find(m, false);
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
 * Displays a message window
 * - Arg 1: Table describing the window
 * - Arg 2: List of options (nil or empty table - no options)
 * - Arg 3: Text input specifications (nil or empty table - no text input)
 * - Ret 1: option chosen (if no options: 0 if there's text input, -2 if escape pressed, else -1)
 * - Ret 2: string entered (empty if none, nil if no text input)
 */
int show_message_dialog(lua_State *L, CVideo & video)
{
	config txt_cfg;
	const bool has_input = !lua_isnoneornil(L, 3) && luaW_toconfig(L, 3, txt_cfg) && !txt_cfg.empty();

	gui2::twml_message_input input;
	input.caption = txt_cfg["label"].str();
	input.text = txt_cfg["text"].str();
	input.maximum_length = txt_cfg["max_length"].to_int(256);
	input.text_input_was_specified = has_input;

	gui2::twml_message_options options;
	if (!lua_isnoneornil(L, 2)) {
		luaL_checktype(L, 2, LUA_TTABLE);
		size_t n = lua_rawlen(L, 2);
		for(size_t i = 1; i <= n; i++) {
			lua_rawgeti(L, 2, i);
			t_string short_opt;
			config opt;
			if(luaW_totstring(L, -1, short_opt)) {
				// Note: Although this currently uses the tlegacy_menu_item class
				// for the deprecated syntax, this branch should still be retained
				// when the deprecated syntax is removed, as a simpler method
				// of specifying options when only a single string is needed.
				const std::string& opt_str = short_opt;
				gui2::tlegacy_menu_item item(opt_str);
				opt["image"] = item.icon();
				opt["label"] = item.label();
				opt["description"] = item.description();
				opt["default"] = item.is_default();
				if(!opt["image"].blank() || !opt["description"].blank() || !opt["default"].blank()) {
					// The exact error message depends on whether & or = was actually present
					if(opt_str.find_first_of('=') == std::string::npos) {
						// They just used a simple message, so the other error would be misleading
						ERR_LUA << "[option]message= is deprecated, use label= instead.\n";
					} else {
						ERR_LUA << "The &image=col1=col2 syntax is deprecated, use new DescriptionWML instead.\n";
					}
				}
			} else if(!luaW_toconfig(L, -1, opt)) {
				std::ostringstream error;
				error << "expected array of config and/or translatable strings, but index ";
				error << i << " was a " << lua_typename(L, lua_type(L, -1));
				return luaL_argerror(L, 2, error.str().c_str());
			}
			gui2::twml_message_option option(opt["label"], opt["description"], opt["image"]);
			if(opt["default"].to_bool(false)) {
				options.chosen_option = i - 1;
			}
			options.option_list.push_back(option);
			lua_pop(L, 1);
		}
		lua_getfield(L, 2, "default");
		if(lua_isnumber(L, -1)) {
			int i = lua_tointeger(L, -1);
			if(i < 1 || size_t(i) > n) {
				std::ostringstream error;
				error << "default= key in options list is not a valid option index (1-" << n << ")";
				return luaL_argerror(L, 2, error.str().c_str());
			}
			options.chosen_option = i - 1;
		}
		lua_pop(L, 1);
	}

	const config& def_cfg = luaW_checkconfig(L, 1);
	const std::string& title = def_cfg["title"];
	const std::string& message = def_cfg["message"];

	using portrait = gui2::twml_message_portrait;
	std::unique_ptr<portrait> left;
	std::unique_ptr<portrait> right;
	const bool is_double = def_cfg.has_attribute("second_portrait");
	const bool left_side = def_cfg["left_side"].to_bool(true);
	if(is_double || left_side) {
		left.reset(new portrait {def_cfg["portrait"], def_cfg["mirror"].to_bool(false)});
	} else {
		// This means right side only.
		right.reset(new portrait {def_cfg["portrait"], def_cfg["mirror"].to_bool(false)});
	}
	if(is_double) {
		right.reset(new portrait {def_cfg["second_portrait"], def_cfg["second_mirror"].to_bool(false)});
	}

	int dlg_result = gui2::show_wml_message(video, title, message, left.get(), right.get(), options, input);

	if (!has_input && options.option_list.empty()) {
		lua_pushinteger(L, dlg_result);
	} else {
		lua_pushinteger(L, options.chosen_option + 1);
	}

	if (has_input) {
		lua_pushlstring(L, input.text.c_str(), input.text.length());
	} else {
		lua_pushnil(L);
	}

	return 2;
}

/**
 * Displays a popup message
 * - Arg 1: Title (allows Pango markup)
 * - Arg 2: Message (allows Pango markup)
 * - Arg 3: Image (optional)
 */
int show_popup_dialog(lua_State *L, CVideo & video) {
	std::string title = luaL_checkstring(L, 1);
	std::string msg = luaL_checkstring(L, 2);
	std::string image = lua_isnoneornil(L, 3) ? "" : luaL_checkstring(L, 3);

	gui2::show_transient_message(video, title, msg, image, true, true);
	return 0;
}

/**
 * Displays a popup menu at the current mouse position
 * Best used from a [set_menu_item], to show a submenu
 * - Arg 1: Configs defining each item, with keys icon, image/label, second_label, tooltip
 * - Args 2, 3: Initial selection (integer); whether to parse markup (boolean)
 */
int show_menu(lua_State* L, CVideo& video) {
	std::vector<config> items = lua_check<std::vector<config>>(L, 1);
	SDL_Rect pos = {1,1,1,1};
	SDL_GetMouseState(&pos.x, &pos.y);

	int initial = -1;
	bool markup = false;
	if(lua_isnumber(L, 2)) {
		initial = lua_tointeger(L, 2) - 1;
		markup = luaW_toboolean(L, 3);
	} else if(lua_isnumber(L, 3)) {
		initial = lua_tointeger(L, 3) - 1;
		markup = luaW_toboolean(L, 2);
	}

	gui2::tdrop_down_list menu(pos, items, initial, markup);
	menu.show(video);
	lua_pushinteger(L, menu.selected_item() + 1);
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
	if (gui2::tlist *list = dynamic_cast<gui2::tlist *>(w))
#else
	if (gui2::tlistbox *list = dynamic_cast<gui2::tlistbox *>(w))
#endif
	{
		int v = luaL_checkinteger(L, 1);
		int n = list->get_item_count();
		if (1 <= v && v <= n)
			list->select_row(v - 1);
		else
			return luaL_argerror(L, 1, "out of bounds");
	}
	else if (gui2::tmulti_page *multi_page = dynamic_cast<gui2::tmulti_page *>(w))
	{
		int v = luaL_checkinteger(L, 1);
		int n = multi_page->get_page_count();
		if (1 <= v && v <= n)
			multi_page->select_page(v - 1);
		else
			return luaL_argerror(L, 1, "out of bounds");
	}
	else if (gui2::tselectable_ *selectable = dynamic_cast<gui2::tselectable_ *>(w))
	{
		if(selectable->num_states() == 2) {
			selectable->set_value_bool(luaW_toboolean(L, 1));
		}
		else {
			selectable->set_value(luaL_checkinteger(L, 1) -1);
		}
	}
	else if (gui2::ttext_box *text_box = dynamic_cast<gui2::ttext_box *>(w))
	{
		const t_string& text = luaW_checktstring(L, 1);
		text_box->set_value(text.str());
	}
	else if (gui2::tslider *slider = dynamic_cast<gui2::tslider *>(w))
	{
		const int v = luaL_checkinteger(L, 1);
		const int m = slider->get_minimum_value();
		const int n = slider->get_maximum_value();
		if (m <= v && v <= n)
			slider->set_value(v);
		else
			return luaL_argerror(L, 1, "out of bounds");
	}
	else if (gui2::tprogress_bar *progress_bar = dynamic_cast<gui2::tprogress_bar *>(w))
	{
		const int v = luaL_checkinteger(L, 1);
		if (0 <= v && v <= 100)
			progress_bar->set_percentage(v);
		else
			return luaL_argerror(L, 1, "out of bounds");
	}
	else if(gui2::tstacked_widget* stacked_widget = dynamic_cast<gui2::tstacked_widget*>(w)) {
		const int v = luaL_checkinteger(L, 1);
		const int n = stacked_widget->get_layer_count();
		if(v >= 0 && v <= n) {
			stacked_widget->select_layer(v - 1);
		}
	}
	else if(gui2::tunit_preview_pane* unit_preview_pane = dynamic_cast<gui2::tunit_preview_pane*>(w)) {
		if(const unit_type* ut = luaW_tounittype(L, 1)) {
			unit_preview_pane->set_displayed_type(*ut);
		} else if(unit* u = luaW_tounit(L, 1)) {
			unit_preview_pane->set_displayed_unit(*u);
		} else {
			return luaW_type_error(L, 1, "unit or unit type");
		}
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
	if (gui2::tlist *list = dynamic_cast<gui2::tlist *>(w))
#else
	if (gui2::tlistbox *list = dynamic_cast<gui2::tlistbox *>(w))
#endif
	{
		lua_pushinteger(L, list->get_selected_row() + 1);
	} else if (gui2::tmulti_page *multi_page = dynamic_cast<gui2::tmulti_page *>(w)) {
		lua_pushinteger(L, multi_page->get_selected_page() + 1);
	} else if (gui2::tselectable_ *selectable = dynamic_cast<gui2::tselectable_ *>(w)) {

		if(selectable->num_states() == 2) {
			lua_pushboolean(L, selectable->get_value_bool());
		}
		else {
			lua_pushinteger(L, selectable->get_value() + 1);
		}
	} else if (gui2::ttext_box *text_box = dynamic_cast<gui2::ttext_box *>(w)) {
		lua_pushstring(L, text_box->get_value().c_str());
	} else if (gui2::tslider *slider = dynamic_cast<gui2::tslider *>(w)) {
		lua_pushinteger(L, slider->get_value());
	} else if (gui2::tprogress_bar *progress_bar = dynamic_cast<gui2::tprogress_bar *>(w)) {
		lua_pushinteger(L, progress_bar->get_percentage());
	} else if (gui2::ttree_view *tree_view = dynamic_cast<gui2::ttree_view *>(w)) {
		std::vector<int> path = tree_view->selected_item()->describe_path();
		lua_createtable(L, path.size(), 0);
		for(size_t i =0; i < path.size(); ++i) {
			lua_pushinteger(L, path[i] + 1);
			lua_rawseti(L, -2, i + 1);
		}
	} else if(gui2::tstacked_widget* stacked_widget = dynamic_cast<gui2::tstacked_widget*>(w)) {
		lua_pushinteger(L, stacked_widget->current_layer());
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
		if(pos >= node.count_children()) {
			return;
		}
		if(number <= 0 || number + pos > node.count_children()) {
			number = node.count_children() - pos;
		}
		for (int i = 0; i < number; ++i) {
			tv.remove_node(&node.get_child_at(pos));
		}
	}
}
/**
 * Removes an entry from a list.
 * - Arg 1: number, index of the element to delete.
 * - Arg 2: number, number of the elements to delete. (0 to delete all elements after index)
 * - Args 2..n: path of strings and integers.
 */
int intf_remove_dialog_item(lua_State *L)
{
	int pos = luaL_checkinteger(L, 1) - 1;
	int number = luaL_checkinteger(L, 2);
	gui2::twidget *w = find_widget(L, 3, true);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	if (gui2::tlist *list = dynamic_cast<gui2::tlist *>(w))
#else
	if (gui2::tlistbox *list = dynamic_cast<gui2::tlistbox *>(w))
#endif
	{
		list->remove_row(pos, number);
	} else if (gui2::tmulti_page *multi_page = dynamic_cast<gui2::tmulti_page *>(w)) {
		multi_page->remove_page(pos, number);
	} else if (gui2::ttree_view *tree_view = dynamic_cast<gui2::ttree_view *>(w)) {
		remove_treeview_node(tree_view->get_root_node(), pos, number);
	} else if (gui2::ttree_view_node *tree_view_node = dynamic_cast<gui2::ttree_view_node *>(w)) {
		remove_treeview_node(*tree_view_node, pos, number);
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
		lua_pushstring(L, dlgclbkKey);
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
		lua_pushstring(L, dlgclbkKey);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_pushnil(L);
		lua_rawseti(L, -2, i->second);
		lua_pop(L, 1);
		m.erase(i);
	}

	if (lua_isnil(L, 1)) return 0;

	if (gui2::tclickable_ *c = dynamic_cast<gui2::tclickable_ *>(w)) {
		static tdialog_callback_wrapper wrapper;
		c->connect_click_handler(std::bind(&tdialog_callback_wrapper::forward, wrapper, w));
	} else if (gui2::tselectable_ *s = dynamic_cast<gui2::tselectable_ *>(w)) {
		s->set_callback_state_change(&dialog_callback);
	}
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	else if (gui2::tlist *l = dynamic_cast<gui2::tlist *>(w)) {
		static tdialog_callback_wrapper wrapper;
		connect_signal_notify_modified(*l
				, std::bind(
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

	lua_pushstring(L, dlgclbkKey);
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
	c->set_is_dirty(true);
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

/**
 * Sets the visiblity of a widget in the current dialog.
 * - Arg 1: boolean.
 * - Args 2..n: path of strings and integers.
 */
int intf_set_dialog_visible(lua_State *L)
{
	typedef gui2::tcontrol::tvisible tvisible;

	tvisible flag = tvisible::visible;

	switch (lua_type(L, 1)) {
		case LUA_TBOOLEAN:
			flag = luaW_toboolean(L, 1)
					? tvisible::visible
					: tvisible::invisible;
			break;
		case LUA_TSTRING:
			{
				const std::string& str = lua_tostring(L, 1);
				if(str == "visible") {
					flag = tvisible::visible;
				} else if(str == "hidden") {
					flag = tvisible::hidden;
				} else if(str == "invisible") {
					flag = tvisible::invisible;
				} else {
					return luaL_argerror(L, 1, "string must be one of: visible, hidden, invisible");
				}
			}
			break;
		default:
			return luaW_type_error(L, 1, "boolean or string");
	}

	gui2::twidget *w = find_widget(L, 2, true);
	gui2::tcontrol *c = dynamic_cast<gui2::tcontrol *>(w);
	if (!c) return luaL_argerror(L, lua_gettop(L), "unsupported widget");

	c->set_visible(flag);

	if(flag == tvisible::hidden) {
		// HACK: this is needed to force the widget to be repainted immediately
		//       to get rid of its ghost image.
		scoped_dialog::current->window->invalidate_layout();
	}

	return 0;
}

int show_lua_console(lua_State * /*L*/, CVideo & video, lua_kernel_base * lk)
{
	gui2::tlua_interpreter::display(video, lk);
	return 0;
}

int show_gamestate_inspector(CVideo & video, const vconfig & cfg, const game_data& data, const game_state& state)
{
	gui2::tgamestate_inspector inspect_dialog(data.get_variables(), *state.events_manager_, state.board_, cfg["name"]);
	inspect_dialog.show(video);
	return 0;
}

/**
 * Sets a widget's state to active or inactive
 * - Arg 1: string, the type (id of [node_definition]) of the new node.
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
