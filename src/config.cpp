/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2018 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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
 * Routines related to configuration-files / WML.
 */

#include "config.hpp"

#include "lexical_cast.hpp"
#include "log.hpp"
#include "utils/const_clone.hpp"
#include "utils/functional.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <istream>
#include <locale>

#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/variant.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define DBG_CF LOG_STREAM(debug, log_config)

namespace
{
// std::map::operator[] does not support heterogeneous lookup so we need this to work around.
template<typename Map, typename Key>
typename Map::mapped_type& map_get(Map& map, Key&& key)
{
	auto res = map.lower_bound(key);

	if(res == map.end() || key != res->first) {
		res = map.emplace_hint(res, std::piecewise_construct, std::forward_as_tuple(key), std::tuple<>());
	}

	return res->second;
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

struct config_implementation
{
	/**
	 * Implementation for the wrappers for
	 * [const] config& child(const std::string& key, const std::string& parent);
	 *
	 * @tparam T                  A pointer to the config.
	 */
	template<class T>
	static utils::const_clone_ref<config, T> child(T config, config_key_type key, const std::string& parent)
	{
		config->check_valid();

		assert(!parent.empty());
		assert(parent[0] == '[');
		assert(parent[parent.size() - 1] == ']');

		if(config->has_child(key)) {
			return *(config->children_.find(key)->second.front());
		}

		/**
		 * @todo Implement a proper wml_exception here.
		 *
		 * at the moment there seem to be dependency issues, which i don't want
		 * to fix right now.
		 */
		// FAIL(missing_mandatory_wml_section(parent, key));

		std::stringstream sstr;
		sstr << "Mandatory WML child »[" << key << "]« missing in »" << parent << "«. Please report this bug.";

		throw config::error(sstr.str());
	}
};

/* ** config implementation ** */

config config::invalid;

const char* config::diff_track_attribute = "__diff_track";

void config::check_valid() const
{
	if(!*this) {
		throw error("Mandatory WML child missing yet untested for. Please report.");
	}
}

void config::check_valid(const config& cfg) const
{
	if(!*this || !cfg) {
		throw error("Mandatory WML child missing yet untested for. Please report.");
	}
}

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
	} else if(name[0] == '_') {
		// Underscore can't be the first character
		return false;
	} else {
		return std::all_of(name.begin(), name.end(), [](const char& c)
		{
			// Only alphanumeric ASCII characters and underscores are allowed
			return std::isalnum(c, std::locale::classic()) || c == '_';
		});
	}
}

bool config::valid_attribute(config_key_type name)
{
	if(name.empty()) {
		return false;
	}

	for(char c : name) {
		if(!std::isalnum(c, std::locale::classic()) && c != '_') {
			return false;
		}
	}

	return true;
}

bool config::has_attribute(config_key_type key) const
{
	check_valid();
	return values_.find(key) != values_.end();
}

bool config::has_old_attribute(config_key_type key, const std::string& old_key, const std::string& msg) const
{
	check_valid();
	if(values_.find(key) != values_.end()) {
		return true;
	} else if(values_.find(old_key) != values_.end()) {
		if(!msg.empty()) {
			lg::wml_error() << msg;
		}

		return true;
	}

	return false;
}

void config::remove_attribute(config_key_type key)
{
	check_valid();
	map_erase_key(values_, key);
}

void config::append_children(const config& cfg)
{
	check_valid(cfg);

	for(const any_child& value : cfg.all_children_range()) {
		add_child(value.key, value.cfg);
	}
}

void config::append_children(config&& cfg)
{
	check_valid(cfg);

#if 0
	//For some unknown reason this doesn't compile.
	if(children_.empty()) {
		//optimisation
		children_ = std::move(cfg.children_);
		ordered_children = std::move(cfg.ordered_children);
		cfg.clear_all_children();
		return;
	}
#endif
	for(const any_child& value : cfg.all_children_range()) {
		add_child(value.key, std::move(value.cfg));
	}
	cfg.clear_all_children();
}

void config::append_attributes(const config& cfg)
{
	check_valid(cfg);
	for(const attribute& v : cfg.values_) {
		values_[v.first] = v.second;
	}
}

void config::append_children(const config& cfg, const std::string& key)
{
	check_valid(cfg);

	for(const config& value : cfg.child_range(key)) {
		add_child(key, value);
	}
}

void config::append(const config& cfg)
{
	append_children(cfg);
	for(const attribute& v : cfg.values_) {
		values_[v.first] = v.second;
	}
}

void config::append(config&& cfg)
{
	append_children(std::move(cfg));

	if(values_.empty()) {
		//optimisation.
		values_ = std::move(cfg.values_);
	}
	else {
		for(const attribute& v : cfg.values_) {
			//TODO: move the attributes as well?
			values_[v.first] = v.second;
		}
	}
	cfg.clear_attibutes();
}

void config::append_children_by_move(config& cfg, const std::string& key)
{
	check_valid(cfg);

	// DO note this leaves the tags empty in the source config. Not sure if
	// that should be changed.
	for(config& value : cfg.child_range(key)) {
		add_child(key, std::move(value));
	}
}

void config::merge_children(const std::string& key)
{
	check_valid();

	if(child_count(key) < 2) {
		return;
	}

	config merged_children;
	for(const config& cfg : child_range(key)) {
		merged_children.append(cfg);
	}

	clear_children_impl(key);
	add_child(key, merged_children);
}

void config::merge_children_by_attribute(const std::string& key, const std::string& attribute)
{
	check_valid();

	if(child_count(key) < 2) {
		return;
	}

	typedef std::map<std::string, config> config_map;
	config_map merged_children_map;
	for(const config& cfg : child_range(key)) {
		merged_children_map[cfg[attribute]].append(cfg);
	}

	clear_children_impl(key);
	for(const config_map::value_type& i : merged_children_map) {
		add_child(key, i.second);
	}
}

config::child_itors config::child_range(config_key_type key)
{
	check_valid();

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
	check_valid();

	child_map::const_iterator i = children_.find(key);
	static child_list dummy;
	const child_list* p = &dummy;
	if(i != children_.end()) {
		p = &i->second;
	}

	return const_child_itors(const_child_iterator(p->begin()), const_child_iterator(p->end()));
}

unsigned config::child_count(config_key_type key) const
{
	check_valid();

	child_map::const_iterator i = children_.find(key);
	if(i != children_.end()) {
		return i->second.size();
	}

	return 0;
}

unsigned config::all_children_count() const
{
	return ordered_children.size();
}

unsigned config::attribute_count() const
{
	return std::count_if(values_.begin(), values_.end(), [](const attribute& v) { return !v.second.blank(); });
}

bool config::has_child(config_key_type key) const
{
	check_valid();

	return children_.find(key) != children_.end();
}

config& config::child(config_key_type key, int n)
{
	check_valid();

	const child_map::const_iterator i = children_.find(key);
	if(i == children_.end()) {
		DBG_CF << "The config object has no child named »" << key << "«.\n";

		return invalid;
	}

	if(n < 0)
		n = i->second.size() + n;
	if(size_t(n) < i->second.size()) {
		return *i->second[n];
	} else {
		DBG_CF << "The config object has only »" << i->second.size() << "« children named »" << key
			   << "«; request for the index »" << n << "« cannot be honored.\n";

		return invalid;
	}
}

config& config::child(config_key_type key, const std::string& parent)
{
	return config_implementation::child(this, key, parent);
}

const config& config::child(config_key_type key, const std::string& parent) const
{
	return config_implementation::child(this, key, parent);
}

const config& config::child_or_empty(config_key_type key) const
{
	static const config empty_cfg;
	check_valid();

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

config& config::add_child(config_key_type key)
{
	check_valid();

	child_list& v = map_get(children_, key);
	v.emplace_back(new config());
	ordered_children.emplace_back(children_.find(key), v.size() - 1);
	return *v.back();
}

config& config::add_child(config_key_type key, const config& val)
{
	check_valid(val);

	child_list& v = map_get(children_, key);
	v.emplace_back(new config(val));
	ordered_children.emplace_back(children_.find(key), v.size() - 1);

	return *v.back();
}

config& config::add_child(config_key_type key, config&& val)
{
	check_valid(val);

	child_list& v = map_get(children_, key);
	v.emplace_back(new config(std::move(val)));
	ordered_children.emplace_back(children_.find(key), v.size() - 1);

	return *v.back();
}

config& config::add_child_at(config_key_type key, const config& val, unsigned index)
{
	check_valid(val);

	child_list& v = map_get(children_, key);
	if(index > v.size()) {
		throw error("illegal index to add child at");
	}

	v.emplace(v.begin() + index, new config(val));

	bool inserted = false;

	const child_pos value(children_.find(key), index);

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
	check_valid();

	child_map::iterator i = children_.find(key);
	if(i == children_.end())
		return;

	ordered_children.erase(
		std::remove_if(ordered_children.begin(), ordered_children.end(), remove_ordered(i)),
		ordered_children.end());

	children_.erase(i);
}

void config::splice_children(config& src, const std::string& key)
{
	check_valid(src);

	child_map::iterator i_src = src.children_.find(key);
	if(i_src == src.children_.end()) {
		return;
	}

	src.ordered_children.erase(
		std::remove_if(src.ordered_children.begin(), src.ordered_children.end(), remove_ordered(i_src)),
		src.ordered_children.end());

	child_list& dst = map_get(children_, key);
	child_map::iterator i_dst = children_.find(key);

	unsigned before = dst.size();
	dst.insert(dst.end(), std::make_move_iterator(i_src->second.begin()), std::make_move_iterator(i_src->second.end()));
	src.children_.erase(i_src);
	// key might be a reference to i_src->first, so it is no longer usable.

	for(unsigned j = before; j < dst.size(); ++j) {
		ordered_children.emplace_back(i_dst, j);
	}
}

void config::recursive_clear_value(config_key_type key)
{
	check_valid();

	map_erase_key(values_, key);

	for(std::pair<const std::string, child_list>& p : children_) {
		for(auto& cfg : p.second) {
			cfg->recursive_clear_value(key);
		}
	}
}

std::vector<config::child_pos>::iterator config::remove_child(const child_map::iterator& pos, unsigned index)
{
	/* Find the position with the correct index and decrement all the
	   indices in the ordering that are above this index. */
	unsigned found = 0;
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

void config::remove_child(config_key_type key, unsigned index)
{
	check_valid();

	child_map::iterator i = children_.find(key);
	if(i == children_.end() || index >= i->second.size()) {
		ERR_CF << "Error: attempting to delete non-existing child: " << key << "[" << index << "]\n";
		return;
	}

	remove_child(i, index);
}

void config::remove_children(config_key_type key, std::function<bool(const config&)> p)
{
	check_valid();

	child_map::iterator pos = children_.find(key);
	if(pos == children_.end()) {
		return;
	}

	const auto predicate = [p](const std::unique_ptr<config>& child)
	{
		return p(*child);
	};

	auto child_it = std::find_if(pos->second.begin(), pos->second.end(), predicate);
	while(child_it != pos->second.end()) {
		unsigned index = std::distance(pos->second.begin(), child_it);
		remove_child(pos, index);
		child_it = std::find_if(pos->second.begin() + index, pos->second.end(), predicate);
	}
}

const config::attribute_value& config::operator[](config_key_type key) const
{
	check_valid();

	const attribute_map::const_iterator i = values_.find(key);
	if(i != values_.end()) {
		return i->second;
	}

	static const attribute_value empty_attribute;
	return empty_attribute;
}

const config::attribute_value* config::get(config_key_type key) const
{
	check_valid();
	attribute_map::const_iterator i = values_.find(key);
	return i != values_.end() ? &i->second : nullptr;
}

config::attribute_value& config::operator[](config_key_type key)
{
	check_valid();

	auto res = values_.lower_bound(key);

	if(res == values_.end() || key != res->first) {
		res = values_.emplace_hint(res, std::piecewise_construct, std::forward_as_tuple(key), std::tuple<>());
	}

	return res->second;
}

const config::attribute_value& config::get_old_attribute(
		config_key_type key, const std::string& old_key, const std::string& msg) const
{
	check_valid();

	attribute_map::const_iterator i = values_.find(key);
	if(i != values_.end()) {
		return i->second;
	}

	i = values_.find(old_key);
	if(i != values_.end()) {
		if(!msg.empty()) {
			lg::wml_error() << msg;
		}

		return i->second;
	}

	static const attribute_value empty_attribute;
	return empty_attribute;
}

void config::merge_attributes(const config& cfg)
{
	check_valid(cfg);

	assert(this != &cfg);
	for(const attribute& v : cfg.values_) {
		std::string key = v.first;
		if(key.substr(0, 7) == "add_to_") {
			std::string add_to = key.substr(7);
			values_[add_to] = values_[add_to].to_double() + v.second.to_double();
		} else if(key.substr(0, 10) == "concat_to_") {
			std::string concat_to = key.substr(10);
			// TODO: Only use t_string if one or both are actually translatable?
			// That probably requires using a visitor though.
			values_[concat_to] = values_[concat_to].t_str() + v.second.t_str();
		} else {
			values_[v.first] = v.second;
		}
	}
}

config::const_attr_itors config::attribute_range() const
{
	check_valid();

	const_attr_itors range(const_attribute_iterator(values_.begin()), const_attribute_iterator(values_.end()));

	// Ensure the first element is not blank, as a few places assume this
	while(range.begin() != range.end() && range.begin()->second.blank()) {
		range.pop_front();
	}

	return range;
}

config::attr_itors config::attribute_range()
{
	check_valid();
	attr_itors range(attribute_iterator(values_.begin()), attribute_iterator(values_.end()));

	// Ensure the first element is not blank, as a few places assume this
	while(range.begin() != range.end() && range.begin()->second.blank()) {
		range.pop_front();
	}

	return range;
}

config& config::find_child(config_key_type key, const std::string& name, const std::string& value)
{
	check_valid();

	const child_map::iterator i = children_.find(key);
	if(i == children_.end()) {
		DBG_CF << "Key »" << name << "« value »" << value << "« pair not found as child of key »" << key << "«.\n";

		return invalid;
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

	DBG_CF << "Key »" << name << "« value »" << value << "« pair not found as child of key »" << key << "«.\n";

	return invalid;
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

void config::clear_attibutes()
{
	// No validity check for this function.
	values_.clear();
}

bool config::empty() const
{
	check_valid();

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
	check_valid(c);

	config res;
	get_diff(c, res);
	return res;
}

void config::get_diff(const config& c, config& res) const
{
	check_valid(c);
	check_valid(res);

	config* inserts = nullptr;

	for(const auto& v : values_) {
		if(v.second.blank()) {
			continue;
		}

		const attribute_map::const_iterator j = c.values_.find(v.first);
		if(j == c.values_.end() || (v.second != j->second && !v.second.blank())) {
			if(inserts == nullptr) {
				inserts = &res.add_child("insert");
			}

			(*inserts)[v.first] = v.second;
		}
	}

	config* deletes = nullptr;

	for(const auto& v : c.values_) {
		if(v.second.blank()) {
			continue;
		}

		const attribute_map::const_iterator itor = values_.find(v.first);
		if(itor == values_.end() || itor->second.blank()) {
			if(deletes == nullptr) {
				deletes = &res.add_child("delete");
			}

			(*deletes)[v.first] = "x";
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

		size_t ndeletes = 0;
		size_t ai = 0, bi = 0;
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
	check_valid(diff);

	if(track) {
		values_[diff_track_attribute] = "modified";
	}

	if(const config& inserts = diff.child("insert")) {
		for(const attribute& v : inserts.attribute_range()) {
			values_[v.first] = v.second;
		}
	}

	if(const config& deletes = diff.child("delete")) {
		for(const attribute& v : deletes.attribute_range()) {
			values_.erase(v.first);
		}
	}

	for(const config& i : diff.child_range("change_child")) {
		const size_t index = lexical_cast<size_t>(i["index"].str());
		for(const any_child& item : i.all_children_range()) {
			if(item.key.empty()) {
				continue;
			}

			const child_map::iterator itor = children_.find(item.key);
			if(itor == children_.end() || index >= itor->second.size()) {
				throw error("error in diff: could not find element '" + item.key + "'");
			}

			itor->second[index]->apply_diff(item.cfg, track);
		}
	}

	for(const config& i : diff.child_range("insert_child")) {
		const size_t index = lexical_cast<size_t>(i["index"].str());
		for(const any_child& item : i.all_children_range()) {
			config& inserted = add_child_at(item.key, item.cfg, index);
			if(track) {
				inserted[diff_track_attribute] = "new";
			}
		}
	}

	for(const config& i : diff.child_range("delete_child")) {
		const size_t index = lexical_cast<size_t>(i["index"].str());
		for(const any_child& item : i.all_children_range()) {
			if(!track) {
				remove_child(item.key, index);
			} else {
				const child_map::iterator itor = children_.find(item.key);
				if(itor == children_.end() || index >= itor->second.size()) {
					throw error("error in diff: could not find element '" + item.key + "'");
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
		const size_t index = lexical_cast<size_t>(i["index"].str());
		for(const any_child& item : i.all_children_range()) {
			remove_child(item.key, index);
		}
	}

	for(const config& i : diff.child_range("change_child")) {
		const size_t index = lexical_cast<size_t>(i["index"].str());
		for(const any_child& item : i.all_children_range()) {
			if(item.key.empty()) {
				continue;
			}

			const child_map::iterator itor = children_.find(item.key);
			if(itor == children_.end() || index >= itor->second.size()) {
				throw error("error in diff: could not find element '" + item.key + "'");
			}

			itor->second[index]->clear_diff_track(item.cfg);
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
	check_valid(c);

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
	for(const auto& pair : c.children_) {
		const std::string& tag = pair.first;
		unsigned& visits = visitations[tag];
		while(visits < pair.second.size()) {
			add_child(tag, *pair.second[visits++]);
		}
	}

	// Remove those marked so
	std::map<std::string, unsigned> removals;
	for(const child_pos& pos : to_remove) {
		const std::string& tag = pos.pos->first;
		unsigned& removes = removals[tag];
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

bool config::matches(const config& filter) const
{
	check_valid(filter);

	for(const attribute& i : filter.attribute_range()) {
		const attribute_value* v = get(i.first);
		if(!v || *v != i.second) {
			return false;
		}
	}

	for(const any_child& i : filter.all_children_range()) {
		if(i.key == "not") {
			if(matches(i.cfg)) {
				return false;
			}

			continue;
		}

		bool found = false;
		for(const config& j : child_range(i.key)) {
			if(j.matches(i.cfg)) {
				found = true;
				break;
			}
		}

		if(!found) {
			return false;
		}
	}

	return true;
}

std::string config::debug() const
{
	check_valid();

	std::ostringstream outstream;
	outstream << *this;
	return outstream.str();
}

std::ostream& operator<<(std::ostream& outstream, const config& cfg)
{
	static int i = 0;
	i++;

	for(const config::attribute& val : cfg.attribute_range()) {
		if(val.second.blank()) {
			continue;
		}

		for(int j = 0; j < i - 1; j++) {
			outstream << char(9);
		}

		outstream << val.first << " = " << val.second << '\n';
	}

	for(const config::any_child& child : cfg.all_children_range()) {
		for(int j = 0; j < i - 1; ++j) {
			outstream << char(9);
		}

		outstream << "[" << child.key << "]\n";
		outstream << child.cfg;

		for(int j = 0; j < i - 1; ++j) {
			outstream << char(9);
		}

		outstream << "[/" << child.key << "]\n";
	}

	i--;
	return outstream;
}

std::string config::hash() const
{
	check_valid();

	static const unsigned int hash_length = 128;
	static const char hash_string[] = "+-,.<>0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char hash_str[hash_length + 1];

	unsigned int i;
	for(i = 0; i != hash_length; ++i) {
		hash_str[i] = 'a';
	}

	hash_str[hash_length] = 0;

	i = 0;
	for(const attribute& val : values_) {
		if(val.second.blank()) {
			continue;
		}

		for(char c : val.first) {
			hash_str[i] ^= c;
			if(++i == hash_length) {
				i = 0;
			}
		}

		std::string base_str = val.second.t_str().base_str();
		for(const char c : base_str) {
			hash_str[i] ^= c;
			if(++i == hash_length) {
				i = 0;
			}
		}
	}

	for(const any_child& ch : all_children_range()) {
		std::string child_hash = ch.cfg.hash();
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
	check_valid(cfg);

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
	return std::all_of(children_.begin(), children_.end(), [](const child_map::value_type& pair)
	{
		return valid_tag(pair.first) &&
			std::all_of(pair.second.begin(), pair.second.end(),
			[](const std::unique_ptr<config>& c) { return c->validate_wml(); });
	});
}

bool operator==(const config& a, const config& b)
{
	a.check_valid(b);

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
