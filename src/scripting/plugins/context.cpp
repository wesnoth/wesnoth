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

#include "scripting/plugins/context.hpp"

#include "scripting/plugins/manager.hpp"

#include "config_assign.hpp"

#include <cassert>
#include <utility>
#include "utils/functional.hpp"

plugins_context::plugins_context(const std::string & name)
	: callbacks_()
	, accessors_()
	, name_(name)
{}

plugins_context::plugins_context(const std::string& name, const std::vector<Reg>& l, const std::vector<aReg>& r)
	: callbacks_()
	, accessors_()
	, name_(name)
{
	initialize(l, r);
}

void plugins_context::initialize(const std::vector<Reg>& callbacks, const std::vector<aReg>& accessors)
{
	for (const Reg& l : callbacks) {  /* fill the table with given functions */
		if (l.name != nullptr) {
			callbacks_.emplace(l.name, l.func);
		}
	}
	for (const aReg& r : accessors) {  /* fill the table with given functions */
		if (r.name != nullptr) {
			accessors_.emplace(r.name, r.func);
		}
	}
}

void plugins_context::set_callback(const std::string & name, callback_function func)
{
	callbacks_[name] = func;
}

size_t plugins_context::erase_callback(const std::string & name)
{
	return callbacks_.erase(name);
}

size_t plugins_context::clear_callbacks()
{
	size_t ret = callbacks_.size();
	callbacks_ = callback_list();
	return ret;
}

void plugins_context::set_accessor(const std::string & name, accessor_function func)
{
	accessors_[name] = func;
}

void plugins_context::set_accessor_string(const std::string & name, std::function<std::string(config)> func)
{
	set_accessor(name, [func, name](const config& cfg) { return config_of(name, func(cfg)); });
}

void plugins_context::set_accessor_int(const std::string & name, std::function<int(config)> func)
{
	set_accessor(name, [func, name](const config& cfg) { return config_of(name, func(cfg)); });
}


size_t plugins_context::erase_accessor(const std::string & name)
{
	return accessors_.erase(name);
}

size_t plugins_context::clear_accessors()
{
	size_t ret = accessors_.size();
	accessors_ = accessor_list();
	return ret;
}

void plugins_context::play_slice()
{
	assert(plugins_manager::get());
	plugins_manager::get()->play_slice(*this);
}

void plugins_context::set_callback(const std::string & name, std::function<void(config)> func, bool preserves_context)
{
	set_callback(name, [func, preserves_context](config cfg) { func(cfg); return preserves_context; });
}
