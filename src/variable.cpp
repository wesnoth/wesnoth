/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2011 by Philippe Plantier <ayin@anathas.org>

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
 *  @file
 *  Manage WML-variables.
 */

#include "global.hpp"

#include "variable.hpp"

#include "foreach.hpp"
#include "formula_string_utils.hpp"
#include "gamestatus.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "unit.hpp"
#include "unit_map.hpp"
#include "team.hpp"

#include <boost/variant.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "utils/lru_cache.hpp"

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

namespace
{
/**
 * @todo FIXME: the variable repository should be
 * a class of variable.hpp, and not the game_state.
 */
#define repos (resources::state_of_game)

// keeps track of insert_tag variables used by get_parsed_config
typedef boost::unordered_set<std::string> t_vconfig_recursion;
t_vconfig_recursion vconfig_recursion;

/**
 * config_cache is a map to track temp storage of inserted tags on the heap.
 * If an event is spawned from a variable or any ActionWML from a volatile
 * source is to be executed safely then its memory should be managed by vconfig
 */
typedef boost::unordered_map<config const *, int> t_config_cache;
t_config_cache config_cache;

// map by hash for equivalent inserted tags already in the cache
typedef boost::unordered_map<std::string const *, config const *> t_hash_to_cache;
t_hash_to_cache hash_to_cache;

// map to remember config hashes that have already been calculated
typedef boost::unordered_map<config const *, std::string const *> t_config_hashes;
t_config_hashes config_hashes;


//Static tokens instantiated a single time as replacements for string literals
static const config::t_token z_insert_tag("insert_tag", false);
static const config::t_token z_name("name", false);
static const config::t_token z_variable("variable", false);
static const config::t_token z_x("x", false);
static const config::t_token z_y("y", false);
static const config::t_token z_recall("recall", false);
static const config::t_token z_length("length", false);
static const config::t_token z___array("__array", false);
static const config::t_token z___value("__value", false);


config empty_config;

struct compare_str_ptr {
	bool operator()(const std::string* s1, const std::string* s2) const
	{
		return (*s1) < (*s2);
	}
};

class hash_memory_manager {
public:
	hash_memory_manager() :
		mem_()
	{
	}

	const std::string *find(const std::string& str) const {
		// t_mem_::const_iterator itor = mem_.lower_bound(&str);
		t_mem_::const_iterator itor = mem_.find(&str);
		if(itor == mem_.end() || **itor != str) {
			return NULL;
		}
		return *itor;
	}
	void insert(const std::string *newhash) {
		mem_.insert(newhash);
	}
	void clear() {
		hash_to_cache.clear();
		config_hashes.clear();
		t_mem_::iterator mem_it,
			mem_end = mem_.end();
		for(mem_it = mem_.begin(); mem_it != mem_end; ++mem_it) {
			delete *mem_it;
		}
		mem_.clear();
	}
	~hash_memory_manager() {
		clear();
	}
private:
	//typedef boost::unordered_set<std::string const*, compare_str_ptr> t_mem_;
	typedef boost::unordered_set<std::string const*> t_mem_;
	t_mem_ mem_;
};
hash_memory_manager hash_memory;
}

static const std::string* get_hash_of(const config* cp) {
	//first see if the memory of a constant config hash exists
	t_config_hashes::iterator ch_it = config_hashes.find(cp);
	if(ch_it != config_hashes.end()) {
		return ch_it->second;
	}
	//next see if an equivalent hash string has been memorized
	const std::string & temp_hash = cp->hash();
	std::string const* find_hash = hash_memory.find(temp_hash);
	if(find_hash != NULL) {
		return find_hash;
	}
	//finally, we just allocate a new hash string to memory
	std::string* new_hash = new std::string(temp_hash);
	hash_memory.insert(new_hash);
	//do not insert into config_hashes (may be a variable config)
	return new_hash;
}

static void increment_config_usage(const config*& key) {
	if(key == NULL) return;
	t_config_cache::iterator this_usage =  config_cache.find(key);
	if(this_usage != config_cache.end()) {
		++this_usage->second;
		return;
	}
	const std::string *hash = get_hash_of(key);
	const config *& cfg_store = hash_to_cache[hash];
	if(cfg_store == NULL || (key != cfg_store && *key != *cfg_store)) {
		// this is a new volatile config: allocate some memory & update key
		key = new config(*key);
		// remember this cache to prevent an equivalent one from being created
		cfg_store = key;
		// since the config is now constant, we can safely memorize the hash
		config_hashes[key] = hash;
	} else {
		// swap the key with an equivalent or equal one in the cache
		key = cfg_store;
	}
	++(config_cache[key]);
}

static void decrement_config_usage(const config* key) {
	if(key == NULL) return;
	t_config_cache::iterator this_usage = config_cache.find(key);
	assert(this_usage != config_cache.end());
	if(--(this_usage->second) == 0) {
		config_cache.erase(this_usage);
		if(config_cache.empty()) {
			hash_memory.clear();
		} else {
			if(!hash_to_cache.empty()) {
				hash_to_cache.erase(get_hash_of(key));
			}
			config_hashes.erase(key);
		}
		delete key;
	}
}

vconfig::vconfig() :
	cfg_(NULL), cache_key_(NULL)
{
}

vconfig::vconfig(const config* cfg, const config * cache_key) :
	cfg_(cfg), cache_key_(cache_key)
{
	increment_config_usage(cache_key_);
	if(cache_key_ != cache_key) {
		//location of volatile cfg has moved
		cfg_ = cache_key_;
	}
}

vconfig::vconfig(const config &cfg, bool is_volatile) :
	cfg_(&cfg), cache_key_(&cfg)
{
	if(is_volatile) {
		increment_config_usage(cache_key_);
		if (cache_key_ != &cfg) {
			//location of volatile cfg has moved
			cfg_ = cache_key_;
		}
	} else {
		cache_key_ = NULL;
	}
}

vconfig::vconfig(const vconfig& v) :
	cfg_(v.cfg_), cache_key_(v.cache_key_)
{
	increment_config_usage(cache_key_);
}

vconfig::~vconfig()
{
	decrement_config_usage(cache_key_);
}

vconfig vconfig::empty_vconfig()
{
    return vconfig(config(), true);
}

vconfig vconfig::unconstructed_vconfig()
{
    return vconfig();
}

vconfig& vconfig::operator=(const vconfig& cfg)
{
	const config* prev_key = cache_key_;
	cfg_ = cfg.cfg_;
	cache_key_ = cfg.cache_key_;
	increment_config_usage(cache_key_);
	decrement_config_usage(prev_key);
	return *this;
}

const config vconfig::get_parsed_config() const
{
	config res;
	foreach (const config::attribute &i, cfg_->attribute_range()) {
		res[i.first] = expand(i.first);
	}

	foreach (const config::any_child &child, cfg_->all_children_range())
		{
			if (child.key == z_insert_tag) {
				vconfig insert_cfg(child.cfg);
				const t_string& name = insert_cfg[z_name];
				const t_string& vname = insert_cfg[z_variable];
				if(!vconfig_recursion.insert(vname).second) {
					throw recursion_error("vconfig::get_parsed_config() infinite recursion detected, aborting");
				}
				try {
					variable_info vinfo(vname.token(), false, variable_info::TYPE_CONTAINER);
					if(!vinfo.is_valid()) {
						res.add_child(name); //add empty tag
					} else if(vinfo.is_explicit_index()) {
						res.add_child(name, vconfig(vinfo.as_container()).get_parsed_config());
					} else {
						variable_info::array_range range = vinfo.as_array();
						if(range.first == range.second) {
							res.add_child(name); //add empty tag
						}
						while(range.first != range.second) {
							res.add_child(name, vconfig(*range.first++).get_parsed_config());
						}
					}
					vconfig_recursion.erase(vname);
				} catch(recursion_error &err) {
					vconfig_recursion.erase(vname);
					WRN_NG << err.message << std::endl;
					if(vconfig_recursion.empty()) {
						res.add_child(z_insert_tag, insert_cfg.get_config());
					} else {
						// throw to the top [insert_tag] which started the recursion
						throw;
					}
				}
			} else {
				res.add_child(child.key, vconfig(child.cfg).get_parsed_config());
			}
		}
	return res;
}

vconfig::child_list vconfig::get_children(const config::t_token& key) const
{
	vconfig::child_list res;

	foreach (const config::any_child &child, cfg_->all_children_range())
		{
			if (child.key == key) {
				res.push_back(vconfig(&child.cfg, cache_key_));
			} else if (child.key == z_insert_tag) {
				vconfig insert_cfg(child.cfg);
				if(insert_cfg[z_name] == key) {
					variable_info vinfo(insert_cfg[z_variable].token(), false, variable_info::TYPE_CONTAINER);
					if(!vinfo.is_valid()) {
						//push back an empty tag
						res.push_back(vconfig(empty_config));
					} else if(vinfo.is_explicit_index()) {
						config * cp = &(vinfo.as_container());
						res.push_back(vconfig(cp, cp));
					} else {
						variable_info::array_range range = vinfo.as_array();
						if(range.first == range.second) {
							//push back an empty tag
							res.push_back(vconfig(empty_config));
						}
						while(range.first != range.second) {
							config *cp = &*range.first++;
							res.push_back(vconfig(cp, cp));
						}
					}
				}
			}
		}
	return res;
}

vconfig::child_list vconfig::get_children(const std::string& key) const {return get_children(t_token(key));}

vconfig vconfig::child(const config::t_token& key) const
{
	if (const config &natural = cfg_->child(key)) {
		return vconfig(&natural, cache_key_);
	}
	foreach (const config &ins, cfg_->child_range(z_insert_tag))
		{
			vconfig insert_cfg(ins);
			if(insert_cfg[z_name] == key) {
				variable_info vinfo(insert_cfg[z_variable].token(), false, variable_info::TYPE_CONTAINER);
				if(!vinfo.is_valid()) {
					return vconfig(empty_config);
				}
				config * cp = &(vinfo.as_container());
				return vconfig(cp, cp);
			}
		}
	return unconstructed_vconfig();
}
vconfig vconfig::child(const std::string& key) const {return child(t_token(key));}

bool vconfig::has_child(const config::t_token& key) const
{
	if (cfg_->child(key)) {
		return true;
	}
	foreach (const config &ins, cfg_->child_range(z_insert_tag))
		{
			vconfig insert_cfg(ins);
			if(insert_cfg[z_name] == key) {
				return true;
			}
		}
	return false;
}
bool vconfig::has_child(const std::string& key) const {return has_child(t_token(key));}

struct vconfig_expand_visitor : public config::attribute_value::default_visitor {
	using default_visitor::operator();
	config::attribute_value & val;
	vconfig_expand_visitor(config::attribute_value & v):val(v){}
	void operator()(config::t_token const & token){
		config::t_token cp(token);
		utils::interpolate_variables_into_token(cp, *repos);
		val = cp;		
	}
	void operator()(const t_string &s){
		val = utils::interpolate_variables_into_tstring(s, *repos);
	}
};

config::attribute_value vconfig::expand(const config::t_token &key) const {
	config::attribute_value val = (*cfg_)[key];
	if (repos) {
		vconfig_expand_visitor visitor(val);
		val.apply_visitor(visitor);
	}
	return val;
}
config::attribute_value vconfig::expand(const std::string &key) const {return operator[](t_token(key));}

vconfig::all_children_iterator::all_children_iterator(const Itor &i, const config *cache_key)
  : i_(i), inner_index_(0), cache_key_(cache_key)
{
}

vconfig::all_children_iterator& vconfig::all_children_iterator::operator++() {
	if (inner_index_ >= 0 && i_->key == z_insert_tag)
		{
			variable_info vinfo(vconfig(i_->cfg)[z_variable].token(), false, variable_info::TYPE_CONTAINER);
			if(vinfo.is_valid() && !vinfo.is_explicit_index()) {
				variable_info::array_range range = vinfo.as_array();
				if (++inner_index_ < std::distance(range.first, range.second)) {
					return *this;
				}
				inner_index_ = 0;
			}
		}
	++i_;
	return *this;
}

vconfig::all_children_iterator vconfig::all_children_iterator::operator++(int)
{
	vconfig::all_children_iterator i = *this;
	this->operator++();
	return i;
}

vconfig::all_children_iterator::reference vconfig::all_children_iterator::operator*() const
{
	return value_type(get_key(), get_child());
}

vconfig::all_children_iterator::pointer vconfig::all_children_iterator::operator->() const
{
	pointer_proxy p = { value_type(get_key(), get_child()) };
	return p;
}


config::t_token vconfig::all_children_iterator::get_key() const
{
	const config::t_token &key = i_->key;
	if (inner_index_ >= 0 && key == z_insert_tag) {
		return vconfig(i_->cfg)[z_name];
	}
	return key;
}

vconfig vconfig::all_children_iterator::get_child() const
{
	if (inner_index_ >= 0 && i_->key == z_insert_tag)
		{
			config * cp;
			variable_info vinfo(vconfig(i_->cfg)[z_variable].token(), false, variable_info::TYPE_CONTAINER);
			if(!vinfo.is_valid()) {
				return vconfig(empty_config);
			} else if(inner_index_ == 0) {
				cp = &(vinfo.as_container());
				return vconfig(cp, cp);
			}
			variable_info::array_range r = vinfo.as_array();
			std::advance(r.first, inner_index_);
			cp = &*r.first;
			return vconfig(cp, cp);
		}
	return vconfig(&i_->cfg, cache_key_);
}

bool vconfig::all_children_iterator::operator==(const all_children_iterator &i) const
{
	return i_ == i.i_ && inner_index_ == i.inner_index_;
}

vconfig::all_children_iterator vconfig::ordered_begin() const
{
	return all_children_iterator(cfg_->ordered_begin(), cache_key_);
}

vconfig::all_children_iterator vconfig::ordered_end() const
{
	return all_children_iterator(cfg_->ordered_end(), cache_key_);
}

namespace variable
{
manager::~manager()
{
	hash_memory.clear();
}
}

scoped_wml_variable::scoped_wml_variable(const std::string& var_name) :
	previous_val_(), var_name_(var_name), activated_(false) {
	repos->scoped_variables.push_back(this);
}
scoped_wml_variable::scoped_wml_variable(const config::t_token& var_name) :
	previous_val_(), var_name_(var_name), activated_(false) {
	repos->scoped_variables.push_back(this);
}

config &scoped_wml_variable::store(const config &var_value)
{
	foreach (const config &i, repos->get_variables().child_range(var_name_)) {
		previous_val_.add_child(var_name_, i);
	}
	repos->clear_variable_cfg(var_name_);
	config &res = repos->add_variable_cfg(var_name_, var_value);
	LOG_NG << "scoped_wml_variable: var_name \"" << var_name_ << "\" has been auto-stored.\n";
	activated_ = true;
	return res;
}

scoped_wml_variable::~scoped_wml_variable()
{
	if(activated_) {
		repos->clear_variable_cfg(var_name_);
		foreach (const config &i, previous_val_.child_range(var_name_)) {
			repos->add_variable_cfg(var_name_, i);
		}
		LOG_NG << "scoped_wml_variable: var_name \"" << var_name_ << "\" has been reverted.\n";
	}
	assert(repos->scoped_variables.back() == this);
	repos->scoped_variables.pop_back();
}

void scoped_xy_unit::activate()
{
	map_location loc = map_location(x_, y_);
	unit_map::const_iterator itor = umap_.find(loc);
	if(itor != umap_.end()) {
		config &tmp_cfg = store();
		itor->write(tmp_cfg);
		tmp_cfg[z_x] = x_ + 1;
		tmp_cfg[z_y] = y_ + 1;
		LOG_NG << "auto-storing $" << name() << " at (" << loc << ")\n";
	} else {
		ERR_NG << "failed to auto-store $" << name() << " at (" << loc << ")\n";
	}
}

void scoped_weapon_info::activate()
{
	if (data_) {
		store(data_);
	}
}

void scoped_recall_unit::activate()
{
	const t_teams& teams = teams_manager::get_teams();
	t_teams::const_iterator team_it;
	for (team_it = teams.begin(); team_it != teams.end(); ++team_it) {
		if (team_it->save_id() == player_ )
			break;
	}

	if(team_it != teams.end()) {
		if(team_it->recall_list().size() > recall_index_) {
			config &tmp_cfg = store();
			team_it->recall_list()[recall_index_].write(tmp_cfg);
			tmp_cfg[z_x] = z_recall;
			tmp_cfg[z_y] = z_recall;
			LOG_NG << "auto-storing $" << name() << " for player: " << player_
				   << " at recall index: " << recall_index_ << '\n';
			store(tmp_cfg);
		} else {
			ERR_NG << "failed to auto-store $" << name() << " for player: " << player_
				   << " at recall index: " << recall_index_ << '\n';
		}
	} else {
		ERR_NG << "failed to auto-store $" << name() << " for player: " << player_ << '\n';
	}
}


namespace {
static const config::t_token z_dot(".", false);
static const config::t_token z_lbracket("[", false);
static const config::t_token z_rbracket("]", false);


typedef config::t_token t_token;
struct t_parsed { 
	enum {NO_INDEX = -1};
	t_parsed(t_token const & a, int i = NO_INDEX):token(a),index(i){}
	t_parsed(t_parsed const & a):token(a.token), index(a.index){}
	t_token token;  
	int index; };

typedef std::vector<t_parsed> t_parsed_tokens;

class t_parse_token {
public: 
	t_parsed_tokens	operator()(config::t_token const & key) const {
		//an example varname is  "unit_store.modifications.trait[0]"

		t_parsed_tokens parsed_tokens;
		std::string const & skey(key);

		std::size_t i(0), i_start_of_token(0);
		std::size_t iend(skey.size());
		bool is_lbrack(false);
		while(i != iend){
			char c = skey[i];

			if(i == i_start_of_token){
				switch(c){
				case '.' :
				case '[':
				case ']':
					throw game::wml_syntax_error(skey, i, "the first character of identifier is one of these,  .[] invalid characters" ); 
				}
			}
			if(is_lbrack){
				switch(c){
				case '.' :
				case '[': 
					throw game::wml_syntax_error(skey, i, "a dot . or left bracket [ after left bracket [ starting the variable name"); 
					break;
				case ']':				
					std::string index_str(skey.substr(i_start_of_token, i - i_start_of_token ));
					std::istringstream is(index_str);
					size_t index;
					is >> index;
					if(index > game_config::max_loop) {
						ERR_NG << "variable_info: index greater than " << game_config::max_loop << ", truncated\n";
						index = game_config::max_loop;
					}
					parsed_tokens.back().index = index;
					
					//adjust for dot after lbrack  as  in "unit_store.modifications.trait[0].y"
					if (i < (iend-1) ) {
						char next_c = skey[i+1] ;
						if(next_c == '.'){
							++i; } }
					i_start_of_token =  i + 1 ;
					is_lbrack=false;
					break;
				}
			} else {
				switch(c){
				case '.' :
					parsed_tokens.push_back(t_parsed(t_token(skey.substr(i_start_of_token, i - i_start_of_token ))));
					i_start_of_token = i + 1;
					break;
				case '[':
					parsed_tokens.push_back(t_parsed(t_token(skey.substr(i_start_of_token, i - i_start_of_token  ))));
					i_start_of_token = i + 1;
					is_lbrack=true;
					break;
				case ']':
					break;
				}
			}
			++i;
		}
		if(i_start_of_token != i){
			parsed_tokens.push_back(t_token(skey.substr(i_start_of_token, i - i_start_of_token )));	
		}

		return parsed_tokens;
	}
};

//	typedef boost::unordered_map<config::t_token, t_parsed_tokens> t_all_parsed;
static const unsigned int CACHE_SIZE = 10000;
typedef  n_lru_cache::t_lru_cache<config::t_token, t_parsed_tokens, t_parse_token> t_all_parsed;



bool recursive_activation = false;

/** Turns on any auto-stored variables */
void activate_scope_variable(t_parsed_tokens const & tokens)
{
	if(recursive_activation){ return; }

	if(! tokens.empty()){
	
		config::t_token const & first((tokens.begin()->token));

		std::vector<scoped_wml_variable*>::reverse_iterator rit;
		for(rit = repos->scoped_variables.rbegin(); rit != repos->scoped_variables.rend(); ++rit) {
			if((**rit).name() == first) {
				recursive_activation = true;
				if(!(**rit).activated()) {
					(**rit).activate();
				}
				recursive_activation = false;
				break;
			}
		}
	}
}
} //end namespace


void variable_info::init(const config::t_token& varname, bool force_valid) {
	try {

		//an example varname is  "unit_store.modifications.trait[0]"

		assert(repos != NULL);

		static t_all_parsed cache( t_parse_token(), CACHE_SIZE);

		t_parsed_tokens tokens(cache.check(varname));

		if(tokens.empty()){ return; }

		activate_scope_variable(tokens);
		vars = &repos->variables_;

		t_parsed_tokens::iterator i(tokens.begin()), last_token(tokens.end())
			, second_last_token(tokens.end()), i_array_name, i_maybe_tail(i);
		if(!tokens.empty()) { --last_token; }
		if(tokens.size() > 1) { second_last_token-=2; }

		//process subvars
		while (i <  last_token){
			int inner_index = 0;
			int size = vars->child_count( i->token);

			if(i->index != t_parsed::NO_INDEX){
				inner_index = i->index;

				if(size <= inner_index) {
					bool last_key_is_not_length ((i == second_last_token) && (last_token->token != z_length));
					if(force_valid) {
						// Add elements to the array until the requested size is attained
						if( (inner_index > 0) ||  last_key_is_not_length) {
							for(; size <= inner_index; ++size) {
								vars->add_child(i->token);
							}
						}
					} else if(inner_index != 0) {
						WRN_NG << "variable_info: invalid WML array index, " << varname << std::endl;
						return;
					} else if( last_key_is_not_length ) {
						WRN_NG << "variable_info: retrieving member of non-existent WML container, " << varname << std::endl;
						return;
					} //else return length 0 for non-existent WML array (handled below)
				}
			} else {
				if( (i == second_last_token) && (last_token->token == z_length) ) {
					switch(vartype) {
					case variable_info::TYPE_ARRAY:
					case variable_info::TYPE_CONTAINER:
						WRN_NG << "variable_info: using reserved WML variable as wrong type, "
							   << varname << std::endl;
						is_valid_ = force_valid || repos->temporaries_.child(varname);
						break;
					case variable_info::TYPE_SCALAR:
					default:
						// Store the length of the array as a temporary variable
						repos->temporaries_[varname] = int(size);
						is_valid_ = true;
						break;
					}
					key = varname;
					vars = &repos->temporaries_;
					return;
				}
			}
			vars = &vars->child(i->token, inner_index);		
			++i;
		}	

		//Process the last token

		key = i->token;
		if(i->index != t_parsed::NO_INDEX){
			explicit_index_ = true;
			size_t size = vars->child_count(key);
			index = i->index;
			if(size <= index) {
				if(!force_valid) {
					WRN_NG << "variable_info: invalid WML array index, " << varname << std::endl;
					return;
				}
				for(; size <= index; ++size) {
					vars->add_child(key);
				}
			}
			switch(vartype) {
			case variable_info::TYPE_ARRAY:
				vars = &vars->child(key, index);
				key = z___array;
				is_valid_ = force_valid || vars->child(key);
				break;
			case variable_info::TYPE_SCALAR:
				vars = &vars->child(key, index);
				key = z___value;
				is_valid_ = force_valid || vars->has_attribute(key);
				break;
			case variable_info::TYPE_CONTAINER:
			case variable_info::TYPE_UNSPECIFIED:
			default:
				is_valid_ = true;
				return;
			}
			if (force_valid) {
				WRN_NG << "variable_info: using explicitly indexed "
					"container as wrong WML type, " << varname << '\n';
			}
			explicit_index_ = false;
			index = 0;
		} else {
			// Final variable is not an explicit index [...]
			switch(vartype) {
			case variable_info::TYPE_ARRAY:
			case variable_info::TYPE_CONTAINER:
				is_valid_ = force_valid || vars->child(key);
				break;
			case variable_info::TYPE_SCALAR:
				is_valid_ = force_valid || vars->has_attribute(key);
				break;
			case variable_info::TYPE_UNSPECIFIED:
			default:
				is_valid_ = true;
				break;
			}
		}
	} catch (game::wml_syntax_error & e) {
		ERR_NG << e.what()<<"\n";
		is_valid_ = false;
	}
}

variable_info::variable_info(const config::t_token& varname, bool force_valid, TYPE validation_type) 
	: vartype(validation_type), is_valid_(false), explicit_index_(false), key(varname),  index(0), vars(NULL) {
	init( varname,  force_valid) ;}

variable_info::variable_info(const t_string& varname, bool force_valid, TYPE validation_type)
	: vartype(validation_type), is_valid_(false), explicit_index_(false),key(varname.token()),  index(0), vars(NULL) {
	init(varname.token(), force_valid);}

variable_info::variable_info(const std::string& varname, bool force_valid, TYPE validation_type)
	: vartype(validation_type), is_valid_(false), explicit_index_(false), key(varname), index(0), vars(NULL) {
	init(config::t_token(varname), force_valid);}

variable_info::variable_info(const config::attribute_value& varname, bool force_valid, TYPE validation_type)
	: vartype(validation_type), is_valid_(false), explicit_index_(false), key(varname.token()), index(0), vars(NULL) {
	init(varname.token(), force_valid);}



config::attribute_value &variable_info::as_scalar() {
	assert(is_valid_);
	return (*vars)[key];
}

config& variable_info::as_container() {
	assert(is_valid_);
	if(explicit_index_) {
		// Empty data for explicit index was already created if it was needed
		return vars->child(key, index);
	}
	if (config &temp = vars->child(key)) {
		// The container exists, index not specified, return index 0
		return temp;
	}
	// Add empty data for the new variable, since it does not exist yet
	return vars->add_child(key);
}

variable_info::array_range variable_info::as_array() {
	assert(is_valid_);
	return vars->child_range(key);
}
