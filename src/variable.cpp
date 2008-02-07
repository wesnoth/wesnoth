/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2007 by Philippe Plantier <ayin@anathas.org>

   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file variable.cpp
//! Manage WML-variables.

#include "global.hpp"

#include "config.hpp"
#include "gamestatus.hpp"
#include "log.hpp"

#include <cassert>
#include <iostream>

#define LOG_NG LOG_STREAM(info, engine)
#define WRN_NG LOG_STREAM(warn, engine)
#define ERR_NG LOG_STREAM(err, engine)

namespace
{
	//! @todo FIXME: the variable repository should be
	// a class of variable.hpp, and not the game_state.
	game_state* repos = NULL;
}

vconfig::vconfig() :
	cfg_(NULL)
{
}

vconfig::vconfig(const config* cfg) :
	cfg_(cfg)
{
}

vconfig& vconfig::operator=(const vconfig cfg)
{
	cfg_ = cfg.cfg_;
	return *this;
}

vconfig& vconfig::operator=(const config* cfg)
{
	cfg_ = cfg;
	return *this;
}

const config vconfig::get_parsed_config() const
{
	config res;

	for(string_map::const_iterator itor = cfg_->values.begin();
			itor != cfg_->values.end(); ++itor) {

		res[itor->first] = expand(itor->first);
	}

	for(config::all_children_iterator child = cfg_->ordered_begin();
			child != cfg_->ordered_end(); ++child) {

		res.add_child(*(*child).first, vconfig((*child).second).get_parsed_config());
	}
	return res;
}

vconfig::child_list vconfig::get_children(const std::string& key) const
{
	const config::child_list& list = cfg_->get_children(key);
	vconfig::child_list res(list.size());
	std::copy(list.begin(), list.end(),res.begin());
	return res;
}

vconfig vconfig::child(const std::string& key) const
{
	return vconfig(cfg_->child(key));
}

bool vconfig::has_child(const std::string& key) const
{
	return (cfg_->child(key) != NULL);
}

const t_string vconfig::expand(const std::string& key) const
{
	const t_string& val = (*cfg_)[key];
	if(repos != NULL && !val.str().empty()) {
		std::string interp = utils::interpolate_variables_into_string(val.str(), *repos);
		if(val.str() != interp) {
			return t_string(interp);
		}
	}
	return t_string(val);
}

namespace variable
{
	manager::manager(game_state* repository)
	{
		repos = repository;
	}

	manager::~manager()
	{
		repos = NULL;
	}
}

scoped_wml_variable::scoped_wml_variable(const std::string& var_name)
	: var_name_(var_name), activated_(false)
{
	repos->scoped_variables.push_back(this);
}

void scoped_wml_variable::store(const config& var_value)
{
	const config::child_list& children = repos->get_variables().get_children(var_name_);
	for(config::child_list::const_iterator i = children.begin(); i != children.end(); ++i) {
		previous_val_.append(**i);
	}
	repos->clear_variable_cfg(var_name_);
	repos->add_variable_cfg(var_name_, var_value);
	activated_ = true;
}

scoped_wml_variable::~scoped_wml_variable()
{
	if(activated_) {
		repos->clear_variable_cfg(var_name_);
		config::child_list old_val =previous_val_.get_children(var_name_);
		for(config::child_list::iterator j=old_val.begin(); j != old_val.end() ; j++){
			repos->add_variable_cfg(var_name_,**j);
		}
	}
	assert(repos->scoped_variables.back() == this);
	repos->scoped_variables.pop_back();
}

void scoped_xy_unit::activate()
{
	unit_map::const_iterator itor = umap_.find(gamemap::location(x_,y_));
	if(itor != umap_.end()) {
		config tmp_cfg;
		itor->second.write(tmp_cfg);
		tmp_cfg["x"] = lexical_cast<std::string,int>(x_ + 1);
		tmp_cfg["y"] = lexical_cast<std::string,int>(y_ + 1);
		store(tmp_cfg);
	} else {
		ERR_NG << "failed to auto-store $" << name() << " at (" << x_ << ',' << y_ << ")\n";
	}
}

void scoped_recall_unit::activate()
{
	player_info* const player = repos->get_player(player_);
	if(player != NULL) {
		if(player->available_units.size() > recall_index_) {
			config tmp_cfg;
			player->available_units[recall_index_].write(tmp_cfg);
			tmp_cfg["x"] = "recall";
			tmp_cfg["y"] = "recall";
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
bool recursive_activation = false;

//! Turns on any auto-stored variables
void activate_scope_variable(std::string var_name)
{
	if(recursive_activation)
		return;
	const std::string::iterator itor = std::find(var_name.begin(),var_name.end(),'.');
	if(itor != var_name.end()) {
		var_name.erase(itor, var_name.end());
	}
	std::vector<scoped_wml_variable*>::reverse_iterator rit;
	for(rit = repos->scoped_variables.rbegin(); rit != repos->scoped_variables.rend(); ++rit) {
		if((**rit).name() == var_name) {
			recursive_activation = true;
			if(!(**rit).activated()) {
				(**rit).activate();
			}
			recursive_activation = false;
			break;
		}
	}
}
} // end anonymous namespace

variable_info::variable_info(const std::string& varname, bool force_valid, TYPE validation_type)
	: vartype(validation_type), is_valid(false), explicit_index(false), index(0), vars(NULL)
{
	assert(repos != NULL);
	activate_scope_variable(varname);

	vars = &repos->variables;
	key = varname;
	std::string::const_iterator itor = std::find(key.begin(),key.end(),'.');
	int dot_index = key.find('.');
	// example varname = "unit_store.modifications.trait[0]"
	while(itor != key.end()) { // subvar access
		std::string element=key.substr(0,dot_index);
		key = key.substr(dot_index+1);

		size_t inner_index = 0;
		const std::string::iterator index_start = std::find(element.begin(),element.end(),'[');
		const bool inner_explicit_index = index_start != element.end();
		if(inner_explicit_index) {
			const std::string::iterator index_end = std::find(index_start,element.end(),']');
			const std::string index_str(index_start+1,index_end);
			inner_index = static_cast<size_t>(lexical_cast_default<int>(index_str));
			if(inner_index > game_config::max_loop) {
				ERR_NG << "variable_info: index greater than " << game_config::max_loop
				       << ", truncated\n";
				inner_index = game_config::max_loop;
			}
			element = std::string(element.begin(),index_start);
		}

		size_t size = vars->get_children(element).size();
		if(size <= inner_index) {
			if(force_valid) {
				// Add elements to the array until the requested size is attained
				if(inner_explicit_index || key != "length") {
					for(; size <= inner_index; ++size) {
						vars->add_child(element);
					}
				}
			} else if(inner_explicit_index) {
				WRN_NG << "variable_info: invalid WML array index, "
					<< varname << std::endl;
				return;
			} else if(key != "length") {
				WRN_NG << "variable_info: retrieving member of non-existant WML container, "
					<< varname << std::endl;
				return;
			} //else return length 0 for non-existant WML array (handled below)
		}
		if(!inner_explicit_index && key == "length") {
			switch(vartype) {
			case variable_info::TYPE_ARRAY:
			case variable_info::TYPE_CONTAINER:
				WRN_NG << "variable_info: using reserved WML variable as wrong type, "
					<< varname << std::endl;
				is_valid = force_valid || repos->temporaries.child(varname) != NULL;
				break;
			case variable_info::TYPE_SCALAR:
			default:
				// Store the length of the array as a temporary variable
				repos->temporaries[varname] = lexical_cast<std::string>(size);
				is_valid = true;
				break;
			}
			key = varname;
			vars = &repos->temporaries;
			return;
		}

		//std::cerr << "Entering " << element << "[" << inner_index << "] of [" << vars->get_children(element).size() << "]\n";
		vars = vars->get_children(element)[inner_index];
		itor = std::find(key.begin(),key.end(),'.');
		dot_index = key.find('.');
	} // end subvar access

	const std::string::iterator index_start = std::find(key.begin(),key.end(),'[');
	explicit_index = index_start != key.end();
	if(explicit_index) {
		const std::string::iterator index_end = std::find(index_start,key.end(),']');
		const std::string index_str(index_start+1,index_end);
		index = static_cast<size_t>(lexical_cast_default<int>(index_str));
		if(index > game_config::max_loop) {
			ERR_NG << "variable_info: index greater than " << game_config::max_loop
				   << ", truncated\n";
			index = game_config::max_loop;
		}
		key = std::string(key.begin(),index_start);
		size_t size = vars->get_children(key).size();
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
			vars = vars->get_children(key)[index];
			key = "__array";
			is_valid = force_valid || vars->child(key) != NULL;
			break;
		case variable_info::TYPE_SCALAR:
			vars = vars->get_children(key)[index];
			key = "__value";
			is_valid = force_valid || vars->has_attribute(key);
			break;
		case variable_info::TYPE_CONTAINER:
		case variable_info::TYPE_UNSPECIFIED:
		default:
			is_valid = true;
			return;
		}
		WRN_NG << "variable_info: using explicitly indexed Container as wrong WML type, "
			<< varname << std::endl;
		explicit_index = false;
		index = 0;
	} else {
		// Final variable is not an explicit index [...]
		switch(vartype) {
		case variable_info::TYPE_ARRAY:
		case variable_info::TYPE_CONTAINER:
			is_valid = force_valid || vars->child(key);
			break;
		case variable_info::TYPE_SCALAR:
			is_valid = force_valid || vars->has_attribute(key);
			break;
		case variable_info::TYPE_UNSPECIFIED:
		default:
			is_valid = true;
			break;
		}
	}
}

t_string& variable_info::as_scalar() {
	assert(is_valid);
	return vars->values[key];
}

config& variable_info::as_container() {
	assert(is_valid);
	if(explicit_index) {
		// Empty data for explicit index was already created if it was needed
		return *vars->get_children(key)[index];
	}
	config *temp = vars->child(key);
	if(temp) {
		// The container exists, index not specified, return index 0
		return *temp;
	}
	// Add empty data for the new variable, since it does not exist yet
	return vars->add_child(key);
}

variable_info::array_range variable_info::as_array() {
	assert(is_valid);
	return vars->child_range(key);
}
