/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "scripting/lua_formula_bridge.hpp"

#include "boost/variant/static_visitor.hpp"

#include "game_board.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "scripting/lua_unit.hpp"
#include "scripting/lua_common.hpp"
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "formula/callable_objects.hpp"
#include "formula/formula.hpp"
#include "variable.hpp"

#include "resources.hpp"
#include "units/map.hpp"

static const char formulaKey[] = "formula";

using namespace wfl;

void luaW_pushfaivariant(lua_State* L, variant val);
variant luaW_tofaivariant(lua_State* L, int i);

class lua_callable : public formula_callable {
	lua_State* mState;
	int table_i;
public:
	lua_callable(lua_State* L, int i) : mState(L), table_i(lua_absindex(L,i)) {}
	variant get_value(const std::string& key) const {
		if(key == "__list") {
			std::vector<variant> values;
			size_t n = lua_rawlen(mState, table_i);
			if(n == 0) {
				return variant();
			}
			for(size_t i = 1; i <= n; i++) {
				lua_pushinteger(mState, i);
				lua_gettable(mState, table_i);
				values.push_back(luaW_tofaivariant(mState, -1));
			}
			return variant(values);
		} else if(key == "__map") {
			std::map<variant,variant> values;
			for(lua_pushnil(mState); lua_next(mState, table_i); lua_pop(mState, 1)) {
				values[luaW_tofaivariant(mState, -2)] = luaW_tofaivariant(mState, -1);
			}
			return variant(values);
		}
		lua_pushlstring(mState, key.c_str(), key.size());
		lua_gettable(mState, table_i);
		variant result = luaW_tofaivariant(mState, -1);
		lua_pop(mState, 1);
		return result;
	}
	void get_inputs(formula_input_vector* inputs) const {
		add_input(inputs, "__list");
		add_input(inputs, "__map");
		for(lua_pushnil(mState); lua_next(mState, table_i); lua_pop(mState,1)) {
			if(lua_isstring(mState, -2) && !lua_isnumber(mState, -2)) {
				std::string key = lua_tostring(mState, -2);
				if(key.find_first_not_of(formula::id_chars) != std::string::npos) {
					add_input(inputs, key);
				}
			}
		}
	}
	int do_compare(const formula_callable* other) const {
		const lua_callable* lua = dynamic_cast<const lua_callable*>(other);
		if(lua == nullptr) {
			return formula_callable::do_compare(other);
		}
		if(mState == lua->mState) { // Which should always be the case, but let's be safe here
			if(lua_compare(mState, table_i, lua->table_i, LUA_OPEQ)) {
				return 0;
			}
			int top = lua_gettop(mState);
			if(lua_getmetatable(mState, table_i)) {
				lua_getfield(mState, -1, "__lt");
				if(!lua_isnoneornil(mState, -1)) {
					if(lua_getmetatable(mState, lua->table_i)) {
						lua_getfield(mState, -1, "__lt");
						if(!lua_isnoneornil(mState, -1)) {
							lua_settop(mState, top);
							return lua_compare(mState, table_i, lua->table_i, LUA_OPLT) ? -1 : 1;
						}
						if(lua_compare(mState, -4, -2, LUA_OPEQ)) {
							lua_settop(mState, top);
							return 0;
						}
						const void* lhs = lua_topointer(mState, -4);
						const void* rhs = lua_topointer(mState, -2);
						lua_settop(mState, top);
						return lhs < rhs ? -1 : (lhs > rhs ? 1 : 0);
					}
				}
			}
			lua_settop(mState, top);
			return lua_topointer(mState, -2) < lua_topointer(mState, -1) ? -1 : 1;
		}
		return mState < lua->mState ? -1 : 1;
	}
};

void luaW_pushfaivariant(lua_State* L, variant val) {
	if(val.is_int()) {
		lua_pushinteger(L, val.as_int());
	} else if(val.is_decimal()) {
		lua_pushnumber(L, val.as_decimal() / 1000.0);
	} else if(val.is_string()) {
		const std::string result_string = val.as_string();
		lua_pushlstring(L, result_string.c_str(), result_string.size());
	} else if(val.is_list()) {
		lua_newtable(L);
		for(const variant& v : val.as_list()) {
			lua_pushinteger(L, lua_rawlen(L, -1) + 1);
			luaW_pushfaivariant(L, v);
			lua_settable(L, -3);
		}
	} else if(val.is_map()) {
		typedef std::map<variant,variant>::value_type kv_type;
		lua_newtable(L);
		for(const kv_type& v : val.as_map()) {
			luaW_pushfaivariant(L, v.first);
			luaW_pushfaivariant(L, v.second);
			lua_settable(L, -3);
		}
	} else if(val.is_callable()) {
		// First try a few special cases
		if(unit_callable* u_ref = val.try_convert<unit_callable>()) {
			const unit& u = u_ref->get_unit();
			unit_map::iterator un_it = resources::gameboard->units().find(u.get_location());
			if(&*un_it == &u) {
				luaW_pushunit(L, u.underlying_id());
			} else {
				luaW_pushunit(L, u.side(), u.underlying_id());
			}
		} else if(location_callable* loc_ref = val.try_convert<location_callable>()) {
			luaW_pushlocation(L, loc_ref->loc());
		} else {
			// If those fail, convert generically to a map
			const formula_callable* obj = val.as_callable();
			formula_input_vector inputs;
			obj->get_inputs(&inputs);
			lua_newtable(L);
			for(const formula_input& attr : inputs) {
				if(attr.access == FORMULA_WRITE_ONLY) {
					continue;
				}
				lua_pushstring(L, attr.name.c_str());
				luaW_pushfaivariant(L, obj->query_value(attr.name));
				lua_settable(L, -3);
			}
		}
	} else if(val.is_null()) {
		lua_pushnil(L);
	}
}

variant luaW_tofaivariant(lua_State* L, int i) {
	switch(lua_type(L, i)) {
		case LUA_TBOOLEAN:
			return variant(lua_tointeger(L, i));
		case LUA_TNUMBER:
			return variant(lua_tonumber(L, i), variant::DECIMAL_VARIANT);
		case LUA_TSTRING:
			return variant(lua_tostring(L, i));
		case LUA_TTABLE:
			return variant(new lua_callable(L, i));
		case LUA_TUSERDATA:
			static t_string tstr;
			static vconfig vcfg = vconfig::unconstructed_vconfig();
			static map_location loc;
			if(luaW_totstring(L, i, tstr)) {
				return variant(tstr.str());
			} else if(luaW_tovconfig(L, i, vcfg)) {
				return variant(new config_callable(vcfg.get_parsed_config()));
			} else if(unit* u = luaW_tounit(L, i)) {
				return variant(new unit_callable(*u));
			} else if(luaW_tolocation(L, i, loc)) {
				return variant(new location_callable(loc));
			}
			break;
	}
	return variant();
}

/**
 * Evaluates a formula in the formula engine.
 * - Arg 1: Formula string.
 * - Arg 2: optional context; can be a unit or a Lua table.
 * - Ret 1: Result of the formula.
 */
int lua_formula_bridge::intf_eval_formula(lua_State *L)
{
	bool need_delete = false;
	fwrapper* form;
	if(void* ud = luaL_testudata(L, 1, formulaKey)) {
		form = static_cast<fwrapper*>(ud);
	} else {
		need_delete = true;
		form = new fwrapper(luaL_checkstring(L, 1));
	}
	std::shared_ptr<formula_callable> context, fallback;
	if(unit* u = luaW_tounit(L, 2)) {
		context.reset(new unit_callable(*u));
	} else if(lua_istable(L, 2)) {
		context.reset(new lua_callable(L, 2));
	} else {
		context.reset(new map_formula_callable);
	}
	variant result = form->evaluate(*context);
	luaW_pushfaivariant(L, result);
	if(need_delete) {
		delete form;
	}
	return 1;
}

int lua_formula_bridge::intf_compile_formula(lua_State* L)
{
	if(!lua_isstring(L, 1)) {
		luaW_type_error(L, 1, "string");
	}
	new(L) fwrapper(lua_tostring(L, 1));
	luaL_setmetatable(L, formulaKey);
	return 1;
}

lua_formula_bridge::fwrapper::fwrapper(const std::string& code, function_symbol_table* functions)
	: formula_ptr(new formula(code, functions))
{
}

std::string lua_formula_bridge::fwrapper::str() const
{
	if(formula_ptr) {
		return formula_ptr->str();
	}
	return "";
}

variant lua_formula_bridge::fwrapper::evaluate(const formula_callable& variables, formula_debugger* fdb) const
{
	if(formula_ptr) {
		return formula_ptr->evaluate(variables, fdb);
	}
	return variant();
}

static int impl_formula_collect(lua_State* L)
{
	lua_formula_bridge::fwrapper* form = static_cast<lua_formula_bridge::fwrapper*>(lua_touserdata(L, 1));
	form->~fwrapper();
	return 0;
}

static int impl_formula_tostring(lua_State* L)
{
	lua_formula_bridge::fwrapper* form = static_cast<lua_formula_bridge::fwrapper*>(lua_touserdata(L, 1));
	const std::string str = form->str();
	lua_pushlstring(L, str.c_str(), str.size());
	return 1;
}

std::string lua_formula_bridge::register_metatables(lua_State* L)
{
	luaL_newmetatable(L, formulaKey);
	lua_pushcfunction(L, impl_formula_collect);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, impl_formula_tostring);
	lua_setfield(L, -2, "__tostring");
	lua_pushcfunction(L, intf_eval_formula);
	lua_setfield(L, -2, "__call");
	lua_pushstring(L, "formula");
	lua_setfield(L, -2, "__metatable");

	return "Adding formula metatable...\n";
}
