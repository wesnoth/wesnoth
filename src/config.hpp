/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
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
 * Definitions for the interface to Wesnoth Markup Language (WML).
 *
 * This module defines the interface to Wesnoth Markup Language (WML).  WML is
 * a simple hierarchical text-based file format.  The format is defined in
 * Wiki, under BuildingScenariosWML
 *
 * All configuration files are stored in this format, and data is sent across
 * the network in this format.  It is thus used extensively throughout the
 * game.
 */

#ifndef CONFIG_HPP_INCLUDED
#define CONFIG_HPP_INCLUDED

#include "global.hpp"

#include <map>
#include <iosfwd>
#include <vector>
#include <utility> //for relops
#include <boost/variant/variant.hpp>
#include <boost/unordered_map.hpp>

#include "game_errors.hpp"
#include "tstring.hpp"
#include "wesconfig.h"

//debug
#include <iostream>
class config;
class vconfig;
struct lua_State;

bool operator==(const config &, const config &);
inline bool operator!=(const config &a, const config &b) { return !operator==(a, b); }
std::ostream &operator << (std::ostream &, const config &);

/** A config object defines a single node in a WML file, with access to child nodes. */
class config
{
	friend bool operator==(const config& a, const config& b);

	static config * invalid;
	static bool initialize_invalid();

	/**
	 * Raises an exception if @a this is not valid.
	 */
	void check_valid() const {	
		if (!*this){ 
			throw error("Mandatory WML child missing yet untested for. Please report."); }
	}

	/**
	 * Raises an exception if @a this or @a cfg is not valid.
	 */
	void check_valid(const config &cfg) const {check_valid(); cfg.check_valid();}

#ifndef HAVE_CXX0X
	struct safe_bool_impl { void nonnull() {} };
	/**
	 * Used as the return type of the conversion operator for boolean contexts.
	 * Needed, since the compiler would otherwise consider the following
	 * conversion (C legacy): cfg["abc"] -> "abc"[bool(cfg)] -> 'b'
	 */
	typedef void (safe_bool_impl::*safe_bool)();
#endif

public:
	// Create an empty node.
	config();

	config(const config &);
	config &operator=(const config &);

#ifdef HAVE_CXX0X
	config(config &&);
	config &operator=(config &&);
#endif
	/// Define a token that compares faster than a string once created.
	/// It is preferable to use t_token as indices over strings
	typedef n_token::t_token t_token;

	/**
	 * Creates a config object with an empty child of name @a child.
	 */
	explicit config(const std::string &child);
	explicit config(const t_token &child);

	~config();


#ifdef HAVE_CXX0X
	explicit operator bool() const
	{ return this != invalid; }
#else
	operator safe_bool() const
	{ return this != invalid ? &safe_bool_impl::nonnull : 0; }
#endif

	typedef std::vector<config*> child_list;
	//typedef std::map<t_token,child_list> child_map;
	typedef boost::unordered_map<t_token,child_list> child_map;

	typedef boost::unordered_map<config::t_token, config *> t_child_range_index;
	typedef boost::unordered_map<config::t_token, config const *> t_const_child_range_index;

	struct const_child_iterator;

	struct child_iterator
	{
		typedef config value_type;
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;
		typedef config *pointer;
		typedef config &reference;
		typedef child_list::iterator Itor;
		explicit child_iterator(const Itor &i): i_(i) {}

		child_iterator &operator++() { ++i_; return *this; }
		child_iterator operator++(int) { return child_iterator(i_++); }
		child_iterator &operator--() { --i_; return *this; }
		child_iterator operator--(int) { return child_iterator(i_--); }

		config &operator*() const { return **i_; }
		config *operator->() const { return &**i_; }

		bool operator==(const child_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const child_iterator &i) const { return i_ != i.i_; }

	private:
		Itor i_;
		friend struct const_child_iterator;
	};

	struct const_child_iterator
	{
		typedef config value_type;
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;
		typedef const config *pointer;
		typedef const config &reference;
		typedef child_list::const_iterator Itor;
		explicit const_child_iterator(const Itor &i): i_(i) {}
		const_child_iterator(const child_iterator &i): i_(i.i_) {}

		const_child_iterator &operator++() { ++i_; return *this; }
		const_child_iterator operator++(int) { return const_child_iterator(i_++); }
		const_child_iterator &operator--() { --i_; return *this; }
		const_child_iterator operator--(int) { return const_child_iterator(i_--); }

		const config &operator*() const { return **i_; }
		const config *operator->() const { return &**i_; }

		bool operator==(const const_child_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const const_child_iterator &i) const { return i_ != i.i_; }

	private:
		Itor i_;
	};

	typedef std::pair<child_iterator,child_iterator> child_itors;
	typedef std::pair<const_child_iterator,const_child_iterator> const_child_itors;

	/**
	 * Variant for storing WML attributes.
	 * The most efficient type is used when assigning a value. For instance,
	 * strings "yes", "no", "true", "false" will be detected and stored as boolean.
	 * References are returned as results.  Once a value is constructed it is stored so that
	 * future (likely) references will be fast.
	 * Try not to stuff int into double or elimnate the ghost copies as that prevent excess copying elsewhere.
	 */
	class attribute_value
	{
	public:
		enum attribute_type {EMPTY, BOOL, INT, DOUBLE, TSTRING, TOKEN};
	private:
		mutable int int_value_;
		mutable double double_value_;
		mutable t_string t_string_value_;
		mutable t_token token_value_;

		attribute_type type_;
		mutable bool bool_value_ : 1;
		mutable bool is_bool_ : 1, is_int_ : 1, is_double_ : 1, is_t_string_ : 1, is_token_ : 1;

	public:
		/** Default implementation, but defined out-of-line for efficiency reasons. */
		attribute_value();
		/** Default implementation, but defined out-of-line for efficiency reasons. */
		~attribute_value();
		/** Default implementation, but defined out-of-line for efficiency reasons. */
		attribute_value(const attribute_value &);
		/** Default implementation, but defined out-of-line for efficiency reasons. */
		attribute_value &operator=(const attribute_value &);

		attribute_value &operator=(bool v);
		attribute_value &operator=(int v);
		attribute_value &operator=(double v);

		attribute_value &operator=(const char *v) { return operator=(std::string(v)); }
		attribute_value &operator=(const std::string &v);
		attribute_value &operator=(const t_string &v);
		attribute_value &operator=(const t_token &v);

		bool to_bool(bool def = false) const;
		int to_int(int def = 0) const;
		double to_double(double def = 0.) const;

		bool empty() const;
		bool blank() const {return empty();}; ///todo remove and use the standard empty()
		std::string const & str() const;
		t_string const & t_str() const;
		t_token const & token() const;

		operator int() const { return to_int(); }
		operator std::string const &() const { return str(); } ///This is probably ill advised, due to spurious constructions
		operator t_string const &() const { return t_str(); }
		operator t_token const &() const {return token();}

		bool operator==(const attribute_value &other) const;
		bool operator!=(const attribute_value &other) const { return !operator==(other); }

		friend bool operator==(const attribute_value &a, t_token const & b) ;
		friend bool operator==(t_token const & b, const attribute_value &a) { return a == b;}
		friend bool operator!=(const attribute_value &a, t_token const & b) { return !(a == b);}
		friend bool operator!=(t_token const & b, const attribute_value &a) { return a != b;}

		friend bool operator==(const attribute_value &a, t_string const & b) ;
		friend bool operator==(t_string const & b, const attribute_value &a) { return a == b;}
		friend bool operator!=(const attribute_value &a, t_string const & b) { return !(a== b);}
		friend bool operator!=(t_string const & b, const attribute_value &a) { return a != b;}

		friend bool operator==(const attribute_value &a, std::string const & b) {return a == t_token(b);}
		friend bool operator==(std::string const & b, const attribute_value &a) { return a == b;}
		friend bool operator!=(const attribute_value &a, std::string const & b) { return !(a == b);}
		friend bool operator!=(std::string const & b, const attribute_value &a) { return a != b;}

		friend bool operator==(const attribute_value &a, char const * b) {return a == std::string(b);}
		friend bool operator==(char const * b, const attribute_value &a) { return a == b;}
		friend bool operator!=(const attribute_value &a, char const * b) { return !(a == b);}
		friend bool operator!=(char const * b, const attribute_value &a) { return a != b;}

		friend std::ostream& operator<<(std::ostream &os, const attribute_value &v);
		struct default_visitor{
			void operator()() {}
			void operator()(bool const) {}
			void operator()(int const) {}
			void operator()(double const) {}
			void operator()(config::t_token const & ) {}
			void operator()(const t_string & ) {}
		};

		template <typename X> void apply_visitor(X & visitor) const;
		friend class vconfig;
	};

	//Note:: the unordered_map is smaller and the same speed
	//typedef std::map<t_token, attribute_value> attribute_map;
	typedef boost::unordered_map<t_token, attribute_value> attribute_map;
	typedef attribute_map::value_type attribute;

	struct const_attribute_iterator
	{
		typedef attribute value_type;
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;
		typedef const attribute *pointer;
		typedef const attribute &reference;
		typedef attribute_map::const_iterator Itor;
		explicit const_attribute_iterator(const Itor &i): i_(i) {}

		const_attribute_iterator &operator++() { ++i_; return *this; }
		const_attribute_iterator operator++(int) { return const_attribute_iterator(i_++); }

		const attribute &operator*() const { return *i_; }
		const attribute *operator->() const { return &*i_; }

		bool operator==(const const_attribute_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const const_attribute_iterator &i) const { return i_ != i.i_; }

	private:
		Itor i_;
	};

	typedef std::pair<const_attribute_iterator,const_attribute_iterator> const_attr_itors;
	child_itors child_range(const t_token& key);
	const_child_itors child_range(const t_token& key) const;

	/** Creates an index into the child range to allow for quick searches.
		key is the key for the child range and name is the column to index.
		i.e. if the list of save games is stored in a child called save with titles
		then to create a container indexed by titles do

		child_range_index(z_save, z_title);
		@Note the index is not stored in the config and is intended for
		repeated searches of large child ranges.
	 */
	t_child_range_index child_range_index(const t_token& key, config::t_token const & name);
	t_const_child_range_index const_child_range_index(const t_token& key, config::t_token const & name) const;

	unsigned child_count(const t_token &key) const;

	child_itors child_range(const std::string& key);
	const_child_itors child_range(const std::string& key) const;
	unsigned child_count(const std::string &key) const;

	/**
	 * Copies the first child with the given @a key, or an empty config if there is none.
	 */
	config child_or_empty(const t_token &key) const;
	config child_or_empty(const std::string &key) const;

	/**
	 * Returns the nth child with the given @a key, or
	 * a reference to an invalid config if there is none.
	 * @note A negative @a n accesses from the end of the object.
	 *       For instance, -1 is the index of the last child.
	 */
	config &child(const t_token& key, int n = 0);
	config &child(const std::string& key, int n = 0);

	/**
	 * Returns the nth child with the given @a key, or
	 * a reference to an invalid config if there is none.
	 * @note A negative @a n accesses from the end of the object.
	 *       For instance, -1 is the index of the last child.
	 */
	const config &child(const t_token& key, int n = 0) const { return const_cast<config *>(this)->child(key, n); }
	const config &child(const std::string& key, int n = 0) const { return const_cast<config *>(this)->child(key, n); }

	config& add_child(const t_token& key);
	config& add_child(const t_token& key, const config& val);
	config& add_child_at(const t_token &key, const config &val, unsigned index);
	config& add_child(const std::string& key);
	config& add_child(const std::string& key, const config& val);
	config& add_child_at(const std::string &key, const config &val, unsigned index);

#ifdef HAVE_CXX0X
	config &add_child(const std::string &key, config &&val);
#endif

	/**
	 * Returns a reference to the attribute with the given @a key.
	 * Creates it if it does not exist.  The variant using t_token is faster.
	 */
	attribute_value &operator[](const t_token &key);
	attribute_value &operator[](const attribute_value &key);
	attribute_value &operator[](const std::string &key);

	/**
	 * Returns a reference to the attribute with the given @a key
	 * or to a dummy empty attribute if it does not exist.
	 */
	const attribute_value &operator[](const t_token &key) const;
	const attribute_value &operator[](const attribute_value &key) const;
	const attribute_value &operator[](const std::string &key) const;

	/**
	 * Returns a pointer to the attribute with the given @a key
	 * or NULL if it does not exist.
	 */
	const attribute_value *get(const t_token &key) const;
	const attribute_value *get(const std::string &key) const;

	/**
	 * Function to handle backward compatibility
	 * Get the value of key and if missing try old_key
	 * and log msg as a WML error (if not empty)
	*/
	const attribute_value &get_old_attribute(const t_token &key, const t_token &old_key, const std::string& msg = "") const;
	const attribute_value &get_old_attribute(const std::string &key, const std::string &old_key, const std::string& msg = "") const;
	/**
	 * Returns a reference to the first child with the given @a key.
	 * Creates the child if it does not yet exist.
	 */
	config &child_or_add(const t_token &key);
	config &child_or_add(const std::string &key);

	bool has_attribute(const t_token &key) const;
	bool has_attribute(const std::string &key) const;
	/**
	 * Function to handle backward compatibility
	 * Check if has key or old_key
	 * and log msg as a WML error (if not empty)
	*/
	bool has_old_attribute(const t_token &key, const t_token &old_key, const std::string& msg = "") const;
	bool has_old_attribute(const std::string &key, const std::string &old_key, const std::string& msg = "") const;

	void remove_attribute(const t_token &key);
	void remove_attribute(const std::string &key);
	void merge_attributes(const config &);

	const_attr_itors attribute_range() const;

	/**
	 * Returns the first child of tag @a key with a @a name attribute
	 * containing @a value.
	 */
	config &find_child(const t_token &key, const t_token &name, const t_token &value);
	config &find_child(const std::string &key, const std::string &name, const std::string &value);

	const config &find_child(const t_token &key, const t_token &name, const t_token &value) const
	{ return const_cast<config *>(this)->find_child(key, name, value); }
	const config &find_child(const std::string &key, const std::string &name, const std::string &value) const
	{ return const_cast<config *>(this)->find_child(key, name, value); }

	void clear_children(const t_token& key);
	void clear_children(const std::string& key);

	/**
	 * Moves all the children with tag @a key from @a src to this.
	 */
	void splice_children(config &src, const t_token &key);
	void splice_children(config &src, const std::string &key);

	void remove_child(const t_token &key, unsigned index);
	void recursive_clear_value(const t_token& key);
	void remove_child(const std::string &key, unsigned index);
	void recursive_clear_value(const std::string& key);

	void clear();
	bool empty() const;

	std::string debug() const;
	std::string hash() const;

	struct error : public game::error {
		error(const std::string& message) : game::error(message) {}
	};

	struct child_pos
	{
		child_pos(child_map::iterator p, unsigned i) : pos(p), index(i) {}
		child_map::iterator pos;
		unsigned index;

		bool operator==(const child_pos& o) const { return pos == o.pos && index == o.index; }
		bool operator!=(const child_pos& o) const { return !operator==(o); }
	};

	struct any_child
	{
		const child_map::key_type &key;
		const config &cfg;
		any_child(const child_map::key_type *k, const config *c): key(*k), cfg(*c) {}
	};

	struct all_children_iterator
	{
		struct arrow_helper
		{
			any_child data;
			arrow_helper(const all_children_iterator &i): data(*i) {}
			const any_child *operator->() const { return &data; }
		};

		typedef any_child value_type;
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;
		typedef const arrow_helper pointer;
		typedef const any_child reference;
		typedef std::vector<child_pos>::const_iterator Itor;
		explicit all_children_iterator(const Itor &i): i_(i) {}

		all_children_iterator &operator++() { ++i_; return *this; }
		all_children_iterator operator++(int) { return all_children_iterator(i_++); }

		reference operator*() const;
		pointer operator->() const { return *this; }

		bool operator==(const all_children_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const all_children_iterator &i) const { return i_ != i.i_; }

	private:
		Itor i_;

		friend class config;
	};

	typedef std::pair<all_children_iterator, all_children_iterator> all_children_itors;

	/** In-order iteration over all children. */
	all_children_itors all_children_range() const;

	all_children_iterator ordered_begin() const;
	all_children_iterator ordered_end() const;
	all_children_iterator erase(const all_children_iterator& i);

	/**
	 * A function to get the differences between this object,
	 * and 'c', as another config object.
	 * I.e. calling cfg2.apply_diff(cfg1.get_diff(cfg2))
	 * will make cfg1 identical to cfg2.
	 */
	config get_diff(const config& c) const;
	void get_diff(const config& c, config& res) const;

	/**
	 * The name of the attribute used for tracking diff changes
	 */
	static const char* diff_track_attribute;

	/**
	 * A function to apply a diff config onto this config object.
	 *
	 * If the "track" parameter is true, the changes made will be marked in a
	 * magic attribute (defined above) of this and child nodes of this config,
	 * with "new" value indicating an added child, "modified" a modified one,
	 * and "deleted" for the deleted items, *which will not be actually
	 * deleted* (so calling code can easily see what they are).
	 * Use clear_diff_track with the same diff object to clear the tracking
	 * info and actually delete the nodes.
	 */
	void apply_diff(const config& diff, bool track = false); //throw error

	/**
	 * Clear any tracking info from a previous apply_diff call with tracking.
	 * This also removes the nodes that are to be deleted, in effect making
	 * apply_diff(c, true); clear_diff_tracking(c);
	 * equivalent to apply_diff(c, false);
	 */
	void clear_diff_track(const config& diff);

	/**
	 * Merge config 'c' into this config.
	 * Overwrites this config's values.
	 */
	void merge_with(const config& c);

	bool matches(const config &filter) const;

	/**
	 * Append data from another config object to this one.
	 * Attributes in the latter config object will clobber attributes in this one.
	 */
	void append(const config& cfg);

	/**
	 * Adds children from @a cfg.
	 */
	void append_children(const config &cfg);

	/**
	 * All children with the given key will be merged
	 * into the first element with that key.
	 */
	void merge_children(const t_token& key);
	void merge_children(const std::string& key);

	/**
	 * All children with the given key and with equal values
	 * of the specified attribute will be merged into the
	 * element with that key and that value of the attribute
	 */
	void merge_children_by_attribute(const t_token& key, const t_token& attribute);
	void merge_children_by_attribute(const std::string& key, const std::string& attribute);

	//this is a cheap O(1) operation
	void swap(config& cfg);

private:
	/**
	 * Removes the child at position @a pos of @a l.
	 */
	std::vector<child_pos>::iterator remove_child(const child_map::iterator &l, unsigned pos);

	/** All the attributes of this node. */
	attribute_map values;

	/** A list of all children of this node. */
	child_map children;

	std::vector<child_pos> ordered_children;
};

class variable_set
{
public:
	virtual ~variable_set() {}
	virtual config::attribute_value get_variable_const(const config::t_token &id) const = 0;
	virtual config::attribute_value get_variable_const(const std::string &id) const = 0;
};
template <typename X> void config::attribute_value::apply_visitor(X & visitor) const {
	switch(type_){
	case (EMPTY) : return visitor();
	case(BOOL) :   return visitor(bool_value_);
	case(INT) :       return visitor(int_value_);
	case(DOUBLE) :  return visitor(double_value_);
	case(TOKEN) :   return visitor(token_value_);
	case(TSTRING) :  return visitor(t_string_value_);
	default : assert(false);
	}
}

#endif
