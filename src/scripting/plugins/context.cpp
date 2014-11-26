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

#include "scripting/plugins/context.hpp"

#include "scripting/plugins/manager.hpp"

#include <assert.h>
#include <utility>

plugins_context::plugins_context(const std::string & name)
	: callbacks_()
	, name_(name)
{}

plugins_context::plugins_context(const std::string & name, const Reg * l)
	: callbacks_()
	, name_(name)
{
	for (; l->name != NULL; l++) {  /* fill the table with given functions */
		callbacks_.insert(std::make_pair(l->name, l->func));
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
	callbacks_ = std::map<std::string, callback_function > ();
	return ret;
}

void plugins_context::play_slice()
{
	assert(plugins_manager::get());
	plugins_manager::get()->play_slice(*this);
}
