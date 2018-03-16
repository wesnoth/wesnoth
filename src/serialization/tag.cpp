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
#include "boost/optional.hpp"
#include "formatter.hpp"

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
std::shared_ptr<class_type> class_type::from_config(const config& cfg)
{
	boost::optional<config::const_child_itors> composite_range;
	std::shared_ptr<class_type> type;
	if(cfg.has_child("union")) {
		type = std::make_shared<class_type_union>(cfg["name"]);
		composite_range.emplace(cfg.child("union").child_range("type"));
	} else if(cfg.has_child("intersection")) {
		type = std::make_shared<class_type_intersection>(cfg["name"]);
		composite_range.emplace(cfg.child("intersection").child_range("type"));
	} else if(cfg.has_child("list")) {
		const config& list_cfg = cfg.child("list");
		int list_min = list_cfg["min"].to_int();
		int list_max = list_cfg["max"].str() == "infinite" ? -1 : list_cfg["max"].to_int(-1);
		if(list_max < 0) list_max = INT_MAX;
		type = std::make_shared<class_type_list>(cfg["name"], list_cfg["split"].str(","), list_min, list_max);
		composite_range.emplace(list_cfg.child_range("element"));
	} else if(cfg.has_attribute("value")) {
		type = std::make_shared<class_type_simple>(cfg["name"], cfg["value"]);
	} else if(cfg.has_attribute("link")) {
		type = std::make_shared<class_type_alias>(cfg["name"], cfg["link"]);
	}
	if(composite_range) {
		auto composite_type = std::dynamic_pointer_cast<class_type_composite>(type);
		for(const config& elem : *composite_range) {
			composite_type->add_type(class_type::from_config(elem));
		}
	}
	return type;
}

bool class_type_simple::matches(const std::string& value, const map&) const
{
	boost::smatch sub;
	return boost::regex_match(value, sub, pattern_);
}

bool class_type_alias::matches(const std::string& value, const map& type_map) const
{
	if(!cached_) {
		auto it = type_map.find(link_);
		if(it == type_map.end()) {
			// TODO: Error message about the invalid type?
			return false;
		}
		cached_ = it->second;
	}
	return cached_->matches(value, type_map);
}

bool class_type_union::matches(const std::string& value, const map& type_map) const
{
	for(const auto& type : subtypes_) {
		if(type->matches(value, type_map)) {
			return true;
		}
	}
	return false;
}

bool class_type_intersection::matches(const std::string& value, const map& type_map) const
{
	for(const auto& type : subtypes_) {
		if(!type->matches(value, type_map)) {
			return false;
		}
	}
	return true;
}

bool class_type_list::matches(const std::string& value, const map& type_map) const
{
	boost::sregex_token_iterator it(value.begin(), value.end(), split_, -1), end;
	int n = !value.empty();
	bool result = std::all_of(it, end, [this, &type_map, &n](const boost::ssub_match& match){
		if(!match.matched) return true;
		n++;
		return this->class_type_union::matches(std::string(match.first, match.second), type_map);
	});
	return result && n >= min_ && n <= max_;
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

	// TODO: Other attributes

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

	for(const config& sw : cfg.child_range("switch")) {
		add_switch(sw);
	}

	for(const config& filter : cfg.child_range("if")) {
		add_filter(filter);
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

const class_key* class_tag::find_key(const std::string& name, const config& match) const
{
	// Check the conditions first, so that conditional definitions
	// override base definitions in the event of duplicates.
	for(auto& cond : conditions_) {
		if(cond.matches(match)) {
			if(auto key = cond.find_key(name, match)) {
				return key;
			}
		}
	}

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

const class_tag* class_tag::find_tag(const std::string& fullpath, const class_tag& root, const config& match) const
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

	// Check the conditions first, so that conditional definitions
	// override base definitions in the event of duplicates.
	for(auto& cond : conditions_) {
		if(cond.matches(match)) {
			if(auto tag = cond.find_tag(fullpath, root, match)) {
				return tag;
			}
		}
	}

	const auto it_tags = tags_.find(name);
	if(it_tags != tags_.end()) {
		if(next_path.empty()) {
			return &(it_tags->second);
		} else {
			return it_tags->second.find_tag(next_path, root, match);
		}
	}

	const auto it_links = links_.find(name);
	if(it_links != links_.end()) {
		return root.find_tag(it_links->second + "/" + next_path, root, match);
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
			return it_tags->second.find_tag(next_path, root, match);
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
	for(auto& cond : conditions_) {
		cond.expand(root);
		cond.expand_all(root);
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

	// TODO: Other attributes

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
			// TODO: Other attributes
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
	// TODO: Ensure derived tag definitions override base tag definitions in the event of duplicates
	add_keys(tag.keys_);
	add_links(tag.links_);
	add_conditions(tag.conditions_);
	
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
		class_tag* super_tag = root.find_tag(super_, root, config());
		if(super_tag) {
			if(super_tag != this) {
				super_tag->expand(root);
				append_super(*super_tag, super_);
				super_.clear();
			} else {
				// TODO: Detect super cycles too!
				std::cerr << "the same" << super_tag->name_ << "\n";
			}
		}
		// TODO: Warn if the super doesn't exist
	}
}

void class_tag::add_switch(const config& switch_cfg)
{
	config default_cfg;
	const std::string key = switch_cfg["key"];
	for(const auto& case_cfg : switch_cfg.child_range("case")) {
		const std::vector<std::string> values = utils::split(case_cfg["value"]);
		config filter;
		for(const auto& value : values) {
			filter.add_child("or")[key] = value;
			default_cfg.add_child("not")[key] = value;
		}
		conditions_.emplace_back(case_cfg, filter);
		const std::string name = formatter() << get_name() << '[' << key << '=' << case_cfg["value"] << ']';
		conditions_.back().set_name(name);
	}
	if(switch_cfg.has_child("default")) {
		conditions_.emplace_back(switch_cfg.child("default"), default_cfg);
		const std::string name = formatter() << get_name() << "[default]";
		conditions_.back().set_name(name);
	}
}

void class_tag::add_filter(const config& cond_cfg)
{
	config filter = cond_cfg;
	filter.clear_children("then", "else");
	if(cond_cfg.has_child("then")) {
		conditions_.emplace_back(cond_cfg.child("then"), filter);
		const std::string name = formatter() << get_name() << "[then]";
		conditions_.back().set_name(name);
	}
	if(cond_cfg.has_child("else")) {
		conditions_.emplace_back(cond_cfg.child("else"), config{"not", filter});
		const std::string name = formatter() << get_name() << "[else]";
		conditions_.back().set_name(name);
	}
}

bool class_condition::matches(const config& cfg) const
{
	if(cfg.empty()) {
		// Conditions are not allowed to match an empty config.
		// If they were, the conditions might be considered when expanding super-tags.
		// That would result in a condition tag being used for the expansion, rather than
		// the base tag, which would be bad.
		return false;
	}
	return cfg.matches(filter_);
}

void class_tag::tag_iterator::init(const class_tag& base_tag)
{
	current = base_tag.tags_.begin();
	if(current != base_tag.tags_.end()) {
		condition_queue.push(&base_tag);
	}
}

void class_tag::tag_iterator::increment()
{
	++current;
	while(current== condition_queue.front()->tags_.end()) {
		condition_queue.pop();
		if(condition_queue.empty()) {
			return;
		}
		const class_tag& new_base = *condition_queue.front();
		current= new_base.tags_.begin();
		push_new_tag_conditions(new_base);
	}
}

void class_tag::key_iterator::init(const class_tag& base_tag)
{
	current = base_tag.keys_.begin();
	if(current != base_tag.keys_.end()) {
		condition_queue.push(&base_tag);
	}
}

void class_tag::key_iterator::increment()
{
	++current;
	while(current == condition_queue.front()->keys_.end()) {
		condition_queue.pop();
		if(condition_queue.empty()) {
			return;
		}
		const class_tag& new_base = *condition_queue.front();
		current = new_base.keys_.begin();
		push_new_tag_conditions(new_base);
	}
}

} // namespace schema_validation
