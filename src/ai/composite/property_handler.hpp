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
 * Composite AI component
 * @file
 */

#pragma once

#include "utils/functional.hpp"
#include "config.hpp"
#include "ai/composite/component.hpp"

#include <algorithm>

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
	virtual bool handle_change(const path_element &child, config cfg) = 0;
	virtual bool handle_add(const path_element &child, const config &cfg) = 0;
	virtual bool handle_delete(const path_element &child) = 0;
	virtual std::vector< component* > handle_get_children() = 0;
};

typedef std::shared_ptr< base_property_handler > property_handler_ptr;

template<typename T>
class vector_property_handler : public base_property_handler {
public:
	typedef std::shared_ptr<T> ptr;
	typedef std::vector< std::shared_ptr<T> > ptr_vector;

	vector_property_handler(const std::string &property, ptr_vector &values, std::function<void(ptr_vector&, const config&)> &construction_factory)
		: factory_(construction_factory), property_(property), values_(values){}


        component* handle_get(const path_element &child)
	{
			typename ptr_vector::iterator i = std::find_if(values_.begin(),values_.end(),path_element_matches<ptr>(child));
		if (i!=values_.end()){
			return &*(*i);
		}
		return nullptr;
	}
	bool handle_change(const path_element &child, config cfg)
	{
		if (!handle_delete(child)) {
			return false;
		}
		if (!cfg.has_attribute("id")) {
			cfg["id"] = child.id;
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

			typename ptr_vector::iterator i = std::find_if(values_.begin(),values_.end(),path_element_matches<ptr>(child));
		return do_add(i-values_.begin(),cfg);
	}

	bool handle_delete(const path_element &child)
	{
		//* is a special case - 'delete all'
		if (child.id == "*") {
			values_.clear();
			return true;
		}

		typename ptr_vector::iterator i = std::find_if(values_.begin(),values_.end(),path_element_matches<ptr>(child));
		if (i!=values_.end()){
			values_.erase(i);
			return true;
		}
		return false;
	}


	std::vector<component*> handle_get_children()
	{
		std::vector<component*> children;
		for (ptr v : values_) {
			children.push_back(&*v);
		}
		return children;
	}

protected:
	void call_factory(ptr_vector& vec, const config& cfg)
	{
		factory_(vec, cfg);
	}
private:
	bool do_add(int pos, const config &cfg)
	{
		if (pos<0) {
			pos = values_.size();
		}
		ptr_vector values;
		call_factory(values,cfg);
		int j=0;
		for (ptr b : values ) {
			values_.insert(values_.begin()+pos+j,b);
			j++;
		}
		return (j>0);
	}

	std::function<void(ptr_vector&, const config&)> factory_;
	const std::string property_;
	ptr_vector &values_;

};



template<typename T>
class facets_property_handler : public vector_property_handler<T> {
	typedef typename vector_property_handler<T>::ptr ptr;
	typedef typename vector_property_handler<T>::ptr_vector ptr_vector;
public:

	facets_property_handler(const std::string &property, ptr_vector &values, ptr& def, std::function<void(ptr_vector&, const config&)> &construction_factory)
		: vector_property_handler<T>(property, values, construction_factory)
		, default_(def)
	{
	}

	component* handle_get(const path_element &child)
	{
		// special case - 'get the default facet'
		if (child.id == "default_facet") {
			return default_.get();
		}
		return vector_property_handler<T>::handle_get(child);
	}

	bool handle_change(const path_element &child, config cfg)
	{
		// special case - 'replace the default facet'
		if (child.id == "default_facet") {
			ptr_vector values;
			this->call_factory(values,cfg);
			default_ = values.back();
			return true;
		}
		return vector_property_handler<T>::handle_change(child, cfg);
	}

	std::vector<component*> handle_get_children()
	{
		std::vector<component*> children = vector_property_handler<T>::handle_get_children();
		children.push_back(default_.get());
		return children;
	}

private:
	ptr& default_;
};



template<typename T>
class aspect_property_handler : public base_property_handler {
public:
	typedef std::shared_ptr<T> ptr;
	typedef std::map< std::string, ptr > aspect_map;

	aspect_property_handler(const std::string &property, aspect_map &aspects, std::function<void(aspect_map&, const config&, std::string)> &construction_factory)
		: property_(property), aspects_(aspects), factory_(construction_factory)
	{
	}


        component* handle_get(const path_element &child)
	{
		typename aspect_map::const_iterator a = aspects_.find(child.id);
		if (a!=aspects_.end()){
			return &*a->second;
		}
		return nullptr;
	}

	bool handle_change(const path_element &child, config cfg)
	{
		if (aspects_.find(child.id) == aspects_.end()) {
			return false;
		}
		if (!cfg.has_attribute("name")) {
			cfg["name"] = "composite_aspect";
		}
		cfg["id"] = child.id;
		factory_(aspects_, cfg, child.id);
		return true;
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
			for (typename aspect_map::value_type a : aspects_) {
				b |= a.second->delete_all_facets();
			}
			return b;
		}
		return false;
	}


	std::vector<component*> handle_get_children()
	{
		std::vector<component*> children;
		for (typename aspect_map::value_type a : aspects_) {
			children.push_back(&*a.second);
		}
		return children;
	}

private:

	const std::string &property_;
	aspect_map &aspects_;
	std::function<void(aspect_map&, const config&, std::string)> factory_;

};


template<typename X>
static inline void register_vector_property(property_handler_map& property_handlers, const std::string& property,
	std::vector<std::shared_ptr<X>>& values, std::function<void(std::vector<std::shared_ptr<X>>&, const config&)> construction_factory)
{
	property_handler_ptr handler_ptr(new vector_property_handler<X>(property, values, construction_factory));
	property_handlers.emplace(property, handler_ptr);
}

template<typename X>
static inline void register_facets_property(property_handler_map& property_handlers, const std::string& property,
	std::vector<std::shared_ptr<X>>& values, std::shared_ptr<X>& def, std::function<void(std::vector<std::shared_ptr<X>>&, const config&)> construction_factory)
{
	property_handler_ptr handler_ptr(new facets_property_handler<X>(property, values, def, construction_factory));
	property_handlers.emplace(property, handler_ptr);
}

template<typename X>
static inline void register_aspect_property(property_handler_map& property_handlers, const std::string& property,
	std::map<std::string, std::shared_ptr<X>>& aspects, std::function<void(std::map<std::string, std::shared_ptr<X>>&, const config&, std::string)> construction_factory)
{
	property_handler_ptr handler_ptr(new aspect_property_handler<X>(property, aspects, construction_factory));
	property_handlers.emplace(property, handler_ptr);
}

} //of namespace ai
