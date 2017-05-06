/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2017 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

#include <cstdlib>
#include <cstring>
#include <deque>
#include <istream>

#include "utils/functional.hpp"
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/variant.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define DBG_CF LOG_STREAM(debug, log_config)

namespace {
//std::map::operator[] does not support heterogenous lookup so we need this to work around.
template<typename Map, typename Key>
map_get(Map&& map, Key&& key)
{
	auto res = map.lower_bound(key);
	
	if (res == map.end() || key != res->first) {
		res = map.emplace_hint(res, std::piecewise_construct, std::forward_as_tuple(key), std::tuple<>());
	}

	return res->second;
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
	static typename utils::const_clone<config, T>::reference
	child(
			  T config
			, config_key_type key
			, const std::string& parent)
	{
		config->check_valid();

		assert(!parent.empty());
		assert(parent[0] == '[');
		assert(parent[parent.size() - 1] == ']');

		if(config->has_child(key)) {
			return *(config->children.find(key)->second.front());
		}

		/**
		 * @todo Implement a proper wml_exception here.
		 *
		 * at the moment there seem to be dependency issues, which i don't want
		 * to fix right now.
		 */
//		FAIL(missing_mandatory_wml_section(parent, key));

		std::stringstream sstr;
		sstr << "Mandatory WML child »[" << key << "]« missing in »"
				<< parent << "«. Please report this bug.";

		throw config::error(sstr.str());
	}
};


/* ** Attribute value implementation ** */


// Special string values.
const std::string config::attribute_value::s_yes("yes");
const std::string config::attribute_value::s_no("no");
const std::string config::attribute_value::s_true("true");
const std::string config::attribute_value::s_false("false");


/** Default implementation, but defined out-of-line for efficiency reasons. */
config::attribute_value::attribute_value()
	: value_()
{
}

/** Default implementation, but defined out-of-line for efficiency reasons. */
config::attribute_value::~attribute_value()
{
}

/** Default implementation, but defined out-of-line for efficiency reasons. */
config::attribute_value::attribute_value(const config::attribute_value &that)
	: value_(that.value_)
{
}

/** Default implementation, but defined out-of-line for efficiency reasons. */
config::attribute_value &config::attribute_value::operator=(const config::attribute_value &that)
{
	value_ = that.value_;
	return *this;
}

config::attribute_value &config::attribute_value::operator=(bool v)
{
	value_ = yes_no(v);
	return *this;
}

config::attribute_value &config::attribute_value::operator=(int v)
{
	value_ = v;
	return *this;
}

config::attribute_value &config::attribute_value::operator=(long long v)
{
	if ( v > 0 )
		// We can store this unsigned.
		return *this = static_cast<unsigned long long>(v);

	if ( v >= INT_MIN )
		// We can store this as an int.
		return *this = static_cast<int>(v);

	// Getting to this point should be rare. (Currently, getting here means
	// something like there was so much draining in a campaign that the
	// total damage taken is not only negative, but so negative that an
	// int cannot hold the value.) So rare that it is not worth precise
	// treatment; just use a double.
	value_ = static_cast<double>(v);
	return *this;
}

config::attribute_value &config::attribute_value::operator=(unsigned long long v)
{
	// Use int for smaller numbers.
	if ( v <= INT_MAX )
		return *this = static_cast<int>(v);

	value_ = v;
	return *this;
}

config::attribute_value &config::attribute_value::operator=(double v)
{
	// Try to store integers in other types.
	if ( v > 0.0 ) {
		// Convert to unsigned and pass this off to that assignment operator.
		unsigned long long ull = static_cast<unsigned long long>(v);
		if ( static_cast<double>(ull) == v )
			return *this = ull;
	}
	else {
		// Convert to integer and pass this off to that assignment operator.
		int i = static_cast<int>(v);
		if ( static_cast<double>(i) == v )
			return *this = i;
	}

	// If we get here, this does in fact get stored as a double.
	value_ = v;
	return *this;
}

namespace {
	/**
	 * Attempts to convert @a source to the template type.
	 * This is to avoid "overzealous reinterpretations of certain WML strings as
	 * numeric types" (c.f. bug #19201).
	 * @returns true if the conversion was successful and the source string
	 *          can be reobtained by streaming the result.
	 */
	template<typename To>
	bool from_string_verify(const std::string & source, To & res)
	{
		// Check 1: convertable to the target type.
		std::istringstream in_str(source);
		if ( !(in_str >> res) )
			return false;

		// Check 2: convertable back to the same string.
		std::ostringstream out_str;
		out_str << res;
		return out_str.str() == source;
	}
}
config::attribute_value &config::attribute_value::operator=(const std::string &v)
{
	// Handle some special strings.
	if (v.empty()) { value_ = v; return *this; }
	if ( v == s_yes )   { value_ = yes_no(true);  return *this; }
	if ( v == s_no )    { value_ = yes_no(false); return *this; }
	if ( v == s_true )  { value_ = true_false(true);  return *this; }
	if ( v == s_false ) { value_ = true_false(false); return *this; }

	// Attempt to convert to a number.
	char *eptr;
	double d = strtod(v.c_str(), &eptr);
	if ( *eptr == '\0' ) {
		// Possibly a number. See what type it should be stored in.
		// (All conversions will be from the string since the largest integer
		// type could have more precision than a double.)
		if ( d > 0.0 ) {
			// The largest type for positive integers is unsigned long long.
			unsigned long long ull = 0;
			if ( from_string_verify<unsigned long long>(v, ull) )
				return *this = ull;
		}
		else {
			// The largest (variant) type for negative integers is int.
			int i = 0;
			if ( from_string_verify<int>(v, i) )
				return *this = i;
		}

		// This does not look like an integer, so it should be a double.
		// However, make sure it can convert back to the same string (in
		// case this is a string that just looks like a numeric value).
		std::ostringstream tester;
		tester << d;
		if ( tester.str() == v ) {
			value_ = d;
			return *this;
		}
	}

	// No conversion possible. Store the string.
	value_ = v;
	return *this;
}

config::attribute_value &config::attribute_value::operator=(const t_string &v)
{
	if (!v.translatable()) return *this = v.str();
	value_ = v;
	return *this;
}


bool config::attribute_value::to_bool(bool def) const
{
	if ( const yes_no *p = boost::get<const yes_no>(&value_) )
		return *p;
	if ( const true_false *p = boost::get<const true_false>(&value_) )
		return *p;

	// No other types are ever recognized as boolean.
	return def;
}

namespace {
	/// Visitor for converting a variant to a numeric type (T).
	template <typename T>
	class attribute_numeric_visitor : public boost::static_visitor<T>
	{
	public:
		// Constructor stores the default value.
		attribute_numeric_visitor(T def) : def_(def) {}

		T operator()(boost::blank const &) const { return def_; }
		T operator()(bool)                 const { return def_; }
		T operator()(int i)                const { return static_cast<T>(i); }
		T operator()(unsigned long long u) const { return static_cast<T>(u); }
		T operator()(double d)             const { return static_cast<T>(d); }
		T operator()(const std::string& s) const { return lexical_cast_default<T>(s, def_); }
		T operator()(t_string const &)     const { return def_; }

	private:
		const T def_;
	};
}

int config::attribute_value::to_int(int def) const
{
	return apply_visitor(attribute_numeric_visitor<int>(def));
}

long long config::attribute_value::to_long_long(long long def) const
{
	return apply_visitor(attribute_numeric_visitor<long long>(def));
}

unsigned config::attribute_value::to_unsigned(unsigned def) const
{
	return apply_visitor(attribute_numeric_visitor<unsigned>(def));
}

size_t config::attribute_value::to_size_t(size_t def) const
{
	return apply_visitor(attribute_numeric_visitor<size_t>(def));
}

time_t config::attribute_value::to_time_t(time_t def) const
{
	return apply_visitor(attribute_numeric_visitor<time_t>(def));
}

double config::attribute_value::to_double(double def) const
{
	return apply_visitor(attribute_numeric_visitor<double>(def));
}

/// Visitor for converting a variant to a string.
class config::attribute_value::string_visitor
	: public boost::static_visitor<std::string>
{
	const std::string default_;
public:
	string_visitor(const std::string& fallback) : default_(fallback) {}
	std::string operator()(const boost::blank &) const { return default_; }
	std::string operator()(const yes_no & b)     const { return b.str(); }
	std::string operator()(const true_false & b) const { return b.str(); }
	std::string operator()(int i)                const { return lexical_cast<std::string>(i); }
	std::string operator()(unsigned long long u) const { return lexical_cast<std::string>(u); }
	std::string operator()(double d)             const { return lexical_cast<std::string>(d); }
	std::string operator()(const std::string& s) const { return s; }
	std::string operator()(t_string const &s)    const { return s.str(); }
};

std::string config::attribute_value::str(const std::string& fallback) const
{
	return apply_visitor(string_visitor(fallback));
}

t_string config::attribute_value::t_str() const
{
	if (const t_string *p = boost::get<const t_string>(&value_)) return *p;
	return str();
}

/**
 * Tests for an attribute that was never set.
 */
bool config::attribute_value::blank() const
{
	return boost::get<const boost::blank>(&value_) != nullptr;
}

/**
 * Tests for an attribute that either was never set or was set to "".
 */
bool config::attribute_value::empty() const
{
	if (boost::get<const boost::blank>(&value_)) return true;
	if (const std::string *p = boost::get<const std::string>(&value_)) return p->empty();
	return false;
}


/// Visitor handling equality checks.
class config::attribute_value::equality_visitor
	: public boost::static_visitor<bool>
{
public:
	// Most generic: not equal.
	template <typename T, typename U>
	bool operator()(const T &, const U &) const { return false; }

	// Same types are comparable and might be equal.
	template <typename T>
	bool operator()(const T & lhs, const T & rhs) const { return lhs == rhs; }

	// Boolean values can be compared.
	bool operator()(const true_false & lhs, const yes_no & rhs) const { return bool(lhs) == bool(rhs); }
	bool operator()(const yes_no & lhs, const true_false & rhs) const { return bool(lhs) == bool(rhs); }
};

/**
 * Checks for equality of the attribute values when viewed as strings.
 * Exception: Boolean synonyms can be equal ("yes" == "true").
 * Note: Blanks have no string representation, so do not equal "" (an empty string).
 */
bool config::attribute_value::operator==(const config::attribute_value &other) const
{
	return boost::apply_visitor(equality_visitor(), value_, other.value_);
}

/**
 * Checks for equality of the attribute values when viewed as strings.
 * Exception: Boolean synonyms can be equal ("yes" == "true").
 * Note: Blanks have no string representation, so do not equal "" (an empty string).
 * Also note that translatable string are never equal to non translatable strings.
 */
bool config::attribute_value::equals(const std::string &str) const
{
	attribute_value v;
	v = str;
	return *this == v;
	// if c["a"] = "1" then this solution would have resulted in c["a"] == "1" beeing false
	// because a["a"] is '1' and not '"1"'.
	// return boost::apply_visitor(std::bind( equality_visitor(), _1, std::cref(str) ), value_);
	// that's why we don't use it.
}

std::ostream &operator<<(std::ostream &os, const config::attribute_value &v)
{
	// Simple implementation, but defined out-of-line because of the templating
	// involved.
	return os << v.value_;
}


/* ** config implementation ** */


config config::invalid;

const char* config::diff_track_attribute = "__diff_track";

void config::check_valid() const
{
	if (!*this)
		throw error("Mandatory WML child missing yet untested for. Please report.");
}

void config::check_valid(const config &cfg) const
{
	if (!*this || !cfg)
		throw error("Mandatory WML child missing yet untested for. Please report.");
}

config::config() : values(), children(), ordered_children()
{
}

config::config(const config& cfg) : values(cfg.values), children(), ordered_children()
{
	append_children(cfg);
}

config::config(config_key_type child) : values(), children(), ordered_children()
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

config::config(config &&cfg):
	values(std::move(cfg.values)),
	children(std::move(cfg.children)),
	ordered_children(std::move(cfg.ordered_children))
{
}

config &config::operator=(config &&cfg)
{
	clear();
	swap(cfg);
	return *this;
}

bool config::valid_id(config_key_type id)
{
	if (id.empty()) {
		return false;
	}
	for (char c : id) {
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') {
			//valid character.
		}
		else {
			return false;
		}
	}
	return true;
}

bool config::has_attribute(config_key_type key) const
{
	check_valid();
	return values.find(key) != values.end();
}

bool config::has_old_attribute(config_key_type key, const std::string &old_key, const std::string& msg) const
{
	check_valid();
	if (values.find(key) != values.end()) {
		return true;
	} else if (values.find(old_key) != values.end()) {
		if (!msg.empty())
			lg::wml_error() << msg;
		return true;
	}
	return false;
}


void config::remove_attribute(config_key_type key)
{
	check_valid();

	auto i = values.find(key);
	if (i != values.end()) {
		values.erase(i);
	}
}

void config::append_children(const config &cfg)
{
	check_valid(cfg);

	for (const any_child &value : cfg.all_children_range()) {
		add_child(value.key, value.cfg);
	}
}

void config::append_attributes(const config &cfg)
{
	check_valid(cfg);
	for (const attribute &v : cfg.values) {
		values[v.first] = v.second;
	}
}

void config::append_children(const config &cfg, const std::string& key)
{
	check_valid(cfg);

	for (const config &value : cfg.child_range(key)) {
		add_child(key, value);
	}
}

void config::append(const config &cfg)
{
	append_children(cfg);
	for (const attribute &v : cfg.values) {
		values[v.first] = v.second;
	}
}

void config::merge_children(const std::string& key)
{
	check_valid();

	if (child_count(key) < 2) return;

	config merged_children;
	for (const config &cfg : child_range(key)) {
		merged_children.append(cfg);
	}

	clear_children_impl(key);
	add_child(key,merged_children);
}

void config::merge_children_by_attribute(const std::string& key, const std::string& attribute)
{
	check_valid();

	if (child_count(key) < 2) return;

	typedef std::map<std::string, config> config_map;
	config_map merged_children_map;
	for (const config &cfg : child_range(key)) {
		merged_children_map[cfg[attribute]].append(cfg);
	}

	clear_children_impl(key);
	for (const config_map::value_type &i : merged_children_map) {
		add_child(key,i.second);
	}
}

config::child_itors config::child_range(config_key_type key)
{
	check_valid();

	child_map::iterator i = children.find(key);
	static child_list dummy;
	child_list *p = &dummy;
	if (i != children.end()) p = &i->second;
	return child_itors(child_iterator(p->begin()), child_iterator(p->end()));
}

config::const_child_itors config::child_range(config_key_type key) const
{
	check_valid();

	child_map::const_iterator i = children.find(key);
	static child_list dummy;
	const child_list *p = &dummy;
	if (i != children.end()) p = &i->second;
	return const_child_itors(const_child_iterator(p->begin()), const_child_iterator(p->end()));
}

unsigned config::child_count(config_key_type key) const
{
	check_valid();

	child_map::const_iterator i = children.find(key);
	if(i != children.end()) {
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
	return std::count_if(values.begin(), values.end(), [](const attribute& v) {
		return !v.second.blank();
	});
}

bool config::has_child(config_key_type key) const
{
	check_valid();

	return children.find(key) != children.end();
}

config &config::child(config_key_type key, int n)
{
	check_valid();

	const child_map::const_iterator i = children.find(key);
	if (i == children.end()) {
		DBG_CF << "The config object has no child named »"
				<< key << "«.\n";

		return invalid;
	}

	if (n < 0) n = i->second.size() + n;
	if(size_t(n) < i->second.size()) {
		return *i->second[n];
	}
	else {
		DBG_CF << "The config object has only »" << i->second.size()
			<< "« children named »" << key
			<< "«; request for the index »" << n << "« cannot be honored.\n";

		return invalid;
	}
}

config& config::child(config_key_type key, const std::string& parent)
{
	return config_implementation::child(this, key, parent);
}

const config& config::child(
		  config_key_type key
		, const std::string& parent) const
{
	return config_implementation::child(this, key, parent);
}

const config & config::child_or_empty(config_key_type key) const
{
	static const config empty_cfg;
	check_valid();

	child_map::const_iterator i = children.find(key);
	if (i != children.end() && !i->second.empty())
		return *i->second.front();

	return empty_cfg;
}

config &config::child_or_add(config_key_type key)
{
	child_map::const_iterator i = children.find(key);
	if (i != children.end() && !i->second.empty())
		return *i->second.front();

	return add_child(key);
}

config& config::add_child(config_key_type key)
{
	check_valid();

	child_list& v = map_get(children, key);
	v.push_back(new config());
	ordered_children.push_back(child_pos(children.find(key),v.size()-1));
	return *v.back();
}

config& config::add_child(config_key_type key, const config& val)
{
	check_valid(val);

	child_list& v = map_get(children, key);
	v.push_back(new config(val));
	ordered_children.push_back(child_pos(children.find(key),v.size()-1));
	return *v.back();
}

config &config::add_child(config_key_type key, config &&val)
{
	check_valid(val);

	child_list &v = map_get(children, key);
	v.push_back(new config(std::move(val)));
	ordered_children.push_back(child_pos(children.find(key), v.size() - 1));
	return *v.back();
}

config &config::add_child_at(config_key_type key, const config &val, unsigned index)
{
	check_valid(val);

	child_list& v = map_get(children, key);
	if(index > v.size()) {
		throw error("illegal index to add child at");
	}

	v.insert(v.begin()+index,new config(val));

	bool inserted = false;

	const child_pos value(children.find(key),index);

	std::vector<child_pos>::iterator ord = ordered_children.begin();
	for(; ord != ordered_children.end(); ++ord) {
		if (ord->pos != value.pos) continue;
		if (!inserted && ord->index == index) {
			ord = ordered_children.insert(ord,value);
			inserted = true;
		} else if (ord->index >= index) {
			ord->index++;
		}
	}

	if(!inserted) {
		ordered_children.push_back(value);
	}

	return *v[index];
}

namespace {

struct remove_ordered
{
	remove_ordered(const config::child_map::iterator &iter) : iter_(iter) {}

	bool operator()(const config::child_pos &pos) const
	{ return pos.pos == iter_; }
private:
	config::child_map::iterator iter_;
};

}

void config::clear_children_impl(config_key_type key)
{
	check_valid();

	child_map::iterator i = children.find(key);
	if (i == children.end()) return;

	ordered_children.erase(std::remove_if(ordered_children.begin(),
		ordered_children.end(), remove_ordered(i)), ordered_children.end());

	for (config *c : i->second) {
		delete c;
	}

	children.erase(i);
}

void config::splice_children(config &src, const std::string &key)
{
	check_valid(src);

	child_map::iterator i_src = src.children.find(key);
	if (i_src == src.children.end()) return;

	src.ordered_children.erase(std::remove_if(src.ordered_children.begin(),
		src.ordered_children.end(), remove_ordered(i_src)),
		src.ordered_children.end());

	child_list &dst = map_get(children, key);
	child_map::iterator i_dst = children.find(key);
	unsigned before = dst.size();
	dst.insert(dst.end(), i_src->second.begin(), i_src->second.end());
	src.children.erase(i_src);
	// key might be a reference to i_src->first, so it is no longer usable.

	for (unsigned j = before; j < dst.size(); ++j) {
		ordered_children.push_back(child_pos(i_dst, j));
	}
}

void config::recursive_clear_value(config_key_type key)
{
	check_valid();

	values.erase(key);

	for(std::pair<const std::string, child_list>& p : children) {
		for(config* cfg : p.second) {
			cfg->recursive_clear_value(key);
		}
	}
}

std::vector<config::child_pos>::iterator config::remove_child(
	const child_map::iterator &pos, unsigned index)
{
	/* Find the position with the correct index and decrement all the
	   indices in the ordering that are above this index. */
	unsigned found = 0;
	for (child_pos &p : ordered_children)
	{
		if (p.pos != pos) continue;
		if (p.index == index)
			found = &p - &ordered_children.front();
		else if (p.index > index)
			--p.index;
	}

	// Remove from the child map.
	delete pos->second[index];
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

	child_map::iterator i = children.find(key);
	if (i == children.end() || index >= i->second.size()) {
		ERR_CF << "Error: attempting to delete non-existing child: "
			<< key << "[" << index << "]\n";
		return;
	}

	remove_child(i, index);
}

const config::attribute_value &config::operator[](config_key_type key) const
{
	check_valid();

	const attribute_map::const_iterator i = values.find(key);
	if (i != values.end()) return i->second;
	static const attribute_value empty_attribute;
	return empty_attribute;
}

const config::attribute_value *config::get(config_key_type key) const
{
	check_valid();
	attribute_map::const_iterator i = values.find(key);
	return i != values.end() ? &i->second : nullptr;
}

config::attribute_value& config::operator[](config_key_type key)
{
	check_valid();

	auto res = values.lower_bound(key);
	
	if (res == values.end() || key != res->first) {
		res = values.emplace_hint(res, std::piecewise_construct, std::forward_as_tuple(key), std::tuple<>());
	}

	return res->second;
}

const config::attribute_value &config::get_old_attribute(config_key_type key, const std::string &old_key, const std::string &msg) const
{
	check_valid();

	attribute_map::const_iterator i = values.find(key);
	if (i != values.end())
		return i->second;

	i = values.find(old_key);
	if (i != values.end()) {
		if (!msg.empty())
			lg::wml_error() << msg;
		return i->second;
	}

	static const attribute_value empty_attribute;
	return empty_attribute;
}


void config::merge_attributes(const config &cfg)
{
	check_valid(cfg);

	assert(this != &cfg);
	for (const attribute &v : cfg.values) {

		std::string key = v.first;
		if (key.substr(0,7) == "add_to_") {
			std::string add_to = key.substr(7);
			values[add_to] = values[add_to].to_double() + v.second.to_double();
		} else if(key.substr(0,10) == "concat_to_") {
			std::string concat_to = key.substr(10);
			// TODO: Only use t_string if one or both are actually translatable?
			// That probably requires using a visitor though.
			values[concat_to] = values[concat_to].t_str() + v.second.t_str();
		} else
			values[v.first] = v.second;
	}
}

config::const_attr_itors config::attribute_range() const
{
	check_valid();

	const_attr_itors range (const_attribute_iterator(values.begin()),
	                        const_attribute_iterator(values.end()));

	// Ensure the first element is not blank, as a few places assume this
	while(range.begin() != range.end() && range.begin()->second.blank()) {
		range.pop_front();
	}
	return range;
}

config::attr_itors config::attribute_range()
{
	check_valid();
	attr_itors range(attribute_iterator(values.begin()), attribute_iterator(values.end()));

	// Ensure the first element is not blank, as a few places assume this
	while(range.begin() != range.end() && range.begin()->second.blank()) {
		range.pop_front();
	}
	return range;
}

config &config::find_child(config_key_type key, const std::string &name,
	const std::string &value)
{
	check_valid();

	const child_map::iterator i = children.find(key);
	if(i == children.end()) {
		DBG_CF << "Key »" << name << "« value »" << value
				<< "« pair not found as child of key »" << key << "«.\n";

		return invalid;
	}

	const child_list::iterator j = std::find_if(i->second.begin(),
	                                            i->second.end(),
	                                            [&](const config* cfg) { return (*cfg)[name] == value; });
	if(j != i->second.end()) {
		return **j;
	} else {
		DBG_CF << "Key »" << name << "« value »" << value
				<< "« pair not found as child of key »" << key << "«.\n";

		return invalid;
	}
}

namespace {
	/**
	 * Helper struct for iterative config clearing.
	 */
	struct config_clear_state
	{
		config_clear_state()
			: c(nullptr)
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
				//have been deleted, so it's safe to clear the map, delete the
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

config::all_children_iterator::reference config::all_children_iterator::operator*() const
{
	return any_child(&i_->pos->first, i_->pos->second[i_->index]);
}

config::const_all_children_iterator::reference config::const_all_children_iterator::operator*() const
{
	return any_child(&i_->pos->first, i_->pos->second[i_->index]);
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
		const_all_children_iterator(ordered_children.cend()));
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

	config* inserts = nullptr;

	attribute_map::const_iterator i;
	for(i = values.begin(); i != values.end(); ++i) {
		if(i->second.blank()) continue;
		const attribute_map::const_iterator j = c.values.find(i->first);
		if(j == c.values.end() || (i->second != j->second && !i->second.blank() )) {
			if(inserts == nullptr) {
				inserts = &res.add_child("insert");
			}

			(*inserts)[i->first] = i->second;
		}
	}

	config* deletes = nullptr;

	for(i = c.values.begin(); i != c.values.end(); ++i) {
		if(i->second.blank()) continue;
		const attribute_map::const_iterator itor = values.find(i->first);
		if(itor == values.end() || itor->second.blank()) {
			if(deletes == nullptr) {
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
		for (const attribute &v : inserts.attribute_range()) {
			values[v.first] = v.second;
		}
	}

	if (const config &deletes = diff.child("delete")) {
		for (const attribute &v : deletes.attribute_range()) {
			values.erase(v.first);
		}
	}

	for (const config &i : diff.child_range("change_child"))
	{
		const size_t index = lexical_cast<size_t>(i["index"].str());
		for (const any_child &item : i.all_children_range())
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

	for (const config &i : diff.child_range("insert_child"))
	{
		const size_t index = lexical_cast<size_t>(i["index"].str());
		for (const any_child &item : i.all_children_range()) {
			config& inserted = add_child_at(item.key, item.cfg, index);
			if (track) inserted[diff_track_attribute] = "new";
		}
	}

	for (const config &i : diff.child_range("delete_child"))
	{
		const size_t index = lexical_cast<size_t>(i["index"].str());
		for (const any_child &item : i.all_children_range()) {
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
	for (const config &i : diff.child_range("delete_child"))
	{
		const size_t index = lexical_cast<size_t>(i["index"].str());
		for (const any_child &item : i.all_children_range()) {
			remove_child(item.key, index);
		}
	}

	for (const config &i : diff.child_range("change_child"))
	{
		const size_t index = lexical_cast<size_t>(i["index"].str());
		for (const any_child &item : i.all_children_range())
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
	for(std::pair<const std::string, child_list>& p : children) {
		for(config* cfg : p.second) {
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
		const child_map::const_iterator j = c.children.find(tag);
		if (j != c.children.end()) {
			unsigned &visits = visitations[tag];
			if(visits < j->second.size()) {
				// Get a const config so we do not add attributes.
				const config & merge_child = *j->second[visits++];

				if ( merge_child["__remove"].to_bool() ) {
					to_remove.push_back(*i);
				} else
					(i->pos->second[i->index])->merge_with(merge_child);
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

	// Remove those marked so
	std::map<std::string, unsigned> removals;
	for (const child_pos& pos : to_remove) {
		const std::string& tag = pos.pos->first;
		unsigned &removes = removals[tag];
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

bool config::matches(const config &filter) const
{
	check_valid(filter);

	for (const attribute &i : filter.attribute_range())
	{
		const attribute_value *v = get(i.first);
		if (!v || *v != i.second) return false;
	}

	for (const any_child &i : filter.all_children_range())
	{
		if (i.key == "not") {
			if (matches(i.cfg)) return false;
			continue;
		}
		bool found = false;
		for (const config &j : child_range(i.key)) {
			if (j.matches(i.cfg)) {
				found = true;
				break;
			}
		}
		if(!found) return false;
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

std::ostream& operator << (std::ostream& outstream, const config& cfg)
{
	static int i = 0;
	i++;
	for (const config::attribute &val : cfg.attribute_range()) {
		if(val.second.blank()) continue;
		for (int j = 0; j < i-1; j++){ outstream << char(9); }
		outstream << val.first << " = " << val.second << '\n';
	}
	for (const config::any_child &child : cfg.all_children_range())
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

	unsigned int i;
	for(i = 0; i != hash_length; ++i) {
		hash_str[i] = 'a';
	}
	hash_str[hash_length] = 0;

	i = 0;
	for (const attribute &val : values)
	{
		if (val.second.blank()) {
			continue;
		}
		for (char c : val.first) {
			hash_str[i] ^= c;
			if (++i == hash_length) i = 0;
		}
		std::string base_str = val.second.t_str().base_str();
		for (std::string::const_iterator c = base_str.begin(); c != base_str.end(); ++c) {
			hash_str[i] ^= *c;
			if (++i == hash_length) i = 0;
		}
	}

	for (const any_child &ch : all_children_range())
	{
		std::string child_hash = ch.cfg.hash();
		for (char c : child_hash) {
			hash_str[i] ^= c;
			++i;
			if(i == hash_length) {
				i = 0;
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

	config::const_all_children_itors x = a.all_children_range(), y = b.all_children_range();
	for (; !x.empty() && !y.empty(); x.pop_front(), y.pop_front()) {
		if (x.front().key != y.front().key || x.front().cfg != y.front().cfg) {
			return false;
		}
	}

	return x.empty() && y.empty();
}
