/*
   Copyright (C) 2014 - 2015 by Chris Beck <render787@gmail.com>
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

#include <assert.h>
#include <utility>
#include <boost/bind.hpp>

plugins_context::plugins_context(const std::string & name)
	: callbacks_()
	, accessors_()
	, name_(name)
{}

plugins_context::plugins_context(const std::string & name, const Reg * l, const aReg * r)
	: callbacks_()
	, accessors_()
	, name_(name)
{
	for (; l->name != NULL; l++) {  /* fill the table with given functions */
		callbacks_.insert(std::make_pair(l->name, l->func));
	}
	for (; r->name != NULL; r++) {  /* fill the table with given functions */
		accessors_.insert(std::make_pair(r->name, r->func));
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

template<typename T>
static config make_config(const std::string & name, const T & val)
{
	return config_of(name, val);
}

void plugins_context::set_accessor_string(const std::string & name, boost::function<std::string(config)> func)
{
	set_accessor(name, boost::bind(&make_config<std::string>, name, boost::bind(func, _1)));
}

void plugins_context::set_accessor_int(const std::string & name, boost::function<int(config)> func)
{
	set_accessor(name, boost::bind(&make_config<int>, name, boost::bind(func, _1)));
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

static bool shim(config cfg, boost::function<void(config)> func, bool return_value)
{
	func(cfg);
	return return_value;
}

void plugins_context::set_callback(const std::string & name, boost::function<void(config)> func, bool preserves_context)
{
	set_callback(name, boost::bind(shim, _1, func, preserves_context));
}

const boost::function< std::string ( const config & , const std::string & ) > get_str =
	boost::bind(&config::attribute_value::str,
		boost::bind(static_cast<const config::attribute_value &(config::*)(const std::string &) const>(&config::operator[]) , _1, _2));

const boost::function< int ( const config & , const std::string &, int ) > get_int =
	boost::bind(&config::attribute_value::to_int,
		boost::bind(static_cast<const config::attribute_value &(config::*)(const std::string &) const>(&config::operator[]) , _1, _2)
		, _3);

const boost::function< size_t ( const config & , const std::string &, size_t ) > get_size_t =
	boost::bind(&config::attribute_value::to_size_t,
		boost::bind(static_cast<const config::attribute_value &(config::*)(const std::string &) const>(&config::operator[]) , _1, _2)
		, _3);
