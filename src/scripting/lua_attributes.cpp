/*
	Copyright (C) 2024 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "scripting/lua_attributes.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_kernel_base.hpp" // for luaW_get_attributes
#include "scripting/push_check.hpp"
#include <sstream>

luaW_Registry::luaW_Registry(const std::initializer_list<std::string>& mt) : public_metatable(mt) {
	private_metatable = public_metatable.back();
	public_metatable.pop_back();
	lookup.emplace(private_metatable, std::ref(*this));
}

luaW_Registry::~luaW_Registry() {
	lookup.erase(private_metatable);
}

int luaW_Registry::get(lua_State* L) {
	std::string_view str = lua_check<std::string_view>(L, 2);

	auto it = getters.find(std::string(str));
	if(it != getters.end()) {
		if(it->second(L, false)) {
			return 1;
		}
	}
	if(!public_metatable.empty()) {
		auto method = public_metatable;
		method.push_back(std::string(str));
		if(luaW_getglobal(L, method)) {
			return 1;
		}
	}

	std::ostringstream err;
	err << "invalid property of " << private_metatable << ": " << str;
	return luaL_argerror(L, 2, err.str().c_str());
}

int luaW_Registry::set(lua_State* L) {
	std::string_view str = lua_check<std::string_view>(L, 2);

	auto it = setters.find(std::string(str));
	if(it != setters.end()) {
		if(it->second(L, 3, false)) {
			return 0;
		}
	}

	std::ostringstream err;
	err << "invalid modifiable property of " << private_metatable << ": " << str;
	return luaL_argerror(L, 2, err.str().c_str());
}

int luaW_Registry::dir(lua_State *L) {
	std::vector<std::string> keys;
	if(lua_istable(L, 2)) {
		keys = lua_check<std::vector<std::string>>(L, 2);
	}
	// Check for inactive keys
	std::set<std::string> inactive;
	for(const auto& [key, func] : validators) {
		if(!func(L)) {
			inactive.insert(key);
		}
	}
	// Add any readable keys
	for(const auto& [key, func] : getters) {
		if(inactive.count(key) > 0) continue;
		if(func(L, true)){
			keys.push_back(key);
		}
	}
	// Add any writable keys
	for(const auto& [key, func] : setters) {
		if(inactive.count(key) > 0) continue;
		if(func(L, 0, true)){
			keys.push_back(key);
		}
	}
	// Add the metatable methods
	if(!public_metatable.empty()) {
		luaW_getglobal(L, public_metatable);
		auto methods = luaW_get_attributes(L, -1);
		keys.insert(keys.end(), methods.begin(), methods.end());
	}
	lua_push(L, keys);
	return 1;
}
