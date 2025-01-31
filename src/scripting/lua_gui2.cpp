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

#include "scripting/lua_gui2.hpp"

#include "game_display.hpp"
#include "gui/gui.hpp"
#include "gui/core/gui_definition.hpp"
#include "gui/dialogs/drop_down_menu.hpp"
#include "gui/dialogs/gamestate_inspector.hpp"
#include "gui/dialogs/lua_interpreter.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/story_viewer.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/dialogs/units_dialog.hpp"
#include "gui/dialogs/wml_message.hpp"
#include "gui/widgets/retval.hpp"
#include "scripting/lua_unit.hpp"
#include "scripting/lua_unit_type.hpp"
#include "scripting/lua_widget_methods.hpp" //intf_show_dialog

#include "config.hpp"
#include "game_data.hpp"
#include "game_state.hpp"
#include "log.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_cpp_function.hpp"
#include "scripting/lua_kernel_base.hpp"
#include "scripting/push_check.hpp"
#include "help/help.hpp"
#include "tstring.hpp"
#include "sdl/input.hpp" // get_mouse_state
#include "units/ptr.hpp"
#include "units/unit.hpp"
#include "utils/optional_fwd.hpp"

#include <functional>
#include <vector>


static lg::log_domain log_scripting_lua("scripting/lua");
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

namespace lua_gui2 {


/**
 * Displays a message window
 * - Arg 1: Table describing the window
 * - Arg 2: List of options (nil or empty table - no options)
 * - Arg 3: Text input specifications (nil or empty table - no text input)
 * - Ret 1: option chosen (if no options: 0 if there's text input, -2 if escape pressed, else -1)
 * - Ret 2: string entered (empty if none, nil if no text input)
 */
int show_message_dialog(lua_State* L)
{
	config txt_cfg;
	const bool has_input = !lua_isnoneornil(L, 3) && luaW_toconfig(L, 3, txt_cfg) && !txt_cfg.empty();

	gui2::dialogs::wml_message_input input;
	input.caption = txt_cfg["label"].str();
	input.text = txt_cfg["text"].str();
	input.maximum_length = txt_cfg["max_length"].to_int(256);
	input.text_input_was_specified = has_input;

	gui2::dialogs::wml_message_options options{};
	if(!lua_isnoneornil(L, 2)) {
		luaL_checktype(L, 2, LUA_TTABLE);
		std::size_t n = lua_rawlen(L, 2);
		for(std::size_t i = 1; i <= n; i++) {
			lua_rawgeti(L, 2, i);
			t_string short_opt;
			config opt;
			if(luaW_totstring(L, -1, short_opt)) {
				opt["label"] = short_opt;
			} else if(!luaW_toconfig(L, -1, opt)) {
				std::ostringstream error;
				error << "expected array of config and/or translatable strings, but index ";
				error << i << " was a " << lua_typename(L, lua_type(L, -1));
				return luaL_argerror(L, 2, error.str().c_str());
			}
			gui2::dialogs::wml_message_option option(opt["label"], opt["description"], opt["image"]);
			if(opt["default"].to_bool(false)) {
				options.chosen_option = i - 1;
			}
			options.option_list.push_back(option);
			lua_pop(L, 1);
		}
		lua_getfield(L, 2, "default");
		if(lua_isnumber(L, -1)) {
			int i = lua_tointeger(L, -1);
			if(i < 1 || std::size_t(i) > n) {
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

	using portrait = gui2::dialogs::wml_message_portrait;
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

	int dlg_result = gui2::dialogs::show_wml_message(title, message, left.get(), right.get(), options, input);

	if(!has_input && options.option_list.empty()) {
		lua_pushinteger(L, dlg_result);
	} else {
		lua_pushinteger(L, options.chosen_option + 1);
	}

	if(has_input) {
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
int show_popup_dialog(lua_State *L) {
	t_string title = luaW_checktstring(L, 1);
	t_string msg = luaW_checktstring(L, 2);
	std::string image = lua_isnoneornil(L, 3) ? "" : luaL_checkstring(L, 3);

	gui2::show_transient_message(title, msg, image, true, true);
	return 0;
}

/**
 * Displays a story screen
 * - Arg 1: The story config
 * - Arg 2: The default title
 */
int show_story(lua_State* L) {
	config story = luaW_checkconfig(L, 1);
	t_string title = luaW_checktstring(L, 2);
	gui2::dialogs::story_viewer::display(title, story);
	return 0;
}

/**
 * Changes the current ui(gui2) theme
 * - Arg 1: The id of the theme to switch to
 */
int switch_theme(lua_State* L) {
	std::string theme_id = luaL_checkstring(L, 1);
	gui2::switch_theme(theme_id);
	return 0;
}

/**
 * Displays a popup menu at the current mouse position
 * Best used from a [set_menu_item], to show a submenu
 * - Arg 1: Configs defining each item, with keys icon, image/label, second_label, tooltip
 * - Args 2, 3: Initial selection (integer); whether to parse markup (boolean)
 */
int show_menu(lua_State* L) {
	std::vector<config> items = lua_check<std::vector<config>>(L, 1);
	rect pos{ sdl::get_mouse_location(), {1, 1} };

	int initial = -1;
	bool markup = false;
	if(lua_isnumber(L, 2)) {
		initial = lua_tointeger(L, 2) - 1;
		markup = luaW_toboolean(L, 3);
	} else if(lua_isnumber(L, 3)) {
		initial = lua_tointeger(L, 3) - 1;
		markup = luaW_toboolean(L, 2);
	} else if(lua_isboolean(L, 2)) {
		markup = luaW_toboolean(L, 2);
	}

	gui2::dialogs::drop_down_menu menu(pos, items, initial, markup, false);
	menu.show();
	lua_pushinteger(L, menu.selected_item() + 1);
	return 1;
}

/**
 * Displays a simple message box.
 */
int show_message_box(lua_State* L) {
	const t_string title = luaW_checktstring(L, 1), message = luaW_checktstring(L, 2);
	std::string button = luaL_optstring(L, 3, "ok"), btn_style;
	std::transform(button.begin(), button.end(), std::inserter(btn_style, btn_style.begin()), [](char c) { return std::tolower(c); });
	bool markup = lua_isnoneornil(L, 3) ? luaW_toboolean(L, 3) : luaW_toboolean(L, 4);
	using button_style = gui2::dialogs::message::button_style;
	utils::optional<button_style> style;
	if(btn_style.empty()) {
		style = button_style::auto_close;
	} else if(btn_style == "ok") {
		style = button_style::ok_button;
	} else if(btn_style == "close") {
		style = button_style::close_button;
	} else if(btn_style == "ok_cancel") {
		style = button_style::ok_cancel_buttons;
	} else if(btn_style == "cancel") {
		style = button_style::cancel_button;
	} else if(btn_style == "yes_no") {
		style = button_style::yes_no_buttons;
	}
	if(style) {
		int result = gui2::show_message(title, message, *style, markup, markup);
		if(style == button_style::ok_cancel_buttons || style == button_style::yes_no_buttons) {
			lua_pushboolean(L, result == gui2::retval::OK);
			return 1;
		}
	} else {
		gui2::show_message(title, message, button, false, markup, markup);
	}
	return 0;
}

int show_lua_console(lua_State* /*L*/, lua_kernel_base* lk)
{
	gui2::dialogs::lua_interpreter::display(lk);
	return 0;
}

int show_gamestate_inspector(const std::string& name, const game_data& data, const game_state& state)
{
	gui2::dialogs::gamestate_inspector::display(data.get_variables(), *state.events_manager_, state.board_, name);
	return 0;
}

int intf_show_recruit_dialog(lua_State* L)
{
	int idx = 1;
	const size_t len = lua_rawlen(L, idx);
	if (!lua_istable(L, idx)) {
		return luaL_error(L, "List of unit types not specified!");
	}

	std::vector<const unit_type*> types;
	types.reserve(len);
	for (size_t i = 1; i <= len; i++) {
		lua_rawgeti(L, idx, i);
		const unit_type* ut = luaW_tounittype(L, -1);
		if (ut) {
			types.push_back(ut);
		}
		lua_pop(L, idx);
	}

	const display* disp = display::get_singleton();
	if (!types.empty() && disp != nullptr) {
		auto dlg = gui2::dialogs::units_dialog::build_recruit_dialog(types, disp->playing_team());

		idx++;
		const config& cfg = luaW_checkconfig(L, idx);
		if (!cfg.empty()) {
			if (!cfg["title"].empty()) {
				dlg->set_title(cfg["title"]);
			}

			if (!cfg["ok_label"].empty()) {
				dlg->set_ok_label(cfg["ok_label"]);
			}

			if (!cfg["cancel_label"].empty()) {
				dlg->set_cancel_label(cfg["cancel_label"]);
			}

			if (!cfg["help_topic"].empty()) {
				dlg->set_help_topic(cfg["help_topic"]);
			}
		}

		if(dlg->show() && dlg->is_selected()) {
			luaW_pushunittype(L, *types[dlg->get_selected_index()]);
			return 1;
		}
	} else {
		ERR_LUA << "Unable to show recruit dialog";
	}

	return 0;
}


int intf_show_recall_dialog(lua_State* L)
{
	int idx = 1;
	const size_t len = lua_rawlen(L, idx);
	if (!lua_istable(L, idx)) {
		return luaL_error(L, "List of units not specified!");
	}

	std::vector<unit_const_ptr> units;
	units.reserve(len);
	for (size_t i = 1; i <= len; i++) {
		lua_rawgeti(L, idx, i);
		unit_const_ptr u(luaW_tounit_ptr(L, -1));
		if (u) {
			units.push_back(u);
		}
		lua_pop(L, idx);
	}

	const display* disp = display::get_singleton();
	if (!units.empty() && disp != nullptr) {
		auto dlg = gui2::dialogs::units_dialog::build_recall_dialog(units, disp->playing_team());

		idx++;
		const config& cfg = luaW_checkconfig(L, idx);
		if (!cfg.empty()) {
			if (!cfg["title"].empty()) {
				dlg->set_title(cfg["title"]);
			}

			if (!cfg["ok_label"].empty()) {
				dlg->set_ok_label(cfg["ok_label"]);
			}

			if (!cfg["cancel_label"].empty()) {
				dlg->set_cancel_label(cfg["cancel_label"]);
			}

			if (!cfg["help_topic"].empty()) {
				dlg->set_help_topic(cfg["help_topic"]);
			}
		}

		if(dlg->show() && dlg->is_selected()) {
			luaW_pushunit(L, units[dlg->get_selected_index()]->underlying_id());
			return 1;
		}
	} else {
		ERR_LUA << "Unable to show recall dialog";
	}

	return 0;
}

static int show_help(lua_State *L)
{
	help::show_help(luaL_checkstring(L, 1));
	return 0;
}

/**
 * - Arg 1: string, widget type
 * - Arg 3: string, id
 * - Arg 3: config,
 */

int intf_add_widget_definition(lua_State* L)
{
	std::string type = luaL_checkstring(L, 1);
	std::string id = luaL_checkstring(L, 2);
	try {
		if(gui2::add_single_widget_definition(type, id, luaW_checkconfig(L, 3))) {
			lua_kernel_base::get_lua_kernel<lua_kernel_base>(L).add_widget_definition(type, id);
		}
	} catch(const std::invalid_argument& e) {
		return luaL_argerror(L, 1, e.what());
	}
	return 0;
}

int luaW_open(lua_State* L)
{
	auto& lk = lua_kernel_base::get_lua_kernel<lua_kernel_base>(L);
	lk.add_log("Adding gui module...\n");
	static luaL_Reg const gui_callbacks[] = {
		{ "show_menu",              &show_menu },
		{ "show_narration",         &show_message_dialog },
		{ "show_popup",             &show_popup_dialog },
		{ "show_story",             &show_story },
		{ "show_prompt",            &show_message_box },
		{ "show_help",              &show_help   },
		{ "switch_theme",           &switch_theme },
		{ "add_widget_definition",  &intf_add_widget_definition },
		{ "show_dialog",            &intf_show_dialog },
		{ nullptr, nullptr },
	};
	std::vector<lua_cpp::Reg> const cpp_gui_callbacks {
		{"show_lua_console", std::bind(&lua_kernel_base::intf_show_lua_console, &lk, std::placeholders::_1)},
		{nullptr, nullptr}
	};
	lua_newtable(L);
	luaL_setfuncs(L, gui_callbacks, 0);
	lua_cpp::set_functions(L, cpp_gui_callbacks);

	lua_pushstring(L, "widget");
	lua_widget::luaW_open(L);
	lua_rawset(L, -3);

	return 1;
}

} // end namespace lua_gui2
