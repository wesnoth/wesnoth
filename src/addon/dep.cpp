/* $Id$ */
/*
   Copyright (C) 2012 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "addon/dep.hpp"

#include "config.hpp"

namespace {
	inline addon_dep::dep_type dep_type_str_to_id(const std::string& str)
	{
		if(str == "recommends") {
			return addon_dep::recommends;
		} else if(str == "conflicts") {
			return addon_dep::conflicts;
		} else {
			return addon_dep::requires;
		}
	}
}

addon_dep::addon_dep(const addon_dep& o)
	: id(o.id)
	, type(o.type)
	, dependants(o.dependants)
	, version(o.version)
	, version_type(o.version_type)
{
}

addon_dep::addon_dep(const std::string& dep)
	: id(dep)
	, type(requires)
	, dependants()
	, version()
	, version_type(version_any)
{
}

addon_dep::addon_dep(const config& cfg, const std::string& type_str)
	: id()
	, type()
	, dependants()
	, version()
	, version_type()
{
	this->read(cfg, type_str);
}

addon_dep& addon_dep::operator=(const addon_dep& o)
{
	if(&o != this) {
		this->id = o.id;
		this->type = o.type;
		this->dependants = o.dependants;
		this->version = o.version;
		this->version_type = o.version_type;
	}

	return *this;
}

void addon_dep::read(const config& cfg, const std::string& type_str)
{
	this->id = cfg["id"].str();
	this->type = dep_type_str_to_id(type_str);

	if(cfg.has_attribute("version_equals") && cfg["version_equals"].str() != "*") {
		this->version_type = version_equals;
		this->version = cfg["version_equals"].str();
	}
	else if(cfg.has_attribute("version_not_equals")) {
		this->version_type = version_not_equals;
		this->version = cfg["version_not_equals"].str();
	}
	else if(cfg.has_attribute("version_less_than")) {
		this->version_type = version_less_than;
		this->version = cfg["version_less_than"].str();
	}
	else if(cfg.has_attribute("version_less_than_or_equals")) {
		this->version_type = version_less_than_or_equals;
		this->version = cfg["version_less_than_or_equals"].str();
	}
	else if(cfg.has_attribute("version_greater_than")) {
		this->version_type = version_greater_than;
		this->version = cfg["version_greater_than"].str();
	}
	else if(cfg.has_attribute("version_greater_than_or_equals")) {
		this->version_type = version_greater_than_or_equals;
		this->version = cfg["version_greater_than_or_equals"].str();
	}
	else {
		this->version_type = version_any;
		this->version = version_info();
	}
	
}
