/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file config.hpp
//! Definitions for the interface to Wesnoth Markup Language (WML).

#ifndef CONFIG_HPP_INCLUDED
#define CONFIG_HPP_INCLUDED

#include "global.hpp"

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "tstring.hpp"
#include "serialization/string_utils.hpp"


// This module defines the interface to Wesnoth Markup Language (WML).
// WML is a simple hierarchical text-based file format.
// The format is defined in Wiki, under BuildingScenariosWML
//
// All configuration files are stored in this format,
// and data is sent across the network in this format.
// It is thus used extensively throughout the game.

typedef std::map<std::string,t_string> string_map;

//! A config object defines a single node in a WML file, with access to child nodes.
class config
{
public:
	// Create an empty node.
	config();

	config(const config& cfg);
	
	// Create a config with an empty child of name 'child'.
	config(const std::string& child);
	~config();

	config& operator=(const config& cfg);

	typedef std::vector<config*> child_list;
	typedef std::map<std::string,child_list> child_map;

	typedef std::vector<config*>::iterator child_iterator;
	typedef std::vector<config*>::const_iterator const_child_iterator;

	typedef std::pair<child_iterator,child_iterator> child_itors;
	typedef std::pair<const_child_iterator,const_child_iterator> const_child_itors;

	child_itors child_range(const std::string& key);
	const_child_itors child_range(const std::string& key) const;

	const child_list& get_children(const std::string& key) const;
	const child_map& all_children() const;

	config* child(const std::string& key);
	const config* child(const std::string& key) const;
	config& add_child(const std::string& key);
	config& add_child(const std::string& key, const config& val);
	config& add_child_at(const std::string& key, const config& val, size_t index);
	t_string& operator[](const std::string& key);
	const t_string& operator[](const std::string& key) const;

	const t_string& get_attribute(const std::string& key) const;
	bool has_attribute(const std::string& key) const {return values.find(key) != values.end();}

	config* find_child(const std::string& key, const std::string& name,
	                   const t_string& value);
	const config* find_child(const std::string& key, const std::string& name,
	                         const t_string& value) const;

	void clear_children(const std::string& key);
	void remove_child(const std::string& key, size_t index);

	void clear();
	bool empty() const;

	std::string debug() const;
	std::string hash() const;

	struct error {
		error(const std::string& msg) : message(msg) {}
		std::string message;
	};

	struct child_pos {
		child_pos(child_map::const_iterator p, size_t i) : pos(p), index(i) {}
		child_map::const_iterator pos;
		size_t index;

		bool operator==(const child_pos& o) const { return pos == o.pos && index == o.index; }
		bool operator!=(const child_pos& o) const { return !operator==(o); }
	};

	struct all_children_iterator {
		typedef std::pair<const std::string*,const config*> value_type;
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;
		typedef std::auto_ptr<value_type> pointer;
		typedef value_type& reference;
		typedef std::vector<child_pos>::const_iterator Itor;
		explicit all_children_iterator(Itor i=Itor());

		all_children_iterator& operator++();
		all_children_iterator  operator++(int);

		value_type operator*() const;
		pointer operator->() const;

		const std::string& get_key() const;
		size_t get_index() const;
		const config& get_child() const;

		bool operator==(all_children_iterator i) const;
		bool operator!=(all_children_iterator i) const;

	private:
		Itor i_;
	};

	//! In-order iteration over all children.
	all_children_iterator ordered_begin() const;
	all_children_iterator ordered_end() const;
	all_children_iterator erase(const all_children_iterator& i);

	//! A function to get the differences between this object,
	//! and 'c', as another config object.
	//! I.e. calling cfg2.apply_diff(cfg1.get_diff(cfg2))
	//! will make cfg1 identical to cfg2.
	config get_diff(const config& c) const;

	void apply_diff(const config& diff); //throw error

	void merge_with(const config& c);

	bool matches(const config &filter) const;

	//! Removes keys with empty values.
	void prune();

	//! Append data from another config object to this one.
	//! Attributes in the latter config object will clobber attributes in this one.
	void append(const config& cfg);

	//! All children with the given key will be merged
	//! into the first element with that key.
	void merge_children(const std::string& key);

	//! Resets the translated values of all strings contained in this object
	void reset_translation() const;

	//! All the attributes of this node.
	string_map values;

private:
	//! A list of all children of this node.
	child_map children;

	std::vector<child_pos> ordered_children;
};

bool operator==(const config& a, const config& b);
bool operator!=(const config& a, const config& b);

#endif
