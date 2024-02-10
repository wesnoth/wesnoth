/*
	Copyright (C) 2005 - 2024
	by Philippe Plantier <ayin@anathas.org>
	Copyright (C) 2003 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "config.hpp"
#include "map/location.hpp"
#include "variable_info.hpp"

#include <utility>

class unit_map;

class config_variable_set : public variable_set {
	const config& cfg_;
public:
	config_variable_set(const config& cfg) : cfg_(cfg) {}
	virtual config::attribute_value get_variable_const(const std::string &id) const;
	virtual variable_access_const get_variable_access_read(const std::string& varname) const;
};

/**
 * A variable-expanding proxy for the config class. This class roughly behaves
 * as a constant config object, but automatically expands variables.
 *
 * When dealing with a vconfig, keep in mind its lifetime. By default, vconfigs
 * do not maintain a copy their data; if you need a vconfig to stick around,
 * either construct it with manage_memory=true or call make_safe(). This will
 * cause the vconfig to make a copy of the underlying config object.
 */
class vconfig
{
private:

	vconfig();
	vconfig(const config & cfg, const std::shared_ptr<const config> & cache);
	vconfig(const config& cfg, const std::shared_ptr<const config> & cache, const variable_set& variables);
public:
	/**
	 * Constructor from a config.
	 * Equivalent to vconfig(cfg, false).
	 * Do not use if the vconfig will persist after @a cfg is destroyed!
	 */
	explicit vconfig(const config &cfg);
	explicit vconfig(config &&cfg);
	// Construct a vconfig referencing a non-default set of variables.
	// Note that the vconfig does NOT take ownership of these variables,
	// so you need to make sure that their scope encloses the vconfig's scope!
	vconfig(const config& cfg, const variable_set& variables);
	vconfig(const config &cfg, bool manage_memory, const variable_set* variables = nullptr);
	~vconfig();

	static vconfig empty_vconfig(); // Valid to dereference. Contains nothing
	static vconfig unconstructed_vconfig(); // Must not be dereferenced

	/** A vconfig evaluates to true iff it can be dereferenced. */
	explicit operator bool() const	{ return !null(); }

	bool null() const { assert(cfg_); return cfg_ == &default_empty_config; }
	/** instruct the vconfig to make a private copy of its underlying data. */
	const vconfig& make_safe() const;
	const config& get_config() const { return *cfg_; }
	config get_parsed_config() const;

	typedef std::vector<vconfig> child_list;
	child_list get_children(const std::string& key) const;
	std::size_t count_children(const std::string& key) const;
	vconfig child(const std::string& key) const;
	bool has_child(const std::string& key) const;

	/**
	 * Note: vconfig::operator[] returns const, and this should not be changed
	 * because vconfig is often used as a drop-in replacement for config, and
	 * this const will properly warn you if you try to assign vcfg["key"]=val;
	 *
	 * Note: The following construction is unsafe:
	 * const std::string& temp = vcfg["foo"];
	 * This binds temp to a member of a temporary t_string. The lifetime of the
	 * temporary is not extended by this reference binding and the temporary's
	 * lifetime ends which causes UB. Instead use:
	 * const std::string temp = vcfg["foo"];
	 */
	const config::attribute_value operator[](const std::string &key) const
	{ return expand(key); }
	config::attribute_value expand(const std::string&) const; /** < Synonym for operator[] */
	bool has_attribute(const std::string& key) const { return cfg_->has_attribute(key); }
	bool empty() const { return (null() || cfg_->empty()); }

	struct attribute_iterator
	{
		struct pointer_proxy;

		typedef const config::attribute value_type;
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef int difference_type;
		typedef const pointer_proxy pointer;
		typedef const config::attribute reference;
		typedef config::const_attribute_iterator Itor;
		explicit attribute_iterator(const Itor &i, const variable_set& vars): i_(i), variables_(&vars) {}

		attribute_iterator &operator++() { ++i_; return *this; }
		attribute_iterator operator++(int) { return attribute_iterator(i_++, *variables_); }

		attribute_iterator &operator--() { --i_; return *this; }
		attribute_iterator operator--(int) { return attribute_iterator(i_--, *variables_); }

		reference operator*() const;
		pointer operator->() const;

		bool operator==(const attribute_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const attribute_iterator &i) const { return i_ != i.i_; }

	private:
		Itor i_;
		const variable_set* variables_;
	};

	boost::iterator_range<attribute_iterator> attribute_range() {
		config::const_attr_itors range = cfg_->attribute_range();
		return boost::make_iterator_range(attribute_iterator(range.begin(), *variables_), attribute_iterator(range.end(), *variables_));
	}

	struct all_children_iterator
	{
		struct pointer_proxy;

		typedef const std::pair<std::string, vconfig> value_type;
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef int difference_type;
		typedef const pointer_proxy pointer;
		typedef const value_type reference;
		typedef config::const_all_children_iterator Itor;
		explicit all_children_iterator(const Itor &i, const variable_set& vars);
		all_children_iterator(const Itor &i, const variable_set& vars, const std::shared_ptr<const config> & cache);

		all_children_iterator& operator++();
		all_children_iterator  operator++(int);
		all_children_iterator& operator--();
		all_children_iterator  operator--(int);

		reference operator*() const;
		pointer operator->() const;

		std::string get_key() const;
		vconfig get_child() const;
		void disable_insertion() { inner_index_ = -1; }

		bool operator==(const all_children_iterator &i) const;
		bool operator!=(const all_children_iterator &i) const
		{ return !operator==(i); }

	private:
		Itor i_;
		/*
			if wa have game variables
				[a] b = 1 [/a]
				[a] b = 4 [/a]
				[a] b = 6 [/a]
			we want to expand [insert_tag] variable = a name  = "u" [/insert_tag] to
			   [u] b = 1 [/u]
			   [u] b = 4 [/u]
			   [u] b = 6 [/u]
			in this case inner_index_ points to the index in 'a' we are currently processing.
		*/
		int inner_index_;
		std::shared_ptr<const config> cache_;
		const variable_set* variables_;
	};

	struct recursion_error : public config::error {
		recursion_error(const std::string& msg) : error(msg) {}
	};

	/** In-order iteration over all children. */
	all_children_iterator ordered_begin() const;
	all_children_iterator ordered_end() const;
	boost::iterator_range<all_children_iterator> all_ordered() const {
		return boost::make_iterator_range(ordered_begin(), ordered_end());
	}

private:
	/** Returns true if *this has made a copy of its config. */
	bool memory_managed() const { return static_cast<bool>(cache_); }

	/**
	 * Keeps a copy of our config alive when we manage our own memory.
	 * If this is not null, then cfg_ points to *cache_ or a child of *cache_.
	 */
	mutable std::shared_ptr<const config> cache_;
	/** Used to access our config (original or copy, as appropriate). */
	mutable const config* cfg_;
	const variable_set* variables_;
	static const config default_empty_config;
};

struct vconfig::attribute_iterator::pointer_proxy
{
	value_type p;
	pointer_proxy(value_type p) : p(p) {}
	value_type *operator->() const { return &p; }
};

struct vconfig::all_children_iterator::pointer_proxy
{
	value_type p;
	pointer_proxy(value_type p) : p(p) {}
	value_type *operator->() const { return &p; }
};


class scoped_wml_variable
{
public:
	scoped_wml_variable(const std::string& var_name);
	virtual ~scoped_wml_variable();
	const std::string& name() const { return var_name_; }
	virtual void activate() = 0;
	config &store(const config &var_value = config());
	bool activated() const { return activated_; }
private:
	config previous_val_;
	const std::string var_name_;
	bool activated_;
};

class scoped_weapon_info : public scoped_wml_variable
{
public:
	scoped_weapon_info(const std::string& var_name, optional_const_config data)
		: scoped_wml_variable(var_name), data_(data) {}
	void activate();
private:
	optional_const_config data_;
};

class scoped_xy_unit : public scoped_wml_variable
{
public:
	scoped_xy_unit(const std::string& var_name, map_location loc, const unit_map& umap)
		: scoped_wml_variable(var_name), loc_(loc), umap_(umap) {}
	void activate();
private:
	const map_location loc_;
	const unit_map& umap_;
};

class scoped_recall_unit : public scoped_wml_variable
{
public:
	scoped_recall_unit(const std::string& var_name, const std::string& player,
		unsigned int recall_index) : scoped_wml_variable(var_name), player_(player),
		recall_index_(recall_index) {}
	void activate();
private:
	const std::string player_;
	unsigned int recall_index_;
};
