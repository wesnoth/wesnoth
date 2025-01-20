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

#include "scripting/lua_kernel_base.hpp"

#include "game_config.hpp"
#include "game_errors.hpp"
#include "gui/core/gui_definition.hpp" // for remove_single_widget_definition
#include "log.hpp"
#include "lua_jailbreak_exception.hpp"  // for lua_jailbreak_exception
#include "seed_rng.hpp"
#include "deprecation.hpp"
#include "language.hpp"                 // for get_language
#include "team.hpp" // for shroud_map

#ifdef DEBUG_LUA
#include "scripting/debug_lua.hpp"
#endif

#include "scripting/lua_attributes.hpp"
#include "scripting/lua_color.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_cpp_function.hpp"
#include "scripting/lua_fileops.hpp"
#include "scripting/lua_formula_bridge.hpp"
#include "scripting/lua_gui2.hpp"
#include "scripting/lua_wml.hpp"
#include "scripting/lua_stringx.hpp"
#include "scripting/lua_map_location_ops.hpp"
#include "scripting/lua_mathx.hpp"
#include "scripting/lua_rng.hpp"
#include "scripting/lua_widget.hpp"
#include "scripting/push_check.hpp"

#include "game_version.hpp"                  // for do_version_check, etc

#include <functional>
#include "utils/name_generator.hpp"
#include "utils/markov_generator.hpp"
#include "utils/context_free_grammar_generator.hpp"
#include "utils/scope_exit.hpp"

#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <numeric>

#include "lua/wrapper_lualib.h"

static lg::log_domain log_scripting_lua("scripting/lua");
static lg::log_domain log_user("scripting/lua/user");
#define DBG_LUA LOG_STREAM(debug, log_scripting_lua)
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

// Registry key for metatable
static const char * Gen = "name generator";
static const char * Version = "version";
// Registry key for lua interpreter environment
static const char * Interp = "lua interpreter";

// Callback implementations

/**
 * Compares two versions.
 * - Args 1,2: version strings
 * - Ret 1: comparison result
 */
template<VERSION_COMP_OP vop>
static int impl_version_compare(lua_State* L)
{
	version_info& v1 = *static_cast<version_info*>(luaL_checkudata(L, 1, Version));
	version_info& v2 = *static_cast<version_info*>(luaL_checkudata(L, 2, Version));
	const bool result = do_version_check(v1, vop, v2);
	lua_pushboolean(L, result);
	return 1;
}

/**
 * Decomposes a version into its component parts
 */
static int impl_version_get(lua_State* L)
{
	version_info& vers = *static_cast<version_info*>(luaL_checkudata(L, 1, Version));
	if(lua_isinteger(L, 2)) {
		int n = lua_tointeger(L, 2) - 1;
		auto& components = vers.components();
		if(n >= 0 && size_t(n) < components.size()) {
			lua_pushinteger(L, vers.components()[n]);
		} else {
			lua_pushnil(L);
		}
		return 1;
	}
	char const *m = luaL_checkstring(L, 2);
	return_int_attrib("major", vers.major_version());
	return_int_attrib("minor", vers.minor_version());
	return_int_attrib("revision", vers.revision_level());
	return_bool_attrib("is_canonical", vers.is_canonical());
	return_string_attrib("special", vers.special_version());
	if(char sep = vers.special_version_separator()) {
		return_string_attrib("sep", std::string(1, sep));
	} else if(strcmp(m, "sep") == 0) {
		lua_pushnil(L);
		return 1;
	}
	return 0;
}

static int impl_version_dir(lua_State* L)
{
	static const std::vector<std::string> fields{"major", "minor", "revision", "is_canonical", "special", "sep"};
	lua_push(L, fields);
	return 1;
}

/**
 * Destroy a version
 */
static int impl_version_finalize(lua_State* L)
{
	version_info* vers = static_cast<version_info*>(luaL_checkudata(L, 1, Version));
	vers->~version_info();
	return 0;
}

/**
 * Convert a version to string form
 */
static int impl_version_tostring(lua_State* L)
{
	version_info& vers = *static_cast<version_info*>(luaL_checkudata(L, 1, Version));
	lua_push(L, vers.str());
	return 1;
}

/**
 * Builds a version from its component parts, or parses it from a string
 */
static int intf_make_version(lua_State* L)
{
	// If passed a version, just return it unchanged
	if(luaL_testudata(L, 1, Version)) {
		lua_settop(L, 1);
		return 1;
	}
	// If it's a string, parse it; otherwise build from components
	// The components method only supports canonical versions
	if(lua_type(L, 1) == LUA_TSTRING) {
		new(L) version_info(lua_check<std::string>(L, 1));
	} else {
		int major = luaL_checkinteger(L, 1), minor = luaL_optinteger(L, 2, 0), rev = luaL_optinteger(L, 3, 0);
		std::string sep, special;
		if(lua_type(L, -1) == LUA_TSTRING) {
			special = lua_tostring(L, -1);
			if(!special.empty() && std::isalpha(special[0])) {
				sep.push_back('+');
			} else {
				sep.push_back(special[0]);
				special = special.substr(1);
			}
		} else {
			sep.push_back(0);
		}
		new(L) version_info(major, minor, rev, sep[0], special);
	}
	if(luaL_newmetatable(L, Version)) {
		static const luaL_Reg metafuncs[] {
			{ "__index", &impl_version_get },
			{ "__dir", &impl_version_dir },
			{ "__tostring", &impl_version_tostring },
			{ "__lt", &impl_version_compare<VERSION_COMP_OP::OP_LESS> },
			{ "__le", &impl_version_compare<VERSION_COMP_OP::OP_LESS_OR_EQUAL> },
			{ "__eq", &impl_version_compare<VERSION_COMP_OP::OP_EQUAL> },
			{ "__gc", &impl_version_finalize },
			{ nullptr, nullptr }
		};
		luaL_setfuncs(L, metafuncs, 0);
		luaW_table_set<std::string>(L, -1, "__metatable", Version);
	}
	lua_setmetatable(L, -2);
	return 1;
}

/**
 * Returns the current Wesnoth version
 */
static int intf_current_version(lua_State* L) {
	lua_settop(L, 0);
	lua_push(L, game_config::wesnoth_version.str());
	intf_make_version(L);
	return 1;
}

/**
 * Replacement print function -- instead of printing to std::cout, print to the command log.
 * Intended to be bound to this' command_log at registration time.
 */
int lua_kernel_base::intf_print(lua_State* L)
{
	DBG_LUA << "intf_print called:";
	std::size_t nargs = lua_gettop(L);

	lua_getglobal(L, "tostring");
	for (std::size_t i = 1; i <= nargs; ++i) {
		lua_pushvalue(L, -1); // function to call: "tostring"
		lua_pushvalue(L, i); // value to pass through tostring() before printing
		lua_call(L, 1, 1);
		const char * str = lua_tostring(L, -1);
		if (!str) {
			LOG_LUA << "'tostring' must return a value to 'print'";
			str = "";
		}
		if (i > 1) {
			cmd_log_ << "\t"; //separate multiple args with tab character
		}
		cmd_log_ << str;
		DBG_LUA << "'" << str << "'";
		lua_pop(L, 1); // Pop the output of tostrring()
	}
	lua_pop(L, 1); // Pop 'tostring' global

	cmd_log_ << "\n";
	DBG_LUA;

	return 0;
}

static void impl_warn(void* p, const char* msg, int tocont) {
	static const char*const prefix = "Warning:\n  ";
	static std::ostringstream warning(prefix);
	warning.seekp(0, std::ios::end);
	warning << msg << ' ';
	if(!tocont) {
		auto L = reinterpret_cast<lua_State*>(p);
		luaW_getglobal(L, "debug", "traceback");
		lua_push(L, warning.str());
		lua_pushinteger(L, 2);
		lua_call(L, 2, 1);
		auto& lk = lua_kernel_base::get_lua_kernel<lua_kernel_base>(L);
		lk.add_log_to_console(luaL_checkstring(L, -1));
		warning.str(prefix);
	}
}

void lua_kernel_base::add_log_to_console(const std::string& msg) {
	cmd_log_ << msg << "\n";
	DBG_LUA << "'" << msg << "'";
}

/**
 * Replacement load function. Mostly the same as regular load, but disallows loading binary chunks
 * due to CVE-2018-1999023.
 */
static int intf_load(lua_State* L)
{
	std::string chunk = luaL_checkstring(L, 1);
	const char* name = luaL_optstring(L, 2, chunk.c_str());
	std::string mode = luaL_optstring(L, 3, "t");
	bool override_env = !lua_isnone(L, 4);

	if(mode != "t") {
		return luaL_argerror(L, 3, "binary chunks are not allowed for security reasons");
	}

	int result = luaL_loadbufferx(L, chunk.data(), chunk.length(), name, "t");
	if(result != LUA_OK) {
		lua_pushnil(L);
		// Move the nil as the first return value, like Lua's own load() does.
		lua_insert(L, -2);

		return 2;
	}

	if(override_env) {
		// Copy "env" to the top of the stack.
		lua_pushvalue(L, 4);
		// Set "env" as the first upvalue.
		const char* upvalue_name = lua_setupvalue(L, -2, 1);
		if(upvalue_name == nullptr) {
			// lua_setupvalue() didn't remove the copy of "env" from the stack, so we need to do it ourselves.
			lua_pop(L, 1);
		}
	}

	return 1;
}

/**
 * Wrapper for pcall and xpcall functions to rethrow jailbreak exceptions
 */
static int intf_pcall(lua_State *L)
{
	lua_CFunction function = lua_tocfunction(L, lua_upvalueindex(1));
	assert(function); // The upvalue should be Lua's pcall or xpcall, or else something is very wrong.

	int nRets = function(L);

	// If a jailbreak exception was stored while running (x)pcall, rethrow it so Lua doesn't continue.
	lua_jailbreak_exception::rethrow();

	return nRets;
}

// The show lua console callback is similarly a method of lua kernel
int lua_kernel_base::intf_show_lua_console(lua_State *L)
{
	if (cmd_log_.external_log_) {
		std::string message = "There is already an external logger attached to this lua kernel, you cannot open the lua console right now.";
		log_error(message.c_str());
		cmd_log_ << message << "\n";
		return 0;
	}

	return lua_gui2::show_lua_console(L, this);
}

static int impl_name_generator_call(lua_State *L)
{
	name_generator* gen = static_cast<name_generator*>(lua_touserdata(L, 1));
	lua_pushstring(L, gen->generate().c_str());
	return 1;
}

static int impl_name_generator_collect(lua_State *L)
{
	name_generator* gen = static_cast<name_generator*>(lua_touserdata(L, 1));
	gen->~name_generator();
	return 0;
}

static int intf_name_generator(lua_State *L)
{
	std::string type = luaL_checkstring(L, 1);
	name_generator* gen = nullptr;
	try {
		if(type == "markov" || type == "markov_chain") {
			std::vector<std::string> input;
			if(lua_istable(L, 2)) {
				input = lua_check<std::vector<std::string>>(L, 2);
			} else {
				input = utils::parenthetical_split(luaW_checktstring(L, 2).str(), ',');
			}
			int chain_sz = luaL_optinteger(L, 3, 2);
			int max_len = luaL_optinteger(L, 4, 12);
			gen = new(L) markov_generator(input, chain_sz, max_len);
			// Ensure the pointer didn't change when cast
			assert(static_cast<void*>(gen) == dynamic_cast<markov_generator*>(gen));
		} else if(type == "context_free" || type == "cfg" || type == "CFG") {
			if(lua_istable(L, 2)) {
				std::map<std::string, std::vector<std::string>> data;
				for(lua_pushnil(L); lua_next(L, 2); lua_pop(L, 1)) {
					if(lua_type(L, -2) != LUA_TSTRING) {
						lua_pushstring(L, "CFG generator: invalid nonterminal name (must be a string)");
						return lua_error(L);
					}
					if(lua_isstring(L, -1)) {
						auto& productions = data[lua_tostring(L,-2)] = utils::split(luaW_checktstring(L,-1).str(), '|');
						if(productions.size() > 1) {
							deprecated_message("wesnoth.name_generator('cfg', {nonterminal = 'a|b'})", DEP_LEVEL::INDEFINITE, "1.17", "Non-terminals should now be assigned an array of productions instead of a single string containing productions separated by | - but a single string is fine if it's only one production");
						}
					} else if(lua_istable(L, -1)) {
						const auto& split = lua_check<std::vector<t_string>>(L, -1);
						auto& productions = data[lua_tostring(L,-2)];
						std::transform(split.begin(), split.end(), std::back_inserter(productions), std::mem_fn(&t_string::str));
					} else {
						lua_pushstring(L, "CFG generator: invalid nonterminal value (must be a string or list of strings)");
						return lua_error(L);
					}
				}
				if(!data.empty()) {
					gen = new(L) context_free_grammar_generator(data);
				}
			} else {
				gen = new(L) context_free_grammar_generator(luaW_checktstring(L, 2));
			}
			if(gen) {
				assert(static_cast<void*>(gen) == dynamic_cast<context_free_grammar_generator*>(gen));
			}
		} else {
			return luaL_argerror(L, 1, "should be either 'markov_chain' or 'context_free'");
		}
	}
	catch (const name_generator_invalid_exception& ex) {
		lua_pushstring(L, ex.what());
		return lua_error(L);
	}

	// We set the metatable now, even if the generator is invalid, so that it
	// will be properly collected if it was invalid.
	luaL_getmetatable(L, Gen);
	lua_setmetatable(L, -2);

	return 1;
}

/**
* Logs a message
* Arg 1: (optional) Logger
* Arg 2: Message
*/
static int intf_log(lua_State *L) {
	const std::string& logger = lua_isstring(L, 2) ? luaL_checkstring(L, 1) : "";
	std::string msg = lua_isstring(L, 2) ? luaL_checkstring(L, 2) : luaL_checkstring(L, 1);
	if(msg.empty() || msg.back() != '\n') {
		msg += '\n';
	}

	if(logger == "err" || logger == "error") {
		LOG_STREAM(err, log_user) << msg;
	} else if(logger == "warn" || logger == "wrn" || logger == "warning") {
		LOG_STREAM(warn, log_user) << msg;
	} else if((logger == "debug" || logger == "dbg")) {
		LOG_STREAM(debug, log_user) << msg;
	} else {
		LOG_STREAM(info, log_user) << msg;
	}
	return 0;
}

/**
 * Logs a deprecation message. See deprecation.cpp for details
 * Arg 1: Element to be deprecated.
 * Arg 2: Deprecation level.
 * Arg 3: Version when element may be removed.
 * Arg 4: Additional detail message.
 */
static int intf_deprecated_message(lua_State* L) {
	const std::string elem = luaL_checkstring(L, 1);
	// This could produce an invalid deprecation level, but that possibility is handled in deprecated_message()
	const DEP_LEVEL level = DEP_LEVEL(luaL_checkinteger(L, 2));
	const std::string ver_str = lua_isnoneornil(L, 3) ? "" : luaL_checkstring(L, 3);
	const std::string detail = luaW_checktstring(L, 4);
	const version_info ver = ver_str.empty() ? game_config::wesnoth_version.str() : ver_str;
	const std::string msg = deprecated_message(elem, level, ver, detail);
	if(level < DEP_LEVEL::INDEFINITE || level >= DEP_LEVEL::REMOVED) {
		// Invalid deprecation level or level 4 deprecation should raise an interpreter error
		lua_push(L, msg);
		return lua_error(L);
	}
	lua_warning(L, msg.c_str(), false);
	return 0;
}

/**
 * Converts a Lua array to a named tuple.
 * Arg 1: A Lua array
 * Arg 2: An array of strings
 * Ret: A copy of arg 1 that's now a named tuple with the names in arg 2.
 * The copy will only include the array portion of the input array.
 * Any non-integer keys or non-consecutive keys will be gone.
 * Note: This exists so that wml.tag can use it but is not really intended as a public API.
 */
static int intf_named_tuple(lua_State* L)
{
	if(!lua_istable(L, 1)) {
		return luaW_type_error(L, 1, lua_typename(L, LUA_TTABLE));
	}
	auto names = lua_check<std::vector<std::string>>(L, 2);
	lua_len(L, 1);
	int len = luaL_checkinteger(L, -1);
	luaW_push_namedtuple(L, names);
	for(int i = 1; i <= std::max<int>(len, names.size()); i++) {
		lua_geti(L, 1, i);
		lua_seti(L, -2, i);
	}
	return 1;
}

static int intf_parse_shroud_bitmap(lua_State* L)
{
	shroud_map temp;
	temp.set_enabled(true);
	temp.read(luaL_checkstring(L, 1));
	std::set<map_location> locs;
	for(int x = 1; x <= temp.width(); x++) {
		for(int y = 1; y <= temp.height(); y++) {
			if(!temp.value(x, y)) {
				locs.emplace(x, y, wml_loc());
			}
		}
	}
	luaW_push_locationset(L, locs);
	return 1;
}

static int intf_make_shroud_bitmap(lua_State* L)
{
	shroud_map temp;
	temp.set_enabled(true);
	auto locs = luaW_check_locationset(L, 1);
	for(const auto& loc : locs) {
		temp.clear(loc.wml_x(), loc.wml_y());
	}
	lua_push(L, temp.write());
	return 1;
}

/**
* Returns the time stamp, exactly as [set_variable] time=stamp does.
* - Ret 1: integer
*/
static int intf_ms_since_init(lua_State *L) {
	lua_pushinteger(L, SDL_GetTicks());
	return 1;
}

static int intf_get_language(lua_State* L)
{
	lua_push(L, get_language().localename);
	return 1;
}

static void dir_meta_helper(lua_State* L, std::vector<std::string>& keys)
{
	switch(luaL_getmetafield(L, -1, "__dir")) {
		case LUA_TFUNCTION:
			lua_pushvalue(L, 1);
			lua_push(L, keys);
			if(lua_pcall(L, 2, 1, 0) == LUA_OK) {
				keys = lua_check<std::vector<std::string>>(L, -1);
			} else {
				lua_warning(L, "wesnoth.print_attributes: __dir metamethod raised an error", false);
			}
			break;
		case LUA_TTABLE:
			auto dir_keys = lua_check<std::vector<std::string>>(L, -1);
			std::copy(dir_keys.begin(), dir_keys.end(), std::back_inserter(keys));
			break;
	}
	lua_pop(L, 1);
}

// This is a separate function so I can use a protected call on it to catch errors.
static int impl_is_deprecated(lua_State* L)
{
	auto key = luaL_checkstring(L, 2);
	auto type = lua_getfield(L, 1, key);
	if(type == LUA_TTABLE) {
		lua_pushliteral(L, "__deprecated");
		if(lua_rawget(L, -2) == LUA_TBOOLEAN) {
			auto deprecated = luaW_toboolean(L, -1);
			lua_pushboolean(L, deprecated);
			return 1;
		}
		lua_pop(L, 1);
	}
	lua_pushboolean(L, false);
	return 1;
}

// This is also a separate function so I can use a protected call on it to catch errors.
static int impl_get_dir_suffix(lua_State*L)
{
	auto key = luaL_checkstring(L, 2);
	std::string suffix = " ";
	auto type = lua_getfield(L, 1, key);
	if(type == LUA_TTABLE) {
		suffix = "†";
	} else if(type == LUA_TFUNCTION) {
		suffix = "ƒ";
	} else if(type == LUA_TUSERDATA) {
		lua_getglobal(L, "getmetatable");
		lua_pushvalue(L, -2);
		lua_call(L, 1, 1);
		if(lua_type(L, -1) == LUA_TSTRING) {
			auto meta = lua_check<std::string>(L, -1);
			if(meta == "function") {
				suffix = "ƒ";
			}
		}
		lua_pop(L, 1);
		if(suffix.size() == 1) {
			// ie, the above block didn't identify it as a function
			if(auto t = luaL_getmetafield(L, -1, "__dir_tablelike"); t == LUA_TBOOLEAN) {
				if(luaW_toboolean(L, -1)) {
					suffix = "†";
				}
				lua_pop(L, 1);
			} else if(t != LUA_TNIL) {
				lua_pop(L,  1);
			}
		}
	}
	suffix = " " + suffix;
	lua_pushlstring(L, suffix.c_str(), suffix.size());
	return 1;
}

/**
 * This function does the actual work of grabbing all the attribute names.
 * It's a separate function so that it can be used by tab-completion as well.
 */
std::vector<std::string> luaW_get_attributes(lua_State* L, int idx)
{
	if(idx < 0 && idx >= -lua_gettop(L)) {
		idx = lua_absindex(L, idx);
	}
	std::vector<std::string> keys;
	if(lua_istable(L, idx)) {
		// Walk the metatable chain (as long as __index is a table)...
		// If we reach an __index that's a function, check for a __dir metafunction.
		int save_top = lua_gettop(L);
		lua_pushvalue(L, idx);
		ON_SCOPE_EXIT(&) {
			lua_settop(L, save_top);
		};
		do {
			int table_idx = lua_absindex(L, -1);
			for(lua_pushnil(L); lua_next(L, table_idx); lua_pop(L, 1)) {
				if(lua_type(L, -2) == LUA_TSTRING) {
					keys.push_back(lua_tostring(L,-2));
				}
			}
			// Two possible exit cases:
			// 1. getmetafield returns TNIL because there is no __index
			// In this case, the stack is unchanged, so the while condition is still true.
			// 2. The __index is not a table
			// In this case, obviously the while condition fails
			if(luaL_getmetafield(L, table_idx, "__index") == LUA_TNIL) break;
		} while(lua_istable(L, -1));
		if(lua_isfunction(L, -1)) {
			lua_pop(L, 1);
			dir_meta_helper(L, keys);
		}
	} else if(lua_isuserdata(L, idx) && !lua_islightuserdata(L, idx)) {
		lua_pushvalue(L, idx);
		dir_meta_helper(L, keys);
		lua_pop(L, 1);
	}
	// Sort and remove any duplicates
	std::sort(keys.begin(), keys.end());
	auto new_end = std::unique(keys.begin(), keys.end());
	new_end = std::remove_if(keys.begin(), new_end, [L, idx](const std::string& key) {
		if(key.compare(0, 2, "__") == 0) {
			return true;
		}
		int save_top = lua_gettop(L);
		ON_SCOPE_EXIT(&) {
			lua_settop(L, save_top);
		};
		// Exclude deprecated elements
		// Some keys may be write-only, which would raise an exception here
		// In that case we just ignore it and assume not deprecated
		// (the __dir metamethod would be responsible for excluding deprecated write-only keys)
		lua_pushcfunction(L, impl_is_deprecated);
		lua_pushvalue(L, idx);
		lua_push(L, key);
		if(lua_pcall(L, 2, 1, 0) == LUA_OK) {
			return luaW_toboolean(L, -1);
		}
		return false;
	});
	keys.erase(new_end, keys.end());
	return keys;
}

/**
 * Prints out a list of keys available in an object.
 * A list of keys is gathered from the following sources:
 * - For a table, all keys defined in the table
 * - Any keys accessible through the metatable chain (if __index on the metatable is a table)
 * - The output of the __dir metafunction
 * - Filtering out any keys beginning with two underscores
 * - Filtering out any keys for which object[key].__deprecated exists and is true
 * The list is then sorted alphabetically and formatted into columns.
 * - Arg 1: Any object
 * - Arg 2: (optional) Function to use for output; defaults to _G.print
 */
static int intf_object_dir(lua_State* L)
{
	if(lua_isnil(L, 1)) return luaL_argerror(L, 1, "Can't dir() nil");
	if(!lua_isfunction(L, 2)) {
		luaW_getglobal(L, "print");
	}
	int fcn_idx = lua_gettop(L);
	auto keys = luaW_get_attributes(L, 1);
	size_t max_len = std::accumulate(keys.begin(), keys.end(), 0, [](size_t max, const std::string& next) {
		return std::max(max, next.size());
	});
	// Let's limit to about 80 characters of total width with minimum 3 characters padding between columns
	static const size_t MAX_WIDTH = 80, COL_PADDING = 3, SUFFIX_PADDING = 2;
	size_t col_width = max_len + COL_PADDING + SUFFIX_PADDING;
	size_t n_cols = (MAX_WIDTH + COL_PADDING) / col_width;
	size_t n_rows = ceil(keys.size() / double(n_cols));
	for(size_t i = 0; i < n_rows; i++) {
		std::ostringstream line;
		line.fill(' ');
		line.setf(std::ios::left);
		for(size_t j = 0; j < n_cols && j + (i * n_cols) < keys.size(); j++) {
			int save_top = lua_gettop(L);
			ON_SCOPE_EXIT(&) {
				lua_settop(L, save_top);
			};
			lua_pushcfunction(L, impl_get_dir_suffix);
			lua_pushvalue(L, 1);
			const auto& key = keys[j + i * n_cols];
			lua_pushlstring(L, key.c_str(), key.size());
			std::string suffix = " !"; // Exclamation mark to indicate an error
			if(lua_pcall(L, 2, 1, 0) == LUA_OK) {
				suffix = luaL_checkstring(L, -1);
			}
			// This weird calculation is because width counts in bytes, not code points
			// Since the suffix is a Unicode character, that messes up the alignment
			line.width(col_width - SUFFIX_PADDING + suffix.size());
			// Concatenate key and suffix beforehand so they share the same field width.
			line << (key + suffix) << std::flush;
		}
		lua_pushvalue(L, fcn_idx);
		lua_push(L, line.str());
		lua_call(L, 1, 0);
	}
	return 0;
}

// End Callback implementations

// Template which allows to push member functions to the lua kernel base into lua as C functions, using a shim
typedef int (lua_kernel_base::*member_callback)(lua_State *L);

template <member_callback method>
int dispatch(lua_State *L) {
	return ((lua_kernel_base::get_lua_kernel<lua_kernel_base>(L)).*method)(L);
}

// Ctor, initialization
lua_kernel_base::lua_kernel_base()
 : mState(luaL_newstate())
 , cmd_log_()
{
	get_lua_kernel_base_ptr(mState) = this;
	lua_State *L = mState;

	cmd_log_ << "Initializing " << my_name() << "...\n";

	// Define the CPP_function metatable ( so we can override print to point to a C++ member function, add certain functions for this kernel, etc. )
	// Do it first of all in case C++ functions are ever used in the core Wesnoth libs loaded in the next step
	cmd_log_ << "Adding boost function proxy...\n";

	lua_cpp::register_metatable(L);

	// Open safe libraries.
	// Debug and OS are not, but most of their functions will be disabled below.
	cmd_log_ << "Adding standard libs...\n";

	static const luaL_Reg safe_libs[] {
		{ "",       luaopen_base   },
		{ "table",  luaopen_table  },
		{ "string", luaopen_string },
		{ "math",   luaopen_math   },
		{ "coroutine",   luaopen_coroutine   },
		{ "debug",  luaopen_debug  },
		{ "os",     luaopen_os     },
		{ "utf8",	luaopen_utf8   }, // added in Lua 5.3
		// Wesnoth libraries
		{ "stringx",lua_stringx::luaW_open },
		{ "mathx",  lua_mathx::luaW_open },
		{ "wml",    lua_wml::luaW_open },
		{ "gui",    lua_gui2::luaW_open },
		{ "filesystem", lua_fileops::luaW_open },
		{ nullptr, nullptr }
	};
	for (luaL_Reg const *lib = safe_libs; lib->func; ++lib)
	{
		luaL_requiref(L, lib->name, lib->func, strlen(lib->name));
		lua_pop(L, 1);  /* remove lib */
	}

	// Disable functions from os which we don't want.
	lua_getglobal(L, "os");
	lua_pushnil(L);
	while(lua_next(L, -2) != 0) {
		lua_pop(L, 1);
		char const* function = lua_tostring(L, -1);
		if(strcmp(function, "clock") == 0 || strcmp(function, "date") == 0
			|| strcmp(function, "time") == 0 || strcmp(function, "difftime") == 0) continue;
		lua_pushnil(L);
		lua_setfield(L, -3, function);
	}
	lua_pop(L, 1);

	// Delete dofile and loadfile.
	lua_pushnil(L);
	lua_setglobal(L, "dofile");
	lua_pushnil(L);
	lua_setglobal(L, "loadfile");

	// Store the error handler.
	cmd_log_ << "Adding error handler...\n";
	push_error_handler(L);


	lua_settop(L, 0);

	// Add some callback from the wesnoth lib
	cmd_log_ << "Registering basic wesnoth API...\n";

	static luaL_Reg const callbacks[] {
		{ "deprecated_message",       &intf_deprecated_message              },
		{ "textdomain",               &lua_common::intf_textdomain   		},
		{ "dofile",                   &dispatch<&lua_kernel_base::intf_dofile>           },
		{ "require",                  &dispatch<&lua_kernel_base::intf_require>          },
		{ "kernel_type",              &dispatch<&lua_kernel_base::intf_kernel_type>          },
		{ "compile_formula",          &lua_formula_bridge::intf_compile_formula},
		{ "eval_formula",             &lua_formula_bridge::intf_eval_formula},
		{ "name_generator",           &intf_name_generator           },
		{ "named_tuple",              &intf_named_tuple              },
		{ "log",                      &intf_log                      },
		{ "ms_since_init",            &intf_ms_since_init           },
		{ "get_language",             &intf_get_language             },
		{ "version",                  &intf_make_version       },
		{ "current_version",          &intf_current_version    },
		{ "print_attributes",         &intf_object_dir         },
		{ nullptr, nullptr }
	};

	lua_getglobal(L, "wesnoth");
	if (!lua_istable(L,-1)) {
		lua_newtable(L);
	}
	luaL_setfuncs(L, callbacks, 0);
	//lua_cpp::set_functions(L, cpp_callbacks, 0);
	lua_setglobal(L, "wesnoth");

	// Create the gettext metatable.
	cmd_log_ << lua_common::register_gettext_metatable(L);
	// Create the tstring metatable.
	cmd_log_ << lua_common::register_tstring_metatable(L);

	lua_widget::register_metatable(L);

	// Override the print function
	cmd_log_ << "Redirecting print function...\n";

	lua_getglobal(L, "print");
	lua_setglobal(L, "std_print"); //storing original impl as 'std_print'
	lua_settop(L, 0); //clear stack, just to be sure

	lua_setwarnf(L, &::impl_warn, L);
	lua_pushcfunction(L, &dispatch<&lua_kernel_base::intf_print>);
	lua_setglobal(L, "print");

	lua_pushcfunction(L, intf_load);
	lua_setglobal(L, "load");
	lua_pushnil(L);
	lua_setglobal(L, "loadstring");

	// Wrap the pcall and xpcall functions
	cmd_log_ << "Wrapping pcall and xpcall functions...\n";
	lua_getglobal(L, "pcall");
	lua_pushcclosure(L, intf_pcall, 1);
	lua_setglobal(L, "pcall");
	lua_getglobal(L, "xpcall");
	lua_pushcclosure(L, intf_pcall, 1);
	lua_setglobal(L, "xpcall");

	cmd_log_ << "Initializing package repository...\n";
	// Create the package table.
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	lua_setfield(L, -2, "package");
	lua_pop(L, 1);
	lua_settop(L, 0);
	lua_pushstring(L, "lua/package.lua");
	int res = intf_require(L);
	if(res != 1) {
		cmd_log_ << "Error: Failed to initialize package repository. Falling back to less flexible C++ implementation.\n";
	}

	// Get some callbacks for map locations
	cmd_log_ << "Adding map table...\n";

	static luaL_Reg const map_callbacks[] {
		{ "get_direction",		&lua_map_location::intf_get_direction         		},
		{ "hex_vector_sum",			&lua_map_location::intf_vector_sum			},
		{ "hex_vector_diff",			&lua_map_location::intf_vector_diff			},
		{ "hex_vector_negation",		&lua_map_location::intf_vector_negation			},
		{ "rotate_right_around_center",	&lua_map_location::intf_rotate_right_around_center	},
		{ "are_hexes_adjacent",		&lua_map_location::intf_tiles_adjacent			},
		{ "get_adjacent_hexes",		&lua_map_location::intf_get_adjacent_tiles		},
		{ "get_hexes_in_radius",		&lua_map_location::intf_get_tiles_in_radius		},
		{ "get_hexes_at_radius",		&lua_map_location::intf_get_tile_ring		},
		{ "distance_between",		&lua_map_location::intf_distance_between		},
		{ "get_cubic",		&lua_map_location::intf_get_in_cubic		},
		{ "from_cubic",		&lua_map_location::intf_get_from_cubic		},
		{ "get_relative_dir",		&lua_map_location::intf_get_relative_dir		},
		// Shroud bitmaps
		{"parse_bitmap", intf_parse_shroud_bitmap},
		{"make_bitmap", intf_make_shroud_bitmap},
		{ nullptr, nullptr }
	};

	// Create the map_location table.
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	luaL_setfuncs(L, map_callbacks, 0);
	lua_setfield(L, -2, "map");
	lua_pop(L, 1);

	// Create the game_config variable with its metatable.
	cmd_log_ << "Adding game_config table...\n";

	lua_getglobal(L, "wesnoth");
	lua_newuserdatauv(L, 0, 0);
	lua_createtable(L, 0, 3);
	lua_pushcfunction(L, &dispatch<&lua_kernel_base::impl_game_config_get>);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, &dispatch<&lua_kernel_base::impl_game_config_set>);
	lua_setfield(L, -2, "__newindex");
	lua_pushcfunction(L, &dispatch<&lua_kernel_base::impl_game_config_dir>);
	lua_setfield(L, -2, "__dir");
	lua_pushboolean(L, true);
	lua_setfield(L, -2, "__dir_tablelike");
	lua_pushstring(L, "game config");
	lua_setfield(L, -2, "__metatable");
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "game_config");
	lua_pop(L, 1);

	// Add mersenne twister rng wrapper
	cmd_log_ << "Adding rng tables...\n";
	lua_rng::load_tables(L);

	cmd_log_ << "Adding name generator metatable...\n";
	luaL_newmetatable(L, Gen);
	static luaL_Reg const generator[] {
		{ "__call", &impl_name_generator_call},
		{ "__gc", &impl_name_generator_collect},
		{ nullptr, nullptr}
	};
	luaL_setfuncs(L, generator, 0);

	// Create formula bridge metatables
	cmd_log_ << lua_formula_bridge::register_metatables(L);

	cmd_log_ << lua_colors::register_metatables(L);

	// Create the Lua interpreter table
	cmd_log_ << "Sandboxing Lua interpreter...\nTo make variables visible outside the interpreter, assign to _G.variable.\n";
	cmd_log_ << "The special variable _ holds the result of the last expression (if any).\n";
	lua_newtable(L);
	lua_createtable(L, 0, 1);
	lua_getglobal(L, "_G");
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);
	lua_pushcfunction(L, intf_object_dir);
	lua_setfield(L, -2, "dir");
	lua_setfield(L, LUA_REGISTRYINDEX, Interp);

	// Loading ilua:
	cmd_log_ << "Loading ilua...\n";

	lua_settop(L, 0);
	luaW_getglobal(L, "wesnoth", "require");
	lua_pushstring(L, "lua/ilua.lua");
	if(protected_call(1, 1)) {
		//run "ilua.set_strict()"
		lua_pushstring(L, "set_strict");
		lua_gettable(L, -2);
		if (!this->protected_call(0,0, std::bind(&lua_kernel_base::log_error, this, std::placeholders::_1, std::placeholders::_2))) {
			cmd_log_ << "Failed to activate strict mode.\n";
		} else {
			cmd_log_ << "Activated strict mode.\n";
		}

		lua_setglobal(L, "ilua"); //save ilua table as a global
	} else {
		cmd_log_ << "Error: failed to load ilua.\n";
	}
	lua_settop(L, 0);

	// Disable functions from debug which we don't want.
	// We do this last because ilua needs to be able to use debug.getmetatable
	lua_getglobal(L, "debug");
	lua_pushnil(L);
	while(lua_next(L, -2) != 0) {
		lua_pop(L, 1);
		char const* function = lua_tostring(L, -1);
		if(strcmp(function, "traceback") == 0 || strcmp(function, "getinfo") == 0) continue;	//traceback is needed for our error handler
		lua_pushnil(L);										//getinfo is needed for ilua strict mode
		lua_setfield(L, -3, function);
	}
	lua_pop(L, 1);
}

lua_kernel_base::~lua_kernel_base()
{
	for (const auto& pair : this->registered_widget_definitions_) {
		gui2::remove_single_widget_definition(std::get<0>(pair), std::get<1>(pair));
	}
	lua_close(mState);
}

void lua_kernel_base::log_error(char const * msg, char const * context)
{
	ERR_LUA << context << ": " << msg;
}

void lua_kernel_base::throw_exception(char const * msg, char const * context)
{
	throw game::lua_error(msg, context);
}

bool lua_kernel_base::protected_call(int nArgs, int nRets)
{
	error_handler eh = std::bind(&lua_kernel_base::log_error, this, std::placeholders::_1, std::placeholders::_2 );
	return this->protected_call(nArgs, nRets, eh);
}

bool lua_kernel_base::load_string(char const * prog, const std::string& name)
{
	error_handler eh = std::bind(&lua_kernel_base::log_error, this, std::placeholders::_1, std::placeholders::_2 );
	return this->load_string(prog, name, eh);
}

bool lua_kernel_base::protected_call(int nArgs, int nRets, const error_handler& e_h)
{
	return this->protected_call(mState, nArgs, nRets, e_h);
}

bool lua_kernel_base::protected_call(lua_State * L, int nArgs, int nRets, const error_handler& e_h)
{
	int errcode = luaW_pcall_internal(L, nArgs, nRets);

	if (errcode != LUA_OK) {
		char const * msg = lua_tostring(L, -1);

		std::string context = "When executing, ";
		if (errcode == LUA_ERRRUN) {
			context += "Lua runtime error: ";
		} else if (errcode == LUA_ERRERR) {
			context += "Lua error in attached debugger: ";
		} else if (errcode == LUA_ERRMEM) {
			context += "Lua out of memory error: ";
		} else {
			context += "unknown lua error: ";
		}
		if(lua_isstring(L, -1)) {
			context +=  msg ? msg : "null string";
		} else {
			context += lua_typename(L, lua_type(L, -1));
		}

		lua_pop(L, 1);

		e_h(context.c_str(), "Lua Error");

		return false;
	}

	return true;
}

bool lua_kernel_base::load_string(const std::string& prog, const std::string& name, const error_handler& e_h, bool allow_unsafe)
{
	// pass 't' to prevent loading bytecode which is unsafe and can be used to escape the sandbox.
	int errcode = luaL_loadbufferx(mState, prog.c_str(), prog.size(), name.empty() ? prog.c_str() : name.c_str(), allow_unsafe ? "tb" : "t");
	if (errcode != LUA_OK) {
		char const * msg = lua_tostring(mState, -1);
		std::string message = msg ? msg : "null string";

		std::string context = "When parsing a string to lua, ";

		if (errcode == LUA_ERRSYNTAX) {
			context += " a syntax error";
		} else if(errcode == LUA_ERRMEM){
			context += " a memory error";
		} else {
			context += " an unknown error";
		}

		lua_pop(mState, 1);

		e_h(message.c_str(), context.c_str());

		return false;
	}
	return true;
}

void lua_kernel_base::run_lua_tag(const config& cfg)
{
	int nArgs = 0;
	if (auto args = cfg.optional_child("args")) {
		luaW_pushconfig(this->mState, *args);
		++nArgs;
	}
	this->run(cfg["code"].str().c_str(), cfg["name"].str(), nArgs);
}

config luaW_serialize_function(lua_State* L, int func)
{
	if(lua_iscfunction(L, func)) {
		throw luafunc_serialize_error("cannot serialize C function");
	}
	if(!lua_isfunction(L, func)) {
		throw luafunc_serialize_error("cannot serialize callable non-function");
	}
	config data;
	lua_Debug info;
	lua_pushvalue(L, func); // push copy of function because lua_getinfo will pop it
	lua_getinfo(L, ">u", &info);
	data["params"] = info.nparams;
	luaW_getglobal(L, "string", "dump");
	lua_pushvalue(L, func);
	lua_call(L, 1, 1);
	data["code"] = lua_check<std::string>(L, -1);
	lua_pop(L, 1);
	config upvalues;
	for(int i = 1; i <= info.nups; i++, lua_pop(L, 1)) {
		std::string_view name = lua_getupvalue(L, func, i);
		if(name == "_ENV") {
			upvalues.add_child(name)["upvalue_type"] = "_ENV";
			continue;
		}
		int idx = lua_absindex(L, -1);
		switch(lua_type(L, idx)) {
		case LUA_TBOOLEAN: case LUA_TNUMBER: case LUA_TSTRING:
			luaW_toscalar(L, idx, upvalues[name]);
			break;
		case LUA_TFUNCTION:
			upvalues.add_child(name, luaW_serialize_function(L, idx))["upvalue_type"] = "function";
			break;
		case LUA_TNIL:
			upvalues.add_child(name, config{"upvalue_type", "nil"});
			break;
		case LUA_TTABLE:
			if(std::vector<std::string> names = luaW_to_namedtuple(L, idx); !names.empty()) {
				for(size_t i = 1; i <= lua_rawlen(L, -1); i++, lua_pop(L, 1)) {
					lua_rawgeti(L, idx, i);
					config& cfg = upvalues.add_child(name);
					luaW_toscalar(L, -1, cfg["value"]);
					cfg["name"] = names[0];
					cfg["upvalue_type"] = "named tuple";
					names.erase(names.begin());
				}
				break;
			} else if(config cfg; luaW_toconfig(L, idx, cfg)) {
				std::vector<std::string> names;
				int save_top = lua_gettop(L);
				if(luaL_getmetafield(L, idx, "__name") && lua_check<std::string>(L, -1) == "named tuple") {
					luaL_getmetafield(L, -2, "__names");
					names = lua_check<std::vector<std::string>>(L, -1);
				}
				lua_settop(L, save_top);
				upvalues.add_child(name, cfg)["upvalue_type"] = names.empty() ? "config" : "named tuple";
				break;
			} else {
				for(size_t i = 1; i <= lua_rawlen(L, -1); i++, lua_pop(L, 1)) {
					lua_rawgeti(L, idx, i);
					config& cfg = upvalues.add_child(name);
					luaW_toscalar(L, -1, cfg["value"]);
					cfg["upvalue_type"] = "array";
				}
				bool found_non_array = false;
				for(lua_pushnil(L); lua_next(L, idx); lua_pop(L, 1)) {
					if(lua_type(L, -2) != LUA_TNUMBER) {
						found_non_array = true;
						break;
					}
				}
				if(!found_non_array) break;
			}
			[[fallthrough]];
		default:
			std::ostringstream os;
			os << "cannot serialize function with upvalue " << name << " = ";
			luaW_getglobal(L, "wesnoth", "as_text");
			lua_pushvalue(L, idx);
			lua_call(L, 1, 1);
			os << luaL_checkstring(L, -1);
			lua_pushboolean(L, false);
			throw luafunc_serialize_error(os.str());
		}
	}
	if(!upvalues.empty()) data.add_child("upvalues", upvalues);
	return data;
}

bool lua_kernel_base::load_binary(const config& cfg, const error_handler& eh)
{
	if(!load_string(cfg["code"].str(), cfg["name"], eh, true)) return false;
	if(auto upvalues = cfg.optional_child("upvalues")) {
		lua_pushvalue(mState, -1); // duplicate function because lua_getinfo will pop it
		lua_Debug info;
		lua_getinfo(mState, ">u", &info);
		int funcindex = lua_absindex(mState, -1);
		for(int i = 1; i <= info.nups; i++) {
			std::string_view name = lua_getupvalue(mState, funcindex, i);
			lua_pop(mState, 1); // we only want the upvalue's name, not its value
			if(name == "_ENV") {
				lua_pushglobaltable(mState);
			} else if(upvalues->has_attribute(name)) {
				luaW_pushscalar(mState, (*upvalues)[name]);
			} else if(upvalues->has_child(name)) {
				const auto& child = upvalues->mandatory_child(name);
				if(child["upvalue_type"] == "array") {
					auto children = upvalues->child_range(name);
					lua_createtable(mState, children.size(), 0);
					for(const auto& cfg : children) {
						luaW_pushscalar(mState, cfg["value"]);
						lua_rawseti(mState, -2, lua_rawlen(mState, -2) + 1);
					}
				} else if(child["upvalue_type"] == "config") {
					luaW_pushconfig(mState, child);
				} else if(child["upvalue_type"] == "function") {
					if(!load_binary(child, eh)) return false;
				} else if(child["upvalue_type"] == "nil") {
					lua_pushnil(mState);
				}
			} else continue;
			lua_setupvalue(mState, funcindex, i);
		}
	}
	return true;
}

config lua_kernel_base::run_binary_lua_tag(const config& cfg)
{
	int top = lua_gettop(mState);
	try {
		error_handler eh = std::bind(&lua_kernel_base::throw_exception, this, std::placeholders::_1, std::placeholders::_2 );
		if(load_binary(cfg, eh)) {
			lua_pushvalue(mState, -1);
			protected_call(0, LUA_MULTRET, eh);
		}
	} catch (const game::lua_error & e) {
		cmd_log_ << e.what() << "\n";
		lua_kernel_base::log_error(e.what(), "In function lua_kernel::run()");
		config error;
		error["name"] = "execute_error";
		error["error"] = e.what();
		return error;
	}
	config result;
	result["ref"] = cfg["ref"];
	result.add_child("executed") = luaW_serialize_function(mState, top + 1);
	lua_remove(mState, top + 1);
	result["name"] = "execute_result";
	for(int i = top + 1; i < lua_gettop(mState); i++) {
		std::string index = std::to_string(i - top);
		switch(lua_type(mState, i)) {
		case LUA_TNUMBER: case LUA_TBOOLEAN: case LUA_TSTRING:
			luaW_toscalar(mState, i, result[index]);
			break;
		case LUA_TTABLE:
			luaW_toconfig(mState, i, result.add_child(index));
			break;
		}
	}
	return result;
}
// Call load_string and protected call. Make them throw exceptions.
//
void lua_kernel_base::throwing_run(const char * prog, const std::string& name, int nArgs, bool in_interpreter)
{
	cmd_log_ << "$ " << prog << "\n";
	error_handler eh = std::bind(&lua_kernel_base::throw_exception, this, std::placeholders::_1, std::placeholders::_2 );
	this->load_string(prog, name, eh);
	if(in_interpreter) {
		lua_getfield(mState, LUA_REGISTRYINDEX, Interp);
		if(lua_setupvalue(mState, -2, 1) == nullptr)
			lua_pop(mState, 1);
	}
	lua_insert(mState, -nArgs - 1);
	this->protected_call(nArgs, in_interpreter ? LUA_MULTRET : 0, eh);
}

// Do a throwing run, but if we catch a lua_error, reformat it with signature for this function and log it.
void lua_kernel_base::run(const char * prog, const std::string& name, int nArgs)
{
	try {
		this->throwing_run(prog, name, nArgs);
	} catch (const game::lua_error & e) {
		cmd_log_ << e.what() << "\n";
		lua_kernel_base::log_error(e.what(), "In function lua_kernel::run()");
	}
}

// Tests if a program resolves to an expression, and pretty prints it if it is, otherwise it runs it normally. Throws exceptions.
void lua_kernel_base::interactive_run(char const * prog) {
	std::string experiment = "return ";
	experiment += prog;
	int top = lua_gettop(mState);

	error_handler eh = std::bind(&lua_kernel_base::throw_exception, this, std::placeholders::_1, std::placeholders::_2 );
	luaW_getglobal(mState, "ilua", "_pretty_print");

	try {
		// Try to load the experiment without syntax errors
		this->load_string(experiment.c_str(), "interactive", eh);
		lua_getfield(mState, LUA_REGISTRYINDEX, Interp);
		if(lua_setupvalue(mState, -2, 1) == nullptr)
			lua_pop(mState, 1);
	} catch (const game::lua_error &) {
		this->throwing_run(prog, "interactive", 0, true);	// Since it failed, fall back to the usual throwing_run, on the original input.
		if(lua_gettop(mState) == top + 1) {
			// Didn't return anything
			lua_settop(mState, top);
		return;
		} else goto PRINT;
	}
	// experiment succeeded, now run but log normally.
	cmd_log_ << "$ " << prog << "\n";
	this->protected_call(0, LUA_MULTRET, eh);
PRINT:
	int nRets = lua_gettop(mState) - top - 1;
	{
		// Assign first result to _
		lua_getfield(mState, LUA_REGISTRYINDEX, Interp);
		int env_idx = lua_gettop(mState);
		lua_pushvalue(mState, top + 2);
		lua_setfield(mState, -2, "_");
		// Now duplicate EVERY result and pass it to table.pack, assigning to _all
		luaW_getglobal(mState, "table", "pack");
		for(int i = top + 2; i < env_idx; i++)
			lua_pushvalue(mState, i);
		this->protected_call(nRets, 1, eh);
		lua_setfield(mState, -2, "_all");
		lua_pop(mState, 1);
	}
	// stack is now ilua._pretty_print followed by any results of prog
	this->protected_call(lua_gettop(mState) - top - 1, 0, eh);
}
/**
 * Loads and executes a Lua file.
 * - Arg 1: string containing the file name.
 * - Ret *: values returned by executing the file body.
 */
int lua_kernel_base::intf_dofile(lua_State* L)
{
	luaL_checkstring(L, 1);
	lua_rotate(L, 1, -1);
	if (lua_fileops::load_file(L) != 1) return 0;
	//^ should end with the file contents loaded on the stack. actually it will call lua_error otherwise, the return 0 is redundant.
	lua_rotate(L, 1, 1);
	// Using a non-protected call here appears to fix an issue in plugins.
	// The protected call isn't technically necessary anyway, because this function is called from Lua code,
	// which should already be in a protected environment.
	lua_call(L, lua_gettop(L) - 1, LUA_MULTRET);
	return lua_gettop(L);
}

/**
 * Loads and executes a Lua file, if there is no corresponding entry in wesnoth.package.
 * Stores the result of the script in wesnoth.package and returns it.
 * - Arg 1: string containing the file name.
 * - Ret 1: value returned by the script.
 */
int lua_kernel_base::intf_require(lua_State* L)
{
	const char * m = luaL_checkstring(L, 1);
	if(!m) {
		return luaL_argerror(L, 1, "found a null string argument to wesnoth require");
	}

	// Check if there is already an entry.

	lua_getglobal(L, "wesnoth");
	lua_pushstring(L, "package");
	lua_rawget(L, -2);
	lua_pushvalue(L, 1);
	lua_rawget(L, -2);
	if(!lua_isnil(L, -1) && !game_config::debug_lua) {
		return 1;
	}
	lua_pop(L, 1);
	lua_pushvalue(L, 1);
	// stack is now [packagename] [wesnoth] [package] [packagename]

	if(lua_fileops::load_file(L) != 1) {
		// should end with the file contents loaded on the stack. actually it will call lua_error otherwise, the return 0 is redundant.
		// stack is now [packagename] [wesnoth] [package] [chunk]
		return 0;
	}
	DBG_LUA << "require: loaded a file, now calling it";

	if (!this->protected_call(L, 0, 1, std::bind(&lua_kernel_base::log_error, this, std::placeholders::_1, std::placeholders::_2))) {
		// historically if wesnoth.require fails it just yields nil and some logging messages, not a lua error
		return 0;
	}
	// stack is now [packagename] [wesnoth] [package] [results]

	lua_pushvalue(L, 1);
	lua_pushvalue(L, -2);
	// stack is now [packagename] [wesnoth] [package] [results] [packagename] [results]
	// Add the return value to the table.

	lua_settable(L, -4);
	// stack is now [packagename] [wesnoth] [package] [results]
	return 1;
}
int lua_kernel_base::intf_kernel_type(lua_State* L)
{
	lua_push(L, my_name());
	return 1;
}
static void push_color_palette(lua_State* L, const std::vector<color_t>& palette) {
	lua_createtable(L, palette.size(), 1);
	lua_rotate(L, -2, 1); // swap new table with previous element on stack
	lua_setfield(L, -2, "name");
	for(size_t i = 0; i < palette.size(); i++) {
		luaW_push_namedtuple(L, {"r", "g", "b", "a"});
		lua_pushinteger(L, palette[i].r);
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, palette[i].g);
		lua_rawseti(L, -2, 2);
		lua_pushinteger(L, palette[i].b);
		lua_rawseti(L, -2, 3);
		lua_pushinteger(L, palette[i].a);
		lua_rawseti(L, -2, 4);
		lua_rawseti(L, -2, i);
	}
}
static int impl_palette_get(lua_State* L)
{
	char const *m = luaL_checkstring(L, 2);
	lua_pushvalue(L, 2);
	push_color_palette(L, game_config::tc_info(m));
	return 1;
}

// suppress missing prototype warning (not static because game_lua_kernel referenes it);
luaW_Registry& gameConfigReg();
luaW_Registry& gameConfigReg() {
	static luaW_Registry gameConfigReg{"game config"};
	return gameConfigReg;
}
static auto& dummy = gameConfigReg(); // just to ensure it's constructed.

#define GAME_CONFIG_SIMPLE_GETTER(name) \
GAME_CONFIG_GETTER(#name, decltype(game_config::name), lua_kernel_base) { \
	(void) k; \
	return game_config::name; \
}

namespace {
GAME_CONFIG_SIMPLE_GETTER(base_income);
GAME_CONFIG_SIMPLE_GETTER(village_income);
GAME_CONFIG_SIMPLE_GETTER(village_support);
GAME_CONFIG_SIMPLE_GETTER(poison_amount);
GAME_CONFIG_SIMPLE_GETTER(rest_heal_amount);
GAME_CONFIG_SIMPLE_GETTER(recall_cost);
GAME_CONFIG_SIMPLE_GETTER(kill_experience);
GAME_CONFIG_SIMPLE_GETTER(combat_experience);
GAME_CONFIG_SIMPLE_GETTER(debug);
GAME_CONFIG_SIMPLE_GETTER(debug_lua);
GAME_CONFIG_SIMPLE_GETTER(strict_lua);
GAME_CONFIG_SIMPLE_GETTER(mp_debug);

GAME_CONFIG_GETTER("palettes", lua_index_raw, lua_kernel_base) {
	(void)k;
	lua_newtable(L);
	if(luaL_newmetatable(L, "color palettes")) {
		lua_pushcfunction(L, impl_palette_get);
		lua_setfield(L, -2, "__index");
	}
	lua_setmetatable(L, -2);
	return lua_index_raw(L);
}

GAME_CONFIG_GETTER("red_green_scale", lua_index_raw, lua_kernel_base) {
	(void)k;
	lua_pushstring(L, "red_green_scale");
	push_color_palette(L, game_config::red_green_scale);
	return lua_index_raw(L);
}

GAME_CONFIG_GETTER("red_green_scale_text", lua_index_raw, lua_kernel_base) {
	(void)k;
	lua_pushstring(L, "red_green_scale_text");
	push_color_palette(L, game_config::red_green_scale_text);
	return lua_index_raw(L);
}

GAME_CONFIG_GETTER("blue_white_scale", lua_index_raw, lua_kernel_base) {
	(void)k;
	lua_pushstring(L, "blue_white_scale");
	push_color_palette(L, game_config::blue_white_scale);
	return lua_index_raw(L);
}

GAME_CONFIG_GETTER("blue_white_scale_text", lua_index_raw, lua_kernel_base) {
	(void)k;
	lua_pushstring(L, "blue_white_scale_text");
	push_color_palette(L, game_config::blue_white_scale_text);
	return lua_index_raw(L);
}
}

/**
 * Gets some game_config data (__index metamethod).
 * - Arg 1: userdata (ignored).
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
int lua_kernel_base::impl_game_config_get(lua_State* L)
{
	return gameConfigReg().get(L);
}
/**
 * Sets some game_config data (__newindex metamethod).
 * - Arg 1: userdata (ignored).
 * - Arg 2: string containing the name of the property.
 * - Arg 3: something containing the attribute.
 */
int lua_kernel_base::impl_game_config_set(lua_State* L)
{
	return gameConfigReg().set(L);
}
/**
 * Gets a list of game_config data (__dir metamethod).
 */
int lua_kernel_base::impl_game_config_dir(lua_State* L)
{
	return gameConfigReg().dir(L);
}
/**
 * Loads the "package" package into the Lua environment.
 * This action is inherently unsafe, as Lua scripts will now be able to
 * load C libraries on their own, hence granting them the same privileges
 * as the Wesnoth binary itself.
 */
void lua_kernel_base::load_package()
{
	lua_State *L = mState;
	lua_pushcfunction(L, luaopen_package);
	lua_pushstring(L, "package");
	lua_call(L, 1, 0);
}

void lua_kernel_base::load_core()
{
	lua_State* L = mState;
	lua_settop(L, 0);
	cmd_log_ << "Loading core...\n";
	luaW_getglobal(L, "wesnoth", "require");
	lua_pushstring(L, "lua/core");
	if(!protected_call(1, 1)) {
		cmd_log_ << "Error: Failed to load core.\n";
	}
	lua_settop(L, 0);
}

/**
 * Gets all the global variable names in the Lua environment. This is useful for tab completion.
 */
std::vector<std::string> lua_kernel_base::get_global_var_names()
{
	std::vector<std::string> ret;

	lua_State *L = mState;

	int idx = lua_gettop(L);
	lua_getglobal(L, "_G");
	lua_pushnil(L);

	while (lua_next(L, idx+1) != 0) {
		if (lua_isstring(L, -2)) {
			ret.push_back(lua_tostring(L,-2));
		}
		lua_pop(L,1);
	}
	lua_settop(L, idx);
	return ret;
}

/**
 * Gets all attribute names of an extended variable name. This is useful for tab completion.
 */
std::vector<std::string> lua_kernel_base::get_attribute_names(const std::string & input)
{
	std::vector<std::string> ret;
	std::string base_path = input;
	std::size_t last_dot = base_path.find_last_of('.');
	std::string partial_name = base_path.substr(last_dot + 1);
	base_path.erase(last_dot);
	std::string load = "return " + base_path;

	lua_State* L = mState;
	int save_stack = lua_gettop(L);
	int result = luaL_loadstring(L, load.c_str());
	if(result != LUA_OK) {
		// This isn't at error level because it's a really low priority error; it just means the user tried to tab-complete something that doesn't exist.
		LOG_LUA << "Error when attempting tab completion:";
		LOG_LUA << luaL_checkstring(L, -1);
		// Just return an empty list; no matches were found
		lua_settop(L, save_stack);
		return ret;
	}

	luaW_pcall(L, 0, 1);
	if(lua_istable(L, -1) || lua_isuserdata(L, -1)) {
		int top = lua_gettop(L);
		int obj = lua_absindex(L, -1);
		if(luaL_getmetafield(L, obj, "__tab_enum") == LUA_TFUNCTION) {
			lua_pushvalue(L, obj);
			lua_pushlstring(L, partial_name.c_str(), partial_name.size());
			luaW_pcall(L, 2, 1);
			ret = lua_check<std::vector<std::string>>(L, -1);
		} else if(lua_type(L, -1) != LUA_TTABLE) {
			LOG_LUA << "Userdata missing __tab_enum meta-function for tab completion";
			lua_settop(L, save_stack);
			return ret;
		} else {
			lua_settop(L, top);
			// Metafunction not found, so use lua_next to enumerate the table
			for(lua_pushnil(L); lua_next(L, obj); lua_pop(L, 1)) {
				if(lua_type(L, -2) == LUA_TSTRING) {
					std::string attr = lua_tostring(L, -2);
					if(attr.empty()) {
						continue;
					}
					if(!isalpha(attr[0]) && attr[0] != '_') {
						continue;
					}
					if(std::any_of(attr.begin(), attr.end(), [](char c){
						return !isalpha(c) && !isdigit(c) && c != '_';
					})) {
						continue;
					}
					if(attr.substr(0, partial_name.size()) == partial_name) {
						ret.push_back(base_path + "." + attr);
					}
				}
			}
		}
	}
	lua_settop(L, save_stack);
	return ret;
}

lua_kernel_base*& lua_kernel_base::get_lua_kernel_base_ptr(lua_State *L)
{
	#ifdef __GNUC__
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wold-style-cast"
	#endif
	return *reinterpret_cast<lua_kernel_base**>(lua_getextraspace(L));
	#ifdef __GNUC__
		#pragma GCC diagnostic pop
	#endif
}

uint32_t lua_kernel_base::get_random_seed()
{
	return seed_rng::next_seed();
}
