/*
   Copyright (C) 2011 - 2018 by Sytyi Nick <nsytyi@gmail.com>
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
 * Implementation of tag.hpp.
 */

#include "serialization/tag.hpp"

#include "config.hpp"

namespace schema_validation
{
/*WIKI
 * @begin{parent}{name="wml_schema/tag/"}
 * @begin{tag}{name="key"}{min=0}{max=-1}
 * @begin{table}{config}
 *     name & string & &              The name of key. $
 *     type & string & &              The type of key value. $
 *     default & string & &        The default value of the key. $
 *     mandatory & string & &   Shows if key is mandatory $
 * @end{table}
 * @end{tag}{name="key"}
 * @end{parent}{name="wml_schema/tag/"}
 */

class_key::class_key(const config& cfg)
	: name_(cfg["name"].str())
	, type_(cfg["type"].str())
	, default_()
	, mandatory_(false)
{
	if(cfg.has_attribute("mandatory")) {
		mandatory_ = cfg["mandatory"].to_bool();
	} else {
		if(cfg.has_attribute("default")) {
			default_ = cfg["default"].str();
		}
	}
}

void class_key::print(std::ostream& os, int level) const
{
	std::string s;
	for(int j = 0; j < level; j++) {
		s.append(" ");
	}

	os << s << "[key]\n" << s << "    name=\"" << name_ << "\"\n" << s << "    type=\"" << type_ << "\"\n";

	if(is_mandatory()) {
		os << s << "    mandatory=\"true\"\n";
	} else {
		os << s << "    default=" << default_ << "\n";
	}

	os << s << "[/key]\n";
}

class_tag::class_tag(const config& cfg)
	: name_(cfg["name"].str())
	, min_(cfg["min"].to_int())
	, max_(cfg["max"].to_int())
	, super_("")
	, tags_()
	, keys_()
	, links_()
{
	if(max_ < 0) {
		max_ = INT_MAX;
	}

	if(cfg.has_attribute("super")) {
		super_ = cfg["super"].str();
	}

	for(const config& child : cfg.child_range("tag")) {
		class_tag child_tag(child);
		add_tag(child_tag);
	}

	for(const config& child : cfg.child_range("key")) {
		class_key child_key(child);
		add_key(child_key);
	}

	for(const config& link : cfg.child_range("link")) {
		std::string link_name = link["name"].str();
		add_link(link_name);
	}
}

void class_tag::print(std::ostream& os)
{
	printl(os, 4, 4);
}

void class_tag::add_link(const std::string& link)
{
	std::string::size_type pos_last = link.rfind('/');
	// if (pos_last == std::string::npos) return;
	std::string name_link = link.substr(pos_last + 1, link.length());
	links_.emplace(name_link, link);
}

const class_key* class_tag::find_key(const std::string& name) const
{
	const auto it_keys = keys_.find(name);
	if(it_keys != keys_.end()) {
		return &(it_keys->second);
	}

	return nullptr;
}

const std::string* class_tag::find_link(const std::string& name) const
{
	const auto it_links = links_.find(name);
	if(it_links != links_.end()) {
		return &(it_links->second);
	}

	return nullptr;
}

const class_tag* class_tag::find_tag(const std::string& fullpath, const class_tag& root) const
{
	if(fullpath.empty()) {
		return nullptr;
	}

	std::string::size_type pos = fullpath.find('/');
	std::string name;
	std::string next_path;

	if(pos != std::string::npos) {
		name = fullpath.substr(0, pos);
		next_path = fullpath.substr(pos + 1, fullpath.length());
	} else {
		name = fullpath;
	}

	const auto it_tags = tags_.find(name);
	if(it_tags != tags_.end()) {
		if(next_path.empty()) {
			return &(it_tags->second);
		} else {
			return it_tags->second.find_tag(next_path, root);
		}
	}

	const auto it_links = links_.find(name);
	if(it_links != links_.end()) {
		return root.find_tag(it_links->second + "/" + next_path, root);
	}

	return nullptr;
}

void class_tag::expand_all(class_tag& root)
{
	for(auto& tag : tags_) {
		tag.second.expand(root);
		tag.second.expand_all(root);
	}
}

void class_tag::remove_keys_by_type(const std::string& type)
{
	auto i = keys_.begin();
	while(i != keys_.end()) {
		if(i->second.get_type() == type) {
			keys_.erase(i++);
		} else {
			++i;
		}
	}

	for(auto& tag : tags_) {
		tag.second.remove_keys_by_type(type);
	}
}

/*WIKI
 * @begin{parent}{name="wml_schema/"}
 * @begin{tag}{name="tag"}{min=0}{max=1}
 * @begin{table}{config}
 *     name & string & &          The name of tag. $
 *     min & int & &           The min number of occurrences. $
 *     max & int & &           The max number of occurrences. $
 *     super & string & "" &   The super-tag of this tag $
 * @end{table}
 * @begin{tag}{name="link"}{min=0}{max=-1}
 * @begin{table}{config}
 *     name & string & &          The name of link. $
 * @end{table}
 * @end{tag}{name="link"}
 * @begin{tag}{name="tag"}{min=0}{max=-1}{super="wml_schema/tag"}
 * @end{tag}{name="tag"}
 * @end{tag}{name="tag"}
 * @begin{tag}{name="type"}{min=0}{max=-1}
 * @begin{table}{config}
 *     name & string & &          The name of type. $
 *     value & string & &         The value of the type, regex. $
 * @end{table}
 * @end{tag}{name="type"}
 * @end{parent}{name="wml_schema/"}
 */
void class_tag::printl(std::ostream& os, int level, int step)
{
	std::string s;
	for(int j = 0; j < level; j++) {
		s.append(" ");
	}

	os << s << "[tag]\n"
	   << s << "    name=\"" << name_ << "\"\n"
	   << s << "    min=\"" << min_ << "\"\n"
	   << s << "    max=\"" << max_ << "\"\n";

	if(!super_.empty()) {
		os << s << "    super=\"" << super_ << "\"\n";
	}

	for(auto& tag : tags_) {
		tag.second.printl(os, level + step, step);
	}

	for(auto& link : links_) {
		os << s << ""
		   << "[link]\n"
		   << s << ""
		   << "    name=\"" << link.second << "\"\n"
		   << s << ""
		   << "[/link]\n";
	}

	for(auto& key : keys_) {
		key.second.print(os, level + step);
	}

	os << s << "[/tag]\n";
}

class_tag* class_tag::find_tag(const std::string& fullpath, class_tag& root)
{
	if(fullpath.empty()) {
		return nullptr;
	}

	std::string::size_type pos = fullpath.find('/');
	std::string name;
	std::string next_path;

	if(pos != std::string::npos) {
		name = fullpath.substr(0, pos);
		next_path = fullpath.substr(pos + 1, fullpath.length());
	} else {
		name = fullpath;
	}

	auto it_tags = tags_.find(name);
	if(it_tags != tags_.end()) {
		if(next_path.empty()) {
			return &(it_tags->second);
		} else {
			return it_tags->second.find_tag(next_path, root);
		}
	}

	auto it_links = links_.find(name);
	if(it_links != links_.end()) {
		return root.find_tag(it_links->second + "/" + next_path, root);
	}

	return nullptr;
}

#if 0
class_tag& class_tag::operator=(const class_tag& t)
{
	if(&t != this) {
		name_ = t.name_;
		min_ = t.min_;
		max_ = t.max_;
		super_ = t.super_;
		tags_ = t.tags_;
		keys_ = t.keys_;
		links_ = t.links_;
	}

	return *this;
}
#endif

void class_tag::add_tag(const std::string& path, const class_tag& tag, class_tag& root)
{
	if(path.empty() || path == "/") {
		auto it = tags_.find(tag.name_);

		if(it == tags_.end()) {
			tags_.emplace(tag.name_, tag);
		} else {
			it->second.set_min(tag.min_);
			it->second.set_max(tag.max_);
			it->second.add_tags(tag.tags_);
			it->second.add_keys(tag.keys_);
			it->second.add_links(tag.links_);
		}

		links_.erase(tag.get_name());
		return;
	}

	std::string::size_type pos = path.find('/');
	std::string name = path.substr(0, pos);
	std::string next_path = path.substr(pos + 1, path.length());

	auto it_links = links_.find(name);
	if(it_links != links_.end()) {
		root.add_tag(it_links->second + "/" + next_path, tag, root);
	}

	auto it_tags = tags_.find(name);
	if(it_tags == tags_.end()) {
		class_tag subtag;
		subtag.set_name(name);
		subtag.add_tag(next_path, tag, root);
		tags_.emplace(name, subtag);
		return;
	}

	it_tags->second.add_tag(next_path, tag, root);
}

void class_tag::append_super(const class_tag& tag, const std::string& path)
{
	add_keys(tag.keys_);
	add_links(tag.links_);

	for(const auto& t : tag.tags_) {
		links_.erase(t.first);
		add_link(path + "/" + t.first);
	}
}

void class_tag::expand(class_tag& root)
{
	if(!super_.empty()) {
		class_tag* super_tag = root.find_tag(super_, root);
		if(super_tag) {
			if(super_tag != this) {
				super_tag->expand(root);
				append_super(*super_tag, super_);
				super_.clear();
			} else {
				std::cerr << "the same" << super_tag->name_ << "\n";
			}
		}
	}
}

} // namespace schema_validation
