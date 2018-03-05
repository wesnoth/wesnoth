/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#pragma once

#include "global.hpp"

#include <climits>
#include <ctime>
#include <functional>
#include <iosfwd>
#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <type_traits>
#include <memory>

#include <boost/exception/exception.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>
#include <boost/range/iterator_range.hpp>

#include "config_attribute_value.hpp"
#include "exceptions.hpp"
#include "tstring.hpp"

#ifdef HAVE_CXX14
#	ifdef __clang__ // Check this first, because clang also defines __GNUC__
#		ifdef __apple_build_version__ // Apple clang
#			if (__clang_major__ == 5 && __clang_minor__ >= 1) || __clang_major__ > 5 // Apple clang 5.1+
#				define USE_HETEROGENOUS_LOOKUPS
#			endif
#		else // Non-Apple clang
#			if (__clang_major__ == 3 && __clang_minor__ >= 4) || __clang_major__ > 3 // clang 3.4+
#				define USE_HETEROGENOUS_LOOKUPS
#			endif
#		endif
#	elif defined(__GNUC__) && __GNUC__ >= 5 // GCC 5.0+
#		define USE_HETEROGENOUS_LOOKUPS
#	endif
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1900 // MSVC 2015
#	define USE_HETEROGENOUS_LOOKUPS
#endif

#ifdef USE_HETEROGENOUS_LOOKUPS
#if BOOST_VERSION > 106100
#include <boost/utility/string_view.hpp>
using config_key_type = boost::string_view;
#else
#include <boost/utility/string_ref.hpp>
using config_key_type = boost::string_ref;
#endif
#else
using config_key_type = const std::string &;
#endif

class config;
class enum_tag;

bool operator==(const config &, const config &);
inline bool operator!=(const config &a, const config &b) { return !operator==(a, b); }
std::ostream &operator << (std::ostream &, const config &);

/** A config object defines a single node in a WML file, with access to child nodes. */
class config
{
	friend bool operator==(const config& a, const config& b);
	friend struct config_implementation;

	static config invalid;

	/**
	 * Raises an exception if @a this is not valid.
	 */
	void check_valid() const;

	/**
	 * Raises an exception if @a this or @a cfg is not valid.
	 */
	void check_valid(const config &cfg) const;

public:
	// Create an empty node.
	config();

	config(const config &);
	config &operator=(const config &);

	config(config &&);
	config &operator=(config &&);

	/**
	 * Creates a config object with an empty child of name @a child.
	 */
	explicit config(config_key_type child);

	/**
	 * Creates a config with several attributes and children.
	 * Pass the keys/tags and values/children alternately.
	 * @example config("key", 42, "value", config())
	 */
	template<typename... T>
	explicit config(config_key_type first, T&&... args);

	~config();

	// Verifies that the string can be used as a tag name
	static bool valid_tag(config_key_type name);

	// Verifies that the string can be used as an attribute name
	static bool valid_attribute(config_key_type name);

	explicit operator bool() const
	{ return this != &invalid; }

	typedef std::vector<std::unique_ptr<config>> child_list;
	typedef std::map<std::string, child_list
#ifdef USE_HETEROGENOUS_LOOKUPS
		, std::less<>
#endif
	> child_map;

	struct const_child_iterator;

	struct child_iterator
	{
		typedef config value_type;
		typedef std::random_access_iterator_tag iterator_category;
		typedef int difference_type;
		typedef config *pointer;
		typedef config &reference;
		typedef child_list::iterator Itor;
		typedef child_iterator this_type;
		explicit child_iterator(const Itor &i): i_(i) {}

		child_iterator &operator++() { ++i_; return *this; }
		child_iterator operator++(int) { return child_iterator(i_++); }
		child_iterator &operator--() { --i_; return *this; }
		child_iterator operator--(int) { return child_iterator(i_--); }

		reference operator*() const { return **i_; }
		pointer operator->() const { return &**i_; }

		bool operator==(const child_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const child_iterator &i) const { return i_ != i.i_; }
		bool operator==(const const_child_iterator &i) const { return i == *this; }
		bool operator!=(const const_child_iterator &i) const { return i != *this; }

		friend bool operator<(const this_type& a, const this_type& b) { return a.i_ < b.i_; }
		friend bool operator<=(const this_type& a, const this_type& b) { return a.i_ <= b.i_; }
		friend bool operator>=(const this_type& a, const this_type& b) { return a.i_ >= b.i_; }
		friend bool operator>(const this_type& a, const this_type& b) { return a.i_ > b.i_; }

		this_type& operator+=(Itor::difference_type n) { i_ += n; return *this; }
		this_type& operator-=(Itor::difference_type n) { i_ -= n; return *this; }

		config &operator[](Itor::difference_type n) const { return *i_[n]; }
		friend Itor::difference_type operator-(const this_type& a, const this_type& b) { return a.i_ - b.i_; }
		friend this_type operator-(const this_type& a, Itor::difference_type n) { return this_type(a.i_ - n); }
		friend this_type operator+(const this_type& a, Itor::difference_type n) { return this_type(a.i_ + n); }
		friend this_type operator+(Itor::difference_type n, const this_type& a) { return this_type(a.i_ + n); }
	private:
		Itor i_;
		friend struct const_child_iterator;
	};

	struct const_child_iterator
	{
		typedef const config value_type;
		typedef std::random_access_iterator_tag iterator_category;
		typedef int difference_type;
		typedef const config *pointer;
		typedef const config &reference;
		typedef child_list::const_iterator Itor;
		typedef const_child_iterator this_type;
		explicit const_child_iterator(const Itor &i): i_(i) {}
		const_child_iterator(const child_iterator &i): i_(i.i_) {}

		const_child_iterator &operator++() { ++i_; return *this; }
		const_child_iterator operator++(int) { return const_child_iterator(i_++); }
		const_child_iterator &operator--() { --i_; return *this; }
		const_child_iterator operator--(int) { return const_child_iterator(i_--); }

		reference operator*() const { return **i_; }
		pointer operator->() const { return &**i_; }

		bool operator==(const const_child_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const const_child_iterator &i) const { return i_ != i.i_; }
		bool operator==(const child_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const child_iterator &i) const { return i_ != i.i_; }

		friend bool operator<(const this_type& a, const this_type& b) { return a.i_ < b.i_; }
		friend bool operator<=(const this_type& a, const this_type& b) { return a.i_ <= b.i_; }
		friend bool operator>=(const this_type& a, const this_type& b) { return a.i_ >= b.i_; }
		friend bool operator>(const this_type& a, const this_type& b) { return a.i_ > b.i_; }

		this_type& operator+=(Itor::difference_type n) { i_ += n; return *this; }
		this_type& operator-=(Itor::difference_type n) { i_ -= n; return *this; }

		const config &operator[](Itor::difference_type n) const { return *i_[n]; }
		friend Itor::difference_type operator-(const this_type& a, const this_type& b) { return a.i_ - b.i_; }
		friend this_type operator-(const this_type& a, Itor::difference_type n) { return this_type(a.i_ - n); }
		friend this_type operator+(const this_type& a, Itor::difference_type n) { return this_type(a.i_ + n); }
		friend this_type operator+(Itor::difference_type n, const this_type& a) { return this_type(a.i_ + n); }

	private:
		Itor i_;
	};

	typedef boost::iterator_range<child_iterator> child_itors;
	typedef boost::iterator_range<const_child_iterator> const_child_itors;

	/**
	 * Variant for storing WML attributes.
	 * The most efficient type is used when assigning a value. For instance,
	 * strings "yes", "no", "true", "false" will be detected and stored as boolean.
	 * @note The blank variant is only used when querying missing attributes.
	 *       It is not stored in config objects.
	 */
	using attribute_value = config_attribute_value;

	typedef std::map<
		std::string
		, attribute_value
#ifdef USE_HETEROGENOUS_LOOKUPS
		, std::less<>
#endif
	> attribute_map;
	typedef attribute_map::value_type attribute;
	struct const_attribute_iterator;

	struct attribute_iterator
	{
		typedef attribute value_type;
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef int difference_type;
		typedef attribute *pointer;
		typedef attribute &reference;
		typedef attribute_map::iterator Itor;
		explicit attribute_iterator(const Itor &i): i_(i) {}

		attribute_iterator &operator++() { ++i_; return *this; }
		attribute_iterator operator++(int) { return attribute_iterator(i_++); }
		attribute_iterator &operator--() { --i_; return *this; }
		attribute_iterator operator--(int) { return attribute_iterator(i_--); }

		reference operator*() const { return *i_; }
		pointer operator->() const { return &*i_; }

		bool operator==(const attribute_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const attribute_iterator &i) const { return i_ != i.i_; }
		bool operator==(const const_attribute_iterator &i) const { return i == *this; }
		bool operator!=(const const_attribute_iterator &i) const { return i != *this; }

	private:
		friend struct config::const_attribute_iterator;
		Itor i_;
	};

	struct const_attribute_iterator
	{
		typedef const attribute value_type;
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef int difference_type;
		typedef const attribute *pointer;
		typedef const attribute &reference;
		typedef attribute_map::const_iterator Itor;
		explicit const_attribute_iterator(const Itor &i): i_(i) {}
		const_attribute_iterator(attribute_iterator& i): i_(i.i_) {}

		const_attribute_iterator &operator++() { ++i_; return *this; }
		const_attribute_iterator operator++(int) { return const_attribute_iterator(i_++); }

		const_attribute_iterator &operator--() { --i_; return *this; }
		const_attribute_iterator operator--(int) { return const_attribute_iterator(i_--); }

		reference operator*() const { return *i_; }
		pointer operator->() const { return &*i_; }

		bool operator==(const const_attribute_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const const_attribute_iterator &i) const { return i_ != i.i_; }
		bool operator==(const attribute_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const attribute_iterator &i) const { return i_ != i.i_; }

	private:
		Itor i_;
	};

	typedef boost::iterator_range<const_attribute_iterator> const_attr_itors;
	typedef boost::iterator_range<attribute_iterator> attr_itors;

	child_itors child_range(config_key_type key);
	const_child_itors child_range(config_key_type key) const;
	unsigned child_count(config_key_type key) const;
	unsigned all_children_count() const;
	/** Count the number of non-blank attributes */
	unsigned attribute_count() const;

	/**
	 * Determine whether a config has a child or not.
	 *
	 * @param key                 The key of the child to find.
	 *
	 * @returns                   Whether a child is available.
	 */
	bool has_child(config_key_type key) const;

	/**
	 * Returns the first child with the given @a key, or an empty config if there is none.
	 */
	const config & child_or_empty(config_key_type key) const;

	/**
	 * Returns the nth child with the given @a key, or
	 * a reference to an invalid config if there is none.
	 * @note A negative @a n accesses from the end of the object.
	 *       For instance, -1 is the index of the last child.
	 */
	config &child(config_key_type key, int n = 0);

	/**
	 * Returns the nth child with the given @a key, or
	 * a reference to an invalid config if there is none.
	 * @note A negative @a n accesses from the end of the object.
	 *       For instance, -1 is the index of the last child.
	 */
	const config& child(config_key_type key, int n = 0) const
	{ return const_cast<config *>(this)->child(key, n); }
	/**
	 * Returns a mandatory child node.
	 *
	 * If the child is not found a @ref wml_exception is thrown.
	 *
	 * @pre                       parent[0] == '['
	 * @pre                       parent[parent.size() - 1] == ']'
	 *
	 * @param key                 The key of the child item to return.
	 * @param parent              The section in which the child should reside.
	 *                            This is only used for error reporting.
	 *
	 * @returns                   The wanted child node.
	 */
	config& child(config_key_type key, const std::string& parent);

	/**
	 * Returns a mandatory child node.
	 *
	 * If the child is not found a @ref wml_exception is thrown.
	 *
	 * @pre                       parent[0] == '['
	 * @pre                       parent[parent.size() - 1] == ']'
	 *
	 * @param key                 The key of the child item to return.
	 * @param parent              The section in which the child should reside.
	 *                            This is only used for error reporting.
	 *
	 * @returns                   The wanted child node.
	 */
	const config& child(
		config_key_type key
			, const std::string& parent) const;

	config& add_child(config_key_type key);
	config& add_child(config_key_type key, const config& val);
	config& add_child_at(config_key_type key, const config &val, unsigned index);

	config &add_child(config_key_type key, config &&val);

	/**
	 * Returns a reference to the attribute with the given @a key.
	 * Creates it if it does not exist.
	 */
	attribute_value& operator[](config_key_type key);

	/**
	 * Returns a reference to the attribute with the given @a key
	 * or to a dummy empty attribute if it does not exist.
	 */
	const attribute_value& operator[](config_key_type key) const;

#ifdef USE_HETEROGENOUS_LOOKUPS
	/**
	* Returns a reference to the attribute with the given @a key.
	* Creates it if it does not exist.
	*/
	attribute_value& operator[](const std::string& key)
	{
		return operator[](config_key_type(key));
	}

	/**
	* Returns a reference to the attribute with the given @a key
	* or to a dummy empty attribute if it does not exist.
	*/
	const attribute_value& operator[](const std::string& key) const
	{
		return operator[](config_key_type(key));
	}
#endif

	/**
	* Returns a reference to the attribute with the given @a key.
	* Creates it if it does not exist.
	*/
	attribute_value& operator[](const char* key)
	{
		return operator[](config_key_type(key));
	}

	/**
	* Returns a reference to the attribute with the given @a key
	* or to a dummy empty attribute if it does not exist.
	*/
	const attribute_value& operator[](const char* key) const
	{
		return operator[](config_key_type(key));
	}

	/**
	 * Returns a pointer to the attribute with the given @a key
	 * or nullptr if it does not exist.
	 */
	const attribute_value *get(config_key_type key) const;

	/**
	 * Function to handle backward compatibility
	 * Get the value of key and if missing try old_key
	 * and log msg as a WML error (if not empty)
	*/
	const attribute_value &get_old_attribute(config_key_type key, const std::string &old_key, const std::string& in_tag = "") const;
	/**
	 * Returns a reference to the first child with the given @a key.
	 * Creates the child if it does not yet exist.
	 */

	/**
	 * Inserts an attribute into the config
	 * @param key The name of the attribute
	 * @param value The attribute value
	 */
	template<typename T>
	void insert(config_key_type key, T&& value)
	{
		operator[](key) = std::forward<T>(value);
	}

	config &child_or_add(config_key_type key);

	bool has_attribute(config_key_type key) const;
	/**
	 * Function to handle backward compatibility
	 * Check if has key or old_key
	 * and log msg as a WML error (if not empty)
	*/
	bool has_old_attribute(config_key_type key, const std::string &old_key, const std::string& msg = "") const;

	void remove_attribute(config_key_type key);
	void merge_attributes(const config &);
	template<typename... T>
	void remove_attributes(T... keys) {
		for(const std::string& key : {keys...}) {
			remove_attribute(key);
		}
	}

	const_attr_itors attribute_range() const;
	attr_itors attribute_range();

	/**
	 * Returns the first child of tag @a key with a @a name attribute
	 * containing @a value.
	 */
	config& find_child(config_key_type key, const std::string &name,
		const std::string &value);

	const config& find_child(config_key_type key, const std::string &name,
		const std::string &value) const
	{ return const_cast<config *>(this)->find_child(key, name, value); }

	void clear_children_impl(config_key_type key);
	template<typename... T>
	void clear_children(T... keys) {
		for(auto key : {keys...}) {
			clear_children_impl(key);
		}
	}

	/**
	 * Moves all the children with tag @a key from @a src to this.
	 */
	void splice_children(config &src, const std::string &key);

	void remove_child(config_key_type key, unsigned index);
	/**
	 * Removes all children with tag @a key for which @a p returns true.
	 */
	void remove_children(config_key_type key, std::function<bool(const config&)> p);
	void recursive_clear_value(config_key_type key);

	void clear();
	void clear_all_children();
	void clear_attributes();
	bool empty() const;

	std::string debug() const;
	std::string hash() const;

	struct error : public game::error, public boost::exception {
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
		config &cfg;
		any_child(const child_map::key_type *k, config *c): key(*k), cfg(*c) {}
	};

	struct const_all_children_iterator;

	struct all_children_iterator
	{
		struct arrow_helper
		{
			any_child data;
			arrow_helper(const all_children_iterator &i): data(*i) {}
			const any_child *operator->() const { return &data; }
		};

		typedef any_child value_type;
		typedef std::random_access_iterator_tag iterator_category;
		typedef int difference_type;
		typedef arrow_helper pointer;
		typedef any_child reference;
		typedef std::vector<child_pos>::iterator Itor;
		typedef all_children_iterator this_type;
		explicit all_children_iterator(const Itor &i): i_(i) {}

		all_children_iterator &operator++() { ++i_; return *this; }
		all_children_iterator operator++(int) { return all_children_iterator(i_++); }
		this_type &operator--() { --i_; return *this; }
		this_type operator--(int) { return this_type(i_--); }

		reference operator*() const;
		pointer operator->() const { return *this; }

		bool operator==(const all_children_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const all_children_iterator &i) const { return i_ != i.i_; }
		bool operator==(const const_all_children_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const const_all_children_iterator &i) const { return i_ != i.i_; }

		friend bool operator<(const this_type& a, const this_type& b) { return a.i_ < b.i_; }
		friend bool operator<=(const this_type& a, const this_type& b) { return a.i_ <= b.i_; }
		friend bool operator>=(const this_type& a, const this_type& b) { return a.i_ >= b.i_; }
		friend bool operator>(const this_type& a, const this_type& b) { return a.i_ > b.i_; }

		this_type& operator+=(difference_type n) { i_ += n; return *this; }
		this_type& operator-=(difference_type n) { i_ -= n; return *this; }

		reference operator[](difference_type n) const { return any_child(&i_[n].pos->first, i_[n].pos->second[i_->index].get()); }
		friend difference_type operator-(const this_type& a, const this_type& b) { return a.i_ - b.i_; }
		friend this_type operator-(const this_type& a, difference_type n) { return this_type(a.i_ - n); }
		friend this_type operator+(const this_type& a, difference_type n) { return this_type(a.i_ + n); }
		friend this_type operator+(difference_type n, const this_type& a) { return this_type(a.i_ + n); }

	private:
		Itor i_;

		friend class config;
		friend struct const_all_children_iterator;
	};

	struct const_all_children_iterator
	{
		struct arrow_helper
		{
			const any_child data;
			arrow_helper(const const_all_children_iterator &i): data(*i) {}
			const any_child *operator->() const { return &data; }
		};

		typedef const any_child value_type;
		typedef std::random_access_iterator_tag iterator_category;
		typedef int difference_type;
		typedef const arrow_helper pointer;
		typedef const any_child reference;
		typedef std::vector<child_pos>::const_iterator Itor;
		typedef const_all_children_iterator this_type;
		explicit const_all_children_iterator(const Itor &i): i_(i) {}
		const_all_children_iterator(all_children_iterator& i): i_(i.i_) {}

		const_all_children_iterator &operator++() { ++i_; return *this; }
		const_all_children_iterator operator++(int) { return const_all_children_iterator(i_++); }
		this_type &operator--() { --i_; return *this; }
		this_type operator--(int) { return this_type(i_--); }

		reference operator*() const;
		pointer operator->() const { return *this; }

		bool operator==(const const_all_children_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const const_all_children_iterator &i) const { return i_ != i.i_; }
		bool operator==(const all_children_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const all_children_iterator &i) const { return i_ != i.i_; }

		friend bool operator<(const this_type& a, const this_type& b) { return a.i_ < b.i_; }
		friend bool operator<=(const this_type& a, const this_type& b) { return a.i_ <= b.i_; }
		friend bool operator>=(const this_type& a, const this_type& b) { return a.i_ >= b.i_; }
		friend bool operator>(const this_type& a, const this_type& b) { return a.i_ > b.i_; }

		this_type& operator+=(difference_type n) { i_ += n; return *this; }
		this_type& operator-=(difference_type n) { i_ -= n; return *this; }

		reference operator[](difference_type n) const { return any_child(&i_[n].pos->first, i_[n].pos->second[i_->index].get()); }
		friend difference_type operator-(const this_type& a, const this_type& b) { return a.i_ - b.i_; }
		friend this_type operator-(const this_type& a, difference_type n) { return this_type(a.i_ - n); }
		friend this_type operator+(const this_type& a, difference_type n) { return this_type(a.i_ + n); }
		friend this_type operator+(difference_type n, const this_type& a) { return this_type(a.i_ + n); }

	private:
		Itor i_;

		friend class config;
	};

	typedef boost::iterator_range<all_children_iterator> all_children_itors;
	typedef boost::iterator_range<const_all_children_iterator> const_all_children_itors;

	/** In-order iteration over all children. */
	const_all_children_itors all_children_range() const;
	all_children_itors all_children_range();

	const_all_children_iterator ordered_cbegin() const;
	const_all_children_iterator ordered_cend() const;
	const_all_children_iterator ordered_begin() const;
	const_all_children_iterator ordered_end() const;
	all_children_iterator ordered_begin();
	all_children_iterator ordered_end();
	all_children_iterator erase(const all_children_iterator& i);

	/**
	 * A function to get the differences between this object,
	 * and 'c', as another config object.
	 * I.e. calling cfg2.apply_diff(cfg1.get_diff(cfg2))
	 * will make cfg2 identical to cfg1.
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
	 * Merge config 'c' into this config, overwriting this config's values.
	 */
	void merge_with(const config& c);

	/**
	 * Merge config 'c' into this config, preserving this config's values.
	 */
	void inherit_from(const config& c);

	bool matches(const config &filter) const;

	/**
	 * Append data from another config object to this one.
	 * Attributes in the latter config object will clobber attributes in this one.
	 */
	void append(const config& cfg);
	void append(config&& cfg);

	/**
	 * Adds children from @a cfg.
	 */
	void append_children(const config &cfg);
	void append_children(config&& cfg);

	/**
	 * Adds children from @a cfg.
	 */
	void append_children(const config &cfg, const std::string& key);

	/** Moves children with the given name from the given config to this one. */
	void append_children_by_move(config& cfg, const std::string& key);

	/**
	 * Adds attributes from @a cfg.
	 */
	void append_attributes(const config &cfg);

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

	/**
	 * Returns true if this object represents valid WML,
	 * i.e. can be saved to disk and again loaded by the WML parser.
	 */
	bool validate_wml() const;

private:
	/**
	 * Removes the child at position @a pos of @a l.
	 */
	std::vector<child_pos>::iterator remove_child(const child_map::iterator &l, unsigned pos);

	/** All the attributes of this node. */
	attribute_map values_;

	/** A list of all children of this node. */
	child_map children_;

	std::vector<child_pos> ordered_children;
};

/** Implement non-member swap function for std::swap (calls @ref config::swap). */
void swap(config& lhs, config& rhs);

namespace detail {
	template<typename... T>
	struct config_construct_unpacker;

	template<>
	struct config_construct_unpacker<>
	{
		void visit(config&) {}
	};

	template<typename K, typename V, typename... Rest>
	struct config_construct_unpacker<K, V, Rest...>
	{
		template<typename K2 = K, typename V2 = V>
		void visit(config& cfg, K2&& key, V2&& val, Rest... fwd)
		{
			cfg.insert(std::forward<K>(key), std::forward<V>(val));
			config_construct_unpacker<Rest...> unpack;
			unpack.visit(cfg, std::forward<Rest>(fwd)...);
		}
	};

	template<typename T, typename... Rest>
	struct config_construct_unpacker<T, config, Rest...>
	{
		template<typename T2 = T, typename C = config>
		void visit(config& cfg, T2&& tag, C&& child, Rest... fwd)
		{
			cfg.add_child(std::forward<T>(tag), std::forward<config>(child));
			config_construct_unpacker<Rest...> unpack;
			unpack.visit(cfg, std::forward<Rest>(fwd)...);
		}
	};

	template<typename T, typename... Rest>
	struct config_construct_unpacker<T, config&, Rest...>
	{
		template<typename T2 = T>
		void visit(config& cfg, T2&& tag, config& child, Rest... fwd)
		{
			cfg.add_child(std::forward<T>(tag), std::forward<config>(child));
			config_construct_unpacker<Rest...> unpack;
			unpack.visit(cfg, std::forward<Rest>(fwd)...);
		}
	};
}

template<typename... T>
inline config::config(config_key_type first, T&&... args)
{
	detail::config_construct_unpacker<config_key_type, T...> unpack;
	unpack.visit(*this, first, std::forward<T>(args)...);
}

class variable_set
{
public:
	virtual ~variable_set() {}
	virtual config::attribute_value get_variable_const(const std::string &id) const = 0;
};

