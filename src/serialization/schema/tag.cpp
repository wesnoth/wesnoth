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
 * Implementation of tag.hpp.
 */

#include "serialization/schema/tag.hpp"
#include "serialization/string_utils.hpp"
#include "formatter.hpp"

namespace schema_validation
{

wml_tag any_tag("", 0, -1, "", true);

wml_tag::wml_tag(const config& cfg)
	: name_(cfg["name"].str())
	, min_(cfg["min"].to_int())
	, max_(cfg["max"].str() == "infinite" ? -1 : cfg["max"].to_int(1))
	, min_children_(cfg["min_tags"].to_int())
	, max_children_(cfg["max_tags"].str() == "infinite" ? -1 : cfg["max_tags"].to_int(-1))
	, super_("")
	, tags_()
	, keys_()
	, links_()
	, conditions_()
	, super_refs_()
	, fuzzy_(name_.find_first_of("*?+") != std::string::npos)
	, any_tag_(cfg["any_tag"].to_bool())
{
	if(max_ < 0) {
		max_ = INT_MAX;
	}
	if(max_children_ < 0) {
		max_children_ = INT_MAX;
	}

	if(cfg.has_attribute("super")) {
		super_ = cfg["super"].str();
	}

	for(const config& child : cfg.child_range("tag")) {
		wml_tag child_tag(child);
		add_tag(child_tag);
	}

	for(const config& child : cfg.child_range("key")) {
		wml_key child_key(child);
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

void wml_tag::print(std::ostream& os)
{
	printl(os, 4, 4);
}

void wml_tag::set_min(const std::string& s)
{
	std::istringstream i(s);
	if(!(i >> min_)) {
		min_ = 0;
	}
}

void wml_tag::set_max(const std::string& s)
{
	std::istringstream i(s);
	if(!(i >> max_)) {
		max_ = 0;
	}
}

void wml_tag::add_link(const std::string& link)
{
	std::string::size_type pos_last = link.rfind('/');
	// if (pos_last == std::string::npos) return;
	std::string name_link = link.substr(pos_last + 1, link.length());
	links_.emplace(name_link, link);
}

const wml_key* wml_tag::find_key(const std::string& name, const config& match, bool ignore_super) const
{
	// Check the conditions first, so that conditional definitions
	// override base definitions in the event of duplicates.
	for(auto& cond : conditions_) {
		if(cond.matches(match)) {
			if(auto key = cond.find_key(name, match, true)) {
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

	if(!ignore_super) {
		for(auto& cond : conditions_) {
			if(cond.matches(match)) {
				// This results in a little redundancy (checking things twice) but at least it still works.
				if(auto key = cond.find_key(name, match, false)) {
					return key;
				}
			}
		}
		for(auto& super_tag : super_refs_) {
			if(const wml_key* found_key = super_tag->find_key(name, match)) {
				return found_key;
			}
		}
	}

	return nullptr;
}

const std::string* wml_tag::find_link(const std::string& name) const
{
	const auto it_links = links_.find(name);
	if(it_links != links_.end()) {
		return &(it_links->second);
	}

	return nullptr;
}

const wml_tag* wml_tag::find_tag(const std::string& fullpath, const wml_tag& root, const config& match, bool ignore_super) const
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
			if(auto tag = cond.find_tag(fullpath, root, match, true)) {
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

	if(!ignore_super) {
		for(auto& cond : conditions_) {
			if(cond.matches(match)) {
				// This results in a little redundancy (checking things twice) but at least it still works.
				if(auto tag = cond.find_tag(fullpath, root, match, false)) {
					return tag;
				}
			}
		}
		for(auto& super_tag : super_refs_) {
			if(const wml_tag* found_tag = super_tag->find_tag(fullpath, root, match)) {
				return found_tag;
			}
		}
	}

	if(any_tag_) {
		return &any_tag;
	}

	return nullptr;
 }

void wml_tag::expand_all(wml_tag& root)
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

void wml_tag::remove_keys_by_type(const std::string& type)
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

void wml_tag::printl(std::ostream& os, int level, int step)
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

void wml_tag::add_tag(const std::string& path, const wml_tag& tag, wml_tag& root)
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
		wml_tag subtag;
		subtag.set_name(name);
		subtag.add_tag(next_path, tag, root);
		tags_.emplace(name, subtag);
		return;
	}

	it_tags->second.add_tag(next_path, tag, root);
}

void wml_tag::add_conditions(const condition_list& list)
{
	conditions_.insert(conditions_.end(), list.begin(), list.end());
}

void wml_tag::expand(wml_tag& root)
{
	for(auto& super : utils::split(super_)) {
		wml_tag* super_tag = root.find_tag(super, root, config());
		if(super_tag) {
			if(super_tag != this) {
				super_refs_.push_back(super_tag);
			} else {
				// TODO: Detect super cycles too!
				//PLAIN_LOG << "the same" << super_tag->name_;
			}
		}
		// TODO: Warn if the super doesn't exist
	}
}

void wml_tag::add_switch(const config& switch_cfg)
{
	config default_cfg;
	const std::string key = switch_cfg["key"];
	bool allow_missing = false;
	for(const auto& case_cfg : switch_cfg.child_range("case")) {
		if(case_cfg.has_attribute("value")) {
			const std::vector<std::string> values = utils::split(case_cfg["value"].str(), ',', utils::STRIP_SPACES);
			config filter;
			for(const auto& value : values) {
				// An [or] filter only works if there's something in the main filter.
				// So, the first case cannot be wrapped in [or].
				if(filter.empty()) {
					filter[key] = value;
				} else {
					filter.add_child("or")[key] = value;
				}
				default_cfg.add_child("not")[key] = value;
			}
			if(!allow_missing && case_cfg["trigger_if_missing"].to_bool()) {
				config& missing_filter = filter.add_child("or").add_child("not");
				missing_filter["glob_on_" + key] = "*";
				allow_missing = true;
			}
			conditions_.emplace_back(case_cfg, filter);
		} else {
			// Match if the attribute is missing
			conditions_.emplace_back(case_cfg, config());
		}
		const std::string name = formatter() << get_name() << '[' << key << '=' << case_cfg["value"] << ']';
		conditions_.back().set_name(name);
	}
	if(switch_cfg.has_child("else")) {
		if(allow_missing) {
			// If a [case] matches the absence of the key, then [else] should not
			// The previous [not] keys already failed if it had a value matched by another [case]
			// So just add an [and] tag that matches any other value
			default_cfg.add_child("and")["glob_on_" + key] = "*";
		}
		conditions_.emplace_back(switch_cfg.mandatory_child("else"), default_cfg);
		const std::string name = formatter() << get_name() << "[else]";
		conditions_.back().set_name(name);
	}
}

void wml_tag::add_filter(const config& cond_cfg)
{
	config filter = cond_cfg, else_filter;
	filter.clear_children("then", "else", "elseif");
	// Note in case someone gets trigger-happy:
	// DO NOT MOVE THIS! It needs to be copied!
	else_filter.add_child("not", filter);
	if(cond_cfg.has_child("then")) {
		conditions_.emplace_back(cond_cfg.mandatory_child("then"), filter);
		const std::string name = formatter() << get_name() << "[then]";
		conditions_.back().set_name(name);
	}
	int i = 1;
	for(auto elseif_cfg : cond_cfg.child_range("elseif")) {
		config elseif_filter = elseif_cfg, old_else_filter = else_filter;
		elseif_filter.clear_children("then");
		else_filter.add_child("not", elseif_filter);
		// Ensure it won't match for any of the preceding cases, either
		elseif_filter.append_children(old_else_filter);
		conditions_.emplace_back(elseif_cfg.child_or_empty("then"), elseif_filter);
		const std::string name = formatter() << get_name() << "[elseif " << i++ << "]";
		conditions_.back().set_name(name);
	}
	if(cond_cfg.has_child("else")) {
		conditions_.emplace_back(cond_cfg.mandatory_child("else"), else_filter);
		const std::string name = formatter() << get_name() << "[else]";
		conditions_.back().set_name(name);
	}
}

bool wml_condition::matches(const config& cfg) const
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

template<>
void wml_tag::tag_iterator::init(const wml_tag& base_tag)
{
	current = base_tag.tags_.begin();
	condition_queue.push(&base_tag);
}

template<>
void wml_tag::tag_iterator::ensure_valid_or_end() {
	while(current == condition_queue.front()->tags_.end()) {
		condition_queue.pop();
		if(condition_queue.empty()) {
			return;
		}
		const wml_tag& new_base = *condition_queue.front();
		current= new_base.tags_.begin();
		push_new_tag_conditions(new_base);
	}
}

template<>
void wml_tag::key_iterator::init(const wml_tag& base_tag)
{
	current = base_tag.keys_.begin();
	condition_queue.push(&base_tag);
}

template<>
void wml_tag::key_iterator::ensure_valid_or_end() {
	while(current == condition_queue.front()->keys_.end()) {
		condition_queue.pop();
		if(condition_queue.empty()) {
			return;
		}
		const wml_tag& new_base = *condition_queue.front();
		current = new_base.keys_.begin();
		push_new_tag_conditions(new_base);
	}
}

void wml_tag::push_new_tag_conditions(std::queue<const wml_tag*>& q, const config& match, const wml_tag& tag) {
	for(const auto& condition : tag.conditions_) {
		if(condition.matches(match)) {
			q.push(&condition);
		}
	}
}

} // namespace schema_validation
