/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2011 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

#include "global.hpp"

#include "config.hpp"
#include "foreach.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"

#include <cstdlib>
#include <cstring>
#include <deque>

#ifdef DEBUG
#include "utils/count_logger.hpp"
#endif

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)


typedef config::t_token t_token;

config::attribute_value::attribute_value()
  : int_value_(),double_value_() , t_string_value_() , token_value_() , type_(EMPTY) , bool_value_()
	, is_bool_(false), is_int_(false), is_double_(false) ,is_t_string_(false), is_token_(false)  { }

config::attribute_value::~attribute_value() {}

config::attribute_value::attribute_value(const config::attribute_value &that)
	: int_value_ (that.int_value_),double_value_(that.double_value_)
	, t_string_value_(that.t_string_value_)
	, token_value_(that.token_value_), type_(that.type_) , bool_value_(that.bool_value_)
	, is_bool_(that.is_bool_), is_int_(that.is_int_), is_double_(that.is_double_)
	,is_t_string_(that.is_t_string_), is_token_(that.is_token_)  { }

config::attribute_value &config::attribute_value::operator=(const config::attribute_value &that) {
	bool_value_ = that.bool_value_; int_value_ = that.int_value_;
	double_value_ = that.double_value_; t_string_value_ = that.t_string_value_;
	token_value_ = that.token_value_; type_ = that.type_;
	is_bool_ = that.is_bool_; is_int_ = that.is_int_; is_double_ = that.is_double_;
	is_t_string_ = that.is_t_string_; is_token_ = that.is_token_;
	return *this;
}

config::attribute_value &config::attribute_value::operator=(bool v)
{
	type_ = BOOL;
	bool_value_ = v;
	is_bool_ = true; is_int_ = false; is_double_ = false; is_t_string_ = false; is_token_ =false;
	return *this;
}

config::attribute_value &config::attribute_value::operator=(int v)
{
	type_ = INT;
	int_value_ = v;
	is_bool_ = false; is_int_ = true; is_double_ = false; is_t_string_ = false; is_token_ =false;
	return *this;
}

config::attribute_value &config::attribute_value::operator=(double v)
{
	type_ = DOUBLE;
	double_value_ = v;
	is_bool_ = false; is_int_ = false; is_double_ = true; is_t_string_ = false; is_token_ =false;
	return *this;
}

config::attribute_value &config::attribute_value::operator=(const t_token &v)  {
	static const config::t_token & z_empty( generate_safe_static_const_t_interned(n_token::t_token("")) );
	static const config::t_token & z_yes( generate_safe_static_const_t_interned(n_token::t_token("yes")) );
	static const config::t_token & z_true( generate_safe_static_const_t_interned(n_token::t_token("true")) );
	static const config::t_token & z_no( generate_safe_static_const_t_interned(n_token::t_token("no")) );
	static const config::t_token & z_false( generate_safe_static_const_t_interned(n_token::t_token("false")) );
	if (v == z_empty) {
		type_ = EMPTY;
		is_bool_ = false; is_int_ = false; is_double_ = false; is_t_string_ = false; is_token_ =false;
		token_value_ = v; return *this; }
	if (v == z_yes || v == z_true) return *this = true;
	if (v == z_no || v == z_false) return *this = false;

	std::istringstream is(*v);
	int i;
	char extra_char;
	if( (is >> i) && ! is.get(extra_char) ) { *this = i; is_token_=true; token_value_ = v; return *this; }

	is.clear();
	is.str(*v);
	double d;
	if( (is >> d) && ! is.get(extra_char) ) { *this = d; is_token_=true; token_value_ = v; return *this; }

	type_ = TOKEN;
	is_bool_ = false; is_int_ = false; is_double_ = false; is_t_string_ = false; is_token_ =true;
	token_value_ = v;
	return *this;
}

config::attribute_value &config::attribute_value::operator=(const std::string &v) {
	static const config::t_token & z_empty( generate_safe_static_const_t_interned(n_token::t_token("")) );
	if (v.empty()) { return *this = z_empty;}
	if (v == "yes" || v == "true") return *this = true;
	if (v == "no" || v == "false") return *this = false;

	std::istringstream is( v );
	int i;
	char extra_char;
	if( (is >> i) && ! is.get(extra_char) ) { *this = i; return *this; }

	is.clear();
	is.str( v );
	double d;
	if( (is >> d) && ! is.get(extra_char) ) { *this = d; return *this; }
	type_ = TOKEN;
	is_bool_ = false; is_int_ = false; is_double_ = false; is_t_string_ = false; is_token_ =true;
	token_value_ = t_token(v);
	return *this;
}

config::attribute_value &config::attribute_value::operator=(const t_string &v) {
	if (!v.translatable()) { return *this = v.token(); }
	type_ = TSTRING;
	is_bool_ = false; is_int_ = false; is_double_ = false; is_t_string_ = true; is_token_ =false;
	t_string_value_ = v;
	return *this;
}

bool config::attribute_value::operator==(const config::attribute_value &other) const {
	bool retval = type_ == other.type_;
	if(retval){
		switch(type_){
		case(BOOL) :  retval &= bool_value_ == other.bool_value_; break;
		case(INT) :  retval &= int_value_ == other.int_value_; break;
		case(DOUBLE) :  retval &= double_value_ == other.double_value_; break;
		case(TSTRING) :  retval &= t_string_value_ == other.t_string_value_; break;
		case(TOKEN) :  retval &= token_value_ == other.token_value_; break;
		case(EMPTY) : break;
		}
	}
	return retval;
}

bool operator==(const config::attribute_value &a, config::t_token const & b) {
	static const config::t_token & z_yes( generate_safe_static_const_t_interned(n_token::t_token("yes")) );
	static const config::t_token & z_true( generate_safe_static_const_t_interned(n_token::t_token("true")) );
	static const config::t_token & z_no( generate_safe_static_const_t_interned(n_token::t_token("no")) );
	static const config::t_token & z_false( generate_safe_static_const_t_interned(n_token::t_token("false")) );
	//	if(a.type_ == a.EMPTY){return false;}
	//note: having 4 different acceptable boolean string values has a cost
	if ((a.type_ == a.BOOL) || ( a.is_bool_)){
		return (a.bool_value_ ?(b==z_true || b == z_yes) : (b == z_false || b == z_no)); }
	return a.token() ==  b;
}

bool operator==(const config::attribute_value &a, t_string const & b) {
	//	if(a.type_ == a.EMPTY){return false;}
	static const config::t_token & z_empty( generate_safe_static_const_t_interned(n_token::t_token("")) );
	static const config::t_token & z_yes( generate_safe_static_const_t_interned(n_token::t_token("yes")) );
	static const config::t_token & z_true( generate_safe_static_const_t_interned(n_token::t_token("true")) );
	static const config::t_token & z_no( generate_safe_static_const_t_interned(n_token::t_token("no")) );
	static const config::t_token & z_false( generate_safe_static_const_t_interned(n_token::t_token("false")) );
	static const t_string tstring_empty(z_empty)
		, tstring_true(z_true), tstring_false(z_false)
		, tstring_yes(z_yes), tstring_no(z_no);
	if ((a.type_ == a.BOOL) || ( a.is_bool_)){
		return (a.bool_value_ ? (b==tstring_true || b == tstring_yes) : (b == tstring_false || b == tstring_no)); }
	return a.t_str() == b; }


std::ostream &operator<<(std::ostream &os, const config::attribute_value &v)  {
	return os << v.str(); }

// bool config::attribute_value::blank() const { return (type_ == EMPTY); }

bool config::attribute_value::empty() const {
	if (type_ == EMPTY){ return true; }
	switch(type_){
	case(TSTRING) :  return t_string_value_.empty();
	case(TOKEN) :  return token_value_.empty();
	default : break;
	}
	return false;
}

bool config::attribute_value::to_bool(bool def) const {
	if ((type_ == BOOL) || ( is_bool_)){ return bool_value_; }
	return def;
}

int config::attribute_value::to_int(int def) const {
	if ((type_ == INT) || ( is_int_)){ return int_value_; }
	if ((type_ == DOUBLE) || ( is_double_)){ is_int_=true; int_value_ = int(double_value_); return int_value_;}
	return def;}

double config::attribute_value::to_double(double def) const {
	if ((type_ == DOUBLE) || ( is_double_)){ return double_value_; }
	if ((type_ == INT) || ( is_int_)){  is_double_ = true; double_value_= double(int_value_); return double_value_; }
	return def;
}

t_token const & config::attribute_value::token() const {
	static const config::t_token & z_empty( generate_safe_static_const_t_interned(n_token::t_token("")) );
	static const config::t_token & z_yes( generate_safe_static_const_t_interned(n_token::t_token("yes")) );
	static const config::t_token & z_no( generate_safe_static_const_t_interned(n_token::t_token("no")) );

	if ((type_ == TOKEN) || ( is_token_)){ return token_value_; }
	switch(type_){
	case (EMPTY) : return z_empty;
	case(BOOL) :  return bool_value_ ? z_yes : z_no;
	case(INT) :
		is_token_ = true;
		token_value_ = t_token(str_cast(int_value_));
		return token_value_;
	case(DOUBLE) :
		is_token_ = true;
		token_value_ = t_token(str_cast(double_value_));
		return token_value_;
	case(TSTRING) :
		return t_string_value_.token();
	default : break;
	}
	assert(false);
	return token_value_;
}

std::string const & config::attribute_value::str() const {
	return static_cast<std::string const &>(token());
}


t_string const & config::attribute_value::t_str() const {
	static const config::t_token & z_empty( generate_safe_static_const_t_interned(n_token::t_token("")) );
	static const config::t_token & z_yes( generate_safe_static_const_t_interned(n_token::t_token("yes")) );
	static const config::t_token & z_no( generate_safe_static_const_t_interned(n_token::t_token("no")) );
	static const t_string tstring_empty(z_empty)
		, tstring_yes(z_yes), tstring_no(z_no);

	if ((type_ == TSTRING) || ( is_t_string_)){
		return t_string_value_; }
	switch(type_){
	case (EMPTY) :
		return tstring_empty;
	case(BOOL) :
		return bool_value_ ? tstring_yes : tstring_no;
	case(INT) :
	case(DOUBLE) :
	case(TOKEN) :
		is_t_string_ = true;
		t_string_value_ = t_string(token());
		return t_string_value_;
	default : break;
	}
	assert(false);
	return t_string_value_;
}



config * config::invalid;
bool config::initialize_invalid() {
	//Run initialization once
	static bool ran_once = false;
	if (!ran_once ) {
		ran_once = true;
		invalid = NULL; }
	return true;
}

void config::throw_missing_child_exception() const {
	throw error("Mandatory WML child missing yet untested for. Please report."); }

config::config() : values(), children(), ordered_children() {
	//Run initialization once
	static bool invalid_initialized(initialize_invalid());
	(void) invalid_initialized; //quiet unused variable warning
}

config::config(const config& cfg) : values(cfg.values), children(), ordered_children() {
	static bool invalid_initialized(initialize_invalid());
	(void) invalid_initialized; //quiet unused variable warning
	append_children(cfg);
}

config::config(const t_token& child) : values(), children(), ordered_children() {
	static bool invalid_initialized(initialize_invalid());
	(void) invalid_initialized; //quiet unused variable warning
	add_child(child);
}
config::config(const std::string& child) : values(), children(), ordered_children() {
	static bool invalid_initialized(initialize_invalid());
	(void) invalid_initialized; //quiet unused variable warning
	add_child(t_token(child));
}

config::~config() {
	clear();
}

config& config::operator=(const config& cfg)
{
	if(this == &cfg) {
		return *this;
	}

	clear();
	append_children(cfg);
	values.insert(cfg.values.begin(), cfg.values.end());
	return *this;
}

#ifdef HAVE_CXX0X
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
#endif

bool config::has_attribute(const t_token &key) const
{
	check_valid();
	return values.find(key) != values.end();
}

bool config::has_attribute(const std::string &key) const  {
	return has_attribute(t_token(key)); }


bool config::has_old_attribute(const t_token &key, const t_token &old_key, const std::string& msg) const
{
	check_valid();
	if (values.find(key) != values.end()) {
		return true;
	} else if (values.find(old_key) != values.end()) {
		if (!msg.empty())
			lg::wml_error << msg;
		return true;
	}
	return false;
}
bool config::has_old_attribute(const std::string &key, const std::string &old_key, const std::string& msg) const  {
	return has_old_attribute(t_token(key), t_token(old_key), msg );}


void config::remove_attribute(const t_token &key)
{
	check_valid();
	values.erase(key);
}
void config::remove_attribute(const std::string &key) { remove_attribute(t_token(key));}

void config::append_children(const config &cfg)
{
	check_valid(cfg);

	foreach (const any_child &value, cfg.all_children_range()) {
		add_child(value.key, value.cfg);
	}
}

void config::append(const config &cfg)
{
	append_children(cfg);
	foreach (const attribute &v, cfg.values) {
		values[v.first] = v.second;
	}
}

void config::merge_children(const t_token& key)
{
	check_valid();

	if (child_count(key) < 2) return;

	config merged_children;
	foreach (const config &cfg, child_range(key)) {
		merged_children.append(cfg);
	}

	clear_children(key);
	add_child(key,merged_children);
}
void config::merge_children(const std::string& key) {merge_children(t_token(key));}

void config::merge_children_by_attribute(const t_token& key, const t_token& attribute)
{
	check_valid();

	if (child_count(key) < 2) return;

	typedef std::map<t_token, config> config_map;
	config_map merged_children_map;
	foreach (const config &cfg, child_range(key)) {
		const attribute_value &value = cfg[attribute];
		config_map::iterator m = merged_children_map.find(value.token());
		if ( m!=merged_children_map.end() ) {
			m->second.append(cfg);
		} else {
			merged_children_map.insert(std::make_pair(value.token(), cfg));
		}
	}

	clear_children(key);
	foreach (const config_map::value_type &i, merged_children_map) {
		add_child(key,i.second);
	}
}
void config::merge_children_by_attribute(const std::string& key, const std::string& attribute){
	merge_children_by_attribute(t_token(key), t_token(attribute));}

config::child_itors config::child_range(const t_token& key) {
	check_valid();

	child_map::iterator i = children.find(key);
	if (i != children.end()) {
		return child_itors(child_iterator(i->second.begin()), child_iterator(i->second.end()));
	}
	static child_list dummy;
	static const child_itors dummy_iters(child_iterator(dummy.begin()), child_iterator(dummy.end()));
	return dummy_iters;
}
config::child_itors config::child_range(const std::string& key){ return child_range( t_token(key) );}

config::const_child_itors config::child_range(const t_token& key) const {
	check_valid();

	child_map::const_iterator i = children.find(key);
	if (i != children.end()) {
		return const_child_itors(const_child_iterator(i->second.begin()), const_child_iterator(i->second.end())); }

	static const child_list dummy;
	//Use begin() and begin() in case someone violates the const contract
	static const_child_itors dummy_iters(const_child_iterator(dummy.begin()), const_child_iterator(dummy.begin()));
	return dummy_iters;
}

config::const_child_itors config::child_range(const std::string& key) const { return child_range(t_token(key));}

unsigned config::child_count(const t_token &key) const {
	check_valid();

	child_map::const_iterator i = children.find(key);
	if(i != children.end()) {
		return i->second.size();
	}
	return 0;
}
unsigned config::child_count(const std::string &key) const { return child_count(t_token(key)); }

config::t_child_range_index config::child_range_index(config::t_token const & key, config::t_token const & name) {
	t_child_range_index index;

	child_map::iterator irange = children.find(key);
	if (irange != children.end()) {
		foreach(config * c, irange->second) {
			index.insert(std::make_pair((*c)[name].token(), c)); } }
		return index; }

config::t_const_child_range_index config::const_child_range_index(config::t_token const & key, config::t_token const & name) const {
	t_const_child_range_index index;

	child_map::const_iterator irange = children.find(key);
	if (irange != children.end()) {
		foreach(config const * c, irange->second){
			index.insert(std::make_pair((*c)[name].token(), c)); } }
		return index; }

config &config::child(const t_token& key, int n)
{
	check_valid();

	const child_map::const_iterator i = children.find(key);
	if (i == children.end()) return *invalid;
	if (n < 0) n = i->second.size() + n;
	return  size_t(n) < i->second.size() ? *i->second[n] : *invalid;
}
config &config::child(const std::string& key, int n){ return child(t_token( key ), n); }

config config::child_or_empty(const t_token& key) const
{
	check_valid();

	child_map::const_iterator i = children.find(key);
	if (i != children.end() && !i->second.empty())
		return *i->second.front();

	return config();
}
config config::child_or_empty(const std::string& key) const { return child_or_empty(t_token( key )); }

config &config::child_or_add(const t_token &key)
{
	child_map::const_iterator i = children.find(key);
	if (i != children.end() && !i->second.empty())
		return *i->second.front();

	return add_child(key);
}
config &config::child_or_add(const std::string &key) {
	return child_or_add(t_token( key )) ;}

config& config::add_child(const t_token& key) {
	check_valid();

	child_list& v = children[key];
	v.push_back(new config());
	ordered_children.push_back(child_pos(children.find(key),v.size()-1)); //note doing find twice
	return *v.back();
}
config& config::add_child(const std::string& key) { return add_child(t_token( key ));}

config& config::add_child(const t_token& key, const config& val)
{
	check_valid(val);

	child_list& v = children[key];
	v.push_back(new config(val));
	ordered_children.push_back(child_pos(children.find(key),v.size()-1));
	return *v.back();
}
config& config::add_child(const std::string& key, const config& val) {return add_child(t_token(key), val);}

#ifdef HAVE_CXX0X
config &config::add_child(const t_token &key, config &&val)
{
	check_valid(val);

	child_list &v = children[key];
	v.push_back(new config(std::move(val)));
	ordered_children.push_back(child_pos(children.find(key), v.size() - 1));
	return *v.back();
}
config &config::add_child(const std::string &key, config &&val) {return add_child(t_token( key ), val);}
#endif

config &config::add_child_at(const t_token &key, const config &val, unsigned index)
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
config &config::add_child_at(const std::string &key, const config &val, unsigned index) {
	return add_child_at(t_token( key ), val, index); }

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

void config::clear_children(const t_token& key)
{
	check_valid();

	child_map::iterator i = children.find(key);
	if (i == children.end()) return;

	ordered_children.erase(std::remove_if(ordered_children.begin(),
		ordered_children.end(), remove_ordered(i)), ordered_children.end());

	foreach (config *c, i->second) {
		delete c;
	}

	children.erase(i);
}
void config::clear_children(const std::string& key) {return clear_children(t_token( key ));}

void config::splice_children(config &src, const t_token &key)
{
	check_valid(src);

	child_map::iterator i_src = src.children.find(key);
	if (i_src == src.children.end()) return;

	src.ordered_children.erase(std::remove_if(src.ordered_children.begin(),
		src.ordered_children.end(), remove_ordered(i_src)),
		src.ordered_children.end());

	child_list &dst = children[key];
	child_map::iterator i_dst = children.find(key);
	unsigned before = dst.size();
	dst.insert(dst.end(), i_src->second.begin(), i_src->second.end());
	src.children.erase(i_src);
	// key might be a reference to i_src->first, so it is no longer usable.

	for (unsigned j = before; j < dst.size(); ++j) {
		ordered_children.push_back(child_pos(i_dst, j));
	}
}
void config::splice_children(config &src, const std::string &key) {return splice_children(src, t_token( key ));}

void config::recursive_clear_value(const t_token& key)
{
	check_valid();

	values.erase(key);

	foreach (const any_child &value, all_children_range()) {
		const_cast<config *>(&value.cfg)->recursive_clear_value(key);
	}
}
void config::recursive_clear_value(const std::string& key) {recursive_clear_value(t_token( key ));}

std::vector<config::child_pos>::iterator config::remove_child(
	const child_map::iterator &pos, unsigned index)
{
	/* Find the position with the correct index and decrement all the
	   indices in the ordering that are above this index. */
	unsigned found = 0;
	foreach (child_pos &p, ordered_children)
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

void config::remove_child(const t_token &key, unsigned index)
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

void config::remove_child(const std::string &key, unsigned index) {return remove_child(t_token( key ), index);}

//#define COUNT_THE_NUMBER_OF_STRING_TO_TOKEN_CONVERSIONS
#ifdef COUNT_THE_NUMBER_OF_STRING_TO_TOKEN_CONVERSIONS
namespace{
	static n_count_logger::t_count_logger<std::string> count_btok1("config index as std::string (BAD)",11);
}
#endif

const config::attribute_value &config::operator[](const t_token &key) const {
	check_valid();

	const attribute_map::const_iterator i = values.find(key);
	if (i != values.end()){ return i->second; }
	static const attribute_value empty_attribute;
	return empty_attribute;
}

const config::attribute_value &config::operator[](const attribute_value &key) const {
	return operator[](key.token()); }

const config::attribute_value &config::operator[](const std::string &key) const{
#ifdef COUNT_THE_NUMBER_OF_STRING_TO_TOKEN_CONVERSIONS
	count_btok1.inc(key); //debug
#endif
	return operator[](t_token( key )); }

config::attribute_value &config::operator[](const t_token &key) {
	check_valid();
	return values[key];
}
config::attribute_value &config::operator[](const attribute_value &key){
	return operator[](key.token());}

config::attribute_value &config::operator[](const std::string &key){
#ifdef COUNT_THE_NUMBER_OF_STRING_TO_TOKEN_CONVERSIONS
	count_btok1.inc(key); //debug
#endif
	return operator[](t_token(key));}

const config::attribute_value *config::get(const t_token &key) const {
	check_valid();
	attribute_map::const_iterator i = values.find(key);
	return i != values.end() ? &i->second : NULL;
}
const config::attribute_value *config::get(const std::string &key) const{
#ifdef COUNT_THE_NUMBER_OF_STRING_TO_TOKEN_CONVERSIONS
	count_btok1.inc(key); //debug
#endif
	return get(t_token( key )); }

const config::attribute_value &config::get_old_attribute(const t_token &key, const t_token &old_key, const std::string &msg) const
{
	check_valid();

	attribute_map::const_iterator i = values.find(key);
	if (i != values.end())
		return i->second;

	i = values.find(old_key);
	if (i != values.end()) {
		if (!msg.empty())
			lg::wml_error << msg;
		return i->second;
	}

	static const attribute_value empty_attribute;
	return empty_attribute;
}
const config::attribute_value &config::get_old_attribute(const std::string &key, const std::string &old_key, const std::string &msg) const{
	return get_old_attribute(t_token( key ), t_token(old_key ), msg);}


void config::merge_attributes(const config &cfg)
{
	check_valid(cfg);

	assert(this != &cfg);
	foreach (const attribute &v, cfg.values) {
		values[v.first] = v.second;
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
	config_has_value(const t_token& name, const t_token& value)
		: name_(name), value_(value) {}

	bool operator()(const config* cfg) const {
		return (*cfg)[name_] == value_; }

private:
	t_token const &name_;
	t_token const &value_;
};

} // end namespace

config &config::find_child(const t_token &key, const t_token &name, const t_token &value) {
	check_valid();

	const child_map::iterator i = children.find(key);
	if(i == children.end()) { return *invalid; }

	const child_list::iterator j = std::find_if(i->second.begin(),
	                                            i->second.end(),
	                                            config_has_value(name,value));

	return (j != i->second.end()) ? **j :  *invalid;
}
config &config::find_child(const std::string &key, const std::string &name, const std::string &value){
	return find_child(t_token( key ), t_token(name), t_token(value));
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

config::all_children_iterator::reference config::all_children_iterator::operator*() const
{
	return any_child(&i_->pos->first, i_->pos->second[i_->index]);
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

void config::get_diff(const config& c, config& res) const {
	static const config::t_token & z_empty( generate_safe_static_const_t_interned(n_token::t_token("")) );
	static const config::t_token & z_index( generate_safe_static_const_t_interned(n_token::t_token("index")) );
	static const t_token & z_insert_child( generate_safe_static_const_t_interned(n_token::t_token("insert_child")) );
	static const t_token & z_delete_child( generate_safe_static_const_t_interned(n_token::t_token("delete_child")) );
	static const t_token & z_change_child( generate_safe_static_const_t_interned(n_token::t_token("change_child")) );
	static const t_token & z_insert( generate_safe_static_const_t_interned(n_token::t_token("insert")) );
	static const t_token & z_delete( generate_safe_static_const_t_interned(n_token::t_token("delete")) );
	static const t_token & z_x( generate_safe_static_const_t_interned(n_token::t_token("x")) );

	check_valid(c);
	check_valid(res);

	config* inserts = NULL;

	attribute_map::const_iterator i;
	for(i = values.begin(); i != values.end(); ++i) {
		const attribute_map::const_iterator j = c.values.find(i->first);
		if(j == c.values.end() || (i->second != j->second && i->second != z_empty)) {
			if(inserts == NULL) {
				inserts = &res.add_child(z_insert);
			}

			(*inserts)[i->first] = i->second;
		}
	}

	config* deletes = NULL;

	for(i = c.values.begin(); i != c.values.end(); ++i) {
		const attribute_map::const_iterator itor = values.find(i->first);
		if(itor == values.end() || itor->second == z_empty) {
			if(deletes == NULL) {
				deletes = &res.add_child(z_delete);
			}

			(*deletes)[i->first] = z_x;
		}
	}

	std::vector<t_token> entities;

	child_map::const_iterator ci;
	for(ci = children.begin(); ci != children.end(); ++ci) {
		entities.push_back(ci->first);
	}

	for(ci = c.children.begin(); ci != c.children.end(); ++ci) {
		if(children.count(ci->first) == 0) {
			entities.push_back(ci->first);
		}
	}

	for(std::vector<t_token>::const_iterator itor = entities.begin(); itor != entities.end(); ++itor) {

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
					config& new_delete = res.add_child(z_delete_child);
					buf << bi - ndeletes;
					new_delete.values[z_index] = buf.str();
					new_delete.add_child(*itor);

					++ndeletes;
					++bi;
				}

				// If b has less elements than a, then we assume this element
				// is an element that needs inserting.
				else if(b.size() - bi < a.size() - ai) {
					config& new_insert = res.add_child(z_insert_child);
					buf << ai;
					new_insert.values[z_index] = buf.str();
					new_insert.add_child(*itor,*a[ai]);

					++ai;
				}

				// Otherwise, they have the same number of elements,
				// so try just changing this element to match.
				else {
					config& new_change = res.add_child(z_change_child);
					buf << bi;
					new_change.values[z_index] = buf.str();
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
	static const config::t_token & z_index( generate_safe_static_const_t_interned(n_token::t_token("index")) );
	static const t_token & z_diff_track_attribute_( generate_safe_static_const_t_interned(n_token::t_token("diff_track_attribute_")) );
	static const t_token & z_modified( generate_safe_static_const_t_interned(n_token::t_token("modified")) );
	static const t_token & z_insert( generate_safe_static_const_t_interned(n_token::t_token("insert")) );
	static const t_token & z_delete( generate_safe_static_const_t_interned(n_token::t_token("delete")) );
	static const t_token & z_insert_child( generate_safe_static_const_t_interned(n_token::t_token("insert_child")) );
	static const t_token & z_delete_child( generate_safe_static_const_t_interned(n_token::t_token("delete_child")) );
	static const t_token & z_change_child( generate_safe_static_const_t_interned(n_token::t_token("change_child")) );
	static const t_token & z_deleted( generate_safe_static_const_t_interned(n_token::t_token("deleted")) );
	static const t_token & z_new( generate_safe_static_const_t_interned(n_token::t_token("new")) );

	check_valid(diff);

	if (track) values[z_diff_track_attribute_] = z_modified;

	if (const config &inserts = diff.child(z_insert)) {
		foreach (const attribute &v, inserts.attribute_range()) {
			values[v.first] = v.second;
		}
	}

	if (const config &deletes = diff.child(z_delete)) {
		foreach (const attribute &v, deletes.attribute_range()) {
			values.erase(v.first);
		}
	}

	foreach (const config &i, diff.child_range(z_change_child))
	{
		const size_t index = lexical_cast<size_t>(i[z_index].str());
		foreach (const any_child &item, i.all_children_range())
		{
			if (item.key.empty()) {
				continue;
			}

			const child_map::iterator itor = children.find(item.key);
			if(itor == children.end() || index >= itor->second.size()) {
				throw error("error in diff: could not find element '" + std::string(item.key) + "'");
			}

			itor->second[index]->apply_diff(item.cfg, track);
		}
	}

	foreach (const config &i, diff.child_range(z_insert_child))
	{
		const size_t index = lexical_cast<size_t>(i[z_index].str());
		foreach (const any_child &item, i.all_children_range()) {
			config& inserted = add_child_at(item.key, item.cfg, index);
			if (track) inserted[z_diff_track_attribute_] = z_new;
		}
	}

	foreach (const config &i, diff.child_range(z_delete_child))
	{
		const size_t index = lexical_cast<size_t>(i[z_index].str());
		foreach (const any_child &item, i.all_children_range()) {
			if (!track) {
				remove_child(item.key, index);
			} else {
				const child_map::iterator itor = children.find(item.key);
				if(itor == children.end() || index >= itor->second.size()) {
					throw error("error in diff: could not find element '" + std::string(item.key) + "'");
				}
				itor->second[index]->values[z_diff_track_attribute_] = z_deleted;
			}
		}
	}
}

void config::clear_diff_track(const config& diff)
{
	static const t_token & z_diff_track_attribute_( generate_safe_static_const_t_interned(n_token::t_token("diff_track_attribute_")) );
	static const t_token & z_index( generate_safe_static_const_t_interned(n_token::t_token("index")) );
	static const t_token & z_delete_child( generate_safe_static_const_t_interned(n_token::t_token("delete_child")) );
	static const t_token & z_change_child( generate_safe_static_const_t_interned(n_token::t_token("change_child")) );

	remove_attribute(z_diff_track_attribute_);
	foreach (const config &i, diff.child_range(z_delete_child))
	{
		const size_t index = lexical_cast<size_t>(i[z_index].str());
		foreach (const any_child &item, i.all_children_range()) {
			remove_child(item.key, index);
		}
	}

	foreach (const config &i, diff.child_range(z_change_child))
	{
		const size_t index = lexical_cast<size_t>(i[z_index].str());
		foreach (const any_child &item, i.all_children_range())
		{
			if (item.key.empty()) {
				continue;
			}

			const child_map::iterator itor = children.find(item.key);
			if(itor == children.end() || index >= itor->second.size()) {
				throw error("error in diff: could not find element '" + std::string(item.key) + "'");
			}

			itor->second[index]->clear_diff_track(item.cfg);
		}
	}
	foreach (const any_child &value, all_children_range()) {
		const_cast<config *>(&value.cfg)->remove_attribute(z_diff_track_attribute_);
	}
}

void config::merge_with(const config& c)
{
	check_valid(c);

	std::map<t_token, unsigned> visitations;

	// Merge attributes first
	merge_attributes(c);

	// Now merge shared tags
	all_children_iterator::Itor i, i_end = ordered_children.end();
	for(i = ordered_children.begin(); i != i_end; ++i) {
		const t_token& tag = i->pos->first;
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
		const t_token& tag = j->first;
		unsigned &visits = visitations[tag];
		while(visits < j->second.size()) {
			add_child(tag, *j->second[visits++]);
		}
	}
}

bool config::matches(const config &filter) const
{
	static const t_token & z_not( generate_safe_static_const_t_interned(n_token::t_token("not")) );

	check_valid(filter);

	foreach (const attribute &i, filter.attribute_range())
	{
		const attribute_value *v = get(i.first);
		if (!v || *v != i.second) return false;
	}

	foreach (const any_child &i, filter.all_children_range())
	{
		if (i.key == z_not) {
			if (matches(i.cfg)) return false;
			continue;
		}
		bool found = false;
		foreach (const config &j, child_range(i.key)) {
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
	std::map<config::t_token, config::attribute_value> sorted;
	foreach (const config::attribute &ipresorted, cfg.attribute_range()) {
		sorted.insert( ipresorted ); }
	foreach (const config::attribute &val, sorted) {
		for (int j = 0; j < i-1; j++){ outstream << char(9); }
		outstream << val.first << " = " << val.second << '\n';
	}
	foreach (const config::any_child &child, cfg.all_children_range())
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

	//@todo change these to a standard machine independent string hash function

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
	foreach (const attribute &val, values)
	{
		std::string key_str(val.first);
		std::string base_str = val.second.t_str().base_str();
		std::string long_string = key_str + base_str;
		
		size_t j =0, jend = long_string.size();
		for (; j != jend; ++j) {
			char c = long_string[j];
			hash_str[(j + c + jend) % hash_length] ^= c;
		}
	}

	foreach (const any_child &ch, all_children_range())
	{
		std::string child_hash = ch.cfg.hash();
		foreach (char c, child_hash) {
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

	config::all_children_itors x = a.all_children_range(), y = b.all_children_range();
	for (; x.first != x.second && y.first != y.second; ++x.first, ++y.first) {
		if (x.first->key != y.first->key || x.first->cfg != y.first->cfg) {
			return false;
		}
	}

	return x.first == x.second && y.first == y.second;
}
