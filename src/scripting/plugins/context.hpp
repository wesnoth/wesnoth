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

/**
 * Manages the availability of wesnoth callbacks to plug-ins while the
 * application is context switching.
 */

#pragma once

#include <functional>

#include <map>
#include <string>
#include <vector>

class config;

class plugins_context {

public:
	typedef std::function<bool(config)> callback_function;
	struct Reg { char const * name; callback_function func; };

	typedef std::function<config(config)> accessor_function;
	struct aReg { char const * name; accessor_function func; };

	using reg_vec = std::vector<Reg>;
	using areg_vec = std::vector<aReg>;

	plugins_context( const std::string & name );
	plugins_context( const std::string & name, const reg_vec& callbacks, const areg_vec& accessors);

	void play_slice();

	void set_callback(const std::string & name, callback_function);
	void set_callback(const std::string & name, const std::function<void(config)>& function, bool preserves_context);
	void set_callback_execute(class lua_kernel_base& kernel);
	std::size_t erase_callback(const std::string & name);
	std::size_t clear_callbacks();

	void set_accessor(const std::string & name, accessor_function);
	void set_accessor_string(const std::string & name, const std::function<std::string(config)>&);	//helpers which create a config from a simple type
	void set_accessor_int(const std::string & name, const std::function<int(config)>&);
	void set_accessor_bool(const std::string & name, const std::function<bool(config)>&);
	std::size_t erase_accessor(const std::string & name);
	std::size_t clear_accessors();

	friend class application_lua_kernel;

private:
	typedef std::map<std::string, callback_function > callback_list;
	typedef std::map<std::string, accessor_function > accessor_list;

	void initialize(const reg_vec& callbacks, const areg_vec& accessors);

	callback_list callbacks_;
	accessor_list accessors_;
	std::string name_;
	lua_kernel_base* execute_kernel_;
};
