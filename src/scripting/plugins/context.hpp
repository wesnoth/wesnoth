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

/**
 * Manages the availability of wesnoth callbacks to plug-ins while the
 * application is context switching.
 */

#ifndef INCLUDED_PLUGINS_CONTEXT_HPP_
#define INCLUDED_PLUGINS_CONTEXT_HPP_

#include <map>
#include <string>
#include <boost/function.hpp>

class config;

class plugins_context {

public:
	typedef boost::function<bool(config)> callback_function;
	typedef struct { char const * name; callback_function func; } Reg;

	typedef boost::function<config(void)> accessor_function;
	typedef struct { char const * name; accessor_function func; } aReg;

	plugins_context( const std::string & name );
	plugins_context( const std::string & name, const Reg * callbacks, const aReg * accessors);

	void play_slice();

	void set_callback(const std::string & name, callback_function);
	void set_callback(const std::string & name, boost::function<void(config)> function, bool preserves_context);
	size_t erase_callback(const std::string & name);
	size_t clear_callbacks();

	void set_accessor(const std::string & name, accessor_function);
	size_t erase_accessor(const std::string & name);
	size_t clear_accessors();

	friend class application_lua_kernel;

private:
	typedef std::map<std::string, callback_function > callback_list;
	typedef std::map<std::string, accessor_function > accessor_list;

	callback_list callbacks_;
	accessor_list accessors_;
	std::string name_;
};

//A shim to assist in retrieving config attribute values
extern const boost::function< const std::string & ( const config & , const std::string & ) > get_str;

#endif
