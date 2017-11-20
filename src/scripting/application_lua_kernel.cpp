/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Provides a Lua interpreter, to drive the game_controller.
 *
 * @note Naming conventions:
 *   - intf_ functions are exported in the wesnoth domain,
 *   - impl_ functions are hidden inside metatables,
 *   - cfun_ functions are closures,
 *   - luaW_ functions are helpers in Lua style.
 */

#include "scripting/application_lua_kernel.hpp"

#include "config.hpp"
#include "game_errors.hpp"
#include "log.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_cpp_function.hpp"
#include "scripting/lua_fileops.hpp"
#include "scripting/lua_kernel_base.hpp"
#include "scripting/lua_preferences.hpp"
#include "scripting/plugins/context.hpp"
#include "scripting/plugins/manager.hpp"

#ifdef DEBUG_LUA
#include "scripting/debug_lua.hpp"
#endif

#include <map>
#include <sstream>
#include <string>
#include <utility>

#include "utils/functional.hpp"
#include <boost/range/adaptors.hpp>
#include <SDL.h>

#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/luaconf.h"

struct lua_State;

static lg::log_domain log_scripting_lua("scripting/lua");
#define DBG_LUA LOG_STREAM(debug, log_scripting_lua)
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

static int intf_describe_plugins(lua_State * L)
{
	std::cerr << "describe plugins (" << plugins_manager::get()->size() << "):\n";
	lua_getglobal(L, "print");
	for (size_t i = 0; i < plugins_manager::get()->size(); ++i) {
		lua_pushvalue(L,-1); //duplicate the print

		std::stringstream line;
		line << i
		     << ":\t"
		     << plugins_manager::get()->get_status(i)
		     << "\t\t"
		     << plugins_manager::get()->get_name(i)
		     << "\n";

		DBG_LUA << line.str();

		lua_pushstring(L, line.str().c_str());
		lua_call(L, 1, 0);
	}
	if (!plugins_manager::get()->size()) {
		lua_pushstring(L, "No plugins available.\n");
		lua_call(L, 1, 0);
	}
	return 0;
}

static int intf_delay(lua_State* L)
{
	unsigned int delay = static_cast<unsigned int>(luaL_checkinteger(L, 1));
	SDL_Delay(delay);
	return 0;
}

application_lua_kernel::application_lua_kernel()
 : lua_kernel_base()
{
	lua_getglobal(mState, "wesnoth");
	lua_pushcfunction(mState, intf_delay);
	lua_setfield(mState, -2, "delay");

	lua_settop(mState, 0);

	lua_pushcfunction(mState, &intf_describe_plugins);
	lua_setglobal(mState, "describe_plugins");
	lua_settop(mState, 0);

	// Create the preferences table.
	cmd_log_ << lua_preferences::register_table(mState);
}

application_lua_kernel::thread::thread(lua_State * T) : T_(T), started_(false) {}

std::string application_lua_kernel::thread::status()
{
	if (!started_) {
		if (lua_status(T_) == LUA_OK) {
			return "not started";
		} else {
			return "load error";
		}
	}
	switch (lua_status(T_)) {
		case LUA_OK:
			return "dead";
		case LUA_YIELD:
			return "started";
		default:
			return "error";
	}
}

bool application_lua_kernel::thread::is_running() {
	return started_ ? (lua_status(T_) == LUA_YIELD) : (lua_status(T_) == LUA_OK);
}

static char * v_threadtableKey = 0;
static void * const threadtableKey = static_cast<void *> (& v_threadtableKey);

static lua_State * get_new_thread(lua_State * L)
{
	lua_pushlightuserdata(L	, threadtableKey);
	lua_pushvalue(L,1);				// duplicate script key, since we need to store later
							// stack is now [script key] [script key]

	lua_rawget(L, LUA_REGISTRYINDEX);		// get the script table from the registry, on the top of the stack
	if (!lua_istable(L,-1)) {			// if it doesn't exist create it
		lua_pop(L,1);
		lua_newtable(L);
	}						// stack is now [script key] [table]

	lua_pushinteger(L, lua_rawlen(L, -1) + 1);	// push #table + 1 onto the stack

	lua_State * T = lua_newthread(L);		// create new thread T
							// stack is now [script key] [table] [#table + 1] [thread]
	lua_rawset(L, -3);				// store the new thread at #table +1 index of the table.
							// stack is now [script key] [table]
	lua_rawset(L, LUA_REGISTRYINDEX);
							// stack L is now empty
	return T;					// now we can set up T's stack appropriately
}

application_lua_kernel::thread * application_lua_kernel::load_script_from_string(const std::string & prog)
{
	lua_State * T = get_new_thread(mState);
	// now we are operating on T's stack, leaving a compiled C function on it.

	DBG_LUA << "created thread: status = " << lua_status(T) << (lua_status(T) == LUA_OK ? " == OK" : " == ?") << "\n";
	DBG_LUA << "loading script from string:\n<<\n" << prog << "\n>>\n";

	int errcode = luaL_loadstring(T, prog.c_str());
	if (errcode != LUA_OK) {
		const char * err_str = lua_tostring(T, -1);
		std::string msg = err_str ? err_str : "null string";

		std::string context = "When parsing a string to a lua thread, ";

		if (errcode == LUA_ERRSYNTAX) {
			context += " a syntax error";
		} else if(errcode == LUA_ERRMEM){
			context += " a memory error";
		} else if(errcode == LUA_ERRGCMM) {
			context += " an error in garbage collection metamethod";
		} else {
			context += " an unknown error";
		}

		throw game::lua_error(msg, context);
	}
	if (!lua_kernel_base::protected_call(T, 0, 1, std::bind(&lua_kernel_base::log_error, this, _1, _2))) {
		throw game::lua_error("Error when executing a script to make a lua thread.");
	}
	if (!lua_isfunction(T, -1)) {
		throw game::lua_error(std::string("Error when executing a script to make a lua thread -- function was not produced, found a ") + lua_typename(T, lua_type(T, -1)) );
	}

	return new application_lua_kernel::thread(T);
}

application_lua_kernel::thread * application_lua_kernel::load_script_from_file(const std::string & file)
{
	lua_State * T = get_new_thread(mState);
	// now we are operating on T's stack, leaving a compiled C function on it.

	lua_pushstring(T, file.c_str());
	lua_fileops::load_file(T);
	if (!lua_kernel_base::protected_call(T, 0, 1, std::bind(&lua_kernel_base::log_error, this, _1, _2))) {
		throw game::lua_error("Error when executing a file to make a lua thread.");
	}
	if (!lua_isfunction(T, -1)) {
		throw game::lua_error(std::string("Error when executing a file to make a lua thread -- function was not produced, found a ") + lua_typename(T, lua_type(T, -1)) );
	}

	return new application_lua_kernel::thread(T);
}

struct lua_context_backend {
	std::vector<plugins_manager::event> requests;
	bool valid;

	lua_context_backend()
		: requests()
		, valid(true)
	{}
};

static int impl_context_backend(lua_State * L, std::shared_ptr<lua_context_backend> backend, std::string req_name)
{
	if (!backend->valid) {
		luaL_error(L , "Error, you tried to use an invalid context object in a lua thread");
	}

	plugins_manager::event evt;
	evt.name = req_name;
	evt.data = luaW_checkconfig(L, -1);

	backend->requests.push_back(evt);
	return 0;
}

static int impl_context_accessor(lua_State * L, std::shared_ptr<lua_context_backend> backend, plugins_context::accessor_function func)
{
	if (!backend->valid) {
		luaL_error(L , "Error, you tried to use an invalid context object in a lua thread");
	}

	if(lua_gettop(L)) {
		config temp;
		if(!luaW_toconfig(L, 1, temp)) {
			luaL_argerror(L, 1, "Error, tried to parse a config but some fields were invalid");
		}
		luaW_pushconfig(L, func(temp));
		return 1;
	} else {
		luaW_pushconfig(L, func(config()));
		return 1;
	}
}

application_lua_kernel::request_list application_lua_kernel::thread::run_script(const plugins_context & ctxt, const std::vector<plugins_manager::event> & queue)
{
	// There are two possibilities: (1) this is the first execution, and the C function is the only thing on the stack
	// (2) this is a subsequent execution, and there is nothing on the stack.
	// Either way we push the arguments to the function and call resume.

	// First we have to create the event table, by concatenating the event queue into a table.
	lua_newtable(T_); //this will be the event table
	for (size_t i = 0; i < queue.size(); ++i) {
		lua_newtable(T_);
		lua_pushstring(T_, queue[i].name.c_str());
		lua_rawseti(T_, -2, 1);
		luaW_pushconfig(T_, queue[i].data);
		lua_rawseti(T_, -2, 2);
		lua_rawseti(T_, -2, i+1);
	}

	// Now we have to create the context object. It is arranged as a table of boost functions.
	std::shared_ptr<lua_context_backend> this_context_backend = std::make_shared<lua_context_backend> (lua_context_backend());
	lua_newtable(T_); // this will be the context table
	for (const std::string & key : ctxt.callbacks_ | boost::adaptors::map_keys ) {
		lua_pushstring(T_, key.c_str());
		lua_cpp::push_function(T_, std::bind(&impl_context_backend, _1, this_context_backend, key));
		lua_settable(T_, -3);
	}

	// Now we have to create the info object (context accessors). It is arranged as a table of boost functions.
	lua_newtable(T_); // this will be the info table
	lua_pushstring(T_, "name");
	lua_pushstring(T_, ctxt.name_.c_str());
	lua_settable(T_, -3);
	for (const plugins_context::accessor_list::value_type & v : ctxt.accessors_) {
		const std::string & key = v.first;
		const plugins_context::accessor_function & func = v.second;
		lua_pushstring(T_, key.c_str());
		lua_cpp::push_function(T_, std::bind(&impl_context_accessor, _1, this_context_backend, func));
		lua_settable(T_, -3);
	}

	// Now we resume the function, calling the coroutine with the three arguments (events, context, info).
	lua_resume(T_, nullptr, 3);

	started_ = true;

	this_context_backend->valid = false; //invalidate the context object for lua

	if (lua_status(T_) != LUA_YIELD) {
		LOG_LUA << "Thread status = '" << lua_status(T_) << "'\n";
		if (lua_status(T_) != LUA_OK) {
			std::stringstream ss;
			ss << "encountered a";
			switch(lua_status(T_)) {
				case LUA_ERRSYNTAX:
					ss << " syntax ";
					break;
				case LUA_ERRRUN:
					ss << " runtime ";
					break;
				case LUA_ERRERR:
					ss << " error-handler ";
					break;
				default:
					ss << " ";
					break;
			}
			ss << "error:\n" << lua_tostring(T_, -1) << "\n";
			ERR_LUA << ss.str() << std::endl;
		}
	}

	application_lua_kernel::request_list results;

	for (const plugins_manager::event & req : this_context_backend->requests) {
		results.push_back(std::bind(ctxt.callbacks_.find(req.name)->second, req.data));
		//results.emplace_back(ctxt.callbacks_.find(req.name)->second, req.data);
	}
	return results;
}
