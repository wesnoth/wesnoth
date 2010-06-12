/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file config.hpp
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
#include <string>
#include <vector>
#include <boost/variant/variant.hpp>

#include "game_errors.hpp"
#include "tstring.hpp"

typedef std::map<std::string, t_string> string_map;

class config;

bool operator==(const config &, const config &);
inline bool operator!=(const config &a, const config &b) { return !operator==(a, b); }
std::ostream &operator << (std::ostream &, const config &);

/** A config object defines a single node in a WML file, with access to child nodes. */
class config
{
	friend bool operator==(const config& a, const config& b);

	static config invalid;

	/**
	 * Raises an exception if @a this is not valid.
	 */
	void check_valid() const;

	/**
	 * Raises an exception if @a this or @a cfg is not valid.
	 */
	void check_valid(const config &cfg) const;

	struct safe_bool_impl { void nonnull() {} };
	/**
	 * Used as the return type of the conversion operator for boolean contexts.
	 * Needed, since the compiler would otherwise consider the following
	 * conversion (C legacy): cfg["abc"] -> "abc"[bool(cfg)] -> 'b'
	 */
	typedef void (safe_bool_impl::*safe_bool)();

public:
	// Create an empty node.
	config();

	config(const config& cfg);

	/**
	 * Creates a config object with an empty child of name @a child.
	 */
	explicit config(const std::string &child);

	~config();

	config& operator=(const config& cfg);

	operator safe_bool() const
	{ return this != &invalid ? &safe_bool_impl::nonnull : 0; }

	typedef std::vector<config*> child_list;
	typedef std::map<std::string,child_list> child_map;

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
	 * @note The blank variant is only used when querying missing attributes.
	 *       It is not stored in config objects.
	 */
	class attribute_value
	{
		typedef boost::variant<boost::blank, bool, int, double, std::string, t_string> value_type;
		value_type value;

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

		attribute_value &operator=(const char *v)
		{ return operator=(std::string(v)); }
		attribute_value &operator=(const std::string &v);
		attribute_value &operator=(const t_string &v);

		bool to_bool(bool def = false) const;
		int to_int(int def = 0) const;
		double to_double(double def = 0.) const;

		bool empty() const;
		std::string str() const;
		t_string t_str() const;

		operator int() const { return to_int(); }
		operator std::string() const { return str(); }
		operator t_string() const { return t_str(); }

		bool operator==(const attribute_value &other) const;
		bool operator!=(const attribute_value &other) const
		{ return !operator==(other); }

		friend std::ostream& operator<<(std::ostream &os, const attribute_value &v);
	};

	typedef std::map<std::string, attribute_value> attribute_map;
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

	child_itors child_range(const std::string& key);
	const_child_itors child_range(const std::string& key) const;
	size_t child_count(const std::string& key) const;

	/**
	 * Copies the first child with the given @a key, or an empty config if there is none.
	 */
	config child_or_empty(const std::string &key) const;

	/**
	 * Returns the nth child with the given @a key, or
	 * a reference to an invalid config if there is none.
	 */
	config &child(const std::string& key, int n = 0);

	/**
	 * Returns the nth child with the given @a key, or
	 * a reference to an invalid config if there is none.
	 */
	const config &child(const std::string& key, int n = 0) const
	{ return const_cast<config *>(this)->child(key, n); }

	config& add_child(const std::string& key);
	config& add_child(const std::string& key, const config& val);
	config& add_child_at(const std::string& key, const config& val, size_t index);

	/**
	 * Returns a reference to the attribute with the given @a key.
	 * Creates it if it does not exist.
	 */
	attribute_value &operator[](const std::string &key);

	/**
	 * Returns a reference to the attribute with the given @a key
	 * or to a dummy empty attribute if it does not exist.
	 */
	const attribute_value &operator[](const std::string &key) const;

	/**
	 * Function to handle backward compatibility
	 * Get the value of key and if missing try old_key
	 * and log msg as a WML error (if not empty)
	*/
	const attribute_value &get_old_attribute(const std::string &key, const std::string &old_key, const std::string& msg = "") const;
	/**
	 * Returns a reference to the first child with the given @a key.
	 * Creates the child if it does not yet exist.
	 */
	config &child_or_add(const std::string &key);

	bool has_attribute(const std::string &key) const;
	/**
	 * Function to handle backward compatibility
	 * Check if has key or old_key
	 * and log msg as a WML error (if not empty)
	*/
	bool has_old_attribute(const std::string &key, const std::string &old_key, const std::string& msg = "") const;

	void remove_attribute(const std::string &key);
	void merge_attributes(const config &);

	const_attr_itors attribute_range() const;

	config &find_child(const std::string& key, const std::string& name,
	                   const t_string& value);
	const config &find_child(const std::string& key, const std::string& name,
	                         const t_string& value) const;
	const config &find_child_recursive(const std::string& key, const std::string& name,
	                         const t_string& value) const;

	void clear_children(const std::string& key);
	void remove_child(const std::string& key, size_t index);
	void recursive_clear_value(const std::string& key);

	void clear();
	bool empty() const;

	std::string debug() const;
	std::string hash() const;

	struct error : public game::error {
		error(const std::string& message) : game::error(message) {}
	};

	struct child_pos {
		child_pos(child_map::const_iterator p, size_t i) : pos(p), index(i) {}
		child_map::const_iterator pos;
		size_t index;

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
		typedef any_child value_type;
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;
		typedef any_child *pointer;
		typedef any_child reference;
		typedef std::vector<child_pos>::const_iterator Itor;
		explicit all_children_iterator(const Itor &i): i_(i) {}

		all_children_iterator &operator++() { ++i_; return *this; }
		all_children_iterator operator++(int) { return all_children_iterator(i_++); }

		struct arrow_helper
		{
			any_child data;
			arrow_helper(const all_children_iterator &i): data(*i) {}
			any_child *operator->() { return &data; }
		};

		any_child operator*() const;
		arrow_helper operator->() const { return *this; }

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
	 * All children with the given key will be merged
	 * into the first element with that key.
	 */
	void merge_children(const std::string& key);

	/**
	 * All children with the given key and with equal values
	 * of the specified attribute will be merged into the
	 * element with that key and that value of the attribute
	 */
	void merge_children_by_attribute(const std::string& key, const std::string& attribute);

	//this is a cheap O(1) operation
	void swap(config& cfg);

private:
	/** All the attributes of this node. */
	attribute_map values;

	/** A list of all children of this node. */
	child_map children;

	std::vector<child_pos> ordered_children;
};

#endif
