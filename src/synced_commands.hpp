/*
   Copyright (C) 2014 - 2018 by David White <dave@whitevine.net>
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

#include <map>
#include <exception>

#include "utils/functional.hpp"

class config;

class synced_command {
	public:
		/*
			the parameters or error handlers are
			1) the message of the error
			2) a boolean that indicates whether the error is heavy enough to make proceeding impossible.
			TODO: remove the second argument because it isn't used.

		*/
		typedef std::function<void(const std::string&, bool)> error_handler_function;
		/*
			returns: true if the action succeeded correctly,

		*/
		typedef bool (*handler)(const config &, bool use_undo, bool show, error_handler_function error_handler);
		typedef std::map<std::string, handler> map;


		synced_command(const std::string & tag, handler function);

		/// using static function variable instead of static member variable to prevent static initialization fiasco when used in other files.
		static map& registry();
	};

/*
	this is currently only used in "synced_commands.cpp" and there is no reason to use it anywhere else.
	but if you have a good reason feel free to do so.
*/

#define SYNCED_COMMAND_HANDLER_FUNCTION(pname, pcfg, use_undo, show, error_handler) \
	static bool synced_command_func_##pname(const config & pcfg, bool use_undo, bool show, synced_command::error_handler_function error_handler ); \
	static synced_command synced_command_action_##pname(#pname, &synced_command_func_##pname);  \
	static bool synced_command_func_##pname(const config & pcfg, bool use_undo, bool show, synced_command::error_handler_function error_handler)
