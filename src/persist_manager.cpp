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

#include "persist_context.hpp"
#include "persist_manager.hpp"

#include "foreach.hpp"

persist_context &persist_manager::get_context(const std::string &ns)
{
	persist_context::name_space name(ns,true);
	std::string key(name.root_);
	context_map::iterator i = contexts_.find(key);
	if (i == contexts_.end()) {
		contexts_[key] = new persist_file_context(key);
	}
	persist_context *ret = contexts_[key];
	if (ret->get_node() != ns)
		ret->set_node(name.descendants_);
	return *ret;
}

bool persist_manager::start_transaction() {
	bool result = true;
	foreach (context_map::reference ctx, contexts_){
		result &= ctx.second->start_transaction();
	}
	return result;
}

bool persist_manager::end_transaction() {
	bool result = true;
	foreach (context_map::reference ctx, contexts_){
		result &= ctx.second->end_transaction();
	}
	return result;
}

bool persist_manager::cancel_transaction() {
	bool result = true;
	foreach (context_map::reference ctx, contexts_){
		result &= ctx.second->cancel_transaction();
	}
	return result;
}