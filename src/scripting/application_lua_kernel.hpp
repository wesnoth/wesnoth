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

#pragma once

#include "scripting/lua_kernel_base.hpp"
#include "scripting/plugins/context.hpp"
#include "scripting/plugins/manager.hpp"

#include <string>
#include <vector>

struct lua_State;

class application_lua_kernel : public lua_kernel_base {
public:
	application_lua_kernel();

	virtual std::string my_name() { return "Application Lua Kernel"; }

	typedef std::vector<std::function<bool(void)> > request_list;

	class thread {
		lua_State * T_;
		bool started_;

		thread(const thread&) = delete;
		thread& operator=(const thread&) = delete;

		thread(lua_State *);
	public :
		bool is_running();
		std::string status();

		request_list run_script(const plugins_context & ctxt, const std::vector<plugins_manager::event> & queue);

		friend class application_lua_kernel;
	};

	thread * load_script_from_string(const std::string &);	//throws
	thread * load_script_from_file(const std::string &);	//throws
};
