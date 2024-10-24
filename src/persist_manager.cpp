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

#include "persist_context.hpp"
#include "persist_manager.hpp"

persist_manager::persist_manager()
	: in_transaction_(false)
	, contexts_()
{}

persist_manager::~persist_manager() {
	cancel_transaction();
}

persist_context &persist_manager::get_context(const std::string &ns)
{
	persist_context::name_space name(ns,true);
	std::string key(name.root_);
	context_map::iterator iter = contexts_.find(key);
	if (iter == contexts_.end()) {
		auto pfc = std::make_unique<persist_file_context>(key);
		if (in_transaction_) pfc->start_transaction();
		std::tie(iter, std::ignore) = contexts_.emplace(key, std::move(pfc));
	}
	auto& ret = iter->second;
	if (ret->get_node() != ns)
		ret->set_node(name.descendants_);
	return *ret;
}

bool persist_manager::start_transaction() {
	if (in_transaction_) return false;
	bool result = true;
	for (context_map::reference ctx : contexts_) {
		result &= ctx.second->start_transaction();
	}
	in_transaction_ = true;
	return result;
}

bool persist_manager::end_transaction() {
	if (!in_transaction_) return false;
	bool result = true;
	for (context_map::reference ctx : contexts_) {
		result &= ctx.second->end_transaction();
	}
	in_transaction_ = !result;
	return result;
}

bool persist_manager::cancel_transaction() {
	if (!in_transaction_) return false;
	bool result = true;
	for (context_map::reference ctx : contexts_) {
		result &= ctx.second->cancel_transaction();
	}
	in_transaction_ = false;
	return result;
}
