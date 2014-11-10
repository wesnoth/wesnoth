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

#include "filesystem.hpp"
#include "game_errors.hpp"
#include "game_config.hpp" //for game_config::debug_lua
#include "log.hpp"
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/lualib.h"

#ifdef DEBUG_LUA
#include "scripting/debug_lua.hpp"
#endif

#include "scripting/lua_api.hpp"
#include "scripting/lua_common.hpp"

#include "version.hpp"                  // for do_version_check, etc

#include <cstring>
#include <string>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

// Callback implementations

/**
 * Checks if a file exists (not necessarily a Lua script).
 * - Arg 1: string containing the file name.
 * - Ret 1: boolean
 */
static int intf_have_file(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	std::string p = filesystem::get_wml_location(m);
	if (p.empty()) { lua_pushboolean(L, false); }
	else { lua_pushboolean(L, true); }
	return 1;
}

class lua_filestream
{
public:
	lua_filestream(const std::string& fname)
		: pistream_(filesystem::istream_file(fname))
	{

	}

	static const char * lua_read_data(lua_State * /*L*/, void *data, size_t *size)
	{
		lua_filestream* lfs = static_cast<lua_filestream*>(data);

		//int startpos = lfs->pistream_->tellg();
		lfs->pistream_->read(lfs->buff_, LUAL_BUFFERSIZE);
		//int newpos = lfs->pistream_->tellg();
		*size = lfs->pistream_->gcount();
#if 0
		ERR_LUA << "read bytes from " << startpos << " to " << newpos << " in total " *size << " from steam\n";
		ERR_LUA << "streamstate beeing "
			<< " goodbit:" << lfs->pistream_->good()
			<< " endoffile:" << lfs->pistream_->eof()
			<< " badbit:" <<  lfs->pistream_->bad()
			<< " failbit:" << lfs->pistream_->fail() << "\n";
#endif
		return lfs->buff_;
	}

	static int lua_loadfile(lua_State *L, const std::string& fname)
	{
		lua_filestream lfs(fname);
		//lua uses '@' to know that this is a file (as opposed to a something as opposed to something loaded via loadstring )
		std::string chunkname = '@' + fname;
		LOG_LUA << "starting to read from " << fname << "\n";
		return  lua_load(L, &lua_filestream::lua_read_data, &lfs, chunkname.c_str(), NULL);
	}
private:
	char buff_[LUAL_BUFFERSIZE];
	boost::scoped_ptr<std::istream> pistream_;
};

/**
 * Loads and executes a Lua file.
 * - Arg 1: string containing the file name.
 * - Ret *: values returned by executing the file body.
 */
static int intf_dofile(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	std::string p = filesystem::get_wml_location(m);
	if (p.empty())
		return luaL_argerror(L, 1, "file not found");

	lua_settop(L, 0);

#if 1
	try
	{
		if(lua_filestream::lua_loadfile(L, p))
			return lua_error(L);
	}
	catch(const std::exception & ex)
	{
		luaL_argerror(L, 1, ex.what());
	}
#else
	//oldcode to be deleted if newcode works
	if (luaL_loadfile(L, p.c_str()))
		return lua_error(L);
#endif

	lua_call(L, 0, LUA_MULTRET);
	return lua_gettop(L);
}


/**
 * Loads and executes a Lua file, if there is no corresponding entry in wesnoth.package.
 * Stores the result of the script in wesnoth.package and returns it.
 * - Arg 1: string containing the file name.
 * - Ret 1: value returned by the script.
 */
static int intf_require(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);

	// Check if there is already an entry.

	luaW_getglobal(L, "wesnoth", NULL);
	lua_pushstring(L, "package");
	lua_rawget(L, -2);
	lua_pushvalue(L, 1);
	lua_rawget(L, -2);
	if (!lua_isnil(L, -1) && !game_config::debug_lua) return 1;
	lua_pop(L, 1);


	std::string p = filesystem::get_wml_location(m);
	if (p.empty())
		return luaL_argerror(L, 1, "file not found");

	// Compile the file.

#if 1
	try
	{
		if(lua_filestream::lua_loadfile(L, p))
			throw game::error(lua_tostring(L, -1));
	}
	catch(const std::exception & ex)
	{
		chat_message("Lua error", ex.what());
		ERR_LUA << ex.what() << '\n';
		return 0;
	}
#else
	//oldcode to be deleted if newcode works

	int res = luaL_loadfile(L, p.c_str());
	if (res)
	{
		char const *m = lua_tostring(L, -1);
		chat_message("Lua error", m);
		ERR_LUA << m << '\n';
		return 0;
	}
#endif

	// Execute it.
	if (!luaW_pcall(L, 0, 1)) return 0;

	// Add the return value to the table.
	lua_pushvalue(L, 1);
	lua_pushvalue(L, -2);
	lua_settable(L, -4);
	return 1;
}


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

// End Callback implementations

lua_kernel_base::lua_kernel_base()
 : mState(luaL_newstate())
{
	lua_State *L = mState;

	// Open safe libraries.
	// Debug and OS are not, but most of their functions will be disabled below.
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
		if(strcmp(function, "traceback") == 0) continue;
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
	lua_pushlightuserdata(L
			, executeKey);
	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");
	lua_remove(L, -2);
	lua_rawset(L, LUA_REGISTRYINDEX);
	lua_pop(L, 1);

	// Create the gettext metatable.
	lua_pushlightuserdata(L
			, gettextKey);
	lua_createtable(L, 0, 2);
	lua_pushcfunction(L, lua_common::impl_gettext);
	lua_setfield(L, -2, "__call");
	lua_pushstring(L, "message domain");
	lua_setfield(L, -2, "__metatable");
	lua_rawset(L, LUA_REGISTRYINDEX);

	// Create the tstring metatable.
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

	// Add some callback from the wesnoth lib

	static luaL_Reg const callbacks[] = {
		{ "compare_versions",         &intf_compare_versions         },
		{ "dofile",                   &intf_dofile                   },
		{ "have_file",                &intf_have_file                },
		{ "require",                  &intf_require                  },
		{ "textdomain",               &lua_common::intf_textdomain   },
		{ NULL, NULL }
	};

	lua_getglobal(L, "wesnoth");
	if (!lua_istable(L,-1)) {
		lua_newtable(L);
	}
	luaL_setfuncs(L, callbacks, 0);
	lua_setglobal(L, "wesnoth");

	// Create the package table.
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	lua_setfield(L, -2, "package");
	lua_pop(L, 1);

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
			context += "Lua runtime error: ";
		} else if (errcode == LUA_ERRERR) {
			context += "Lua error in attached debugger: ";
		} else if (errcode == LUA_ERRMEM) {
			context += "Lua out of memory error: ";
		} else if (errcode == LUA_ERRGCMM) {
			context += "Lua error in garbage collection metamethod: ";
		} else {
			context += "unknown lua error: ";
		}

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
			msg += " a syntax error: ";
		} else if(errcode == LUA_ERRMEM){
			msg += " a memory error: ";
		} else if(errcode == LUA_ERRGCMM) {
			msg += " an error in garbage collection metamethod: ";
		} else {
			msg += " an unknown error: ";
		}

		e_h(msg.c_str(), context.c_str());

		return false;
	}
	return true;
}

// Call load_string and protected call. Make them throw exceptions, and if we catch one, reformat it with signature for this function and log it.
void lua_kernel_base::run(const char * prog) {
	try {
		error_handler eh = boost::bind(&lua_kernel_base::throw_exception, this, _1, _2 );
		load_string(prog, eh);
		protected_call(0, 0, eh);
	} catch (game::lua_error & e) {
		lua_kernel_base::log_error(e.what(), "In function lua_kernel::run()");
	}
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
