/*
	Copyright (C) 2017 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "scripting/lua_formula_bridge.hpp"

#include <set>

#include "game_board.hpp"
#include "scripting/lua_unit.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_kernel_base.hpp"
#include "scripting/lua_team.hpp"
#include "scripting/lua_unit_attacks.hpp"
#include "scripting/lua_unit_type.hpp"
#include "lua/wrapper_lauxlib.h"
#include "formula/callable_objects.hpp"
#include "formula/formula.hpp"
#include "formula/function.hpp"
#include "variable.hpp"

#include "resources.hpp"
#include "units/map.hpp"
#include "units/unit.hpp"

static const char formulaKey[] = "formula";
static const char formfcntbKey[] = "formula function table";

using namespace wfl;

void luaW_pushfaivariant(lua_State* L, const variant& val);
variant luaW_tofaivariant(lua_State* L, int i);

typedef function_symbol_table* functionstb_ptr;

std::set<functionstb_ptr> lua_owned_fcntb;

class lua_callable : public formula_callable {
	lua_State* mState;
	int table_i;
public:
	bool is_list() const {
		for(lua_pushnil(mState); lua_next(mState, table_i); lua_pop(mState, 1)) {
			if(!lua_isnumber(mState, -1)) {
				lua_pop(mState, 2);
				return false;
			}
		}
		return true;
	}
	lua_callable(lua_State* L, int i) : mState(L), table_i(lua_absindex(L,i)) {
		luaL_checktype(L, i, LUA_TTABLE);
	}
	variant get_value(const std::string& key) const {
		if(key == "__list") {
			std::vector<variant> values;
			std::size_t n = lua_rawlen(mState, table_i);
			if(n == 0) {
				return variant();
			}
			for(std::size_t i = 1; i <= n; i++) {
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
	void get_inputs(formula_input_vector& inputs) const {
		add_input(inputs, "__list");
		add_input(inputs, "__map");
		for(lua_pushnil(mState); lua_next(mState, table_i); lua_pop(mState,1)) {
			lua_pushvalue(mState, -2);
			bool is_valid_key = (lua_type(mState, -1) == LUA_TSTRING) && !lua_isnumber(mState, -1);
			lua_pop(mState, 1);
			if(is_valid_key) {
				std::string key = lua_tostring(mState, -2);
				if(key.find_first_not_of(formula::id_chars) == std::string::npos) {
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

class lua_formula_function : public function_expression {
	lua_kernel_base& kernel;
	const void* registry_index;
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		lua_State* L = kernel.get_state();
		lua_rawgetp(L, LUA_REGISTRYINDEX, registry_index);
		luaW_pushfaivariant(L, variant(variables.fake_ptr()));
		for(auto arg : args()) {
			if(fdb) {
				arg.reset(new wrapper_formula(arg));
			}
			new(L) lua_formula_bridge::fwrapper(arg);
			luaL_setmetatable(L, formulaKey);
		}
		bool success = luaW_pcall(L, args().size() + 1, 2);
		variant result;
		if(!success || lua_isnoneornil(L, -2)) {
			return result;
		} else if(lua_isnoneornil(L, -1)) {
			// Use normal guessing to determine the type
			result = luaW_tofaivariant(L, -2);
		} else {
			std::string type = lua_tostring(L, -1);
			if(type == "string") {
				result = variant(luaL_checkstring(L, -2));
			} else if(type == "integer") {
				result = variant(luaL_checkinteger(L, -2));
			} else if(type == "decimal") {
				result = variant(luaL_checknumber(L, -2), variant::DECIMAL_VARIANT);
			} else if(type == "list") {
				result = lua_callable(L, -2).get_value("__list");
			} else if(type == "map") {
				result = lua_callable(L, -2).get_value("__map");
			} else if(type == "unit") {
				unit& u = luaW_checkunit(L, -2);
				result = variant(std::make_shared<unit_callable>(u));
			} else if(type == "loc") {
				map_location loc = luaW_checklocation(L, -2);
				result = variant(std::make_shared<location_callable>(loc));
			}
		}
		return result;
	}
public:
	lua_formula_function(const std::string name, const args_list& args, int min_args, int max_args, const void* ridx, lua_kernel_base& k)
		: function_expression(name, args, min_args, max_args)
		, kernel(k)
		, registry_index(ridx)
	{
	}
};

class lua_formula_function_defn : public formula_function {
	int min_args, max_args;
	lua_kernel_base& kernel;
public:
	lua_formula_function_defn(const std::string name, int min, int max, lua_kernel_base& k)
		: formula_function(name)
		, min_args(min)
		, max_args(max)
		, kernel(k)
	{
	}
	virtual function_expression_ptr generate_function_expression(const std::vector<expression_ptr>& args) const {
		return function_expression_ptr(new lua_formula_function(name_, args, min_args, max_args, this, kernel));
	}
	virtual ~lua_formula_function_defn() {}
};

void luaW_pushfaivariant(lua_State* L, const variant& val) {
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
		if(auto u_ref = val.try_convert<unit_callable>()) {
			const unit& u = u_ref->get_unit();
			unit_map::iterator un_it = resources::gameboard->units().find(u.get_location());
			if(&*un_it == &u) {
				luaW_pushunit(L, u.underlying_id());
			} else {
				luaW_pushunit(L, u.side(), u.underlying_id());
			}
		} else if(auto ut_ref = val.try_convert<unit_type_callable>()) {
			const unit_type& ut = ut_ref->get_unit_type();
			luaW_pushunittype(L, ut);
		} else if(auto atk_ref = val.try_convert<attack_type_callable>()) {
			const auto& atk = atk_ref->get_attack_type();
			luaW_pushweapon(L, atk.shared_from_this());
		} else if(auto team_ref = val.try_convert<team_callable>()) {
			auto t = team_ref->get_team();
			luaW_pushteam(L, t);
		} else if(auto loc_ref = val.try_convert<location_callable>()) {
			luaW_pushlocation(L, loc_ref->loc());
		} else {
			// If those fail, convert generically to a map
			auto obj = val.as_callable();
			formula_input_vector inputs;
			obj->get_inputs(inputs);
			lua_newtable(L);
			for(const formula_input& attr : inputs) {
				if(attr.access == formula_access::write_only) {
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
			if(lua_isinteger(L, i)) return variant(lua_tointeger(L, i));
			return variant(lua_tonumber(L, i), variant::DECIMAL_VARIANT);
		case LUA_TSTRING:
			return variant(lua_tostring(L, i));
		case LUA_TTABLE:
			return variant(std::make_shared<lua_callable>(L, i));
		case LUA_TUSERDATA:
			static t_string tstr;
			static vconfig vcfg = vconfig::unconstructed_vconfig();
			static map_location loc;
			if(luaW_totstring(L, i, tstr)) {
				return variant(tstr.str());
			} else if(luaW_tovconfig(L, i, vcfg)) {
				return variant(std::make_shared<config_callable>(vcfg.get_parsed_config()));
			} else if(unit* u = luaW_tounit(L, i)) {
				return variant(std::make_shared<unit_callable>(*u));
			} else if(const unit_type* ut = luaW_tounittype(L, i)) {
				return variant(std::make_shared<unit_type_callable>(*ut));
			} else if(const_attack_ptr atk = luaW_toweapon(L, i)) {
				return variant(std::make_shared<attack_type_callable>(*atk));
			} else if(team* t = luaW_toteam(L, i)) {
				return variant(std::make_shared<team_callable>(*t));
			} else if(luaW_tolocation(L, i, loc)) {
				return variant(std::make_shared<location_callable>(loc));
			}
			break;
	}
	return variant();
}

/**
 * Get a formula from the stack. If @a allow_str is true, it compiles a formula string if found.
 * Otherwise, it must be an already-compiled formula.
 * Raises an error if a formula is not found, or if there's an error in compilation.
 * Thus, it never returns a null pointer.
 */
lua_formula_bridge::fpointer luaW_check_formula(lua_State* L, int idx, bool allow_str) {
	using namespace lua_formula_bridge;
	fpointer form;
	if(void* ud = luaL_testudata(L, idx, formulaKey)) {
		form.get_deleter() = [](fwrapper*) {};
		form.reset(static_cast<fwrapper*>(ud));
		// Setting a no-op deleter guarantees the Lua-held object is not deleted
	} else if(allow_str) {
		form.get_deleter() = std::default_delete<fwrapper>();
		form.reset(new fwrapper(luaL_checkstring(L, idx)));
		// Set deleter to default so it's deleted properly later
	} else {
		luaW_type_error(L, idx, "formula");
	}
	return form;
}

/**
 * Evaluates a formula in the formula engine.
 * - Arg 1: Formula string or object.
 * - Arg 2: optional context; can be a unit or a Lua table.
 * - Ret 1: Result of the formula.
 */
int lua_formula_bridge::intf_eval_formula(lua_State *L)
{
	fpointer form = luaW_check_formula(L, 1, true);
	std::shared_ptr<formula_callable> context, fallback;
	if(unit* u = luaW_tounit(L, 2)) {
		context.reset(new unit_callable(*u));
	} else if(const unit_type* ut = luaW_tounittype(L, 2)) {
		context.reset(new unit_type_callable(*ut));
	} else if(const_attack_ptr atk = luaW_toweapon(L, 2)) {
		context.reset(new attack_type_callable(*atk));
	} else if(team* t = luaW_toteam(L, 2)) {
		context.reset(new team_callable(*t));
	} else if(lua_istable(L, 2)) {
		context.reset(new lua_callable(L, 2));
	} else {
		context.reset(new map_formula_callable);
	}
	variant result = form->evaluate(*context);
	luaW_pushfaivariant(L, result);
	return 1;
}

int lua_formula_bridge::intf_compile_formula(lua_State* L)
{
	if(!lua_isstring(L, 1)) {
		luaW_type_error(L, 1, "string");
	}
	functionstb_ptr fcns = nullptr;
	if(luaL_testudata(L, 2, formfcntbKey)) {
		fcns = *static_cast<functionstb_ptr*>(lua_touserdata(L, 2));
	} else if(lua_istable(L, 2)) {
		// Create a functions table on the Lua stack
		fcns = *new(L) functionstb_ptr(new function_symbol_table);
		luaL_setmetatable(L, formfcntbKey);
		// Set it to be collected
		lua_owned_fcntb.insert(fcns);
		// Iterate through the Lua table and register each entry as a function
		int i_fcntb = lua_absindex(L, -1);
		for(lua_pushnil(L); lua_next(L, i_fcntb - 1); lua_pop(L, 1)) {
			// Just forward to the metafunction, which handles any error checking
			lua_pushvalue(L, i_fcntb + 1); // The key (hopefully a valid string)
			lua_pushvalue(L, i_fcntb + 2); // The value (hopefully a valid function)
			lua_settable(L, i_fcntb); // Invokes the __newindex metafunction
		}
	}
	new(L) fwrapper(lua_tostring(L, 1), fcns);
	luaL_setmetatable(L, formulaKey);
	return 1;
}

lua_formula_bridge::fwrapper::fwrapper(const std::string& code, function_symbol_table* functions)
	: formula_ptr(new formula(code, functions))
{
}

lua_formula_bridge::fwrapper::fwrapper(std::shared_ptr<formula_expression> expr)
	: expr_ptr(expr)
{
}

std::string lua_formula_bridge::fwrapper::str() const
{
	if(formula_ptr) {
		return formula_ptr->str();
	}
	return "";
}

std::shared_ptr<formula> lua_formula_bridge::fwrapper::to_formula() const
{
	if(formula_ptr) {
		return formula_ptr;
	}
	return nullptr;
}

std::shared_ptr<formula_expression> lua_formula_bridge::fwrapper::to_expr() const
{
	if(formula_ptr) {
		return formula_ptr->expr_;
	} else if(expr_ptr) {
		return expr_ptr;
	}
	return nullptr;
}

variant lua_formula_bridge::fwrapper::evaluate(const formula_callable& variables, formula_debugger* fdb) const
{
	if(formula_ptr) {
		return formula_ptr->evaluate(variables, fdb);
	} else if(expr_ptr) {
		return expr_ptr->evaluate(variables, fdb);
	}
	return variant();
}

static int impl_formula_collect(lua_State* L)
{
	lua_formula_bridge::fwrapper* form = static_cast<lua_formula_bridge::fwrapper*>(lua_touserdata(L, 1));
	form->~fwrapper();
	return 0;
}

static int impl_formula_get(lua_State* L)
{
	lua_formula_bridge::fwrapper* form = static_cast<lua_formula_bridge::fwrapper*>(lua_touserdata(L, 1));
	std::string key = luaL_checkstring(L, 2);
	if(key == "functions") {
		if(std::shared_ptr<formula> f = form->to_formula()) {
			// This should not happen, but just in case...
			if(f->get_functions() == nullptr) return 0;
			new(L) functionstb_ptr(f->get_functions());
			luaL_setmetatable(L, formfcntbKey);
			return 1;
		} else {
			// The fwrapper is either invalid or wraps just a formula_expression, not a full formula.
			// This form is only available from being passed to a function, I think.
			lua_pushstring(L, "can't get functions table from argument formula");
			return lua_error(L);
		}
	}
	return 0;
}

static int impl_formula_tostring(lua_State* L)
{
	lua_formula_bridge::fwrapper* form = static_cast<lua_formula_bridge::fwrapper*>(lua_touserdata(L, 1));
	const std::string str = form->str();
	lua_pushlstring(L, str.c_str(), str.size());
	return 1;
}

static int impl_fcntb_collect(lua_State* L)
{
	functionstb_ptr tb = *static_cast<functionstb_ptr*>(lua_touserdata(L, 1));
	if(lua_owned_fcntb.count(tb)) {
		lua_owned_fcntb.erase(tb);
		tb->~function_symbol_table();
	}
	return 0;
}

static int cfun_fcntb_call(lua_State* L)
{
	functionstb_ptr tb = *static_cast<functionstb_ptr*>(lua_touserdata(L, lua_upvalueindex(1)));
	std::string name = luaL_checkstring(L, lua_upvalueindex(2));

	std::vector<expression_ptr> args;
	for(int i = 1; i <= lua_gettop(L); i++) {
		if(luaL_testudata(L, i, formulaKey)) {
			lua_formula_bridge::fwrapper* form = static_cast<lua_formula_bridge::fwrapper*>(lua_touserdata(L, i));
			args.push_back(form->to_expr());
		} else {
			variant v = luaW_tofaivariant(L, i);
			std::string var;
			if(!v.is_callable()) {
				var = v.serialize_to_string();
			} else if(auto lc = v.try_convert<lua_callable>()) {
				std::vector<formula_input> inputs;
				lc->get_inputs(inputs);
				if(inputs.size() > 2) {
					// Treat it as a map since it has string keys
					variant vmap = lc->get_value("__map");
					var = vmap.serialize_to_string();
				} else {
					// It could be a list or a map; check all keys
					if(lc->is_list()) {
						variant vlist = lc->get_value("__list");
						var = vlist.serialize_to_string();
					} else {
						variant vmap = lc->get_value("__map");
						var = vmap.serialize_to_string();
					}
				}
			} else {
				// Just try to serialize it. It might actually work,
				// or it might throw an error.
				// (For example, location objects are serializable.)
				var = v.serialize_to_string();
			}
			if(!var.empty()) {
				// It's not an object, so push code for generating the value
				lua_formula_bridge::fwrapper form(var, tb);
				args.push_back(form.to_expr());
			}
		}
	}

	expression_ptr fcn = tb->create_function(name, args);
	new(L) lua_formula_bridge::fwrapper(fcn);
	luaL_setmetatable(L, formulaKey);
	return 1;
}

static int impl_fcntb_get(lua_State* L)
{
	if(!luaL_testudata(L, 1, formfcntbKey)) {
		return luaW_type_error(L, 2, "formula function table");
	}
	if(!lua_isstring(L, 2)) {
		return luaW_type_error(L, 2, lua_typename(L, lua_type(L, 2)));
	}
	// This closure simply consumes the two arguments to __index
	lua_pushcclosure(L, cfun_fcntb_call, 2);
	return 1;
}

static int impl_fcntb_set(lua_State* L)
{
	functionstb_ptr tb = *static_cast<functionstb_ptr*>(lua_touserdata(L, 1));
	std::string fcn_name = luaL_checkstring(L, 2);
	if(lua_iscfunction(L, 3)) {
		return luaL_argerror(L, 3, "expected Lua function, not C function");
	} else if(!lua_isfunction(L, 3)) {
		return luaW_type_error(L, 3, lua_typename(L, LUA_TFUNCTION));
	}
	if(formula::keywords.count(fcn_name)) {
		return luaL_argerror(L, 2, "can't create a function whose name is a formula keyword");
	}
	// Make sure the function is at the top
	// It probably is already, though
	lua_settop(L, 3);
	lua_pushvalue(L, -1);
	lua_Debug fcn_info;
	lua_getinfo(L, ">u", &fcn_info);
	if(fcn_info.nparams == 0) {
		return luaL_argerror(L, 3, "expected Lua function taking at least one named argument");
	}
	int min_args = 1, max_args = fcn_info.isvararg ? -1 : fcn_info.nparams;
	for(int i = 2; i <= fcn_info.nparams; i++) {
		std::string var_name = lua_getlocal(L, nullptr, i);
		if(var_name.find("_opt") != std::string::npos) {
			min_args--;
		}
	}
	lua_kernel_base& kernel = lua_kernel_base::get_lua_kernel<lua_kernel_base>(L);
	formula_function_ptr fcn(new lua_formula_function_defn(fcn_name, min_args - 1, max_args - 1, kernel));
	lua_pushvalue(L, 3);
	if(fcn_info.nups > 0) {
		// Replace _ENV with a minimal environment containing only pure functions
		static const char*const globals[] = {
			// Basic iteration functions
			"pairs", "ipairs", "next",
			// Basic information functions
			"type", "select", "rawget", "rawlen", "rawequal", "_VERSION",
			// Conversions
			"tostring", "tonumber",
			// Pure modules
			"math", "mathx", "string", "stringx", "utf8",
		};
		static const char*const wesnoth[] = {
			// Version info
			"current_version", "version",
			// Misc functions
			"ms_since_init", "named_tuple", "textdomain",
			// Global data
			"colors", "game_config", "get_language",
		};
		static const char*const wml[] = {
			"attribute_count", "child_array", "child_count", "child_range", "find_child",
			"get_child", "get_nth_child", "tag", "clone", "equal", "valid", "matches_filter",
			"parse", "merge", "diff", "patch", "interpolate", "tostring"
		};
		lua_newtable(L);
		for(auto glob : globals) {
			lua_getglobal(L, glob);
			lua_setfield(L, -2, glob);
		}
		lua_newtable(L);
		for(auto wes : wesnoth) {
			luaW_getglobal(L, "wesnoth", wes);
			lua_setfield(L, -2, wes);
		}
		lua_setfield(L, -2, "wesnoth");
		lua_newtable(L);
		for(auto w : wml) {
			luaW_getglobal(L, "wml", w);
			lua_setfield(L, -2, w);
		}
		lua_setfield(L, -2, "wml");
		// Also add the function table itself as a global
		lua_pushvalue(L, 1);
		lua_setfield(L, -2, "wfl_functions");
		lua_setupvalue(L, -2, 1);
	}
	lua_rawsetp(L, LUA_REGISTRYINDEX, fcn.get());
	tb->add_function(fcn_name, std::move(fcn));
	return 0;
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
	lua_pushcfunction(L, impl_formula_get);
	lua_setfield(L, -2, "__index");
	lua_pushstring(L, "formula");
	lua_setfield(L, -2, "__metatable");

	luaL_newmetatable(L, formfcntbKey);
	lua_pushcfunction(L, impl_fcntb_collect);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, impl_fcntb_get);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, impl_fcntb_set);
	lua_setfield(L, -2, "__newindex");
	lua_pushstring(L, "formula functions");
	lua_setfield(L, -2, "__metatable");

	return "Adding formula metatable...\nAdding formula function table metatable";
}
