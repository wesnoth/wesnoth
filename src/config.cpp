/*
	Copyright (C) 2003 - 2024
	by Guillaume Melquiond <guillaume.melquiond@gmail.com>
	Copyright (C) 2003 by David White <dave@whitevine.net>
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
 * Routines related to configuration-files / WML.
 */

#include "config.hpp"

#include "formatter.hpp"
#include "lexical_cast.hpp"
#include "log.hpp"
#include "deprecation.hpp"
#include "game_version.hpp"
#include "serialization/string_utils.hpp"
#include "utils/general.hpp"

#include <algorithm>
#include <cstring>
#include <iterator>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define DBG_CF LOG_STREAM(debug, log_config)

static lg::log_domain log_wml("wml");
#define ERR_WML LOG_STREAM(err, log_wml)

namespace
{
// std::map::operator[] does not support heterogeneous lookup so we need this to work around.
template<typename Map, typename Key>
typename Map::iterator map_get(Map& map, Key&& key)
{
	auto res = map.lower_bound(key);

	if(res == map.end() || key != res->first) {
		res = map.emplace_hint(res, std::piecewise_construct, std::forward_as_tuple(key), std::tuple<>());
	}

	return res;
}

// std::map::erase does not support heterogeneous lookup so we need this to work around.
template<typename Map, typename Key>
int map_erase_key(Map& map, Key&& key)
{
	auto i = map.find(key);
	if(i != map.end()) {
		map.erase(i);
		return 1;
	}
	return 0;
}

}

/* ** config implementation ** */

const char* config::diff_track_attribute = "__diff_track";

config::config()
	: values_()
	, children_()
	, ordered_children()
{
}

config::config(const config& cfg)
	: values_(cfg.values_)
	, children_()
	, ordered_children()
{
	append_children(cfg);
}

config::config(config_key_type child)
	: values_()
	, children_()
	, ordered_children()
{
	add_child(child);
}

config::~config()
{
	clear();
}

config& config::operator=(const config& cfg)
{
	if(this == &cfg) {
		return *this;
	}

	config tmp(cfg);
	swap(tmp);
	return *this;
}

config::config(config&& cfg)
	: values_(std::move(cfg.values_))
	, children_(std::move(cfg.children_))
	, ordered_children(std::move(cfg.ordered_children))
{
}

config& config::operator=(config&& cfg)
{
	clear();
	swap(cfg);
	return *this;
}

bool config::valid_tag(config_key_type name)
{
	if(name == "") {
		// Empty strings not allowed
		return false;
	} else if(name == "_") {
		// A lone underscore isn't a valid tag name
		return false;
	} else {
		return std::all_of(name.begin(), name.end(), [](const char& c)
		{
			/* Only alphanumeric ASCII characters and underscores are allowed.

			We're using a manual check mainly for performance. @gfgtdf measured
			that a manual check can be up to 30 times faster than std::isalnum().

			- Jyrki, 2019-01-19 */
			return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
				(c >= '0' && c <= '9') || (c == '_');
		});
	}
}

bool config::valid_attribute(config_key_type name)
{
	return valid_tag(name);
}

bool config::has_attribute(config_key_type key) const
{
	return values_.find(key) != values_.end();
}

void config::remove_attribute(config_key_type key)
{
	map_erase_key(values_, key);
}

void config::append_children(const config& cfg)
{
	for(const auto [key, cfg] : cfg.all_children_view()) {
		add_child(key, cfg);
	}
}

void config::append_attributes(const config& cfg)
{
	for(const auto& [key, value] : cfg.values_) {
		values_[key] = value;
	}
}

void config::append_children(const config& cfg, config_key_type key)
{
	for(const config& value : cfg.child_range(key)) {
		add_child(key, value);
	}
}

void config::append(const config& cfg)
{
	append_children(cfg);
	for(const auto& [key, value] : cfg.values_) {
		values_[key] = value;
	}
}

void config::append(config&& cfg)
{
	if(children_.empty()) {
		//optimisation
		children_ = std::move(cfg.children_);
		ordered_children = std::move(cfg.ordered_children);
		cfg.clear_all_children();
	}
	else {
		for(const auto [child_key, child_value] : cfg.all_children_view()) {
			add_child(child_key, std::move(child_value));
		}
		cfg.clear_all_children();
	}

	if(values_.empty()) {
		//optimisation.
		values_ = std::move(cfg.values_);
	}
	else {
		for(const auto& [key, value] : cfg.values_) {
			//TODO: move the attributes as well?
			values_[key] = value;
		}
	}
	cfg.clear_attributes();
}

void config::append_children_by_move(config& cfg, config_key_type key)
{
	// DO note this leaves the tags empty in the source config. Not sure if
	// that should be changed.
	for(config& value : cfg.child_range(key)) {
		add_child(key, std::move(value));
	}

	cfg.clear_children_impl(key);
}

void config::merge_children(config_key_type key)
{
	if(child_count(key) < 2) {
		return;
	}

	config merged_children;
	for(config& cfg : child_range(key)) {
		merged_children.append(std::move(cfg));
	}

	clear_children_impl(key);
	add_child(key, std::move(merged_children));
}

void config::merge_children_by_attribute(config_key_type key, config_key_type attribute)
{
	if(child_count(key) < 2) {
		return;
	}

	typedef std::map<std::string, config> config_map;
	config_map merged_children_map;
	for(config& cfg : child_range(key)) {
		merged_children_map[cfg[attribute]].append(std::move(cfg));
	}

	clear_children_impl(key);
	for(const config_map::value_type& i : merged_children_map) {
		add_child(key, i.second); // TODO: can we use std::move?
	}
}

config::child_itors config::child_range(config_key_type key)
{
	child_map::iterator i = children_.find(key);
	static child_list dummy;
	child_list* p = &dummy;
	if(i != children_.end()) {
		p = &i->second;
	}

	return child_itors(child_iterator(p->begin()), child_iterator(p->end()));
}

config::const_child_itors config::child_range(config_key_type key) const
{
	child_map::const_iterator i = children_.find(key);
	static child_list dummy;
	const child_list* p = &dummy;
	if(i != children_.end()) {
		p = &i->second;
	}

	return const_child_itors(const_child_iterator(p->begin()), const_child_iterator(p->end()));
}

std::size_t config::child_count(config_key_type key) const
{
	child_map::const_iterator i = children_.find(key);
	if(i != children_.end()) {
		return i->second.size();
	}

	return 0;
}

std::size_t config::all_children_count() const
{
	return ordered_children.size();
}

std::size_t config::attribute_count() const
{
	return std::count_if(values_.begin(), values_.end(), [](const attribute& v) { return !v.second.blank(); });
}

bool config::has_child(config_key_type key) const
{
	child_map::const_iterator i = children_.find(key);
	return i != children_.end() && !i->second.empty();
}

namespace {
template<class Tchildren>
auto get_child_impl(Tchildren& children, config_key_type key, int n) -> optional_config_impl<std::remove_reference_t<decltype(**(*children.begin()).second.begin())>>
{

	auto i = children.find(key);
	if(i == children.end()) {
		DBG_CF << "The config object has no child named ‘" << key << "’.";
		return utils::nullopt;
	}

	if(n < 0) {
		n = static_cast<int>(i->second.size()) + n;
	}

	try {
		return *i->second.at(n);
	} catch(const std::out_of_range&) {
		DBG_CF << "The config object has only ‘" << i->second.size() << "’ children named ‘" << key
			   << "’; request for the index ‘" << n << "’ cannot be honored.";
		return utils::nullopt;
	}
}

}

config& config::mandatory_child(config_key_type key, const std::string& parent)
{
	if(auto res = get_child_impl(children_, key, 0)) {
		return *res;
	} else {
		throw error("Mandatory WML child ‘[" + std::string(key) + "]’ missing in ‘" + parent + "’. Please report this bug.");
	}
}

const config& config::mandatory_child(config_key_type key, const std::string& parent) const
{
	if(auto res = get_child_impl(children_, key, 0)) {
		return *res;
	} else {
		throw error("Mandatory WML child ‘[" + std::string(key) + "]’ missing in ‘" + parent + "’. Please report this bug.");
	}
}

config& config::mandatory_child(config_key_type key, int n)
{
	if(auto res = get_child_impl(children_, key, n)) {
		return *res;
	} else {
		throw error("Child [" + std::string(key) + "] at index " + std::to_string(n) + " not found");
	}
}

const config& config::mandatory_child(config_key_type key, int n) const
{
	if(auto res = get_child_impl(children_, key, n)) {
		return *res;
	} else {
		throw error("Child [" + std::string(key) + "] at index " + std::to_string(n) + " not found");
	}
}

optional_config config::optional_child(config_key_type key, int n)
{
	return get_child_impl(children_, key, n);
}

optional_const_config config::optional_child(config_key_type key, int n) const
{
	return get_child_impl(children_, key, n);
}

const config& config::child_or_empty(config_key_type key) const
{
	static const config empty_cfg;
	child_map::const_iterator i = children_.find(key);
	if(i != children_.end() && !i->second.empty()) {
		return *i->second.front();
	}

	return empty_cfg;
}

config& config::child_or_add(config_key_type key)
{
	child_map::const_iterator i = children_.find(key);
	if(i != children_.end() && !i->second.empty()) {
		return *i->second.front();
	}

	return add_child(key);
}

optional_config_impl<const config> config::get_deprecated_child(config_key_type old_key, const std::string& in_tag, DEP_LEVEL level, const std::string& message) const
{
	if(auto res = optional_child(old_key)) {
		const std::string what = formatter() << "[" << in_tag << "][" << old_key << "]";
		deprecated_message(what, level, "", message);
		return res;
	}

	return utils::nullopt;
}

config::const_child_itors config::get_deprecated_child_range(config_key_type old_key, const std::string& in_tag, DEP_LEVEL level, const std::string& message) const
{
	static child_list dummy;
	const child_list* p = &dummy;

	if(auto i = children_.find(old_key); i != children_.end() && !i->second.empty()) {
		const std::string what = formatter() << "[" << in_tag << "][" << old_key << "]";
		deprecated_message(what, level, "", message);
		p = &i->second;
	}

	return const_child_itors(const_child_iterator(p->begin()), const_child_iterator(p->end()));
}

config& config::add_child(config_key_type key)
{
	auto iter = map_get(children_, key);
	child_list& v = iter->second;
	v.emplace_back(new config());
	ordered_children.emplace_back(iter, v.size() - 1);
	return *v.back();
}

config& config::add_child(config_key_type key, const config& val)
{
	auto iter = map_get(children_, key);
	child_list& v = iter->second;
	v.emplace_back(new config(val));
	ordered_children.emplace_back(iter, v.size() - 1);

	return *v.back();
}

config& config::add_child(config_key_type key, config&& val)
{
	auto iter = map_get(children_, key);
	child_list& v = iter->second;
	v.emplace_back(new config(std::move(val)));
	ordered_children.emplace_back(iter, v.size() - 1);

	return *v.back();
}

config& config::add_child_at(config_key_type key, const config& val, std::size_t index)
{
	auto iter = map_get(children_, key);
	child_list& v = iter->second;
	if(index > v.size()) {
		throw error("illegal index to add child at");
	}

	v.emplace(v.begin() + index, new config(val));

	bool inserted = false;

	const child_pos value(iter, index);

	std::vector<child_pos>::iterator ord = ordered_children.begin();
	for(; ord != ordered_children.end(); ++ord) {
		if(ord->pos != value.pos)
			continue;
		if(!inserted && ord->index == index) {
			ord = ordered_children.insert(ord, value);
			inserted = true;
		} else if(ord->index >= index) {
			ord->index++;
		}
	}

	if(!inserted) {
		ordered_children.push_back(value);
	}

	return *v[index];
}

size_t config::find_total_first_of(config_key_type key, size_t start)
{
	assert(start <= ordered_children.size());
	const size_t npos = static_cast<size_t>(-1);

	auto pos = std::find_if(ordered_begin() + start, ordered_end(), [&](const config::any_child& can){ return can.key == key; });

	if(pos == ordered_end()) {
		return npos;
	}

	return static_cast<size_t>(pos - ordered_begin());
}

config& config::add_child_at_total(config_key_type key, const config &val, std::size_t pos)
{
	assert(pos <= ordered_children.size());
	if(pos == ordered_children.size()) {
		//optimisation
		return config::add_child(key, val);
	}

	auto end = ordered_children.end();
	auto pos_it = ordered_children.begin() + pos;
	auto next = std::find_if(pos_it, end,[&](const child_pos& p){ return p.pos->first == key; });

	if(next == end) {
		config& res = config::add_child(key, val);
		//rotate the just inserted element to position pos.
		std::rotate(ordered_children.begin() + pos, ordered_children.end() - 1, ordered_children.end());
		return res;
	}

	auto pl = next->pos;
	child_list& l = pl->second;
	const auto index = next->index;
	config& res = **(l.emplace(l.begin() + index, new config(val)));

	for(auto ord = next; ord != end; ++ord) {
		//this changes next->index and all later refernces to that tag.
		if(ord->pos == pl) {
			++ord->index;
		}
	}

	//finally insert our new child in ordered_children.
	ordered_children.insert(pos_it, { pl, index });
	return res;
}

namespace
{
struct remove_ordered
{
	remove_ordered(const config::child_map::iterator& iter)
		: iter_(iter)
	{
	}

	bool operator()(const config::child_pos& pos) const
	{
		return pos.pos == iter_;
	}

private:
	config::child_map::iterator iter_;
};
} // end anon namespace

void config::clear_children_impl(config_key_type key)
{
	child_map::iterator i = children_.find(key);
	if(i == children_.end())
		return;

	utils::erase_if(ordered_children, remove_ordered{i});
	children_.erase(i);
}

void config::splice_children(config& src, config_key_type key)
{
	child_map::iterator i_src = src.children_.find(key);
	if(i_src == src.children_.end()) {
		return;
	}

	utils::erase_if(src.ordered_children, remove_ordered{i_src});

	auto i_dst = map_get(children_, key);
	child_list& dst = i_dst->second;

	const auto before = dst.size();
	dst.insert(dst.end(), std::make_move_iterator(i_src->second.begin()), std::make_move_iterator(i_src->second.end()));
	src.children_.erase(i_src);
	// key might be a reference to i_src->first, so it is no longer usable.

	for(std::size_t j = before; j < dst.size(); ++j) {
		ordered_children.emplace_back(i_dst, j);
	}
}

void config::recursive_clear_value(config_key_type key)
{
	map_erase_key(values_, key);

	for(std::pair<const std::string, child_list>& p : children_) {
		for(auto& cfg : p.second) {
			cfg->recursive_clear_value(key);
		}
	}
}

std::vector<config::child_pos>::iterator config::remove_child(const child_map::iterator& pos, std::size_t index)
{
	/* Find the position with the correct index and decrement all the
	   indices in the ordering that are above this index. */
	std::size_t found = 0;
	for(child_pos& p : ordered_children) {
		if(p.pos != pos) {
			continue;
		}

		if(p.index == index) {
			found = &p - &ordered_children.front();
		} else if(p.index > index) {
			--p.index;
		}
	}

	// Remove from the child map.
	pos->second.erase(pos->second.begin() + index);

	// Erase from the ordering and return the next position.
	return ordered_children.erase(ordered_children.begin() + found);
}

config::all_children_iterator config::erase(const config::all_children_iterator& i)
{
	return all_children_iterator(remove_child(i.i_->pos, i.i_->index));
}

void config::remove_child(config_key_type key, std::size_t index)
{
	child_map::iterator i = children_.find(key);
	if(i == children_.end() || index >= i->second.size()) {
		ERR_CF << "Error: attempting to delete non-existing child: " << key << "[" << index << "]";
		return;
	}

	remove_child(i, index);
}

void config::remove_children(config_key_type key, const std::function<bool(const config&)>& p)
{
	child_map::iterator pos = children_.find(key);
	if(pos == children_.end()) {
		return;
	}

	const auto predicate = [p](const std::unique_ptr<config>& child)
	{
		return !p || p(*child);
	};

	auto child_it = std::find_if(pos->second.begin(), pos->second.end(), predicate);
	while(child_it != pos->second.end()) {
		const auto index = std::distance(pos->second.begin(), child_it);
		remove_child(pos, index);
		child_it = std::find_if(pos->second.begin() + index, pos->second.end(), predicate);
	}
}

const config::attribute_value& config::operator[](config_key_type key) const
{
	const attribute_map::const_iterator i = values_.find(key);
	if(i != values_.end()) {
		return i->second;
	}

	static const attribute_value empty_attribute;
	return empty_attribute;
}

const config::attribute_value* config::get(config_key_type key) const
{
	attribute_map::const_iterator i = values_.find(key);
	return i != values_.end() ? &i->second : nullptr;
}

const config::attribute_value& config::get_or(const config_key_type key, const config_key_type default_key) const
{
    const config::attribute_value & value = operator[](key);
    return !value.blank() ? value : operator[](default_key);
}

config::attribute_value& config::operator[](config_key_type key)
{
	auto res = values_.lower_bound(key);

	if(res == values_.end() || key != res->first) {
		res = values_.emplace_hint(res, std::piecewise_construct, std::forward_as_tuple(key), std::tuple<>());
	}

	return res->second;
}

const config::attribute_value& config::get_old_attribute(config_key_type key, const std::string& old_key, const std::string& in_tag, const std::string& message) const
{
	if(has_attribute(old_key)) {
		const std::string what = formatter() << "[" << in_tag << "]" << old_key << "=";
		const std::string msg  = formatter() << "Use " << key << "= instead. " << message;
		deprecated_message(what, DEP_LEVEL::INDEFINITE, "", msg);
	}

	attribute_map::const_iterator i = values_.find(key);
	if(i != values_.end()) {
		return i->second;
	}

	i = values_.find(old_key);
	if(i != values_.end()) {
		return i->second;
	}

	static const attribute_value empty_attribute;
	return empty_attribute;
}

const config::attribute_value& config::get_deprecated_attribute(config_key_type old_key, const std::string& in_tag, DEP_LEVEL level, const std::string& message) const
{
	if(auto i = values_.find(old_key); i != values_.end()) {
		const std::string what = formatter() << "[" << in_tag << "]" << old_key << "=";
		deprecated_message(what, level, "", message);
		return i->second;
	}

	static const attribute_value empty_attribute;
	return empty_attribute;
}

void config::merge_attributes(const config& cfg)
{
	assert(this != &cfg);
	for(const auto& [key, value] : cfg.values_) {
		if(key.substr(0, 7) == "add_to_") {
			std::string add_to = key.substr(7);
			values_[add_to] = values_[add_to].to_double() + value.to_double();
		} else if(key.substr(0, 10) == "concat_to_") {
			std::string concat_to = key.substr(10);
			// TODO: Only use t_string if one or both are actually translatable?
			// That probably requires using a visitor though.
			values_[concat_to] = values_[concat_to].t_str() + value.t_str();
		} else {
			values_[key] = value;
		}
	}
}

config::const_attr_itors config::attribute_range() const
{
	const_attr_itors range(const_attribute_iterator(values_.begin()), const_attribute_iterator(values_.end()));

	// Ensure the first element is not blank, as a few places assume this
	while(range.begin() != range.end() && range.begin()->second.blank()) {
		range.pop_front();
	}

	return range;
}

config::attr_itors config::attribute_range()
{
	attr_itors range(attribute_iterator(values_.begin()), attribute_iterator(values_.end()));

	// Ensure the first element is not blank, as a few places assume this
	while(range.begin() != range.end() && range.begin()->second.blank()) {
		range.pop_front();
	}

	return range;
}

optional_config config::find_child(config_key_type key, const std::string& name, const std::string& value)
{
	const child_map::iterator i = children_.find(key);
	if(i == children_.end()) {
		DBG_CF << "Key ‘" << name << "’ value ‘" << value << "’ pair not found as child of key ‘" << key << "’.";


		return utils::nullopt;
	}

	const child_list::iterator j = std::find_if(i->second.begin(), i->second.end(),
		[&](const std::unique_ptr<config>& pcfg) {
			const config& cfg = *pcfg;
			return cfg[name] == value;
		}
	);

	if(j != i->second.end()) {
		return **j;
	}

	DBG_CF << "Key ‘" << name << "’ value ‘" << value << "’ pair not found as child of key ‘" << key << "’.";

	return utils::nullopt;
}

config& config::find_mandatory_child(config_key_type key, const std::string &name, const std::string &value)
{
	auto res = find_child(key, name, value);
	if(res) {
		return *res;
	}
	throw error("Cannot find child [" + std::string(key) + "] with " + name + "=" + value);
}

const config& config::find_mandatory_child(config_key_type key, const std::string &name, const std::string &value) const
{
	auto res = find_child(key, name, value);
	if(res) {
		return *res;
	}
	throw error("Cannot find child [" + std::string(key) + "] with " + name + "=" + value);
}

void config::clear()
{
	// No validity check for this function.
	children_.clear();
	values_.clear();
	ordered_children.clear();
}

void config::clear_all_children()
{
	// No validity check for this function.
	children_.clear();
	ordered_children.clear();
}

void config::clear_attributes()
{
	// No validity check for this function.
	values_.clear();
}

bool config::empty() const
{
	return children_.empty() && values_.empty();
}

config::all_children_iterator::reference config::all_children_iterator::operator*() const
{
	return any_child(&i_->pos->first, i_->pos->second[i_->index].get());
}

config::const_all_children_iterator::reference config::const_all_children_iterator::operator*() const
{
	return any_child(&i_->pos->first, i_->pos->second[i_->index].get());
}

config::const_all_children_iterator config::ordered_begin() const
{
	return const_all_children_iterator(ordered_children.cbegin());
}

config::const_all_children_iterator config::ordered_cbegin() const
{
	return const_all_children_iterator(ordered_children.cbegin());
}

config::const_all_children_iterator config::ordered_end() const
{
	return const_all_children_iterator(ordered_children.cend());
}

config::const_all_children_iterator config::ordered_cend() const
{
	return const_all_children_iterator(ordered_children.cend());
}

config::const_all_children_itors config::all_children_range() const
{
	return const_all_children_itors(
		const_all_children_iterator(ordered_children.cbegin()),
		const_all_children_iterator(ordered_children.cend())
	);
}

config::all_children_iterator config::ordered_begin()
{
	return all_children_iterator(ordered_children.begin());
}

config::all_children_iterator config::ordered_end()
{
	return all_children_iterator(ordered_children.end());
}

config::all_children_itors config::all_children_range()
{
	return all_children_itors(
		all_children_iterator(ordered_children.begin()),
		all_children_iterator(ordered_children.end())
	);
}

config config::get_diff(const config& c) const
{
	config res;
	get_diff(c, res);
	return res;
}

void config::get_diff(const config& c, config& res) const
{
	config* inserts = nullptr;

	for(const auto& [key, value] : values_) {
		if(value.blank()) {
			continue;
		}

		const attribute_map::const_iterator j = c.values_.find(key);
		if(j == c.values_.end() || (value != j->second && !value.blank())) {
			if(inserts == nullptr) {
				inserts = &res.add_child("insert");
			}

			(*inserts)[key] = value;
		}
	}

	config* deletes = nullptr;

	for(const auto& [key, value] : c.values_) {
		if(value.blank()) {
			continue;
		}

		const attribute_map::const_iterator itor = values_.find(key);
		if(itor == values_.end() || itor->second.blank()) {
			if(deletes == nullptr) {
				deletes = &res.add_child("delete");
			}

			(*deletes)[key] = "x";
		}
	}

	std::vector<std::string> entities;

	for(const auto& child : children_) {
		entities.push_back(child.first);
	}

	for(const auto& child : c.children_) {
		if(children_.count(child.first) == 0) {
			entities.push_back(child.first);
		}
	}

	for(const std::string& entity : entities) {
		const child_map::const_iterator itor_a = children_.find(entity);
		const child_map::const_iterator itor_b = c.children_.find(entity);

		static const child_list dummy;

		// Get the two child lists. 'b' has to be modified to look like 'a'.
		const child_list& a = itor_a != children_.end() ? itor_a->second : dummy;
		const child_list& b = itor_b != c.children_.end() ? itor_b->second : dummy;

		std::size_t ndeletes = 0;
		std::size_t ai = 0, bi = 0;
		while(ai != a.size() || bi != b.size()) {
			// If the two elements are the same, nothing needs to be done.
			if(ai < a.size() && bi < b.size() && *a[ai] == *b[bi]) {
				++ai;
				++bi;
			} else {
				// We have to work out what the most appropriate operation --
				// delete, insert, or change is the best to get b[bi] looking like a[ai].
				std::stringstream buf;

				// If b has more elements than a, then we assume this element
				// is an element that needs deleting.
				if(b.size() - bi > a.size() - ai) {
					config& new_delete = res.add_child("delete_child");
					buf << bi - ndeletes;
					new_delete.values_["index"] = buf.str();
					new_delete.add_child(entity);

					++ndeletes;
					++bi;
				}

				// If b has less elements than a, then we assume this element
				// is an element that needs inserting.
				else if(b.size() - bi < a.size() - ai) {
					config& new_insert = res.add_child("insert_child");
					buf << ai;
					new_insert.values_["index"] = buf.str();
					new_insert.add_child(entity, *a[ai]);

					++ai;
				}

				// Otherwise, they have the same number of elements,
				// so try just changing this element to match.
				else {
					config& new_change = res.add_child("change_child");
					buf << bi;
					new_change.values_["index"] = buf.str();
					new_change.add_child(entity, a[ai]->get_diff(*b[bi]));

					++ai;
					++bi;
				}
			}
		}
	}
}

void config::apply_diff(const config& diff, bool track /* = false */)
{
	if(track) {
		values_[diff_track_attribute] = "modified";
	}

	if(const auto inserts = diff.optional_child("insert")) {
		for(const auto& [key, value] : inserts->attribute_range()) {
			values_[key] = value;
		}
	}

	if(const auto deletes = diff.optional_child("delete")) {
		for(const attribute& v : deletes->attribute_range()) {
			values_.erase(v.first);
		}
	}

	for(const config& i : diff.child_range("change_child")) {
		const std::size_t index = lexical_cast<std::size_t>(i["index"].str());
		for(const auto [key, cfg] : i.all_children_view()) {
			if(key.empty()) {
				continue;
			}

			const child_map::iterator itor = children_.find(key);
			if(itor == children_.end() || index >= itor->second.size()) {
				throw error("error in diff: could not find element '" + key + "'");
			}

			itor->second[index]->apply_diff(cfg, track);
		}
	}

	for(const config& i : diff.child_range("insert_child")) {
		const auto index = lexical_cast<std::size_t>(i["index"].str());
		for(const auto [key, cfg] : i.all_children_view()) {
			config& inserted = add_child_at(key, cfg, index);
			if(track) {
				inserted[diff_track_attribute] = "new";
			}
		}
	}

	for(const config& i : diff.child_range("delete_child")) {
		const auto index = lexical_cast<std::size_t>(i["index"].str());
		for(const auto [key, cfg] : i.all_children_view()) {
			if(!track) {
				remove_child(key, index);
			} else {
				const child_map::iterator itor = children_.find(key);
				if(itor == children_.end() || index >= itor->second.size()) {
					throw error("error in diff: could not find element '" + key + "'");
				}

				itor->second[index]->values_[diff_track_attribute] = "deleted";
			}
		}
	}
}

void config::clear_diff_track(const config& diff)
{
	remove_attribute(diff_track_attribute);
	for(const config& i : diff.child_range("delete_child")) {
		const auto index = lexical_cast<std::size_t>(i["index"].str());
		for(const auto [key, cfg] : i.all_children_view()) {
			remove_child(key, index);
		}
	}

	for(const config& i : diff.child_range("change_child")) {
		const std::size_t index = lexical_cast<std::size_t>(i["index"].str());
		for(const auto [key, cfg] : i.all_children_view()) {
			if(key.empty()) {
				continue;
			}

			const child_map::iterator itor = children_.find(key);
			if(itor == children_.end() || index >= itor->second.size()) {
				throw error("error in diff: could not find element '" + key + "'");
			}

			itor->second[index]->clear_diff_track(cfg);
		}
	}

	for(std::pair<const std::string, child_list>& p : children_) {
		for(auto& cfg : p.second) {
			cfg->remove_attribute(diff_track_attribute);
		}
	}
}

/**
 * Merge config 'c' into this config, overwriting this config's values.
 */
void config::merge_with(const config& c)
{
	std::vector<child_pos> to_remove;
	std::map<std::string, unsigned> visitations;

	// Merge attributes first
	merge_attributes(c);

	// Now merge shared tags
	all_children_iterator::Itor i, i_end = ordered_children.end();
	for(i = ordered_children.begin(); i != i_end; ++i) {
		const std::string& tag = i->pos->first;
		const child_map::const_iterator j = c.children_.find(tag);

		if(j != c.children_.end()) {
			unsigned& visits = visitations[tag];

			if(visits < j->second.size()) {
				// Get a const config so we do not add attributes.
				const config& merge_child = *j->second[visits++];

				if(merge_child["__remove"].to_bool()) {
					to_remove.push_back(*i);
				} else {
					(i->pos->second[i->index])->merge_with(merge_child);
				}
			}
		}
	}

	// Now add any unvisited tags
	for(const auto& [tag, list] : c.children_) {
		unsigned& visits = visitations[tag];
		while(visits < list.size()) {
			add_child(tag, *list[visits++]);
		}
	}

	// Remove those marked so
	std::map<std::string, std::size_t> removals;
	for(const child_pos& pos : to_remove) {
		const std::string& tag = pos.pos->first;
		auto& removes = removals[tag];
		remove_child(tag, pos.index - removes++);
	}
}

/**
 * Merge config 'c' into this config, preserving this config's values.
 */
void config::inherit_from(const config& c)
{
	// Using a scratch config and merge_with() seems to execute about as fast
	// as direct coding of this merge.
	config scratch(c);
	scratch.merge_with(*this);
	swap(scratch);
}

/**
 * Merge the attributes of config 'c' into this config, preserving this config's values.
 */
void config::inherit_attributes(const config& cfg)
{
	for(const auto& [key, value] : cfg.values_) {
		attribute_value& v2 = values_[key];
		if(v2.blank()) {
			v2 = value;
		}
	}
}
bool config::matches(const config& filter) const
{
	bool result = true;

	for(const auto& [key, value] : filter.attribute_range()) {
		if(key.compare(0, 8, "glob_on_") == 0) {
			const attribute_value* v = get(key.substr(8));
			if(!v || !utils::wildcard_string_match(v->str(), value.str())) {
				result = false;
				break;
			}
		} else {
			const attribute_value* v = get(key);
			if(!v || *v != value) {
				result = false;
				break;
			}
		}
	}

	for(const auto [key, cfg] : filter.all_children_view()) {
		if(key == "not") {
			result = result && !matches(cfg);
			continue;
		} else if(key == "and") {
			result = result && matches(cfg);
			continue;
		} else if(key == "or") {
			result = result || matches(cfg);
			continue;
		}

		bool found = false;
		for(const config& j : child_range(key)) {
			if(j.matches(cfg)) {
				found = true;
				break;
			}
		}

		result = result && found;
	}

	return result;
}

std::string config::debug() const
{
	std::ostringstream outstream;
	outstream << *this;
	return outstream.str();
}

std::ostream& operator<<(std::ostream& outstream, const config& cfg)
{
	static int i = 0;
	i++;

	for(const auto& [key, value] : cfg.attribute_range()) {
		if(value.blank()) {
			continue;
		}

		for(int j = 0; j < i - 1; j++) {
			outstream << '\t';
		}

		outstream << key << " = " << value << '\n';
	}

	for(const auto [key, cfg] : cfg.all_children_view()) {
		for(int j = 0; j < i - 1; ++j) {
			outstream << '\t';
		}

		outstream << "[" << key << "]\n";
		outstream << cfg;

		for(int j = 0; j < i - 1; ++j) {
			outstream << '\t';
		}

		outstream << "[/" << key << "]\n";
	}

	i--;
	return outstream;
}

std::string config::hash() const
{
	static const unsigned int hash_length = 128;
	static const char hash_string[] = "+-,.<>0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char hash_str[hash_length + 1];

	unsigned int i;
	for(i = 0; i != hash_length; ++i) {
		hash_str[i] = 'a';
	}

	hash_str[hash_length] = 0;

	i = 0;
	for(const auto& [key, value] : values_) {
		if(value.blank()) {
			continue;
		}

		for(char c : key) {
			hash_str[i] ^= c;
			if(++i == hash_length) {
				i = 0;
			}
		}

		std::string base_str = value.t_str().base_str();
		for(const char c : base_str) {
			hash_str[i] ^= c;
			if(++i == hash_length) {
				i = 0;
			}
		}
	}

	for(const auto [key, cfg] : all_children_view()) {
		std::string child_hash = cfg.hash();
		for(char c : child_hash) {
			hash_str[i] ^= c;
			++i;
			if(i == hash_length) {
				i = 0;
			}
		}
	}

	for(i = 0; i != hash_length; ++i) {
		hash_str[i] = hash_string[static_cast<unsigned>(hash_str[i]) % strlen(hash_string)];
	}

	return std::string(hash_str);
}

void config::swap(config& cfg)
{
	values_.swap(cfg.values_);
	children_.swap(cfg.children_);
	ordered_children.swap(cfg.ordered_children);
}

void swap(config& lhs, config& rhs)
{
	lhs.swap(rhs);
}

bool config::validate_wml() const
{
	return std::all_of(children_.begin(), children_.end(), [](const auto& pair)
	{
		return valid_tag(pair.first) &&
			std::all_of(pair.second.begin(), pair.second.end(),
			[](const auto& c) { return c->validate_wml(); });
	}) &&
		std::all_of(values_.begin(), values_.end(), [](const auto& pair)
	{
		return valid_attribute(pair.first);
	});
}

bool operator==(const config& a, const config& b)
{
	if(a.values_ != b.values_) {
		return false;
	}

	config::const_all_children_itors x = a.all_children_range(), y = b.all_children_range();
	for(; !x.empty() && !y.empty(); x.pop_front(), y.pop_front()) {
		if(x.front().key != y.front().key || x.front().cfg != y.front().cfg) {
			return false;
		}
	}

	return x.empty() && y.empty();
}
