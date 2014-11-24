/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "scripting/lua_kernel_base.hpp"

#include "global.hpp"

#include "exceptions.hpp"
#include "game_errors.hpp"
#include "log.hpp"
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua_jailbreak_exception.hpp"  // for tlua_jailbreak_exception

#ifdef DEBUG_LUA
#include "scripting/debug_lua.hpp"
#endif

#include "scripting/lua_common.hpp"
#include "scripting/lua_cpp_function.hpp"
#include "scripting/lua_fileops.hpp"
#include "scripting/lua_gui2.hpp"
#include "scripting/lua_map_location_ops.hpp"
#include "scripting/lua_rng.hpp"
#include "scripting/lua_types.hpp"

#include "version.hpp"                  // for do_version_check, etc

#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>

#include <cstring>
#include <exception>
#include <new>
#include <string>
#include <sstream>
#include <vector>

static lg::log_domain log_scripting_lua("scripting/lua");
#define DBG_LUA LOG_STREAM(debug, log_scripting_lua)
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

// Callback implementations

/**
 * Compares 2 version strings - which is newer.
 * - Args 1,3: version strings
 * - Arg 2: comparison operator (string)
 * - Ret 1: comparison result
 */
static int intf_compare_versions(lua_State* L)
{
	char const *v1 = luaL_checkstring(L, 1);

	const VERSION_COMP_OP vop = parse_version_op(luaL_checkstring(L, 2));
	if(vop == OP_INVALID) return luaL_argerror(L, 2, "unknown version comparison operator - allowed are ==, !=, <, <=, > and >=");

	char const *v2 = luaL_checkstring(L, 3);

	const bool result = do_version_check(version_info(v1), vop, version_info(v2));
	lua_pushboolean(L, result);

	return 1;
}

/**
 * Replacement print function -- instead of printing to std::cout, print to the command log.
 * Intended to be bound to this' command_log at registration time.
 */
int lua_kernel_base::intf_print(lua_State* L)
{
	DBG_LUA << "intf_print called:\n";
	size_t nargs = lua_gettop(L);

	for (size_t i = 1; i <= nargs; ++i) {
		cmd_log_ << lua_tostring(L,i);
		DBG_LUA << "'" << lua_tostring(L,i) << "'\n";
	}

	cmd_log_ << "\n";
	DBG_LUA << "\n";

	return 0;
}

// The show-dialog call back is here implemented as a method of lua kernel, since it needs a pointer to external object CVideo
int lua_kernel_base::intf_show_dialog(lua_State *L)
{
	if (!video_) {
		ERR_LUA << "Cannot show dialog, no video object is available to this lua kernel.";
		lua_error(L);
		return 0;
	}

	return lua_gui2::show_dialog(L, *video_);
}


// End Callback implementations

lua_kernel_base::lua_kernel_base(CVideo * video)
 : mState(luaL_newstate())
 , video_(video)
 , cmd_log_()
{
	lua_State *L = mState;

	cmd_log_ << "Initializing " << my_name() << "...\n";

	// Open safe libraries.
	// Debug and OS are not, but most of their functions will be disabled below.
	cmd_log_ << "Adding standard libs...\n";

	static const luaL_Reg safe_libs[] = {
		{ "",       luaopen_base   },
		{ "table",  luaopen_table  },
		{ "string", luaopen_string },
		{ "math",   luaopen_math   },
		{ "debug",  luaopen_debug  },
		{ "os",     luaopen_os     },
		{ NULL, NULL }
	};
	for (luaL_Reg const *lib = safe_libs; lib->func; ++lib)
	{
		luaL_requiref(L, lib->name, lib->func, 1);
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

	// Disable functions from debug which we don't want.
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

	// Delete dofile and loadfile.
	lua_pushnil(L);
	lua_setglobal(L, "dofile");
	lua_pushnil(L);
	lua_setglobal(L, "loadfile");

	// Store the error handler.
	cmd_log_ << "Adding error handler...\n";

	lua_pushlightuserdata(L
			, executeKey);
	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");
	lua_remove(L, -2);
	lua_rawset(L, LUA_REGISTRYINDEX);
	lua_pop(L, 1);

	// Create the gettext metatable.
	cmd_log_ << "Adding gettext metatable...\n";

	lua_pushlightuserdata(L
			, gettextKey);
	lua_createtable(L, 0, 2);
	lua_pushcfunction(L, lua_common::impl_gettext);
	lua_setfield(L, -2, "__call");
	lua_pushstring(L, "message domain");
	lua_setfield(L, -2, "__metatable");
	lua_rawset(L, LUA_REGISTRYINDEX);

	// Create the tstring metatable.
	cmd_log_ << "Adding tstring metatable...\n";

	lua_pushlightuserdata(L
			, tstringKey);
	lua_createtable(L, 0, 4);
	lua_pushcfunction(L, lua_common::impl_tstring_concat);
	lua_setfield(L, -2, "__concat");
	lua_pushcfunction(L, lua_common::impl_tstring_collect);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, lua_common::impl_tstring_tostring);
	lua_setfield(L, -2, "__tostring");
	lua_pushstring(L, "translatable string");
	lua_setfield(L, -2, "__metatable");
	lua_rawset(L, LUA_REGISTRYINDEX);

	lua_settop(L, 0);

	// Define the CPP_function metatable ( so we can override print to point to a C++ member function, add "show_dialog" for this kernel, etc. )
	cmd_log_ << "Adding boost function proxy...\n";

	lua_cpp::register_metatable(L);

	// Add some callback from the wesnoth lib
	cmd_log_ << "Registering basic wesnoth API...\n";

	static luaL_Reg const callbacks[] = {
		{ "compare_versions",         &intf_compare_versions         		},
		{ "dofile",                   &lua_fileops::intf_dofile                 },
		{ "have_file",                &lua_fileops::intf_have_file              },
		{ "require",                  &lua_fileops::intf_require                },
		{ "textdomain",               &lua_common::intf_textdomain   		},
		{ "tovconfig",                &lua_common::intf_tovconfig		},
		{ "get_dialog_value",         &lua_gui2::intf_get_dialog_value		},
		{ "set_dialog_active",        &lua_gui2::intf_set_dialog_active		},
		{ "set_dialog_callback",      &lua_gui2::intf_set_dialog_callback	},
		{ "set_dialog_canvas",        &lua_gui2::intf_set_dialog_canvas		},
		{ "set_dialog_markup",        &lua_gui2::intf_set_dialog_markup		},
		{ "set_dialog_value",         &lua_gui2::intf_set_dialog_value		},
		{ NULL, NULL }
	};

	lua_getglobal(L, "wesnoth");
	if (!lua_istable(L,-1)) {
		lua_newtable(L);
	}
	luaL_setfuncs(L, callbacks, 0);
	lua_setglobal(L, "wesnoth");

	// Override the print function
	cmd_log_ << "Redirecting print function...\n";

	lua_getglobal(L, "print");
	lua_setglobal(L, "std_print"); //storing original impl as 'std_print'
	lua_settop(L, 0); //clear stack, just to be sure

	lua_cpp::lua_function my_print = boost::bind(&lua_kernel_base::intf_print, this, _1);
	lua_cpp::push_function(L, my_print);
	lua_setglobal(L, "print");

	// Add the show_dialog function
	cmd_log_ << "Adding dialog support...\n";

	lua_getglobal(L, "wesnoth");
	lua_cpp::lua_function show_dialog = boost::bind(&lua_kernel_base::intf_show_dialog, this, _1);
	lua_cpp::push_function(L, show_dialog);
	lua_setfield(L, -2, "show_dialog");
	lua_pop(L,1);

	// Create the package table.
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	lua_setfield(L, -2, "package");
	lua_pop(L, 1);

	// Get some callbacks for map locations
	cmd_log_ << "Adding map_location table...\n";

	static luaL_Reg const map_callbacks[] = {
		{ "get_direction",		&lua_map_location::intf_get_direction         		},
		{ "vector_sum",			&lua_map_location::intf_vector_sum			},
		{ "vector_negation",		&lua_map_location::intf_vector_negation			},
		{ "zero",			&lua_map_location::intf_vector_zero			},
		{ "rotate_right_around_center",	&lua_map_location::intf_rotate_right_around_center	},
		{ "tiles_adjacent",		&lua_map_location::intf_tiles_adjacent			},
		{ "get_adjacent_tiles",		&lua_map_location::intf_get_adjacent_tiles		},
		{ "distance_between",		&lua_map_location::intf_distance_between		},
		{ "get_in_basis_N_NE",		&lua_map_location::intf_get_in_basis_N_NE		},
		{ "get_relative_dir",		&lua_map_location::intf_get_relative_dir		},
		{ "parse_direction",		&lua_map_location::intf_parse_direction			},
		{ "write_direction",		&lua_map_location::intf_write_direction			},
		{ NULL, NULL }
	};

	// Create the map_location table.
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	luaL_setfuncs(L, map_callbacks, 0);
	lua_setfield(L, -2, "map_location");
	lua_pop(L,1);

	// Add mersenne twister rng wrapper
	cmd_log_ << "Adding rng tables...\n";
	lua_rng::load_tables(L);

	// Loading ilua:
	cmd_log_ << "Loading ilua...\n";

	lua_pushstring(L, "lua/ilua.lua");
	int result = lua_fileops::intf_require(L);
	if (result == 1) {
		//run "ilua.set_strict()"
		lua_pushstring(L, "set_strict");
		lua_gettable(L, -2);
		if (!protected_call(0,0, boost::bind(&lua_kernel_base::log_error, this, _1, _2))) {
			cmd_log_ << "Failed to activate strict mode.\n";
		} else {
			cmd_log_ << "Activated strict mode.\n";
		}

		lua_setglobal(L, "ilua"); //save ilua table as a global
	} else {
		cmd_log_ << "Error: failed to load ilua.\n";
	}
	lua_settop(L, 0);
}

lua_kernel_base::~lua_kernel_base()
{
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
	error_handler eh = boost::bind(&lua_kernel_base::log_error, this, _1, _2 );
	return protected_call(nArgs, nRets, eh);
}

bool lua_kernel_base::load_string(char const * prog)
{
	error_handler eh = boost::bind(&lua_kernel_base::log_error, this, _1, _2 );
	return load_string(prog, eh);
}

bool lua_kernel_base::protected_call(int nArgs, int nRets, error_handler e_h)
{
	lua_State *L = mState;

	// Load the error handler before the function and its arguments.
	lua_pushlightuserdata(L
			, executeKey);

	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_insert(L, -2 - nArgs);

	int error_handler_index = lua_gettop(L) - nArgs - 1;

	// Call the function.
	int errcode = lua_pcall(L, nArgs, nRets, -2 - nArgs);
	tlua_jailbreak_exception::rethrow();

	// Remove the error handler.
	lua_remove(L, error_handler_index);

	if (errcode != LUA_OK) {
		std::string message = lua_tostring(L, -1);

		std::string context = "When executing, ";
		if (errcode == LUA_ERRRUN) {
			context += "Lua runtime error";
		} else if (errcode == LUA_ERRERR) {
			context += "Lua error in attached debugger";
		} else if (errcode == LUA_ERRMEM) {
			context += "Lua out of memory error";
		} else if (errcode == LUA_ERRGCMM) {
			context += "Lua error in garbage collection metamethod";
		} else {
			context += "unknown lua error";
		}

		lua_pop(mState, 1);

		e_h(message.c_str(), context.c_str());

		return false;
	}

	return true;
}

bool lua_kernel_base::load_string(char const * prog, error_handler e_h)
{
	int errcode = luaL_loadstring(mState, prog);
	if (errcode != LUA_OK) {
		std::string msg = lua_tostring(mState, -1);

		std::string context = "When parsing a string to lua, ";

		if (errcode == LUA_ERRSYNTAX) {
			context += " a syntax error";
		} else if(errcode == LUA_ERRMEM){
			context += " a memory error";
		} else if(errcode == LUA_ERRGCMM) {
			context += " an error in garbage collection metamethod";
		} else {
			context += " an unknown error";
		}

		lua_pop(mState, 1);

		e_h(msg.c_str(), context.c_str());

		return false;
	}
	return true;
}

// Call load_string and protected call. Make them throw exceptions.
// 
void lua_kernel_base::throwing_run(const char * prog) {
	cmd_log_ << "$ " << prog << "\n";
	error_handler eh = boost::bind(&lua_kernel_base::throw_exception, this, _1, _2 );
	load_string(prog, eh);
	protected_call(0, 0, eh);
}

// Do a throwing run, but if we catch a lua_error, reformat it with signature for this function and log it.
void lua_kernel_base::run(const char * prog) {
	try {
		throwing_run(prog);
	} catch (game::lua_error & e) {
		cmd_log_ << e.what() << "\n";
		lua_kernel_base::log_error(e.what(), "In function lua_kernel::run()");
	}
}

// Tests if a program resolves to an expression, and pretty prints it if it is, otherwise it runs it normally. Throws exceptions.
void lua_kernel_base::interactive_run(char const * prog) {
	std::string experiment = "ilua._pretty_print(";
	experiment += prog;
	experiment += ")";

	error_handler eh = boost::bind(&lua_kernel_base::throw_exception, this, _1, _2 );

	try {
		// Try to load the experiment without syntax errors
		load_string(experiment.c_str(), eh);
	} catch (game::lua_error & e) {
		throwing_run(prog);	// Since it failed, fall back to the usual throwing_run, on the original input.
		return;
	}
	// experiment succeeded, now run but log normally.
	cmd_log_ << "$ " << prog << "\n";
	protected_call(0, 0, eh);
}

/**
 * Loads the "package" package into the Lua environment.
 * This action is inherently unsafe, as Lua scripts will now be able to
 * load C libraries on their own, hence granting them the same privileges
 * as the Wesnoth binary itsef.
 */
void lua_kernel_base::load_package()
{
	lua_State *L = mState;
	lua_pushcfunction(L, luaopen_package);
	lua_pushstring(L, "package");
	lua_call(L, 1, 0);
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
	std::string var_path = input; // it's convenient to make a copy, even if it's a little slower

	lua_State *L = mState;

	int base = lua_gettop(L);
	lua_getglobal(L, "_G");

	size_t idx = var_path.find('.');
	size_t last_dot = 0;
	while (idx != std::string::npos ) {
		last_dot += idx + 1; // Since idx was not npos, add it to the "last_dot" idx, so that last_dot keeps track of indices in input string
		lua_pushstring(L, var_path.substr(0, idx).c_str()); //push the part of the path up to the period
		lua_rawget(L, -2);

		if (!lua_istable(L,-1) && !lua_isuserdata(L,-1)) {
			lua_settop(L, base);
			return ret; //if we didn't get a table or userdata we can't proceed
		}

		var_path = var_path.substr(idx+1); // chop off the part of the path we just dereferenced		
		idx = var_path.find('.'); // find the next .
	}
	
	std::string prefix = input.substr(0, last_dot);

	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		if (lua_isstring(L, -2)) {
			ret.push_back(prefix + lua_tostring(L,-2));
		}
		lua_pop(L,1);
	}
	lua_settop(L, base);
	return ret;
}
