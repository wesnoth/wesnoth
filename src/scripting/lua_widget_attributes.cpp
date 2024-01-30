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

#include "gui/core/canvas.hpp"
#include "gui/dialogs/drop_down_menu.hpp"
#include "gui/auxiliary/iterator/iterator.hpp"
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
#include "config.hpp"
#include "log.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_cpp_function.hpp"
#include "scripting/lua_kernel_base.hpp"
#include "scripting/lua_unit.hpp"
#include "scripting/lua_unit_type.hpp"
#include "scripting/push_check.hpp"
#include "scripting/lua_widget.hpp"
#include "scripting/lua_widget_attributes.hpp"
#include "serialization/string_utils.hpp"
#include "tstring.hpp"
#include "game_data.hpp"
#include "game_state.hpp"

#include <functional>
#include "serialization/string_utils.hpp"

#include <boost/preprocessor/cat.hpp>

#include <map>
#include <utility>
#include <vector>

#include "lua/wrapper_lauxlib.h"        // for luaL_checkinteger, lua_setfield, etc

static lg::log_domain log_scripting_lua("scripting/lua");
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

static gui2::widget* find_child_by_index(gui2::widget& w, int i)
{
	assert(i > 0);
	if(gui2::listbox* list = dynamic_cast<gui2::listbox*>(&w)) {
		int n = list->get_item_count();
		if(i > n) {
			for(; n < i; ++n) {
				list->add_row(gui2::widget_item{});
			}
		}
		return list->get_row_grid(i - 1);
	} else if(gui2::multi_page* multi_page = dynamic_cast<gui2::multi_page*>(&w)) {
		int n = multi_page->get_page_count();
		if(i > n) {
			for(; n < i; ++n) {
				multi_page->add_page(gui2::widget_item{});
			}
		}
		return &multi_page->page_grid(i - 1);
	} else if(gui2::tree_view* tree_view = dynamic_cast<gui2::tree_view*>(&w)) {
		gui2::tree_view_node& tvn = tree_view->get_root_node();
		int n = tvn.count_children();
		if(i > n) {
			throw std::invalid_argument("out of range");
		}
		return &tvn.get_child_at(i - 1);
	} else if(gui2::tree_view_node* tree_view_node = dynamic_cast<gui2::tree_view_node*>(&w)) {
		int n = tree_view_node->count_children();
		if(i > n) {
			throw std::invalid_argument("out of range");
		}
		return &tree_view_node->get_child_at(i - 1);
	} else if(gui2::stacked_widget* stacked_widget = dynamic_cast<gui2::stacked_widget*>(&w)) {
		int n = stacked_widget->get_layer_count();
		if(i > n) {
			throw std::invalid_argument("out of range");
		}
		return stacked_widget->get_layer_grid(i - 1);
	}
	return nullptr;
}

static gui2::widget* find_child_by_name(gui2::widget& w, const std::string& m)
{
	return w.find(m, false);
}

using tgetters = std::map<std::string, std::vector<std::function<bool(lua_State*, gui2::widget&, bool)>>>;
static tgetters getters;

using tsetters = std::map<std::string, std::vector<std::function<bool(lua_State*, int, gui2::widget&, bool)>>>;
static tsetters setters;

template<typename widget_type, typename value_type>
struct widget_getter
{
	virtual value_type get(lua_State* L, widget_type& w) const = 0;
	virtual ~widget_getter() = default;
};

template<typename widget_type, typename value_type>
struct widget_setter
{
	virtual void set(lua_State* L, widget_type& w, const value_type& value) const = 0;
	virtual ~widget_setter() = default;
};

template<typename widget_type, typename value_type, typename action_type, bool setter>
void register_widget_attribute(const char* name)
{
	utils::split_foreach(name, ',', 0, [](std::string_view name_part) {
		using map_type = std::conditional_t<setter, tsetters, tgetters>;
		using list_type = typename map_type::mapped_type;
		using callback_type = typename list_type::value_type;
		map_type* map;
		callback_type fcn;
		if constexpr(setter) {
			map = &setters;
			fcn = [action = action_type()](lua_State* L, int idx, gui2::widget& w, bool nop) {
				if(widget_type* pw = dynamic_cast<widget_type*>(&w)) {
					if(!nop) action.set(L, *pw, lua_check<value_type>(L, idx));
					return true;
				}
				return false;
			};
		} else {
			map = &getters;
			fcn = [action = action_type()](lua_State* L, gui2::widget& w, bool nop) {
				if(widget_type* pw = dynamic_cast<widget_type*>(&w)) {
					if(!nop) lua_push(L, action.get(L, *pw));
					return true;
				}
				return false;
			};
		}
		list_type& list = (*map)[std::string(name_part)];
		list.push_back(fcn);
	});
}

#define WIDGET_GETTER4(name, value_type, widgt_type, id) \
struct BOOST_PP_CAT(getter_, id) : public widget_getter<widgt_type, value_type> { \
	value_type get(lua_State* L, widgt_type& w) const override; \
}; \
struct BOOST_PP_CAT(getter_adder_, id) { \
	BOOST_PP_CAT(getter_adder_, id) () \
	{ \
		register_widget_attribute<widgt_type, value_type, BOOST_PP_CAT(getter_, id), false>(name); \
	} \
}; \
static BOOST_PP_CAT(getter_adder_, id) BOOST_PP_CAT(getter_adder_instance_, id) ; \
value_type BOOST_PP_CAT(getter_, id)::get([[maybe_unused]] lua_State* L, widgt_type& w) const


#define WIDGET_SETTER4(name, value_type, widgt_type, id) \
struct BOOST_PP_CAT(setter_, id) : public widget_setter<widgt_type, value_type> { \
	void set(lua_State* L, widgt_type& w, const value_type& value) const override; \
}; \
struct BOOST_PP_CAT(setter_adder_, id) { \
	BOOST_PP_CAT(setter_adder_, id) ()\
	{ \
		register_widget_attribute<widgt_type, value_type, BOOST_PP_CAT(setter_, id), true>(name); \
	} \
}; \
static BOOST_PP_CAT(setter_adder_, id) BOOST_PP_CAT(setter_adder_instance_, id); \
void BOOST_PP_CAT(setter_, id)::set([[maybe_unused]] lua_State* L, widgt_type& w, const value_type& value) const


/**
 * @param name: string  comma seperated list
 * @param value_type: the type of the attribute, for example int or std::string
 * @param widgt_type: the type of the widget, for example gui2::listbox
 */
#define WIDGET_GETTER(name, value_type, widgt_type) WIDGET_GETTER4(name, value_type, widgt_type, __LINE__)

#define WIDGET_SETTER(name, value_type, widgt_type) WIDGET_SETTER4(name, value_type, widgt_type, __LINE__)


/// CLASSIC

WIDGET_GETTER("value_compat,selected_index", int, gui2::listbox)
{
	return w.get_selected_row() + 1;
}

WIDGET_SETTER("value_compat,selected_index", int, gui2::listbox)
{
	w.select_row(value - 1);
}

WIDGET_GETTER("value_compat,selected_index", int, gui2::multi_page)
{
	return w.get_selected_page() + 1;
}

WIDGET_SETTER("value_compat,selected_index", int, gui2::multi_page)
{
	w.select_page(value -1);
}

WIDGET_GETTER("value_compat,selected_index", int, gui2::stacked_widget)
{
	return w.current_layer() + 1;
}

WIDGET_SETTER("value_compat,selected_index", int, gui2::stacked_widget)
{
	w.select_layer(value - 1);
}

WIDGET_GETTER("selected_index", int, gui2::selectable_item)
{
	return w.get_value() + 1;
}

WIDGET_SETTER("selected_index", int, gui2::selectable_item)
{
	if(value > int(w.num_states())) {
		throw std::invalid_argument("invalid index");
	}
	w.set_value(value - 1);
}

WIDGET_GETTER("value_compat,selected", bool, gui2::selectable_item)
{
	if(w.num_states() == 2) {
		return w.get_value_bool();
	}
	throw std::invalid_argument("invalid widget");
}

WIDGET_SETTER("value_compat,selected", bool, gui2::selectable_item)
{
	w.set_value_bool(value);
}

WIDGET_GETTER("value_compat,text", std::string, gui2::text_box)
{
	return w.get_value();
}

WIDGET_SETTER("value_compat,text", std::string, gui2::text_box)
{
	w.set_value(value);
}

WIDGET_GETTER("value_compat,value", int, gui2::slider)
{
	return w.get_value();
}

WIDGET_SETTER("value_compat,value", int, gui2::slider)
{
	w.set_value(value);
}

WIDGET_GETTER("max_value", int, gui2::slider)
{
	return w.get_maximum_value();
}

WIDGET_SETTER("max_value", int, gui2::slider)
{
	w.set_value_range(w.get_minimum_value(), value);
}

WIDGET_GETTER("min_value", int, gui2::slider)
{
	return w.get_minimum_value();
}

WIDGET_SETTER("min_value", int, gui2::slider)
{
	w.set_value_range(value, w.get_maximum_value());
}

WIDGET_GETTER("value_compat,percentage", int, gui2::progress_bar)
{
	return w.get_percentage();
}

WIDGET_SETTER("value_compat,percentage", int, gui2::progress_bar)
{
	w.set_percentage(value);
}

WIDGET_GETTER("value_compat,selected_item_path", std::vector<int>, gui2::tree_view)
{
	auto res = w.selected_item()->describe_path();
	for(int& a : res) { ++a;}
	return res;
}

WIDGET_GETTER("path", std::vector<int>, gui2::tree_view_node)
{
	auto res = w.describe_path();
	for(int& a : res) { ++a;}
	return res;
}

WIDGET_SETTER("value_compat,unfolded", bool, gui2::tree_view_node)
{
	if(value) {
		w.unfold();
	} else {
		w.fold();
	}
}

WIDGET_SETTER("value_compat,unit", lua_index_raw, gui2::unit_preview_pane)
{
	if(const unit_type* ut = luaW_tounittype(L, value.index)) {
		w.set_displayed_type(*ut);
	} else if(unit* u = luaW_tounit(L, value.index)) {
		w.set_displayed_unit(*u);
	} else {
		luaW_type_error(L, value.index, "unit or unit type");
	}
}

WIDGET_GETTER("item_count", int, gui2::multi_page)
{
	return w.get_page_count();
}

WIDGET_GETTER("item_count", int, gui2::listbox)
{
	return w.get_item_count();
}

WIDGET_SETTER("use_markup", bool, gui2::styled_widget)
{
	w.set_use_markup(value);
}

//TODO: while i think this shortcut is useful, i'm not that happy about
//      the name since  it changes 'label' and not 'text', the first one
//      is the label that is part of most widgets (like checkboxes), the
//      later is specific to input textboxes.
WIDGET_SETTER("marked_up_text", t_string, gui2::styled_widget)
{
	w.set_use_markup(true);
	w.set_label(value);
}

WIDGET_SETTER("enabled", bool, gui2::styled_widget)
{
	w.set_active(value);
}

WIDGET_SETTER("tooltip", t_string, gui2::styled_widget)
{
	w.set_tooltip(value);
}


WIDGET_SETTER("callback", lua_index_raw, gui2::widget)
{
	if(!luaW_getglobal(L, "gui", "widget", "set_callback")) {
		ERR_LUA << "gui.widget.set_callback didn't exist";
	}
	luaW_pushwidget(L, w);
	lua_pushvalue(L, value.index);
	lua_call(L, 2, 0);
}

WIDGET_SETTER("visible", lua_index_raw, gui2::styled_widget)
{

	typedef gui2::styled_widget::visibility visibility;

	visibility flag = visibility::visible;

	switch(lua_type(L, value.index)) {
		case LUA_TBOOLEAN:
			flag = luaW_toboolean(L, value.index)
					? visibility::visible
					: visibility::invisible;
			break;
		case LUA_TSTRING:
			{
				const std::string& str = lua_tostring(L, value.index);
				if(str == "visible") {
					flag = visibility::visible;
				} else if(str == "hidden") {
					flag = visibility::hidden;
				} else if(str == "invisible") {
					flag = visibility::invisible;
				} else {
					luaL_argerror(L, value.index, "string must be one of: visible, hidden, invisible");
				}
			}
			break;
		default:
			luaW_type_error(L, value.index, "boolean or string");
	}

	w.set_visible(flag);

	if(flag == visibility::hidden) {
		// HACK: this is needed to force the widget to be repainted immediately
		//       to get rid of its ghost image.
		gui2::window* window = w.get_window();
		if(window) {
			window->invalidate_layout();
		}
	}
}

//must be last
WIDGET_SETTER("value_compat,label", t_string, gui2::styled_widget)
{
	gui2::window* window = w.get_window();
	if(window) {
		window->invalidate_layout();
	}
	w.set_label(value);
}

WIDGET_GETTER("type", std::string, gui2::widget)
{
	if(gui2::styled_widget* sw = dynamic_cast<gui2::styled_widget*>(&w)) {
		return sw->get_control_type();
	}
	else if(dynamic_cast<gui2::tree_view_node*>(&w)) {
		return "tree_view_node";
	}
	else if(dynamic_cast<gui2::grid*>(&w)) {
		return "grid";
	}
	else {
		return "";
	}
}

///////////////////////////////////////////////////////
////////////////////// CALLBACKS //////////////////////
///////////////////////////////////////////////////////
namespace {

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

WIDGET_SETTER("on_modified", lua_index_raw, gui2::widget)
{
	gui2::window* wd = w.get_window();
	if(!wd) {
		throw std::invalid_argument("the widget has no window assigned");
	}
	lua_pushvalue(L, value.index);
	if (!luaW_setwidgetcallback(L, &w, wd, "on_modified")) {
		connect_signal_notify_modified(w, std::bind(&dialog_callback, L, lua_ptr<gui2::widget>(w), "on_modified"));
	}
}

WIDGET_SETTER("on_left_click", lua_index_raw, gui2::widget)
{
	gui2::window* wd = w.get_window();
	if(!wd) {
		throw std::invalid_argument("the widget has no window assigned");
	}
	lua_pushvalue(L, value.index);
	if (!luaW_setwidgetcallback(L, &w, wd, "on_left_click")) {
		connect_signal_notify_modified(w, std::bind(&dialog_callback, L, lua_ptr<gui2::widget>(w), "on_left_click"));
	}
}

WIDGET_SETTER("on_button_click", lua_index_raw, gui2::widget)
{
	gui2::window* wd = w.get_window();
	gui2::clickable_item* cl = dynamic_cast<gui2::clickable_item*>(&w);

	if(!wd) {
		throw std::invalid_argument("the widget has no window assigned");
	}
	if(!cl) {
		throw std::invalid_argument("unsupported widget");
	}
	lua_pushvalue(L, value.index);
	if (!luaW_setwidgetcallback(L, &w, wd, "on_button_click")) {
		cl->connect_click_handler(std::bind(&dialog_callback, L, lua_ptr<gui2::widget>(w), "on_button_click"));
	}
}

}

namespace lua_widget {

int impl_widget_get(lua_State* L)
{
	gui2::widget& w = luaW_checkwidget(L, 1);
	if(lua_isinteger(L, 2)) {

		if(auto pwidget = find_child_by_index(w, luaL_checkinteger(L, 2))) {
			luaW_pushwidget(L, *pwidget);
			return 1;
		}

	}
	std::string_view str = lua_check<std::string_view>(L, 2);

	tgetters::iterator it = getters.find(std::string(str));
	if(it != getters.end()) {
		for(const auto& func : it->second) {
			if(func(L, w, false)) {
				return 1;
			}
		}
	}
	if(luaW_getglobal(L, "gui", "widget", std::string(str).c_str())) {
		return 1;
	}
	if(auto pwidget = find_child_by_name(w, std::string(str))) {
		luaW_pushwidget(L, *pwidget);
		return 1;
	}
	ERR_LUA << "invalid property of '" <<  typeid(w).name()<< "' widget :" << str;
	return luaL_argerror(L, 2, "invalid property of widget");
}

int impl_widget_set(lua_State* L)
{
	gui2::widget& w = luaW_checkwidget(L, 1);
	std::string_view str = lua_check<std::string_view>(L, 2);


	tsetters::iterator it = setters.find(std::string(str));
	if(it != setters.end()) {
		for(const auto& func : it->second) {
			if(func(L, 3, w, false)) {
				return 0;
			}
		}
		ERR_LUA << "none of "<< it->second.size() << " setters matched";
	}
	else {
		ERR_LUA << "unknown property id : " << str << " #known properties="  << setters.size();

	}
	ERR_LUA << "invalid modifiable property of '" <<  typeid(w).name()<< "' widget:" << str;
	return luaL_argerror(L, 2, "invalid modifiable property of widget");
}

int impl_widget_dir(lua_State* L)
{
	gui2::widget& w = luaW_checkwidget(L, 1);
	std::vector<std::string> keys;
	// Add any readable keys
	for(const auto& [key, funcs] : getters) {
		if(key == "value_compat") continue;
		for(const auto& func : funcs) {
			if(func(L, w, true)){
				keys.push_back(key);
				break;
			}
		}
	}
	// Add any writable keys
	for(const auto& [key, funcs] : setters) {
		if(key == "value_compat") continue;
		if(key == "callback") continue;
		for(const auto& func : funcs) {
			if(func(L, 0, w, true)){
				keys.push_back(key);
				break;
			}
		}
	}
	// Add any nested widget IDs
	using iter_t = gui2::iteration::top_down_iterator<true, true, true>;
	for(auto child = iter_t(w); !child.at_end(); child.next()) {
		const auto& key = child->id();
		if(!key.empty() && key != w.id()) {
			keys.push_back(key);
		}
	}
	// Add the gui.widget methods
	luaW_getglobal(L, "gui", "widget");
	auto methods = luaW_get_attributes(L, -1);
	keys.insert(keys.end(), methods.begin(), methods.end());
	lua_push(L, keys);
	return 1;
}
}
