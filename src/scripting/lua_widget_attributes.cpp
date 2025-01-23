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

#include "gui/auxiliary/iterator/iterator.hpp"
#include "gui/widgets/clickable_item.hpp"
#include "gui/widgets/helper.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/combobox.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/progress_bar.hpp"
#include "gui/widgets/rich_label.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/scroll_text.hpp"
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
#include "scripting/lua_kernel_base.hpp"
#include "scripting/lua_unit.hpp"
#include "scripting/lua_unit_type.hpp"
#include "scripting/push_check.hpp"
#include "scripting/lua_widget.hpp"
#include "scripting/lua_attributes.hpp"
#include "scripting/lua_widget_attributes.hpp"
#include "serialization/string_utils.hpp"

#include <functional>

#include <boost/preprocessor/cat.hpp>

#include <map>
#include <utility>
#include <vector>

static lg::log_domain log_scripting_lua("scripting/lua");
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

static void try_invalidate_layout(gui2::widget& w)
{
	if(auto window = w.get_window()) {
		window->invalidate_layout();
	}
}

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
struct BOOST_PP_CAT(getter_, id) : public lua_getter<widgt_type, value_type> { \
	value_type get(lua_State* L, const widgt_type& w) const override; \
}; \
struct BOOST_PP_CAT(getter_adder_, id) { \
	BOOST_PP_CAT(getter_adder_, id) () \
	{ \
		register_widget_attribute<widgt_type, value_type, BOOST_PP_CAT(getter_, id), false>(name); \
	} \
}; \
static BOOST_PP_CAT(getter_adder_, id) BOOST_PP_CAT(getter_adder_instance_, id) ; \
value_type BOOST_PP_CAT(getter_, id)::get([[maybe_unused]] lua_State* L, const widgt_type& w) const


#define WIDGET_SETTER4(name, value_type, widgt_type, id) \
struct BOOST_PP_CAT(setter_, id) : public lua_setter<widgt_type, value_type> { \
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

WIDGET_GETTER("best_slider_length", int, gui2::slider)
{
	return w.get_best_slider_length();
}

WIDGET_SETTER("best_slider_length", int, gui2::slider)
{
	if(value < 0) {
		throw std::invalid_argument("best_slider_length must be >= 0");
	}
	w.set_best_slider_length(value);
}

WIDGET_GETTER("max_value", int, gui2::slider)
{
	return w.get_maximum_value();
}

WIDGET_SETTER("max_value", int, gui2::slider)
{
	w.set_value_range(w.get_minimum_value(), value);
}

WIDGET_GETTER("maximum_value_label", t_string, gui2::slider)
{
	return w.get_maximum_value_label();
}

WIDGET_SETTER("maximum_value_label", t_string, gui2::slider)
{
	w.set_maximum_value_label(value);
	try_invalidate_layout(w);
}

WIDGET_GETTER("min_value", int, gui2::slider)
{
	return w.get_minimum_value();
}

WIDGET_SETTER("min_value", int, gui2::slider)
{
	w.set_value_range(value, w.get_maximum_value());
}

WIDGET_GETTER("minimum_value_label", t_string, gui2::slider)
{
	return w.get_minimum_value_label();
}

WIDGET_SETTER("minimum_value_label", t_string, gui2::slider)
{
	w.set_minimum_value_label(value);
	try_invalidate_layout(w);
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

WIDGET_GETTER("unfolded", bool, gui2::tree_view)
{
	return !w.get_root_node().is_folded();
}

WIDGET_SETTER("value_compat,unfolded", bool, gui2::tree_view)
{
	if(value) {
		w.get_root_node().unfold();
	} else {
		w.get_root_node().fold();
	}
}

WIDGET_GETTER("unfolded", bool, gui2::tree_view_node)
{
	return !w.is_folded();
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
		w.set_display_data(*ut);
	} else if(unit* u = luaW_tounit(L, value.index)) {
		w.set_display_data(*u);
	} else {
		luaW_type_error(L, value.index, "unit or unit type");
	}
}

WIDGET_GETTER("item_count", int, gui2::combobox)
{
	return w.get_item_count();
}

WIDGET_GETTER("item_count", int, gui2::listbox)
{
	return w.get_item_count();
}

WIDGET_GETTER("item_count", int, gui2::multi_page)
{
	return w.get_page_count();
}

WIDGET_GETTER("item_count", int, gui2::stacked_widget)
{
	return w.get_layer_count();
}

WIDGET_GETTER("item_count", int, gui2::tree_view)
{
	const gui2::tree_view_node& tvn = w.get_root_node();
	return tvn.count_children();
}

WIDGET_GETTER("item_count", int, gui2::tree_view_node)
{
	return w.count_children();
}

WIDGET_GETTER("use_markup", bool, gui2::styled_widget)
{
	return w.get_use_markup();
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

WIDGET_GETTER("characters_per_line", int, gui2::label)
{
	return w.get_characters_per_line();
}

WIDGET_SETTER("characters_per_line", int, gui2::label)
{
	if(value < 0) {
		throw std::invalid_argument("characters_per_line must be >= 0");
	}
	w.set_characters_per_line(value);
	try_invalidate_layout(w);
}

WIDGET_GETTER("editable", bool, gui2::text_box)
{
	return w.is_editable();
}

WIDGET_SETTER("editable", bool, gui2::text_box)
{
	w.set_editable(value);
}

WIDGET_GETTER("ellipsize_mode", std::string, gui2::styled_widget)
{
	return gui2::encode_ellipsize_mode(w.get_text_ellipse_mode());
}

WIDGET_SETTER("ellipsize_mode", std::string, gui2::styled_widget)
{
	w.set_text_ellipse_mode(gui2::decode_ellipsize_mode(value));
	try_invalidate_layout(w);
}

WIDGET_GETTER("enabled", bool, gui2::styled_widget)
{
	return w.get_active();
}

WIDGET_SETTER("enabled", bool, gui2::styled_widget)
{
	w.set_active(value);
}

WIDGET_GETTER("help", t_string, gui2::styled_widget)
{
	return w.help_message();
}

WIDGET_SETTER("help", t_string, gui2::styled_widget)
{
	w.set_help_message(value);
}

WIDGET_GETTER("hint_image", std::string, gui2::combobox)
{
	return w.get_hint_image();
}

WIDGET_SETTER("hint_image", std::string, gui2::combobox)
{
	w.set_hint_image(value);
	try_invalidate_layout(w);
}

WIDGET_GETTER("hint_text", t_string, gui2::combobox)
{
	return w.get_hint_text();
}

WIDGET_SETTER("hint_text", t_string, gui2::combobox)
{
	w.set_hint_text(value);
	try_invalidate_layout(w);
}

WIDGET_GETTER("hint_image", std::string, gui2::text_box)
{
	return w.get_hint_image();
}

WIDGET_SETTER("hint_image", std::string, gui2::text_box)
{
	w.set_hint_image(value);
	try_invalidate_layout(w);
}

WIDGET_GETTER("hint_text", t_string, gui2::text_box)
{
	return w.get_hint_text();
}

WIDGET_SETTER("hint_text", t_string, gui2::text_box)
{
	w.set_hint_text(value);
	try_invalidate_layout(w);
}

WIDGET_SETTER("history", std::string, gui2::text_box)
{
	w.set_history(value);
}

WIDGET_GETTER("indentation_step_size", int, gui2::tree_view)
{
	return w.get_indentation_step_size();
}

WIDGET_SETTER("indentation_step_size", int, gui2::tree_view)
{
	if(value < 0) {
		throw std::invalid_argument("indentation_step_size must be >= 0");
	}
	w.set_indentation_step_size(value);
	try_invalidate_layout(w);
}

WIDGET_GETTER("link_aware", bool, gui2::label)
{
	return w.get_link_aware();
}

WIDGET_SETTER("link_aware", bool, gui2::label)
{
	w.set_link_aware(value);
	try_invalidate_layout(w);
}

WIDGET_GETTER("link_aware", bool, gui2::rich_label)
{
	return w.get_link_aware();
}

WIDGET_SETTER("link_aware", bool, gui2::rich_label)
{
	w.set_link_aware(value);
	try_invalidate_layout(w);
}

WIDGET_GETTER("link_aware", bool, gui2::scroll_label)
{
	return w.get_link_aware();
}

WIDGET_SETTER("link_aware", bool, gui2::scroll_label)
{
	w.set_link_aware(value);
	try_invalidate_layout(w);
}

WIDGET_GETTER("link_aware", bool, gui2::scroll_text)
{
	return w.get_link_aware();
}

WIDGET_SETTER("link_aware", bool, gui2::scroll_text)
{
	w.set_link_aware(value);
	try_invalidate_layout(w);
}

WIDGET_GETTER("link_color", std::string, gui2::label)
{
	return w.get_link_color().to_hex_string();
}

WIDGET_GETTER("link_color", std::string, gui2::rich_label)
{
	return w.get_link_color().to_hex_string();
}

WIDGET_GETTER("max_input_length", int, gui2::combobox)
{
	return w.get_max_input_length();
}

WIDGET_SETTER("max_input_length", int, gui2::combobox)
{
	if(value < 0) {
		throw std::invalid_argument("max_input_length must be >= 0");
	}
	w.set_max_input_length(value);
	try_invalidate_layout(w);
}

WIDGET_GETTER("max_input_length", int, gui2::text_box)
{
	return w.get_max_input_length();
}

WIDGET_SETTER("max_input_length", int, gui2::text_box)
{
	if(value < 0) {
		throw std::invalid_argument("max_input_length must be >= 0");
	}
	w.set_max_input_length(value);
	try_invalidate_layout(w);
}

WIDGET_GETTER("step_size", int, gui2::slider)
{
	return w.get_step_size();
}

WIDGET_SETTER("step_size", int, gui2::slider)
{
	if(value < 0) {
		throw std::invalid_argument("step_size must be >= 0");
	}
	w.set_step_size(value);
}

WIDGET_GETTER("text_alignment", std::string, gui2::styled_widget)
{
	return gui2::encode_text_alignment(w.get_text_alignment());
}

WIDGET_SETTER("text_alignment", std::string, gui2::styled_widget)
{
	w.set_text_alignment(gui2::decode_text_alignment(value));
}

WIDGET_GETTER("tooltip", t_string, gui2::styled_widget)
{
	return w.tooltip();
}

WIDGET_SETTER("tooltip", t_string, gui2::styled_widget)
{
	w.set_tooltip(value);
}

WIDGET_GETTER("overflow_to_tooltip", bool, gui2::styled_widget)
{
	return w.get_use_tooltip_on_label_overflow();
}

WIDGET_SETTER("overflow_to_tooltip", bool, gui2::styled_widget)
{
	w.set_use_tooltip_on_label_overflow(value);
	try_invalidate_layout(w);
}

WIDGET_GETTER("wrap", bool, gui2::label)
{
	return w.can_wrap();
}

WIDGET_SETTER("wrap", bool, gui2::label)
{
	w.set_can_wrap(value);
}

WIDGET_GETTER("wrap", bool, gui2::rich_label)
{
	return w.can_wrap();
}

WIDGET_SETTER("wrap", bool, gui2::rich_label)
{
	w.set_can_wrap(value);
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

WIDGET_GETTER("visible", std::string, gui2::styled_widget)
{
	std::string s;
	switch(w.get_visible()) {
		case gui2::styled_widget::visibility::visible:
			s = "visible";
			break;
		case gui2::styled_widget::visibility::hidden:
			s = "hidden";
			break;
		case gui2::styled_widget::visibility::invisible:
			s = "invisible";
	}

	return s;
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

WIDGET_GETTER("value_compat,label", t_string, gui2::styled_widget)
{
	return w.get_label();
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
	if(const gui2::styled_widget* sw = dynamic_cast<const gui2::styled_widget*>(&w)) {
		return sw->get_control_type();
	}
	else if(dynamic_cast<const gui2::tree_view_node*>(&w)) {
		return "tree_view_node";
	}
	else if(dynamic_cast<const gui2::grid*>(&w)) {
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

void link_callback(lua_State* L, lua_ptr<gui2::widget>& wp, const std::string& id, const std::string& dest)
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
	luaW_getwidgetcallback(L, w, wd, id);
	assert(lua_isfunction(L, -1));
	lua_pushstring(L, dest.c_str());
	luaW_pushwidget(L, *w);
	lua_call(L, 2, 0);
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

WIDGET_SETTER("on_link_click", lua_index_raw, gui2::rich_label)
{
	gui2::window* wd = w.get_window();
	if(!wd) {
		throw std::invalid_argument("the widget has no window assigned");
	}

	lua_pushvalue(L, value.index);
	if (!luaW_setwidgetcallback(L, &w, wd, "on_link_click")) {
		w.register_link_callback(
			std::bind(&link_callback, L, lua_ptr<gui2::widget>(w), "on_link_click", std::placeholders::_1));
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
		connect_signal_mouse_left_click(w, std::bind(&dialog_callback, L, lua_ptr<gui2::widget>(w), "on_left_click"));
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
	std::string err = "invalid property of widget: ";
	err += str;
	return luaL_argerror(L, 2, err.c_str());
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
	std::string err = "invalid modifiable property of widget: ";
	err += str;
	return luaL_argerror(L, 2, err.c_str());
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
