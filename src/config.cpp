/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2011 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file config.cpp
 * Routines related to configuration-files / WML.
 */

#include "global.hpp"

#include "config.hpp"
#include "foreach.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"

#include <cstring>
#include <deque>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

config config::invalid;

const char* config::diff_track_attribute = "__diff_track";

int *volatile null_pointer = 0;

void config::check_valid() const
{
	if (*this) return;
	*null_pointer = 0;
	throw error("Mandatory WML child missing yet untested for. Please report.");
}

void config::check_valid(const config &cfg) const
{
	if (*this && cfg) return;
	*null_pointer = 0;
	throw error("Mandatory WML child missing yet untested for. Please report.");
}

config::config() : values(), children(), ordered_children()
{
}

config::config(const config& cfg) : values(), children(), ordered_children()
{
	cfg.check_valid();
	append(cfg);
}

config::config(const std::string& child) : values(), children(), ordered_children()
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

	check_valid(cfg);

	clear();
	append(cfg);
	return *this;
}

bool config::has_attribute(const std::string &key) const
{
	check_valid();
	return values.find(key) != values.end();
}

void config::remove_attribute(const std::string &key)
{
	check_valid();
	values.erase(key);
}

void config::append(const config& cfg)
{
	check_valid(cfg);

	BOOST_FOREACH (const any_child &value, cfg.all_children_range()) {
		add_child(value.key, value.cfg);
	}

	for(string_map::const_iterator j = cfg.values.begin(); j != cfg.values.end(); ++j) {
		values[j->first] = j->second;
	}
}

void config::merge_children(const std::string& key)
{
	check_valid();

	config merged_children;
	const child_list& children = get_children(key);
	if(children.size() < 2) {
		return;
	}

	for(child_list::const_iterator i = children.begin(); i != children.end(); ++i) {
		merged_children.append(**i);
	}

	clear_children(key);
	add_child(key,merged_children);
}

void config::merge_children_by_attribute(const std::string& key, const std::string& attribute)
{
	check_valid();

	std::map<std::string,config> merged_children_map;
	const child_list& children = get_children(key);
	if(children.size() < 2) {
		return;
	}

	for(child_list::const_iterator i = children.begin(); i != children.end(); ++i) {
		const std::string& value = (**i)[attribute];
		std::map<std::string,config>::iterator m = merged_children_map.find(value);
		if ( m!=merged_children_map.end() ) {
			m->second.append(**i);
		} else {
			merged_children_map.insert(make_pair(value,**i));
		}
	}

	clear_children(key);
	typedef std::map<std::string,config> config_map;
	BOOST_FOREACH (const config_map::value_type &i, merged_children_map) {
		add_child(key,i.second);
	}
}

config::child_itors_bak config::child_range_bak(const std::string& key)
{
	check_valid();

	child_map::iterator i = children.find(key);
	if(i != children.end()) {
		return child_itors_bak(i->second.begin(),i->second.end());
	} else {
		static std::vector<config*> dummy;
		return child_itors_bak(dummy.begin(),dummy.end());
	}
}

config::child_itors config::child_range(const std::string& key)
{
	check_valid();

	child_map::iterator i = children.find(key);
	static child_list dummy;
	child_list *p = &dummy;
	if (i != children.end()) p = &i->second;
	return child_itors(child_iterator(p->begin()), child_iterator(p->end()));
}

config::const_child_itors config::child_range(const std::string& key) const
{
	check_valid();

	child_map::const_iterator i = children.find(key);
	static child_list dummy;
	const child_list *p = &dummy;
	if (i != children.end()) p = &i->second;
	return const_child_itors(const_child_iterator(p->begin()), const_child_iterator(p->end()));
}

size_t config::child_count(const std::string& key) const
{
	check_valid();

	child_map::const_iterator i = children.find(key);
	if(i != children.end()) {
		return i->second.size();
	}
	return 0;
}

const config::child_list& config::get_children(const std::string& key) const
{
	check_valid();

	const child_map::const_iterator i = children.find(key);
	if(i != children.end()) {
		return i->second;
	} else {
		static const child_list dummy;
		return dummy;
	}
}

config &config::child(const std::string& key)
{
	check_valid();

	const child_map::const_iterator i = children.find(key);
	if(i != children.end() && i->second.empty() == false) {
		return *i->second.front();
	} else {
		return invalid;
	}
}

const config &config::child(const std::string& key) const
{
	check_valid();

	const child_map::const_iterator i = children.find(key);
	if(i != children.end() && i->second.empty() == false) {
		return *i->second.front();
	} else {
		return invalid;
	}
}

config config::child_or_empty(const std::string& key) const
{
	check_valid();

	child_map::const_iterator i = children.find(key);
	if (i != children.end() && !i->second.empty())
		return *i->second.front();

	return config();
}

config &config::child_or_add(const std::string &key)
{
	child_map::const_iterator i = children.find(key);
	if (i != children.end() && !i->second.empty())
		return *i->second.front();

	return add_child(key);
}

config& config::add_child(const std::string& key)
{
	check_valid();

	child_list& v = children[key];
	v.push_back(new config());
	ordered_children.push_back(child_pos(children.find(key),v.size()-1));
	return *v.back();
}

config& config::add_child(const std::string& key, const config& val)
{
	check_valid(val);

	child_list& v = children[key];
	v.push_back(new config(val));
	ordered_children.push_back(child_pos(children.find(key),v.size()-1));
	return *v.back();
}

config& config::add_child_at(const std::string& key, const config& val, size_t index)
{
	check_valid(val);

	child_list& v = children[key];
	if(index > v.size()) {
		throw error("illegal index to add child at");
	}

	v.insert(v.begin()+index,new config(val));

	bool inserted = false;

	const child_pos value(children.find(key),index);

	std::vector<child_pos>::iterator ord = ordered_children.begin();
	for(; ord != ordered_children.end(); ++ord) {
		if(!inserted && ord->index == index && ord->pos->first == key) {
			ord = ordered_children.insert(ord,value);
			inserted = true;
		} else if(ord->index >= index && ord->pos->first == key) {
			ord->index++;
		}
	}

	if(!inserted) {
		ordered_children.push_back(value);
	}

	return *v[index];
}

namespace {

struct remove_ordered {
	remove_ordered(const std::string& key) : key_(key) {}

	bool operator()(const config::child_pos& pos) const { return pos.pos->first == key_; }
private:
	std::string key_;
};

}

void config::clear_children(const std::string& key)
{
	check_valid();

	ordered_children.erase(std::remove_if(ordered_children.begin(),ordered_children.end(),remove_ordered(key)),ordered_children.end());

	child_map::iterator i = children.find(key);
	if (i != children.end()) {
		for (child_list::iterator c = i->second.begin(); c != i->second.end(); ++c) {
			delete *c;
		}
		children.erase(i);
	}
}

void config::recursive_clear_value(const std::string& key)
{
	check_valid();

	values.erase(key);

	BOOST_FOREACH (const any_child &value, all_children_range()) {
		const_cast<config *>(&value.cfg)->recursive_clear_value(key);
	}
}

config::all_children_iterator config::erase(const config::all_children_iterator& i)
{
	check_valid();

	config* found_config = NULL;
	std::vector<child_pos>::iterator erase_pos, j, j_end = ordered_children.end();
	for(j = ordered_children.begin(); j != j_end; ++j) {
		if (i->key == j->pos->first) {
			if(i.get_index() == j->index) {
				erase_pos = j;
				found_config = *(j->pos->second.begin() + j->index);
			} else if(i.get_index() < j->index) {
				//decrement subsequent child indeces of the same key
				j->index--;
			}
		}
	}
	child_list& vec = children[i->key];
	assert(found_config && erase_pos->index < vec.size());
	delete found_config;
	vec.erase(vec.begin()+i.get_index());
	return all_children_iterator(ordered_children.erase(erase_pos));
}

void config::remove_child(const std::string& key, size_t index)
{
	check_valid();

	// Remove from the ordering
	const child_pos pos(children.find(key),index);
	ordered_children.erase(std::remove(ordered_children.begin(),ordered_children.end(),pos),ordered_children.end());

	// Decrement all indices in the ordering that are above this index,
	// since everything is getting shifted back by 1.
	for(std::vector<child_pos>::iterator i = ordered_children.begin(); i != ordered_children.end(); ++i) {
		if(i->pos->first == key && i->index > index) {
			i->index--;
		}
	}

	// Remove from the child map
	child_list& v = children[key];
	if(index >= v.size()) {
		ERR_CF << "Error: attempting to delete non-existing child: "
			<< key << "[" << index << "]\n";
		return;
	}
	config* const res = v[index];
	v.erase(v.begin()+index);
	delete res;
}

t_string& config::operator[](const std::string& key)
{
	check_valid();

	return values[key];
}

const t_string& config::operator[](const std::string& key) const
{
	return get_attribute(key);
}

const t_string& config::get_attribute(const std::string& key) const
{
	check_valid();

	const string_map::const_iterator i = values.find(key);
	if(i != values.end()) {
		return i->second;
	} else {
		static const t_string empty_string;
		return empty_string;
	}
}

void config::merge_attributes(const config &cfg)
{
	check_valid(cfg);

	assert(this != &cfg);
	for (string_map::const_iterator i = cfg.values.begin(),
	     i_end = cfg.values.end(); i != i_end; ++i)
	{
		values[i->first] = i->second;
	}
}

config::const_attr_itors config::attribute_range() const
{
	check_valid();

	return const_attr_itors(const_attribute_iterator(values.begin()),
	                        const_attribute_iterator(values.end()));
}

namespace {

struct config_has_value {
	config_has_value(const std::string& name, const std::string& value)
	              : name_(name), value_(value)
	{}

	bool operator()(const config* cfg) const { return (*cfg)[name_] == value_; }

private:
	const std::string name_, value_;
};

} // end namespace

config &config::find_child(const std::string& key,
                           const std::string& name,
                           const t_string& value)
{
	check_valid();

	const child_map::iterator i = children.find(key);
	if(i == children.end())
		return invalid;

	const child_list::iterator j = std::find_if(i->second.begin(),
	                                            i->second.end(),
	                                            config_has_value(name,value));
	if(j != i->second.end())
		return **j;
	else
		return invalid;
}

const config &config::find_child(const std::string& key,
                                 const std::string& name,
                                 const t_string& value) const
{
	return const_cast<config *>(this)->find_child(key, name, value);
}

const config& config::find_child_recursive(const std::string& key,
                                 const std::string& name,
                                 const t_string& value) const
{
	const config& res = this->find_child(key, name, value);
	if (res)
		return res;

	BOOST_FOREACH (const any_child &child, all_children_range()) {
		const config& res2 = child.cfg.find_child_recursive(key, name, value);

		if (res2)
			return res2;
	}

	return invalid;
}
namespace {
	/**
	 * Helper struct for iterative config clearing.
	 */
	struct config_clear_state
	{
		config_clear_state()
			: c(NULL)
			, mi()
			, vi(0)
		{
		}

		config* c; //the config being inspected
		config::child_map::iterator mi; //current child map entry
		size_t vi; //index into the child map item vector
	};
}

void config::clear()
{
	// No validity check for this function.

	if (!children.empty()) {
		//start with this node, the first entry in the child map,
		//zeroeth element in that entry
		config_clear_state init;
		init.c = this;
		init.mi = children.begin();
		init.vi = 0;
		std::deque<config_clear_state> l;
		l.push_back(init);

		while (!l.empty()) {
			config_clear_state& state = l.back();
			if (state.mi != state.c->children.end()) {
				std::vector<config*>& v = state.mi->second;
				if (state.vi < v.size()) {
					config* c = v[state.vi];
					++state.vi;
					if (c->children.empty()) {
						delete c; //special case for a slight speed increase?
					} else {
						//descend to the next level
						config_clear_state next;
						next.c = c;
						next.mi = c->children.begin();
						next.vi = 0;
						l.push_back(next);
					}
				} else {
					state.vi = 0;
					++state.mi;
				}
			} else {
				//reached end of child map for this element - all child nodes
				//have beed deleted, so it's safe to clear the map, delete the
				//node and move up one level
				state.c->children.clear();
				if (state.c != this) delete state.c;
				l.pop_back();
			}
		}
	}

	values.clear();
	ordered_children.clear();
}

bool config::empty() const
{
	check_valid();

	return children.empty() && values.empty();
}

config::any_child config::all_children_iterator::operator*() const
{
	return any_child(&i_->pos->first, i_->pos->second[i_->index]);
}

size_t config::all_children_iterator::get_index() const
{
	return i_->index;
}

config::all_children_iterator config::ordered_begin() const
{
	return all_children_iterator(ordered_children.begin());
}

config::all_children_iterator config::ordered_end() const
{
	return all_children_iterator(ordered_children.end());
}

config::all_children_itors config::all_children_range() const
{
	return all_children_itors(
		all_children_iterator(ordered_children.begin()),
		all_children_iterator(ordered_children.end()));
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

	config* inserts = NULL;

	string_map::const_iterator i;
	for(i = values.begin(); i != values.end(); ++i) {
		const string_map::const_iterator j = c.values.find(i->first);
		if(j == c.values.end() || (i->second != j->second && i->second != "")) {
			if(inserts == NULL) {
				inserts = &res.add_child("insert");
			}

			(*inserts)[i->first] = i->second;
		}
	}

	config* deletes = NULL;

	for(i = c.values.begin(); i != c.values.end(); ++i) {
		const string_map::const_iterator itor = values.find(i->first);
		if(itor == values.end() || itor->second == "") {
			if(deletes == NULL) {
				deletes = &res.add_child("delete");
			}

			(*deletes)[i->first] = "x";
		}
	}

	std::vector<std::string> entities;

	child_map::const_iterator ci;
	for(ci = children.begin(); ci != children.end(); ++ci) {
		entities.push_back(ci->first);
	}

	for(ci = c.children.begin(); ci != c.children.end(); ++ci) {
		if(children.count(ci->first) == 0) {
			entities.push_back(ci->first);
		}
	}

	for(std::vector<std::string>::const_iterator itor = entities.begin(); itor != entities.end(); ++itor) {

		const child_map::const_iterator itor_a = children.find(*itor);
		const child_map::const_iterator itor_b = c.children.find(*itor);

		static const child_list dummy;

		// Get the two child lists. 'b' has to be modified to look like 'a'.
		const child_list& a = itor_a != children.end() ? itor_a->second : dummy;
		const child_list& b = itor_b != c.children.end() ? itor_b->second : dummy;

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
					new_delete.values["index"] = buf.str();
					new_delete.add_child(*itor);

					++ndeletes;
					++bi;
				}

				// If b has less elements than a, then we assume this element
				// is an element that needs inserting.
				else if(b.size() - bi < a.size() - ai) {
					config& new_insert = res.add_child("insert_child");
					buf << ai;
					new_insert.values["index"] = buf.str();
					new_insert.add_child(*itor,*a[ai]);

					++ai;
				}

				// Otherwise, they have the same number of elements,
				// so try just changing this element to match.
				else {
					config& new_change = res.add_child("change_child");
					buf << bi;
					new_change.values["index"] = buf.str();
					new_change.add_child(*itor,a[ai]->get_diff(*b[bi]));

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

	if (track) values[diff_track_attribute] = "modified";

	if (const config &inserts = diff.child("insert")) {
		BOOST_FOREACH (const attribute &v, inserts.attribute_range()) {
			values[v.first] = v.second;
		}
	}

	if (const config &deletes = diff.child("delete")) {
		BOOST_FOREACH (const attribute &v, deletes.attribute_range()) {
			values.erase(v.first);
		}
	}

	BOOST_FOREACH (const config &i, diff.child_range("change_child"))
	{
		const size_t index = lexical_cast<size_t>(i["index"].str());
		BOOST_FOREACH (const any_child &item, i.all_children_range())
		{
			if (item.key.empty()) {
				continue;
			}

			const child_map::iterator itor = children.find(item.key);
			if(itor == children.end() || index >= itor->second.size()) {
				throw error("error in diff: could not find element '" + item.key + "'");
			}

			itor->second[index]->apply_diff(item.cfg, track);
		}
	}

	BOOST_FOREACH (const config &i, diff.child_range("insert_child"))
	{
		const size_t index = lexical_cast<size_t>(i["index"].str());
		BOOST_FOREACH (const any_child &item, i.all_children_range()) {
			config& inserted = add_child_at(item.key, item.cfg, index);
			if (track) inserted[diff_track_attribute] = "new";
		}
	}

	BOOST_FOREACH (const config &i, diff.child_range("delete_child"))
	{
		const size_t index = lexical_cast<size_t>(i["index"].str());
		BOOST_FOREACH (const any_child &item, i.all_children_range()) {
			if (!track) {
				remove_child(item.key, index);
			} else {
				const child_map::iterator itor = children.find(item.key);
				if(itor == children.end() || index >= itor->second.size()) {
					throw error("error in diff: could not find element '" + item.key + "'");
				}
				itor->second[index]->values[diff_track_attribute] = "deleted";
			}
		}
	}
}

void config::clear_diff_track(const config& diff)
{
	remove_attribute(diff_track_attribute);
	BOOST_FOREACH (const config &i, diff.child_range("delete_child"))
	{
		const size_t index = lexical_cast<size_t>(i["index"].str());
		BOOST_FOREACH (const any_child &item, i.all_children_range()) {
			remove_child(item.key, index);
		}
	}

	BOOST_FOREACH (const config &i, diff.child_range("change_child"))
	{
		const size_t index = lexical_cast<size_t>(i["index"].str());
		BOOST_FOREACH (const any_child &item, i.all_children_range())
		{
			if (item.key.empty()) {
				continue;
			}

			const child_map::iterator itor = children.find(item.key);
			if(itor == children.end() || index >= itor->second.size()) {
				throw error("error in diff: could not find element '" + item.key + "'");
			}

			itor->second[index]->clear_diff_track(item.cfg);
		}
	}
	BOOST_FOREACH (const any_child &value, all_children_range()) {
		const_cast<config *>(&value.cfg)->remove_attribute(diff_track_attribute);
	}
}

void config::merge_with(const config& c)
{
	check_valid(c);

	std::map<std::string, unsigned> visitations;

	// Merge attributes first
	string_map::const_iterator attrib_it, attrib_end = c.values.end();
	for(attrib_it = c.values.begin(); attrib_it != attrib_end; ++attrib_it) {
		values[attrib_it->first] = attrib_it->second;
	}

	// Now merge shared tags
	all_children_iterator::Itor i, i_end = ordered_children.end();
	for(i = ordered_children.begin(); i != i_end; ++i) {
		const std::string& tag = i->pos->first;
		child_map::const_iterator j = c.children.find(tag);
		if (j != c.children.end()) {
			unsigned &visits = visitations[tag];
			if(visits < j->second.size()) {
				(i->pos->second[i->index])->merge_with(*j->second[visits++]);
			}
		}
	}

	// Now add any unvisited tags
	for(child_map::const_iterator j = c.children.begin(); j != c.children.end(); ++j) {
		const std::string& tag = j->first;
		unsigned &visits = visitations[tag];
		while(visits < j->second.size()) {
			add_child(tag, *j->second[visits++]);
		}
	}
}

bool config::matches(const config &filter) const
{
	check_valid(filter);

	// First match values. all values should match.
	for(string_map::const_iterator j = filter.values.begin(); j != filter.values.end(); ++j) {
		if(!this->values.count(j->first)) return false;
		const t_string& test_val = this->values.find(j->first)->second;
		if(test_val != j->second) {
			const std::string& boolcheck = j->second.base_str();
			if(boolcheck == "yes" || boolcheck == "true" || boolcheck == "on") {
				if(!utils::string_bool(test_val.base_str(), false)) {
					return false;
				}
			} else if(boolcheck == "no" || boolcheck == "false" || boolcheck == "off") {
				if(utils::string_bool(test_val.base_str(), true)) {
					return false;
				}
			} else {
				return false;
			}
		}
	}

	// Now, match the kids
	BOOST_FOREACH (const any_child &i, filter.all_children_range())
	{
		if (i.key == "not") continue;
		bool found = false;
		BOOST_FOREACH (const config &j, child_range(i.key)) {
			if (j.matches(i.cfg)) {
				found = true;
				break;
			}
		}
		if(!found) return false;
	}
	child_list negative_children = filter.get_children("not");
	for(child_list::iterator j3 = negative_children.begin() ; j3 != negative_children.end() ; ++j3) {
		if(matches(**j3)) return false;
	}
	return true;
}

void config::prune() {
	string_map::iterator val = values.begin();
	while(val != values.end()) {
		if(val->second.empty()) {
			values.erase(val++);
		} else {
			++val;
		}
	}

	for(child_map::const_iterator list = children.begin(); list != children.end(); ++list) {
		for(child_list::const_iterator child = list->second.begin(); child != list->second.end(); ++child) {
			(*child)->prune();
		}
	}
}

std::string config::debug() const
{
	check_valid();

	std::ostringstream outstream;
	outstream << *this;
	return outstream.str();
}

std::ostream& operator << (std::ostream& outstream, const config& cfg)
{
	static int i = 0;
	i++;
	BOOST_FOREACH (const config::attribute &val, cfg.attribute_range()) {
		for (int j = 0; j < i-1; j++){ outstream << char(9); }
		outstream << val.first << " = " << val.second << '\n';
	}
	BOOST_FOREACH (const config::any_child &child, cfg.all_children_range())
	{
		for (int j = 0; j < i - 1; ++j) outstream << char(9);
		outstream << "[" << child.key << "]\n";
		outstream << child.cfg;
		for (int j = 0; j < i - 1; ++j) outstream << char(9);
		outstream << "[/" << child.key << "]\n";
	}
	i--;
    return outstream;
}

std::string config::hash() const
{
	check_valid();

	static const unsigned int hash_length = 128;
	static const char hash_string[] =
		"+-,.<>0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char hash_str[hash_length + 1];
	std::string::const_iterator c;

	unsigned int i;
	for(i = 0; i != hash_length; ++i) {
		hash_str[i] = 'a';
	}
	hash_str[hash_length] = 0;

	i = 0;
	for(string_map::const_iterator val = values.begin(); val != values.end(); ++val) {
		if(val->first.size() && val->second.size()) {
			for(c = val->first.begin(); c != val->first.end(); ++c) {
				if(utils::portable_isspace(*c)) {
					continue;
				}
				hash_str[i] ^= *c;
				++i;
				if(i == hash_length) {
					i = 0;
				}
			}
			const std::string &base_str = val->second.base_str();
			for(c = base_str.begin(); c != base_str.end(); ++c) {
				if(utils::portable_isspace(*c)) {
					continue;
				}
				hash_str[i] ^= *c;
				++i;
				if(i == hash_length) {
					i = 0;
				}
			}
		}
	}
	for(child_map::const_iterator list = children.begin(); list != children.end(); ++list) {
		for(child_list::const_iterator child = list->second.begin(); child != list->second.end(); ++child) {
			std::string child_hash = (*child)->hash();
			for(c = child_hash.begin(); c != child_hash.end(); ++c) {
				hash_str[i] ^= *c;
				++i;
				if(i == hash_length) {
					i = 0;
				}
			}
		}
	}

	for(i = 0; i != hash_length; ++i) {
		hash_str[i] = hash_string[
			static_cast<unsigned>(hash_str[i]) % strlen(hash_string)];
	}

	return std::string(hash_str);
}

void config::swap(config& cfg)
{
	check_valid(cfg);

	values.swap(cfg.values);
	children.swap(cfg.children);
	ordered_children.swap(cfg.ordered_children);
}

bool operator==(const config& a, const config& b)
{
	a.check_valid(b);

	if (a.values != b.values)
		return false;

	config::all_children_itors x = a.all_children_range(), y = b.all_children_range();
	for (; x.first != x.second && y.first != y.second; ++x.first, ++y.first) {
		if (x.first->key != y.first->key || x.first->cfg != y.first->cfg) {
			return false;
		}
	}

	return x.first == x.second && y.first == y.second;
}

//#define TEST_CONFIG

#ifdef TEST_CONFIG

int main()
{
	config cfg(read_file("testconfig"));
	std::cout << cfg.write() << std::endl;
}

#endif
