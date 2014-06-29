/*
   Copyright (C) 2009 - 2014 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * Composite AI component
 * @file
 */


#ifndef AI_COMPOSITE_PROPERTY_HANDLER_HPP_INCLUDED
#define AI_COMPOSITE_PROPERTY_HANDLER_HPP_INCLUDED


#ifndef INCL_BOOST_FUNCTION_HPP_
#define INCL_BOOST_FUNCTION_HPP_
#include <boost/function.hpp>
#endif

#include <boost/foreach.hpp>

#include "config.hpp"
#include "ai/composite/component.hpp"

namespace ai{

template<typename T>
class path_element_matches{
public:
	path_element_matches(const path_element &element)
		: count_(0), element_(element)
	{
	}
	virtual ~path_element_matches(){}

	bool operator()(const T& t)
	{
		if ( (!element_.id.empty()) && (element_.id == t->get_id()) ) {
			return true;
		}
		if (count_ == element_.position) {
			return true;
		}
		count_++;
		return false;
	}

private:
	int count_;
	path_element element_;
};


class component;

class base_property_handler {
public:

	virtual ~base_property_handler() {}

	virtual component* handle_get(const path_element &child) = 0;
	virtual bool handle_change(const path_element &child, const config &cfg) = 0;
	virtual bool handle_add(const path_element &child, const config &cfg) = 0;
	virtual bool handle_delete(const path_element &child) = 0;
	virtual std::vector< component* > handle_get_children() = 0;
};

typedef boost::shared_ptr< base_property_handler > property_handler_ptr;

template<typename T>
class vector_property_handler : public base_property_handler {
public:
	typedef boost::shared_ptr<T> t_ptr;
	typedef std::vector< boost::shared_ptr<T> > t_ptr_vector;

	vector_property_handler(const std::string &property, t_ptr_vector &values, boost::function2<void, t_ptr_vector&, const config&> &construction_factory)
		: factory_(construction_factory), property_(property), values_(values){}


        component* handle_get(const path_element &child)
	{
	      	typename t_ptr_vector::iterator i = std::find_if(values_.begin(),values_.end(),path_element_matches<t_ptr>(child));
		if (i!=values_.end()){
			return &*(*i);
		}
		return NULL;
	}
	bool handle_change(const path_element &child, const config &cfg)
	{
		if (!handle_delete(child)) {
			return false;
		}

		return handle_add(child,cfg);
	}
	bool handle_add(const path_element &child, const config &cfg)
	{
		//if the id is not empty, try to delete all with this id
		if (!cfg["id"].empty()) {
			path_element with_same_id;
			with_same_id.id = cfg["id"].str();
			with_same_id.property = property_;
			with_same_id.position=-1;
			handle_delete(with_same_id);
		}

	      	typename t_ptr_vector::iterator i = std::find_if(values_.begin(),values_.end(),path_element_matches<t_ptr>(child));
		return do_add(i-values_.begin(),cfg);
	}

	bool handle_delete(const path_element &child)
	{
		//* is a special case - 'delete all'
		if (child.id == "*") {
			values_.clear();
			return true;
		}

		typename t_ptr_vector::iterator i = std::find_if(values_.begin(),values_.end(),path_element_matches<t_ptr>(child));
		if (i!=values_.end()){
			values_.erase(i);
			return true;
		}
		return false;
	}


	std::vector<component*> handle_get_children()
	{
		std::vector<component*> children;
		BOOST_FOREACH(t_ptr v, values_) {
			children.push_back(&*v);
		}
		return children;
	}

private:
	bool do_add(int pos, const config &cfg)
	{
		if (pos<0) {
			pos = values_.size();
		}
		t_ptr_vector values;
		factory_(values,cfg);
		int j=0;
		BOOST_FOREACH(t_ptr b, values ){
			values_.insert(values_.begin()+pos+j,b);
			j++;
		}
		return (j>0);
	}

	boost::function2<void, t_ptr_vector&, const config&> factory_;
	const std::string property_;
	t_ptr_vector &values_;

};



template<typename T>
class aspect_property_handler : public base_property_handler {
public:
	typedef boost::shared_ptr<T> t_ptr;
	typedef std::map< std::string, t_ptr > aspect_map;

	aspect_property_handler(const std::string &property, aspect_map &aspects)
		: property_(property), aspects_(aspects)
	{
	}


        component* handle_get(const path_element &child)
	{
		typename aspect_map::const_iterator a = aspects_.find(child.id);
		if (a!=aspects_.end()){
			return &*a->second;
		}
		return NULL;
	}

	bool handle_change(const path_element &/*child*/, const config &/*cfg*/)
	{
		return false;
	}

	bool handle_add(const path_element &/*child*/, const config &/*cfg*/)
	{
		return false;
	}

	bool handle_delete(const path_element &child)
	{
		//* is a special case - 'delete all facets'
		if (child.id == "*") {
			bool b = false;
				BOOST_FOREACH(typename aspect_map::value_type a, aspects_) {
				       	b |= a.second->delete_all_facets();
				}
			return b;
		}
		return false;
	}


	std::vector<component*> handle_get_children()
	{
		std::vector<component*> children;
		BOOST_FOREACH(typename aspect_map::value_type a, aspects_) {
			children.push_back(&*a.second);
		}
		return children;
	}

private:

	const std::string &property_;
	aspect_map &aspects_;

};



template<typename X>
static void register_vector_property(std::map<std::string,property_handler_ptr> &property_handlers, const std::string &property, std::vector< boost::shared_ptr<X> > &values, boost::function2<void, std::vector< boost::shared_ptr<X> >&, const config&> construction_factory)
{
	property_handler_ptr handler_ptr = property_handler_ptr(new vector_property_handler<X>(property,values,construction_factory));
	property_handlers.insert(std::make_pair(property,handler_ptr));
}

template<typename X>
static void register_aspect_property(std::map<std::string,property_handler_ptr> &property_handlers, const std::string &property, std::map< std::string, boost::shared_ptr<X> > &aspects)
{
	property_handler_ptr handler_ptr = property_handler_ptr(new aspect_property_handler<X>(property,aspects));
	property_handlers.insert(std::make_pair(property,handler_ptr));
}


} //of namespace ai

#endif
