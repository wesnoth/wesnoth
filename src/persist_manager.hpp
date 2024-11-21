/*
	Copyright (C) 2010 - 2024
	by Jody Northup
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include <memory>
#include <string>

class persist_context;

class persist_manager {
	protected:
		typedef std::map<std::string, std::unique_ptr<persist_context>> context_map;

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
