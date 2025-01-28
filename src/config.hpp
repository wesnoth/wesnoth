/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "config_attribute_value.hpp"
#include "exceptions.hpp"
#include "utils/const_clone.hpp"
#include "utils/optional_reference.hpp"
#include "utils/ranges.hpp"

#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <boost/range/iterator_range.hpp>

using config_key_type = std::string_view;
enum class DEP_LEVEL : uint8_t;

class config;

template<class T>
class optional_config_impl
{
public:
	optional_config_impl() = default;

	optional_config_impl(T& ref)
		: opt_(&ref)
	{
	}

	optional_config_impl(utils::nullopt_t)
		: opt_()
	{
	}

	T& value() const
	{
		if(opt_) {
			return *opt_;
		} else {
			// We're going to drop this codepath once we can use optional::value anyway, but just
			// noting we want this function to ultimately throw utils::bad_optional_access.
			throw std::runtime_error("Optional reference has no value");
		}
	}

	optional_config_impl<T>& operator=(T& new_ref)
	{
		opt_ = &new_ref;
		return *this;
	}

	bool has_value() const
	{
#ifdef DEBUG_CONFIG
		tested_ = true;
#endif
		return opt_ != nullptr;

	}

	explicit operator bool() const
	{
		return has_value();
	}

	operator optional_config_impl<const T>() const
	{
		return opt_ ? optional_config_impl<const T>(*opt_) : optional_config_impl<const T>();
	}

	/** Returns a pointer to the referenced object or nullptr if no reference is held. */
	T* ptr() const
	{
		if(opt_) {
			return &value();
		} else {
			return nullptr;
		}
	}

	T* operator->() const
	{
		assert(tested());
		return &value();
	}

	T& operator*() const
	{
		assert(tested());
		return value();
	}

	utils::const_clone_t<config_attribute_value, T>& operator[](config_key_type key)
	{
		assert(tested());
		return value()[key];
	}

	operator utils::optional_reference<T>() const
	{
		return has_value() ? utils::optional_reference<T>(value()) : utils::optional_reference<T>();
	}
	bool tested() const
	{
#ifdef DEBUG_CONFIG
		return tested_;
#else
		return true;
#endif
	}
private:
	T* opt_;
#ifdef DEBUG_CONFIG
	mutable bool tested_;
#endif
};

bool operator==(const config &, const config &);
inline bool operator!=(const config &a, const config &b) { return !operator==(a, b); }
std::ostream &operator << (std::ostream &, const config &);

/** A config object defines a single node in a WML file, with access to child nodes. */
class config
{
	friend bool operator==(const config& a, const config& b);
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
	 * For example: config("key", 42, "value", config())
	 */
	template<typename... Args>
	explicit config(config_key_type first, Args&&... args);

	~config();

	// Verifies that the string can be used as a tag name
	static bool valid_tag(config_key_type name);

	// Verifies that the string can be used as an attribute name
	static bool valid_attribute(config_key_type name);

	typedef std::vector<std::unique_ptr<config>> child_list;
	typedef std::map<std::string, child_list, std::less<>> child_map;

	struct const_child_iterator;

	struct child_iterator
	{
		typedef config value_type;
		typedef std::random_access_iterator_tag iterator_category;
		typedef config *pointer;
		typedef config &reference;
		typedef child_list::iterator Itor;
		typedef Itor::difference_type difference_type;
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
		typedef const config *pointer;
		typedef const config &reference;
		typedef child_list::const_iterator Itor;
		typedef Itor::difference_type difference_type;
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
		, std::less<>
	> attribute_map;
	typedef attribute_map::value_type attribute;
	struct const_attribute_iterator;

	struct attribute_iterator
	{
		typedef attribute value_type;
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef attribute *pointer;
		typedef attribute &reference;
		typedef attribute_map::iterator Itor;
		typedef Itor::difference_type difference_type;
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
		typedef const attribute *pointer;
		typedef const attribute &reference;
		typedef attribute_map::const_iterator Itor;
		typedef Itor::difference_type difference_type;
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
	std::size_t child_count(config_key_type key) const;
	std::size_t all_children_count() const;
	/** Count the number of non-blank attributes */
	std::size_t attribute_count() const;

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
	 * throws an error if there is none.
	 * @note A negative @a n accesses from the end of the object.
	 *       For instance, -1 is the index of the last child.
	 */

	config& mandatory_child(config_key_type key, int n = 0);
	/**
	 * Returns the nth child with the given @a key, or
	 * throws an error if there is none.
	 * @note A negative @a n accesses from the end of the object.
	 *       For instance, -1 is the index of the last child.
	 */
	const config& mandatory_child(config_key_type key, int n = 0) const;

	/** Equivalent to @ref mandatory_child, but returns an empty optional if the nth child was not found. */
	optional_config_impl<config> optional_child(config_key_type key, int n = 0);

	/** Equivalent to @ref mandatory_child, but returns an empty optional if the nth child was not found. */
	optional_config_impl<const config> optional_child(config_key_type key, int n = 0) const;

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
	config& mandatory_child(config_key_type key, const std::string& parent);

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
	const config& mandatory_child(config_key_type key, const std::string& parent) const;

	/**
	 * Get a deprecated child and log a deprecation message
	 * @param old_key The deprecated child to return if present
	 * @param in_tag The name of the tag this child appears in
	 * @param level The deprecation level
	 * @param message An explanation of the deprecation, possibly mentioning an alternative
	 * @note The deprecation message will be a level 3 deprecation.
	 */
	optional_config_impl<const config> get_deprecated_child(config_key_type old_key, const std::string& in_tag, DEP_LEVEL level, const std::string& message) const;

	/**
	 * Get a deprecated child range and log a deprecation message
	 * @param old_key The deprecated child to return if present
	 * @param in_tag The name of the tag this child appears in
	 * @param level The deprecation level
	 * @param message An explanation of the deprecation, possibly mentioning an alternative
	 * @note The deprecation message will be a level 3 deprecation.
	 */
	const_child_itors get_deprecated_child_range(config_key_type old_key, const std::string& in_tag, DEP_LEVEL level, const std::string& message) const;

	config& add_child(config_key_type key);
	config& add_child(config_key_type key, const config& val);
	/**
	 * @param key the tag name
	 * @param val the contents of the tag
	 * @param index is the index of the new child within all children of type key.
	 */
	config& add_child_at(config_key_type key, const config &val, std::size_t index);

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

	/**
	 * Returns a pointer to the attribute with the given @a key
	 * or nullptr if it does not exist.
	 */
	const attribute_value *get(config_key_type key) const;

    /**
     * Chooses a value. If the value specified by @a key is
     * blank, then @a default_key is chosen instead.
     * If both values are blank or not set, then an empty value is returned.
     */
    const attribute_value& get_or(const config_key_type key, const config_key_type default_key) const;

	/**
	 * Function to handle backward compatibility
	 * Get the value of key and if missing try old_key
	 * and log a deprecation message
	 * @param key The non-deprecated attribute to return
	 * @param old_key The deprecated attribute to return if @a key is missing
	 * @param in_tag The name of the tag these attributes appear in
	 * @param message An explanation of the deprecation, to be output if @a old_key is present.
	 * @note The deprecation message will be a level 1 deprecation.
	*/
	const attribute_value &get_old_attribute(config_key_type key, const std::string &old_key, const std::string& in_tag, const std::string& message = "") const;

	/**
	 * Get a deprecated attribute without a direct substitute,
	 * and log a deprecation message
	 * @param old_key The deprecated attribute to return if present
	 * @param in_tag The name of the tag this attribute appears in
	 * @param level The deprecation level
	 * @param message An explanation of the deprecation, possibly mentioning an alternative
	 */
	const attribute_value& get_deprecated_attribute(config_key_type old_key, const std::string& in_tag, DEP_LEVEL level, const std::string& message) const;

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

	/**
	 * Returns a reference to the first child with the given @a key.
	 * Creates the child if it does not yet exist.
	 */
	config &child_or_add(config_key_type key);

	bool has_attribute(config_key_type key) const;

	void remove_attribute(config_key_type key);
	void merge_attributes(const config &);

	template<typename... T>
	void remove_attributes(T... keys) { (remove_attribute(keys), ...); }

	/**
	 * Copies attributes that exist in the source config.
	 *
	 * @param from Source config to copy attributes from.
	 * @param keys Attribute names.
	 */
	template<typename... T>
	void copy_attributes(const config& from, T... keys)
	{
		for(const auto& key : {keys...}) {
			auto* attr = from.get(key);
			if(attr) {
				(*this)[key] = *attr;
			}
		}
	}

	/**
	 * Copies or deletes attributes to match the source config.
	 *
	 * Attributes that do not exist in the source are fully erased rather than
	 * set to the unspecified/default attribute value.
	 *
	 * @param from Source config to copy attributes from.
	 * @param keys Attribute names.
	 */
	template<typename... T>
	void copy_or_remove_attributes(const config& from, T... keys)
	{
		for(const auto& key : {keys...}) {
			if(from.has_attribute(key)) {
				(*this)[key] = from[key];
			} else {
				remove_attribute(key);
			}
		}
	}

	const_attr_itors attribute_range() const;
	attr_itors attribute_range();

	/**
	 * Returns the first child of tag @a key with a @a name attribute
	 * containing @a value.
	 */
	optional_config_impl<config> find_child(config_key_type key, const std::string &name,
		const std::string &value);

	optional_config_impl<const config> find_child(config_key_type key, const std::string &name,
		const std::string &value) const
	{ return const_cast<config *>(this)->find_child(key, name, value); }

	config& find_mandatory_child(config_key_type key, const std::string &name,
		const std::string &value);

	const config& find_mandatory_child(config_key_type key, const std::string &name,
		const std::string &value) const;

private:
	void clear_children_impl(config_key_type key);

public:
	template<typename... T>
	void clear_children(T... keys) { (clear_children_impl(keys), ...); }

	/**
	 * Moves all the children with tag @a key from @a src to this.
	 */
	void splice_children(config& src, config_key_type key);

	void remove_child(config_key_type key, std::size_t index);

	/**
	 * Removes all children with tag @a key for which @a p returns true.
	 * If no predicate is provided, all @a key tags will be removed.
	 */
	void remove_children(config_key_type key, const std::function<bool(const config&)>& p = {});

	void recursive_clear_value(config_key_type key);

	void clear();
	void clear_all_children();
	void clear_attributes();
	bool empty() const;

	std::string debug() const;
	std::string hash() const;

	struct error : public game::error {
		error(const std::string& message) : game::error(message) {}
	};

	struct child_pos
	{
		child_pos(child_map::iterator p, std::size_t i) : pos(p), index(i) {}
		child_map::iterator pos;
		std::size_t index;

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
		typedef arrow_helper pointer;
		typedef any_child reference;
		typedef std::vector<child_pos>::iterator Itor;
		typedef Itor::difference_type difference_type;
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
		typedef const arrow_helper pointer;
		typedef const any_child reference;
		typedef std::vector<child_pos>::const_iterator Itor;
		typedef Itor::difference_type difference_type;
		typedef const_all_children_iterator this_type;
		explicit const_all_children_iterator(const Itor &i): i_(i) {}
		const_all_children_iterator(const all_children_iterator& i): i_(i.i_) {}

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

	/**
	 * @param key the tag name
	 * @param val the contents of the tag
	 * @param pos is the index of the new child in _all_ children.
	 */
	config& add_child_at_total(config_key_type key, const config &val, std::size_t pos);
	std::size_t find_total_first_of(config_key_type key, std::size_t start = 0);

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

private:
	template<typename Res>
	static auto any_tag_view(const child_pos& elem) -> std::pair<const child_map::key_type&, Res>
	{
		const auto& [key, list] = *elem.pos;
		return { key, *list[elem.index] };
	}

public:
#ifdef __cpp_explicit_this_parameter // C++23

	/** In-order iteration over all children. */
	template<typename Self>
	auto all_children_view(this Self&& self)
	{ return self.ordered_children | std::views::transform(&config::any_tag_view<Self>); }

#else

	/** In-order iteration over all children. */
	auto all_children_view() const
	{ return ordered_children | utils::views::transform(&config::any_tag_view<const config&>); }

	/** In-order iteration over all children. */
	auto all_children_view()
	{ return ordered_children | utils::views::transform(&config::any_tag_view<config&>); }

#endif // __cpp_explicit_this_parameter

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
	/**
	 * Merge the attributes of config 'c' into this config, preserving this config's values.
	 */
	void inherit_attributes(const config& c);

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

	/**
	 * Adds children from @a cfg.
	 */
	void append_children(const config &cfg, config_key_type key);

	/** Moves children with the given name from the given config to this one. */
	void append_children_by_move(config& cfg, config_key_type key);

	/**
	 * Adds attributes from @a cfg.
	 */
	void append_attributes(const config &cfg);

	/**
	 * All children with the given key will be merged
	 * into the first element with that key.
	 */
	void merge_children(config_key_type key);

	/**
	 * All children with the given key and with equal values
	 * of the specified attribute will be merged into the
	 * element with that key and that value of the attribute
	 */
	void merge_children_by_attribute(config_key_type key, config_key_type attribute);

	//this is a cheap O(1) operation
	void swap(config& cfg);

	/**
	 * Returns true if this object represents valid WML,
	 * i.e. can be saved to disk and again loaded by the WML parser.
	 */
	bool validate_wml() const;

	/** A non-owning view over all child tag names. */
	auto child_name_view() const
	{
		return children_ | utils::views::keys;
	}

private:
	/**
	 * Removes the child at position @a pos of @a l.
	 */
	std::vector<child_pos>::iterator remove_child(const child_map::iterator &l, std::size_t pos);

	/** All the attributes of this node. */
	attribute_map values_;

	/** A list of all children of this node. */
	child_map children_;

	std::vector<child_pos> ordered_children;
};


using optional_config = optional_config_impl<config>;
using optional_const_config = optional_config_impl<const config>;
/** Implement non-member swap function for std::swap (calls @ref config::swap). */
void swap(config& lhs, config& rhs);

namespace detail
{
	template<typename Key, typename Value, typename... Rest>
	inline void config_construct_unpack(config& cfg, Key&& key, Value&& val, Rest... fwd)
	{
		if constexpr(std::is_same_v<std::decay_t<Value>, config>) {
			cfg.add_child(std::forward<Key>(key), std::forward<Value>(val));
		} else {
			cfg.insert(std::forward<Key>(key), std::forward<Value>(val));
		}

		if constexpr(sizeof...(Rest) > 0) {
			config_construct_unpack(cfg, std::forward<Rest>(fwd)...);
		}
	}
}

template<typename... Args>
inline config::config(config_key_type first, Args&&... args)
{
	detail::config_construct_unpack(*this, first, std::forward<Args>(args)...);
}
