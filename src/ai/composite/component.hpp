/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/composite/component.hpp
 * A component of the AI framework
 */

#ifndef AI_COMPOSITE_COMPONENT_HPP_INCLUDED
#define AI_COMPOSITE_COMPONENT_HPP_INCLUDED

#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "property_handler.hpp"

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

//============================================================================
namespace ai {

class component {
public:
	component() {};
	virtual const std::string& get_id() const = 0;
	virtual const std::string& get_name() const = 0;
	virtual const std::string& get_engine() const = 0;
	virtual ~component() {};
	virtual component* get_child(const path_element &child);
	virtual std::vector<component*> get_children(const std::string &type);
	virtual std::vector<std::string> get_children_types();
	virtual bool change_child(const path_element &child, const config &cfg);
	virtual bool add_child(const path_element &child, const config &cfg);
	virtual bool delete_child(const path_element &child);

	template<typename X>
	void register_vector_property(const std::string &property, std::vector< boost::shared_ptr<X> > &values_, boost::function2<void, std::vector< boost::shared_ptr<X> >&, const config&> construction_factory)
	{
	       	property_handlers_.insert(make_pair(property,property_handler_ptr(new vector_property_handler<X>(property,values_,construction_factory))));
	}

	template<typename X>
	void register_aspect_property(const std::string &property, std::map< std::string, boost::shared_ptr<X> > &aspects_)
	{
	       	property_handlers_.insert(make_pair(property,property_handler_ptr(new aspect_property_handler<X>(property,aspects_))));
	}



	typedef std::map<std::string,property_handler_ptr> property_handler_map;
private:
	property_handler_map property_handlers_;
};

class component_manager {
public:
	static bool add_component(component *root, const std::string &path, const config &cfg);
	static bool change_component(component *root, const std::string &path, const config &cfg);
	static bool delete_component(component *root, const std::string &path);
	static std::string print_component_tree(component *root, const std::string &path);
};


} //end of namespace ai

std::ostream &operator<<(std::ostream &o, const ai::path_element &e);

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
