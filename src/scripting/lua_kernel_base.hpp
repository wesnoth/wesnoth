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

#ifndef SCRIPTING_LUA_KERNEL_BASE_HPP
#define SCRIPTING_LUA_KERNEL_BASE_HPP

#include <sstream>
#include <string>
#include <vector>
#include "utils/functional.hpp"
#include <cstdint>

struct lua_State;
class CVideo;
class config;

class lua_kernel_base {
public:
	lua_kernel_base();
	virtual ~lua_kernel_base();

	/** Runs a [lua] tag. Doesn't throw lua_error.*/
	void run_lua_tag(const config& cfg);

	/** Runs a plain script. Doesn't throw lua_error.*/
	void run(char const *prog, int nArgs = 0);

	/** Runs a plain script, but reports errors by throwing lua_error.*/
	void throwing_run(char const * prog, int nArgs);

	/** Tests if a program resolves to an expression, and pretty prints it if it is, otherwise it runs it normally. Throws exceptions.*/
	void interactive_run(char const * prog);

	/** Loads the `package` library into lua environment. Only in allow in `unsafe` modes. */
	void load_package();

	/** Get tab completion strings */
	std::vector<std::string> get_global_var_names();
	std::vector<std::string> get_attribute_names(const std::string & var_path);

	/** User-visible name of the lua kernel that they are talking to */
	virtual std::string my_name() { return "Basic Lua Kernel"; }

	/** Access / manipulate logging of lua kernel activities */
	const std::stringstream & get_log() { cmd_log_.log_ << std::flush; return cmd_log_.log_; }
	void clear_log() { cmd_log_.log_.str(""); cmd_log_.log_.clear(); }

	using external_log_type = std::function<void(const std::string &)>;
	void set_external_log( external_log_type lg ) { cmd_log_.external_log_ = lg; }

	/** Error reporting mechanisms, used by virtual methods protected_call and load_string*/
	virtual void log_error(char const* msg, char const* context = "Lua error");
	virtual void throw_exception(char const* msg, char const* context = "Lua error"); //throws game::lua_error

	typedef std::function<void(char const*, char const*)> error_handler;

	template<typename T>
	static T& get_lua_kernel(lua_State *L)
	{
		return *static_cast<T*>(get_lua_kernel_base_ptr(L));
	}

	virtual uint32_t get_random_seed();
	lua_State * get_state() { return mState; }
	void add_widget_definition(const std::string& type, const std::string& id) { registered_widget_definitions_.emplace_back(type, id); }
protected:
	lua_State *mState;

	/** Log implementation */
	struct command_log {

		std::stringstream log_;
		external_log_type external_log_;

		command_log()
			: log_()
			, external_log_(nullptr)
		{}

		inline command_log & operator<< (const std::string & str) {
			log_ << str;
			if (external_log_) {
				external_log_(str);
			}
			return *this;
		}

		inline command_log & operator<< (char const* str) {
			if (str != nullptr) {
				log_ << str;
				if (external_log_) {
					external_log_(str);
				}
			}
			return *this;
		}
	};

	command_log cmd_log_;

	// Print text to the command log for this lua kernel. Used as a replacement impl for lua print.
	int intf_print(lua_State * L);

	using video_function = int (*)(lua_State*,CVideo&);
	template<video_function> friend int video_dispatch(lua_State*);
	int video_dispatch_impl(lua_State* L, video_function callback);

	// Show the interactive lua console (for debugging purposes)
	int intf_show_lua_console(lua_State * L);

	// Execute a protected call. Error handler is called in case of an error, using syntax for log_error and throw_exception above. Returns true if successful.
	bool protected_call(int nArgs, int nRets, error_handler);
	// Execute a protected call, taking a lua_State as argument. For functions pushed into the lua environment, this version should be used, or the function cannot be used by coroutines without segfaulting (since they have a different lua_State pointer). This version is called by the above version.
	static bool protected_call(lua_State * L, int nArgs, int nRets, error_handler);
	// Load a string onto the stack as a function. Returns true if successful, error handler is called if not.
	bool load_string(char const * prog, error_handler);

	virtual bool protected_call(int nArgs, int nRets); 	// select default error handler polymorphically
	virtual bool load_string(char const * prog);		// select default error handler polymorphically

	// dofile (using lua_fileops)
	int intf_dofile(lua_State * L);

	// require (using lua_fileops, protected_call)
	int intf_require(lua_State * L);
private:
	static lua_kernel_base*& get_lua_kernel_base_ptr(lua_State *L);
	std::vector<std::tuple<std::string, std::string>> registered_widget_definitions_;
};

#endif
