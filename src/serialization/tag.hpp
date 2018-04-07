/*
   Copyright (C) 2011 - 2018 by Sytyi Nick <nsytyi@gmail.com>
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
 * This file contains objects "tag" and "key", which are used to store
 * information about tags and keys while annotation parsing.
 */

#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <queue>

#include <boost/regex.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator.hpp>
#include "config.hpp"

namespace schema_validation
{

/**
 * Stores information about a schema type.
 * This is an abstract base class for several variants of schema type.
 */
class class_type {
protected:
	std::string name_;
public:
	class_type() = delete;
	explicit class_type(const std::string& name) : name_(name) {}
	using ptr = std::shared_ptr<class_type>;
	using map = std::map<std::string, ptr>;
	virtual bool matches(const std::string& value, const map& type_map) const = 0;
	static std::shared_ptr<class_type> from_config(const config& cfg);
};

/**
 * Stores information about a schema type.
 * This type represents a simple pattern match.
 */
class class_type_simple : public class_type {
	boost::regex pattern_;
public:
	class_type_simple(const std::string& name, const std::string& pattern) : class_type(name), pattern_(pattern) {}
	bool matches(const std::string& value, const map& type_map) const override;
};

/**
 * Stores information about a schema type.
 * This type represents a name alias for another type.
 */
class class_type_alias : public class_type {
	mutable std::shared_ptr<class_type> cached_;
	std::string link_;
public:
	class_type_alias(const std::string& name, const std::string& link) : class_type(name), link_(link) {}
	bool matches(const std::string& value, const map& type_map) const override;
};

/**
 * Stores information about a schema type.
 * This is an abstract base class for composite types.
 */
class class_type_composite : public class_type {
protected:
	std::vector<std::shared_ptr<class_type>> subtypes_;
public:
	explicit class_type_composite(const std::string& name) : class_type(name) {}
	void add_type(std::shared_ptr<class_type> type)
	{
		subtypes_.push_back(type);
	}
};

/**
 * Stores information about a schema type.
 * Represents a union type, which matches if any of its subtypes match.
 */
class class_type_union : public class_type_composite {
public:
	explicit class_type_union(const std::string& name) : class_type_composite(name) {}
	bool matches(const std::string& value, const map& type_map) const override;
};

/**
 * Stores information about a schema type.
 * Represents an intersection type, which matches if all of its subtypes match.
 */
class class_type_intersection : public class_type_composite {
public:
	explicit class_type_intersection(const std::string& name) : class_type_composite(name) {}
	bool matches(const std::string& value, const map& type_map) const override;
};

/**
 * Stores information about a schema type.
 * Represents a list type, where each list element is itself a union.
 */
class class_type_list : public class_type_union {
	boost::regex split_;
	int min_ = 0, max_ = -1;
public:
	class_type_list(const std::string& name, const std::string& pattern, int min, int max)
		: class_type_union(name)
		, split_(pattern)
		, min_(min)
		, max_(max)
	{}
	bool matches(const std::string& value, const map& type_map) const override;
};

/**
 * class_key is used to save the information about one key.
 * Key has next info: name, type, default value or key is mandatory.
 */
class class_key
{
public:
	class_key()
		: name_("")
		, type_("")
		, default_("\"\"")
		, mandatory_(false)
		, fuzzy_(false)
	{
	}

	class_key(const std::string& name, const std::string& type, const std::string& def = "\"\"")
		: name_(name)
		, type_(type)
		, default_(def)
		, mandatory_(def.empty())
		, fuzzy_(name.find_first_of("*?") != std::string::npos)
	{
	}

	class_key(const config&);

	const std::string& get_name() const
	{
		return name_;
	}

	const std::string& get_type() const
	{
		return type_;
	}

	const std::string& get_default() const
	{
		return default_;
	}

	bool is_mandatory() const
	{
		return mandatory_;
	}

	bool is_fuzzy() const {
		return fuzzy_;
	}

	void set_name(const std::string& name)
	{
		name_ = name;
	}

	void set_type(const std::string& type)
	{
		type_ = type;
	}

	void set_default(const std::string& def)
	{
		default_ = def;
		if(def.empty()) {
			mandatory_ = true;
		}
	}

	void set_mandatory(bool mandatory)
	{
		mandatory_ = mandatory;
	}

	void set_fuzzy(bool f)
	{
		fuzzy_ = f;
	}

	/** is used to print key info
	 * the format is next
	 *  [key]
	 *      name="name"
	 *      type="type"
	 *      default="default"
	 *      mandatory="true/false"
	 *  [/key]
	 */
	void print(std::ostream& os, int level) const;

	/** Compares keys by name. Used in std::sort, i.e. */
	bool operator<(const class_key& k) const
	{
		return (get_name() < k.get_name());
	}

private:
	/** Name of key. */
	std::string name_;

	/** Type of key. */
	std::string type_;

	/** Default value. */
	std::string default_;

	/** Shows, if key is a mandatory key. */
	bool mandatory_;

	/** Whether the key is a fuzzy match. */
	bool fuzzy_;
};

class class_condition;

/**
 * Stores information about tag.
 * Each tags is an element of great tag tree. This tree is close to filesystem:
 * you can use links and special include directory global/
 * Normally root is not mentioned in path.
 * Each tag has name, minimum and maximum occasions number,
 * and lists of subtags, keys and links.
 */
class class_tag
{
public:
	using tag_map  = std::map<std::string, class_tag>;
	using key_map  = std::map<std::string, class_key>;
	using link_map = std::map<std::string, std::string>;
	using condition_list = std::vector<class_condition>;
	using super_list = std::vector<class_tag*>;
private:
	void push_new_tag_conditions(std::queue<const class_tag*> q, const class_tag& tag);
	template<typename T, typename Map = std::map<std::string, T>>
	class iterator : public boost::iterator_facade<iterator<T>, const typename Map::value_type, std::forward_iterator_tag>
	{
		std::queue<const class_tag*> condition_queue;
		typename Map::const_iterator current;
		const config& match;
	public:
		// Construct a begin iterator
		iterator(const class_tag& base_tag, const config& match) : match(match)
		{
			init(base_tag);
			push_new_tag_conditions(base_tag);
			ensure_valid_or_end();
		}
		// Construct an end iterator
		// That weird expression is to get a reference to an "invalid" config.
		iterator() : match(config().child("a")) {}
	private:
		friend class boost::iterator_core_access;
		void init(const class_tag& base_tag);
		void ensure_valid_or_end();
		void increment()
		{
			++current;
			ensure_valid_or_end();
		}
		void push_new_tag_conditions(const class_tag& tag)
		{
			for(const auto& condition : tag.conditions_) {
				if(condition.matches(match)) {
					condition_queue.push(&condition);
				}
			}
		}
		bool equal(const iterator<T, Map>& other) const
		{
			if(condition_queue.empty() && other.condition_queue.empty()) {
				return true;
			}
			if(condition_queue.empty() || other.condition_queue.empty()) {
				return false;
			}
			if(condition_queue.front() != other.condition_queue.front()) {
				return false;
			}
			if(current != other.current) {
				return false;
			}
			return true;
		}
		typename iterator<T,Map>::reference dereference() const
		{
			return *current;
		}
	};
	template<typename T, typename Map> friend class iterator;
	using tag_iterator = iterator<class_tag>;
	using key_iterator = iterator<class_key>;
public:

	class_tag()
		: name_("")
		, min_(0)
		, max_(0)
		, super_("")
		, tags_()
		, keys_()
		, links_()
		, fuzzy_(false)
		, any_tag_(false)
	{
	}

	class_tag(const std::string& name, int min, int max, const std::string& super = "", bool any = false)
		: name_(name)
		, min_(min)
		, max_(max)
		, super_(super)
		, tags_()
		, keys_()
		, links_()
		, fuzzy_(name.find_first_of("*?") != std::string::npos)
		, any_tag_(any)
	{
	}

	class_tag(const config&);

	~class_tag()
	{
	}

	/** Prints information about tag to outputstream, recursively
	 * is used to print tag info
	 * the format is next
	 *  [tag]
	 *      subtags
	 *      keys
	 *      name="name"
	 *      min="min"
	 *      max="max"
	 *  [/tag]
	 */
	void print(std::ostream& os);

	const std::string& get_name() const
	{
		return name_;
	}

	int get_min() const
	{
		return min_;
	}

	int get_max() const
	{
		return max_;
	}

	const std::string& get_super() const
	{
		return super_;
	}

	bool is_extension() const
	{
		return !super_.empty();
	}

	bool is_fuzzy() const {
		return fuzzy_;
	}
	
	bool accepts_any_tag() const {
		return any_tag_;
	}

	void set_name(const std::string& name)
	{
		name_ = name;
	}

	void set_min(int o)
	{
		min_ = o;
	}

	void set_max(int o)
	{
		max_ = o;
	}

	void set_min(const std::string& s)
	{
		std::istringstream i(s);
		if(!(i >> min_)) {
			min_ = 0;
		}
	}

	void set_max(const std::string& s)
	{
		std::istringstream i(s);
		if(!(i >> max_)) {
			max_ = 0;
		}
	}

	void set_super(const std::string& s)
	{
		super_ = s;
	}
	
	void set_fuzzy(bool f) {
		fuzzy_ = f;
	}

	void set_any_tag(bool any) {
		any_tag_ = any;
	}
	
	void add_key(const class_key& new_key)
	{
		keys_.emplace(new_key.get_name(), new_key);
	}

	void add_tag(const class_tag& new_tag)
	{
		tags_.emplace(new_tag.name_, new_tag);
	}

	void add_link(const std::string& link);

	void add_switch(const config& switch_cfg);

	void add_filter(const config& cond_cfg);

	/**
	 * Tags are usually organized in tree.
	 * This function helps to add a tag to his exact place in tree
	 * @param path - path in subtree to exact place of tag
	 * @param tag  - tag to add
	 * @param root - root of schema tree - use to support of adding to link.
	 * Path is getting shotter and shoter with each call.
	 * Path should look like tag1/tag2/parent/ Slash at end is mandatory.
	 */
	void add_tag(const std::string& path, const class_tag& tag, class_tag& root);

	bool operator<(const class_tag& t) const
	{
		return name_ < t.name_;
	}

	bool operator==(const class_tag& other) const
	{
		return name_ == other.name_;
	}

	/** Returns pointer to child key. */
	const class_key* find_key(const std::string& name, const config& match, bool ignore_super = false) const;

	/** Returns pointer to child link. */
	const std::string* find_link(const std::string& name) const;

	/**
	 * Returns pointer to tag using full path to it.
	 * Also work with links
	 */
	const class_tag* find_tag(const std::string& fullpath, const class_tag& root, const config& match, bool ignore_super = false) const;

	/** Calls the expansion on each child. */
	void expand_all(class_tag& root);

	boost::iterator_range<tag_iterator> tags(const config& cfg_match) const
	{
		return {tag_iterator(*this, cfg_match), tag_iterator()};
	}

	boost::iterator_range<key_iterator> keys(const config& cfg_match) const
	{
		return {key_iterator(*this, cfg_match), key_iterator()};
	}

	const link_map& links() const
	{
		return links_;
	}

	const condition_list& conditions() const
	{
		return conditions_;
	}

	void remove_key_by_name(const std::string& name)
	{
		keys_.erase(name);
	}

	/** Removes all keys with this type. Works recursively */
	void remove_keys_by_type(const std::string& type);

private:
	/** name of tag. */
	std::string name_;

	/** number of minimum occasions. */
	int min_;

	/** number of maximum occasions. */
	int max_;

	/**
	 * name of tag to extend "super-tag"
	 * Extension is smth like inheritance and is used in case
	 * when you need to use another tag with all his
	 * keys, children, etc. But you also want to allow extra subtags of that tags,
	 * so just linking that tag wouldn't help at all.
	 */
	std::string super_;

	/** children tags*/
	tag_map tags_;

	/** keys*/
	key_map keys_;

	/** links to possible children. */
	link_map links_;

	/** conditional partial matches */
	condition_list conditions_;

	/** super-tag references */
	super_list super_refs_;

	/** whether this is a "fuzzy" tag. */
	bool fuzzy_;

	/** whether this tag allows arbitrary subtags. */
	bool any_tag_;

	/**
	 * the same as class_tag::print(std::ostream&)
	 * but indents different levels with step space.
	 * @param os stream to print
	 * @param level  current level of indentation
	 * @param step   step to next indent
	 */
	void printl(std::ostream& os, int level, int step = 4);

	class_tag* find_tag(const std::string & fullpath, class_tag & root, const config& match)
	{
		return const_cast<class_tag*>(const_cast<const class_tag*>(this)->find_tag(fullpath, root, match));
	}

	void add_tags(const tag_map& list)
	{
		tags_.insert(list.begin(), list.end());
	}

	void add_keys(const key_map& list)
	{
		keys_.insert(list.begin(), list.end());
	}

	void add_links(const link_map& list)
	{
		links_.insert(list.begin(), list.end());
	}

	void add_conditions(const condition_list& list)
	{
		conditions_.insert(conditions_.end(), list.begin(), list.end());
	}

	/** Expands all "super", storing direct references for easier access. */
	void expand(class_tag& root);
};

extern template class class_tag::iterator<class_tag>;
extern template class class_tag::iterator<class_key>;

/**
 * Stores information about a conditional portion of a tag.
 * Format is the same as class_tag.
 */
class class_condition : public class_tag {
	config filter_;
public:
	class_condition(const config& info, const config& filter) : class_tag(info), filter_(filter) {}
	bool matches(const config& cfg) const;
};
}
