/*
	Copyright (C) 2011 - 2024
	by Sytyi Nick <nsytyi@gmail.com>
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
 * This file contains object "tag", which is used to store
 * information about tags while annotation parsing.
 */

#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>
#include <queue>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator.hpp>
#include "config.hpp"
#include "serialization/schema/key.hpp"

namespace schema_validation
{

class wml_condition;

/**
 * Stores information about tag.
 * Each tags is an element of great tag tree. This tree is close to filesystem:
 * you can use links and special include directory global/
 * Normally root is not mentioned in path.
 * Each tag has name, minimum and maximum occasions number,
 * and lists of subtags, keys and links.
 */
class wml_tag
{
public:
	using tag_map  = std::map<std::string, wml_tag>;
	using key_map  = std::map<std::string, wml_key>;
	using link_map = std::map<std::string, std::string>;
	using condition_list = std::vector<wml_condition>;
	using super_list = std::vector<wml_tag*>;
private:
	static void push_new_tag_conditions(std::queue<const wml_tag*>& q, const config& match, const wml_tag& tag);
	template<typename T, typename Map = std::map<std::string, T>>
	class iterator : public boost::iterator_facade<iterator<T>, const typename Map::value_type, std::forward_iterator_tag>
	{
		std::queue<const wml_tag*> condition_queue;
		typename Map::const_iterator current;
		std::reference_wrapper<const config> match;
	public:
		// Construct a begin iterator
		iterator(const wml_tag& base_tag, const config& match) : match(match)
		{
			init(base_tag);
			push_new_tag_conditions(base_tag);
			ensure_valid_or_end();
		}
		// Construct an end iterator
		iterator() : match(config().child_or_empty("fsdfsdf")) {}
	private:
		friend class boost::iterator_core_access;
		void init(const wml_tag& base_tag);
		void ensure_valid_or_end();
		void increment()
		{
			++current;
			ensure_valid_or_end();
		}
		void push_new_tag_conditions(const wml_tag& tag)
		{
			wml_tag::push_new_tag_conditions(condition_queue, match.get(), tag);
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
	using tag_iterator = iterator<wml_tag>;
	using key_iterator = iterator<wml_key>;
public:

	wml_tag()
		: name_("")
		, min_(0)
		, max_(0)
		, min_children_(0)
		, max_children_(INT_MAX)
		, super_("")
		, tags_()
		, keys_()
		, links_()
		, fuzzy_(false)
		, any_tag_(false)
	{
	}

	wml_tag(const std::string& name, int min, int max, const std::string& super = "", bool any = false)
		: name_(name)
		, min_(min)
		, max_(max)
		, min_children_(0)
		, max_children_(INT_MAX)
		, super_(super)
		, tags_()
		, keys_()
		, links_()
		, fuzzy_(name.find_first_of("*?") != std::string::npos)
		, any_tag_(any)
	{
	}

	wml_tag(const config&);

	~wml_tag()
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

	int get_min_children() const
	{
		return min_children_;
	}

	int get_max_children() const
	{
		return max_children_;
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

	void set_min_children(int o)
	{
		min_children_ = o;
	}

	void set_max_children(int o)
	{
		max_children_ = o;
	}

	void set_min(const std::string& s);
	void set_max(const std::string& s);

	void set_min_children(const std::string& s);
	void set_max_children(const std::string& s);

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

	void add_key(const wml_key& new_key)
	{
		keys_.emplace(new_key.get_name(), new_key);
	}

	void add_tag(const wml_tag& new_tag)
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
	void add_tag(const std::string& path, const wml_tag& tag, wml_tag& root);

	bool operator<(const wml_tag& t) const
	{
		return name_ < t.name_;
	}

	bool operator==(const wml_tag& other) const
	{
		return name_ == other.name_;
	}

	/** Returns pointer to child key. */
	const wml_key* find_key(const std::string& name, const config& match, bool ignore_super = false) const;

	/** Returns pointer to child link. */
	const std::string* find_link(const std::string& name) const;

	/**
	 * Returns pointer to tag using full path to it.
	 * Also work with links
	 */
	const wml_tag* find_tag(const std::string& fullpath, const wml_tag& root, const config& match, bool ignore_super = false) const;

	/** Calls the expansion on each child. */
	void expand_all(wml_tag& root);

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

	/** minimum number of occurrences. */
	int min_;

	/** maximum number of occurrences. */
	int max_;

	/** minimum number of children. */
	int min_children_;

	/** maximum number of children. */
	int max_children_;

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
	 * the same as wml_tag::print(std::ostream&)
	 * but indents different levels with step space.
	 * @param os stream to print
	 * @param level  current level of indentation
	 * @param step   step to next indent
	 */
	void printl(std::ostream& os, int level, int step = 4);

	wml_tag* find_tag(const std::string & fullpath, wml_tag & root, const config& match)
	{
		return const_cast<wml_tag*>(const_cast<const wml_tag*>(this)->find_tag(fullpath, root, match));
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

	void add_conditions(const condition_list& list);

	/** Expands all "super", storing direct references for easier access. */
	void expand(wml_tag& root);
};

/**
 * Stores information about a conditional portion of a tag.
 * Format is the same as wml_tag.
 */
class wml_condition : public wml_tag {
	config filter_;
public:
	wml_condition(const config& info, const config& filter) : wml_tag(info), filter_(filter) {}
	bool matches(const config& cfg) const;
};
}
