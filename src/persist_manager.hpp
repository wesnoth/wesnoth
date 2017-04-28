/*
   Copyright (C) 2010 - 2017 by Jody Northup
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PERSIST_MANAGER_HPP_INCLUDED
#define PERSIST_MANAGER_HPP_INCLUDED

#include <map>
#include <string>

class persist_context;

class persist_manager {
	protected:
		typedef std::map<std::string,persist_context *> context_map;

		bool in_transaction_;
		context_map contexts_;
	public:
		bool start_transaction();
		bool end_transaction();
		bool cancel_transaction();

		persist_manager();
		virtual ~persist_manager();

		persist_context &get_context(const std::string &ns);
};

#endif
