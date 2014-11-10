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

#ifndef SCRIPTING_LUA_KERNEL_BASE_HPP
#define SCRIPTING_LUA_KERNEL_BASE_HPP

#include <string>                       // for string
#include "utils/boost_function_guarded.hpp"

struct lua_State;

class lua_kernel_base {
public:
	lua_kernel_base();
	virtual ~lua_kernel_base();

	/** Runs a plain script. Doesn't throw lua_error.*/
	void run(char const *prog);

	void load_package();

	virtual void log_error(char const* msg, char const* context = "Lua error");
	virtual void throw_exception(char const* msg, char const* context = "Lua error"); //throws game::lua_error

	typedef boost::function<void(char const*, char const*)> error_handler;

protected:
	lua_State *mState;

	// Execute a protected call. Error handler is called in case of an error, using syntax for log_error and throw_exception above. Returns true if successful.
	bool protected_call(int nArgs, int nRets, error_handler);
	// Load a string onto the stack as a function. Returns true if successful, error handler is called if not.
	bool load_string(char const * prog, error_handler);

	virtual bool protected_call(int nArgs, int nRets); 	// select default error handler polymorphically
	virtual bool load_string(char const * prog);		// select default error handler polymorphically
};

#endif
