/* $Id$ */
/*
   Copyright (C) 2009 - 2011 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * Composite AI component
 * @file ai/composite/component.cpp
 */

#include "component.hpp"
#include "engine.hpp"
#include "../../log.hpp"
#include "../../foreach.hpp"

#include "../formula/ai.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

namespace ai {

static lg::log_domain log_ai_component("ai/component");
#define DBG_AI_COMPONENT LOG_STREAM(debug, log_ai_component)
#define LOG_AI_COMPONENT LOG_STREAM(info, log_ai_component)
#define ERR_AI_COMPONENT LOG_STREAM(err, log_ai_component)


/*
[modify_ai]
    path = "stage[fallback]
    action = "change"
    [stage]...[/stage]
[/modify_ai]

[modify_ai]
    component = "aspect[avoid].facet[zzz]"
    action = "change"
    [facet]...[/facet]
[/modify_ai]

[modify_ai]
    path = "aspect[aggression].facet[zzzz]
    action = "delete"
[/modify_ai]

[modify_ai]
    component = "aspect[aggression].facet"
    action = "add"
    [facet]...[/facet]
[/modify_ai]
*/


component* component::get_child(const path_element &child)
{
	std::map<std::string, property_handler_ptr>::iterator i = property_handlers_.find(child.property);
	if (i!=property_handlers_.end()) {
		return i->second->handle_get(child);
	}
	return NULL;
}


bool component::add_child(const path_element &child, const config &cfg)
{
	std::map<std::string, property_handler_ptr>::iterator i = property_handlers_.find(child.property);
	if (i!=property_handlers_.end()) {
		return i->second->handle_add(child,cfg);
	}
	return false;
}


bool component::change_child(const path_element &child, const config &cfg)
{
	std::map<std::string, property_handler_ptr>::iterator i = property_handlers_.find(child.property);
	if (i!=property_handlers_.end()) {
		return i->second->handle_change(child,cfg);
	}
	return false;
}


bool component::delete_child(const path_element &child)
{
	std::map<std::string, property_handler_ptr>::iterator i = property_handlers_.find(child.property);
	if (i!=property_handlers_.end()) {
		return i->second->handle_delete(child);
	}
	return false;
}


std::vector<component*> component::get_children(const std::string &type)
{
	std::vector<component*> components;
	property_handler_map::iterator i = property_handlers_.find(type);
	if (i!=property_handlers_.end()) {
		return i->second->handle_get_children();
	}

	return components;
}


std::vector<std::string> component::get_children_types()
{
	std::vector<std::string> types;
	BOOST_FOREACH (property_handler_map::value_type &ph, property_handlers_) {
		types.push_back(ph.first);
	}
	return types;
}


static component *find_component(component *root, const std::string &path, path_element &tail)
{
	if (root==NULL) {
		return NULL;
	}

	//match path elements in [modify_ai] tag
	boost::regex re("([^\\.^\\[]+)(\\[(\\d*)\\]|\\[([^\\]]+)\\]|())");
	int const sub_matches[] = {1,3,4};
	boost::sregex_token_iterator i(path.begin(), path.end(), re, sub_matches);
	boost::sregex_token_iterator j;

	component *c  = root;

	std::vector< path_element > elements;
	while(i != j)
	{
		path_element pe;
		pe.property = *i++;
		std::string position = *i++;
		pe.id = *i++;
		if (position.empty()) {
			pe.position = -2;
		} else {
			try {
				pe.position = boost::lexical_cast<int>(position);
			} catch (boost::bad_lexical_cast){
				pe.position = -2;
			}
		}
		//DBG_AI_COMPONENT << "adding path element: "<< pe << std::endl;
		elements.push_back(pe);
	}
	if (elements.size()<1) {
		return NULL;
	}

	std::vector< path_element >::iterator k_max = elements.end()-1;
	for (std::vector< path_element >::iterator k = elements.begin(); k!=k_max; ++k) {
		//not last
		c = c->get_child(*k);
		if (c==NULL) {
			return NULL;
		}
	}

	tail = *k_max;
	return c;

}


bool component_manager::add_component(component *root, const std::string &path, const config &cfg)
{
	path_element tail;
	component *c = find_component(root,path,tail);
	if (c==NULL) {
		return false;
	}
	const config &ch = cfg.child(tail.property);
	if (!ch) {
		return false;
	}
	return c->add_child(tail, ch);

}

bool component_manager::change_component(component *root, const std::string &path, const config &cfg)
{
	path_element tail;
	component *c = find_component(root,path,tail);
	if (c==NULL) {
		return false;
	}
	const config &ch = cfg.child(tail.property);
	if (!ch) {
		return false;
	}
	return c->change_child(tail,ch);
}

bool component_manager::delete_component(component *root, const std::string &path)
{
	path_element tail;
	component *c = find_component(root,path,tail);
	if (c==NULL) {
		return false;
	}
	return c->delete_child(tail);
}


static void print_component(component *root, const std::string &type, std::stringstream &s, int offset)
{
	std::stringstream offset_ss;
	for (int i=0;i<offset;++i) {
		offset_ss<<"    ";
	}
	const std::string &offset_str = offset_ss.str();

	const std::vector<std::string> &t_list = root->get_children_types();

	s << offset_str << type<<"["<<root->get_id() <<"] "<<root->get_engine()<<" "<<root->get_name()<< std::endl;

	BOOST_FOREACH (std::string t, t_list) {
		std::vector<component*> c_list = root->get_children(t);
		BOOST_FOREACH (component *c, c_list) {
			print_component(c,t,s,offset+1);
		}
	}
}

std::string component_manager::print_component_tree(component *root, const std::string &path)
{
	path_element tail;
	component *c;
	if (!path.empty()) {
		c = find_component(root,path,tail);
		if (c==NULL) {
			ERR_AI_COMPONENT << "unable to find component" <<std::endl;
			return "";
		}
	} else {
		c = root;
	}
	std::stringstream s;
	print_component(c, "", s, 0);
	return s.str();
}

} //end of namespace ai


std::ostream &operator<<(std::ostream &o, const ai::path_element &e)
{
	o << "property["<<e.property<<"] id["<<e.id <<"] position["<<e.position<<"]"<<std::endl;
	return o;
}
