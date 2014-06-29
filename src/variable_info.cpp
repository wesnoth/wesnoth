/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2014 by Philippe Plantier <ayin@anathas.org>

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
#include "variable_info.hpp"

#include "log.hpp"
#include "game_config.hpp"
#include "util.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

namespace {
	config temporaries;// lengths of arrays, etc.
}


config& variable_info::get_temporaries() { return temporaries; }

variable_info::variable_info(config& source, const std::string& varname,
		bool force_valid, TYPE validation_type) :
	vartype(validation_type),
	is_valid(false),
	key(),
	original_key(varname),
	explicit_index(false),
	index(0),
	vars(NULL)
{
	vars = &source;//&resources::gamedata->variables_;
	key = varname;
	std::string::const_iterator itor = std::find(key.begin(),key.end(),'.');
	int dot_index = key.find('.');

	bool force_length = false;
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

		size_t size = vars->child_count(element);
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
			} else if(varname.length() >= 7 && varname.substr(varname.length()-7) == ".length") {
				// require '.' to avoid matching suffixes -> requires varname over key to always find length
				// return length 0 for non-existent WML array (handled below)
				force_length = true;
			} else {
				WRN_NG << "variable_info: retrieving member of non-existent WML container, "
				<< varname << std::endl;
				return;
			}
		}
		if((!inner_explicit_index && key == "length") || force_length) {
			switch(vartype) {
			case variable_info::TYPE_ARRAY:
			case variable_info::TYPE_CONTAINER:
				WRN_NG << "variable_info: using reserved WML variable as wrong type, "
					<< varname << std::endl;
				is_valid = force_valid || temporaries.child(varname);
				break;
			case variable_info::TYPE_SCALAR:
			default:
				// Store the length of the array as a temporary variable
				temporaries[varname] = int(size);
				is_valid = true;
				break;
			}
			key = varname;
			vars = &temporaries;
			return;
		}

		vars = &vars->child(element, inner_index);
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
		size_t size = vars->child_count(key);
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
			key = "__array";
			is_valid = force_valid || vars->child(key);
			break;
		case variable_info::TYPE_SCALAR:
			vars = &vars->child(key, index);
			key = "__value";
			is_valid = force_valid || vars->has_attribute(key);
			break;
		case variable_info::TYPE_CONTAINER:
		case variable_info::TYPE_UNSPECIFIED:
		default:
			is_valid = true;
			return;
		}
		if (force_valid) {
			WRN_NG << "variable_info: using explicitly indexed "
				"container as wrong WML type, " << varname << '\n';
		}
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

config::attribute_value &variable_info::as_scalar()
{
	assert(is_valid);
	return (*vars)[key];
}

config::attribute_value variable_info::as_scalar_const()
{

	config::attribute_value* r;
	if(is_valid)
	{
		r = &as_scalar();
	}
	else
	{
		r = &temporaries[this->original_key];
		if (original_key.size() > 7 && original_key.substr(original_key.size() - 7) == ".length") {
			// length is a special attribute, so guarantee its correctness
			*r = 0;
		}
	}
	return *r;
	
}

config& variable_info::as_container() {
	assert(is_valid);
	if(explicit_index) {
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
	assert(is_valid);
	return vars->child_range(key);
}

void variable_info::set_range(config& data, std::string mode)
{

	if(mode == "extend") {
		mode = "append";
	} else if(mode != "append" && mode != "merge") {
		if(mode == "insert") {
			size_t child_count = this->vars->child_count(this->key);
			if(this->index >= child_count) {
				while(this->index >= ++child_count) {
					//inserting past the end requires empty data
					this->vars->append(config(this->key));
				}
				//inserting at the end is handled by an append
				mode = "append";
			}
		} else {
			mode = "replace";
		}
	}


	if(mode == "replace")
	{
		if(this->explicit_index) {
			this->vars->remove_child(this->key, this->index);
		} else {
			this->vars->clear_children(this->key);
		}
	}
	if(!data.empty())
	{
		if(mode == "merge")
		{
			if(this->explicit_index) {
				// merging multiple children into a single explicit index
				// requires that they first be merged with each other
				data.merge_children(this->key);
				this->as_container().merge_with(data.child(this->key));
			} else {
				this->vars->merge_with(data);
			}
		} else if(mode == "insert" || this->explicit_index) {
			BOOST_FOREACH(const config &child, data.child_range(this->key))
			{
				this->vars->add_child_at(this->key, child, this->index++);
			}
		} else {
			this->vars->append(data);
		}
	}
}

void variable_info::clear(bool only_tables)
{
	if(is_valid)
	{
		if(this->explicit_index) {
			this->vars->remove_child(this->key, this->index);
		} else {
			this->vars->clear_children(this->key);
			if(!only_tables)
			{
				this->vars->remove_attribute(this->key);
			}
		}
	}
}

config& variable_info::add_child(const config& value)
{
	return this->vars->add_child(this->key, value);
}
