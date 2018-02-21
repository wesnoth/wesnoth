/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * A component of the AI framework
 */

#pragma once

class config;

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

//============================================================================
namespace ai {

//TODO: find a good place for this
struct path_element {
	path_element()
		: property()
		, id()
		, position(0)
	{
	}

	std::string property;
	std::string id;
	int position;
};

class base_property_handler;
typedef std::shared_ptr<base_property_handler> property_handler_ptr;
typedef std::map<std::string,property_handler_ptr> property_handler_map;

class component {
public:
	component()
		: property_handlers_()
	{
	}

	virtual std::string get_id() const = 0;
	virtual std::string get_name() const = 0;
	virtual std::string get_engine() const = 0;
	virtual ~component() {}
	virtual component* get_child(const path_element &child);
	virtual std::vector<component*> get_children(const std::string &type);
	virtual std::vector<std::string> get_children_types();
	virtual bool change_child(const path_element &child, const config &cfg);
	virtual bool add_child(const path_element &child, const config &cfg);
	virtual bool delete_child(const path_element &child);

	property_handler_map& property_handlers();

private:
	property_handler_map property_handlers_;
};

class component_manager {
public:
	static bool add_component(component *root, const std::string &path, const config &cfg);
	static bool change_component(component *root, const std::string &path, const config &cfg);
	static bool delete_component(component *root, const std::string &path);
	static std::string print_component_tree(component *root, const std::string &path);

	static component* get_component(component *root, const std::string &path);
};


} //end of namespace ai

std::ostream &operator<<(std::ostream &o, const ai::path_element &e);
