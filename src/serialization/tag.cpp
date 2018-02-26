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
 * Implementation of tag.hpp.
 */

#include "serialization/tag.hpp"
#include "serialization/string_utils.hpp"

#include "config.hpp"

namespace schema_validation
{

class_tag any_tag("", 0, -1, "", true);

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
class_type::class_type(const config& cfg)
	: name_(cfg["name"].str())
{
	if(cfg.has_child("union")) {
		join = UNION;
		for(const config& elem : cfg.child("union").child_range("type")) {
			if(elem.has_attribute("value")) {
				patterns_.emplace_back("^(?:" + elem["value"].str() + ")$");
			} else if(elem.has_attribute("link")) {
				links_.emplace_back(elem["link"].str());
			}
		}
	} else if(cfg.has_child("intersection")) {
		join = INTERSECTION;
		for(const config& elem : cfg.child("intersection").child_range("type")) {
			if(elem.has_attribute("value")) {
				patterns_.emplace_back("^(?:" + elem["value"].str() + ")$");
			} else if(elem.has_attribute("link")) {
				links_.emplace_back(elem["link"].str());
			}
		}
	} else if(cfg.has_child("list")) {
		const config& list_cfg = cfg.child("list");
		join = UNION;
		is_list_ = true;
		list_min_ = list_cfg["min"].to_int();
		list_max_ = list_cfg["max"].str() == "infinite" ? -1 : list_cfg["max"].to_int(-1);
		if(list_max_ < 0) list_max_ = INT_MAX;
		split_ = list_cfg["split"].str(",");
		for(const config& elem : cfg.child("list").child_range("element")) {
			if(elem.has_attribute("value")) {
				patterns_.emplace_back("^(?:" + elem["value"].str() + ")$");
			} else if(elem.has_attribute("link")) {
				links_.emplace_back(elem["link"].str());
			}
		}
	} else if(cfg.has_attribute("value")) {
		patterns_.emplace_back(cfg["value"].str());
	}
}

bool class_type::matches(const std::string& value, const std::map<std::string, class_type>& type_map) const {
	if(is_list_ && !in_list_match_) {
		if(this->name_ == "range") {
			in_list_match_ = true;
		}
		in_list_match_ = true;
		boost::sregex_token_iterator it(value.begin(), value.end(), split_, -1);
		int n = !value.empty();
		bool result = std::any_of(it, boost::sregex_token_iterator(), [this, &type_map, &n](const boost::ssub_match& match){
			if(!match.matched) return false;
			n++;
			return this->matches(std::string(match.first, match.second), type_map);
		});
		in_list_match_ = false;
		return result && n >= list_min_ && n <= list_max_;
	}
	for(const auto& pat : patterns_) {
		boost::smatch sub;
		bool res = boost::regex_match(value, sub, pat);
		switch(join) {
			case UNION: if(res) return true; else break;
			case INTERSECTION: if(!res) return false; else break;
		}
	}
	for(const auto& link : links_) {
		auto it = type_map.find(link);
		if(it == type_map.end()) {
			// TODO: Error message about the invalid type?
			continue;
		}
		bool res = it->second.matches(value, type_map);
		switch(join) {
			case UNION: if(res) return true; else break;
			case INTERSECTION: if(!res) return false; else break;
		}
	}
	switch(join) {
		case UNION: return false;
		case INTERSECTION: return true;
	}
	assert(false && "class_type::matches reached end of function because join value was corrupted");
	return false;
}

class_key::class_key(const config& cfg)
	: name_(cfg["name"].str())
	, type_(cfg["type"].str())
	, default_()
	, mandatory_(false)
	, fuzzy_(name_.find_first_of("*?") != std::string::npos)
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
	, max_(cfg["max"].str() == "infinite" ? -1 : cfg["max"].to_int(1))
	, super_("")
	, tags_()
	, keys_()
	, links_()
	, any_tag_(cfg["any_tag"].to_bool())
	, fuzzy_(name_.find_first_of("*?") != std::string::npos)
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
	key_map::const_iterator it_fuzzy = std::find_if(keys_.begin(), keys_.end(), [&name](const key_map::value_type& key){
		if(!key.second.is_fuzzy()) {
			return false;
		}
		return utils::wildcard_string_match(name, key.second.get_name());
	});
	if(it_fuzzy != keys_.end()) {
		return &(it_fuzzy->second);
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

	const auto it_fuzzy = std::find_if(tags_.begin(), tags_.end(), [&name](const tag_map::value_type& tag){
		if(!tag.second.fuzzy_) {
			return false;
		}
		return utils::wildcard_string_match(name, tag.second.name_);
	});
	if(it_fuzzy != tags_.end()) {
		if(next_path.empty()) {
			return &(it_fuzzy->second);
		} else {
			return it_tags->second.find_tag(next_path, root);
		}
	}

	if(any_tag_) {
		return &any_tag;
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
		if(t.second.is_fuzzy()) {
			// Fuzzy tags won't work as links, so make a copy
			// (Links just don't hold enough info for this to work.)
			add_tag(t.second);
		} else {
			add_link(path + "/" + t.first);
		}
	}
	if(tag.any_tag_) {
		any_tag_ = true;
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
		// TODO: Warn if the super doesn't exist
	}
}

} // namespace schema_validation
