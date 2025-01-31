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

#include "scripting/lua_wml.hpp"
#include "scripting/lua_kernel_base.hpp"
#include "scripting/lua_common.hpp"

#include "serialization/schema_validator.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "variable.hpp" // for config_variable_set

#include <fstream>

#include "lua/wrapper_lauxlib.h"

namespace lua_wml {

/**
* Dumps a wml table or userdata wml object into a pretty string.
* - Arg 1: wml table or vconfig userdata
* - Ret 1: string
*/
static int intf_wml_tostring(lua_State* L) {
	const config& arg = luaW_checkconfig(L, 1);
	std::ostringstream stream;
	write(stream, arg);
	lua_pushstring(L, stream.str().c_str());
	return 1;
}

/**
 * Loads a WML file into a config
 * - Arg 1: WML file path
 * - Arg 2: (optional) Array of preprocessor defines, or false to skip preprocessing (true is also valid)
 * - Arg 3: (optional) Path to a schema file for validation (omit for no validation)
 * - Ret: config
 */
static int intf_load_wml(lua_State* L)
{
	std::string file = luaL_checkstring(L, 1);
	bool preprocess = true;
	preproc_map defines_map;
	if(lua_type(L, 2) == LUA_TBOOLEAN) {
		preprocess = luaW_toboolean(L, 2);
	} else if(lua_type(L, 2) == LUA_TTABLE || lua_type(L, 2) == LUA_TUSERDATA) {
		lua_len(L, 2);
		int n = lua_tointeger(L, -1);
		lua_pop(L, 1);
		for(int i = 0; i < n; i++) {
			lua_geti(L, 2, i);
			if(!lua_isstring(L, -1)) {
				return luaL_argerror(L, 2, "expected bool or array of strings");
			}
			std::string define = lua_tostring(L, -1);
			lua_pop(L, 1);
			if(!define.empty()) {
				defines_map.emplace(define, preproc_define(define));
			}
		}
	} else if(!lua_isnoneornil(L, 2)) {
		return luaL_argerror(L, 2, "expected bool or array of strings");
	}
	std::string schema_path = luaL_optstring(L, 3, "");
	std::shared_ptr<schema_validation::schema_validator> validator;
	if(!schema_path.empty()) {
		validator.reset(new schema_validation::schema_validator(filesystem::get_wml_location(schema_path).value()));
		validator->set_create_exceptions(false); // Don't crash if there's an error, just go ahead anyway
	}
	std::string wml_file = filesystem::get_wml_location(file).value();
	filesystem::scoped_istream stream;
	config result;
	if(preprocess) {
		stream = preprocess_file(wml_file, &defines_map);
	} else {
		stream.reset(new std::ifstream(wml_file));
	}
	read(result, *stream, validator.get());
	luaW_pushconfig(L, result);
	return 1;
}

/**
 * Parses a WML string into a config; does not preprocess or validate
 * - Arg 1: WML string
 * - Ret: config
 */
static int intf_parse_wml(lua_State* L)
{
	std::string wml = luaL_checkstring(L, 1);
	std::string schema_path = luaL_optstring(L, 2, "");
	std::shared_ptr<schema_validation::schema_validator> validator;
	if(!schema_path.empty()) {
		validator.reset(new schema_validation::schema_validator(filesystem::get_wml_location(schema_path).value()));
		validator->set_create_exceptions(false); // Don't crash if there's an error, just go ahead anyway
	}
	config result;
	read(result, wml, validator.get());
	luaW_pushconfig(L, result);
	return 1;
}

/**
 * Returns a clone (deep copy) of the passed config, which can be either a normal config or a vconfig
 * If it is a vconfig, the underlying config is also cloned.
 * - Arg 1: a config
 * - Ret: the cloned config
 */
static int intf_clone_wml(lua_State* L)
{
	const vconfig* vcfg = nullptr;
	const config& cfg = luaW_checkconfig(L, 1, vcfg);
	if(vcfg) {
		config clone_underlying = vcfg->get_config();
		vconfig clone(clone_underlying);
		luaW_pushvconfig(L, clone);
	} else {
		luaW_pushconfig(L, cfg);
	}
	return 1;
}

/**
* Interpolates variables into a WML table, including [insert_tag]
* Arg 1: WML table to interpolate into
* Arg 2: WML table of variables
*/
static int intf_wml_interpolate(lua_State* L)
{
	config cfg = luaW_checkconfig(L, 1), vars_cfg = luaW_checkconfig(L, 2);
	config_variable_set vars(vars_cfg);
	vconfig vcfg(cfg, vars);
	luaW_pushconfig(L, vcfg.get_parsed_config());
	return 1;
}

/**
* Tests if a WML table matches a filter
* Arg 1: table to test
* Arg 2: filter
*/
static int intf_wml_matches_filter(lua_State* L)
{
	config cfg = luaW_checkconfig(L, 1);
	config filter = luaW_checkconfig(L, 2);
	lua_pushboolean(L, cfg.matches(filter));
	return 1;
}

/**
* Merges two WML tables
* Arg 1: base table
* Arg 2: table to merge in
*/
static int intf_wml_merge(lua_State* L)
{
	config base = luaW_checkconfig(L, 1);
	config merge = luaW_checkconfig(L, 2);
	const std::string mode = lua_isstring(L, 3) ? luaL_checkstring(L, 3) : "merge";
	if(mode == "append") {
		base.merge_attributes(merge);
		base.append_children(merge);
	} else {
		if(mode == "replace") {
			for(const auto [key, _] : merge.all_children_view()) {
				base.clear_children(key);
			}
		} else if(mode != "merge") {
			return luaL_argerror(L, 3, "invalid merge mode - must be merge, append, or replace");
		}
		base.merge_with(merge);
	}
	luaW_pushconfig(L, base);
	return 1;
}

/**
* Computes a diff of two WML tables
* Arg 1: left table
* Arg 2: right table
*/
static int intf_wml_diff(lua_State* L)
{
	config lhs = luaW_checkconfig(L, 1);
	config rhs = luaW_checkconfig(L, 2);
	luaW_pushconfig(L, lhs.get_diff(rhs));
	return 1;
}

/**
* Applies a diff to a WML table
* Arg 1: base table
* Arg 2: WML diff
*/
static int intf_wml_patch(lua_State* L)
{
	config base = luaW_checkconfig(L, 1);
	config patch = luaW_checkconfig(L, 2);
	base.apply_diff(patch);
	luaW_pushconfig(L, base);
	return 1;
}

/**
* Tests if two WML tables are equal (have the same keys and values, same tags, recursively)
* Arg 1: left table
* Arg 2: right table
*/
static int intf_wml_equal(lua_State* L)
{
	config left = luaW_checkconfig(L, 1);
	config right = luaW_checkconfig(L, 2);
	lua_pushboolean(L, left == right);
	return 1;
}

/**
* Tests if a table represents a valid WML table
* Arg 1: table
*/
static int intf_wml_valid(lua_State* L)
{
	config test;
	if(luaW_toconfig(L, 1, test)) {
		// The validate_wml call is PROBABLY redundant, but included just in case validation changes and toconfig isn't updated to match
		lua_pushboolean(L, test.validate_wml());
	} else lua_pushboolean(L, false);
	return 1;
}

int luaW_open(lua_State* L) {
	auto& lk = lua_kernel_base::get_lua_kernel<lua_kernel_base>(L);
	lk.add_log("Adding wml module...\n");
	static luaL_Reg const wml_callbacks[]= {
		{ "load",      &intf_load_wml},
		{ "parse",     &intf_parse_wml},
		{ "clone",     &intf_clone_wml},
		{ "merge",     &intf_wml_merge},
		{ "diff",     &intf_wml_diff},
		{ "patch",     &intf_wml_patch},
		{ "equal",     &intf_wml_equal},
		{ "valid",     &intf_wml_valid},
		{ "matches_filter", &intf_wml_matches_filter},
		{ "tostring",       &intf_wml_tostring},
		{ "interpolate",    &intf_wml_interpolate},
		{ nullptr, nullptr },
	};
	lua_newtable(L);
	luaL_setfuncs(L, wml_callbacks, 0);
	return 1;
}

}
