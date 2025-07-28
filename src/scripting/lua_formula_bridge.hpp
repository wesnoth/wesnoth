/*
	Copyright (C) 2017 - 2025
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

#include <functional>
#include <memory>
#include <string>

struct lua_State;

namespace wfl {
	class formula;
	class function_symbol_table;
	class formula_debugger;
	class formula_callable;
	class variant;
}

namespace lua_formula_bridge {

	int intf_eval_formula(lua_State*);
	int intf_compile_formula(lua_State*);
	std::string register_metatables(lua_State*);

	class fwrapper {
		std::shared_ptr<wfl::formula> formula_ptr;
	public:
		fwrapper(const std::string& code);
		std::string str() const;
		wfl::variant evaluate(const wfl::formula_callable& variables, wfl::formula_debugger* fdb = nullptr) const;
	};

	using fpointer = std::unique_ptr<fwrapper, std::function<void(fwrapper*)>>;
} // end namespace lua_formula_bridge
lua_formula_bridge::fpointer luaW_check_formula(lua_State* L, int idx, bool allow_str);
