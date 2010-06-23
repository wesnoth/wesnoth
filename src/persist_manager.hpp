/* $Id$ */
/*
   Copyright (C) 2010 by Jody Northup
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "config.hpp"
#include "persist_context.hpp"

class persist_manager {
	protected:
		typedef std::map<std::string,persist_context *> context_map;

		bool in_transaction_;
		context_map contexts_;
	public:
		persist_manager() : in_transaction_(false),contexts_() {}
		~persist_manager() {
			for (context_map::iterator i = contexts_.begin(); i != contexts_.end(); i++)
				delete (i->second);
		}
		persist_context &get_context(const std::string &ns);
		
		//Allow for derived types to save/load from other types of storage
		virtual bool save_data(const std::string &name_space, config &cfg); 
		virtual bool load_data(const std::string &name_space, config &cfg, const bool create_if_missing = true);

		//TODO - Transactions
		bool start_transaction(); 
		bool end_transaction();
		bool cancel_transaction();
};