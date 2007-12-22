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
#ifndef VARIABLE_H_INCLUDED
#define VARIABLE_H_INCLUDED

#include "config.hpp"
#include "tstring.hpp"

#include <vector>
#include <string>

class game_state;
class unit_map;

/**
 * A variable-expanding proxy for the config class. This class roughly behaves
 * as a constant config object, but automatically expands variables.
 */
class vconfig
{
public:
	vconfig();
	vconfig(const config* cfg);

	vconfig& operator=(const vconfig cfg);
	vconfig& operator=(const config* cfg);

	bool null() const { return cfg_ == NULL; }
	const config& get_config() const { return *cfg_; }
	const config get_parsed_config() const;

	typedef std::vector<vconfig> child_list;
	child_list get_children(const std::string& key) const;
	vconfig child(const std::string& key) const;
	bool has_child(const std::string& key) const;

	const t_string expand(const std::string&) const; /** < Synonym for operator[] */
	const t_string operator[](const std::string& key) const { return expand(key); }
	const t_string& get_attribute(const std::string& key) const { return (*cfg_)[key]; }
	bool has_attribute(const std::string& key) const { return cfg_->has_attribute(key); }
	bool empty() const { return (null() || cfg_->empty()); }

private:
	const config* cfg_;
};

namespace variable
{

/**
 * Used to for the functions in variable.cpp to locate the current global
 * variable repository
 */
class manager
{
public:
	manager(game_state* repository);
	~manager();
};

}



class scoped_wml_variable
{
public:
	scoped_wml_variable(const std::string& var_name);
	virtual ~scoped_wml_variable();
	const std::string& name() const { return var_name_; }
	virtual void activate() = 0;
	void store(const config& var_value);
	bool activated() const { return activated_; }
private:
	config previous_val_;
	const std::string var_name_;
	bool activated_;
};

class scoped_xy_unit : public scoped_wml_variable
{
public:
	scoped_xy_unit(const std::string& var_name, const int x, const int y, const unit_map& umap)
		: scoped_wml_variable(var_name), x_(x), y_(y), umap_(umap) {}
	void activate();
private:
	const int x_, y_;
	const unit_map& umap_;
};

class scoped_recall_unit : public scoped_wml_variable
{
public:
	scoped_recall_unit(const std::string& var_name, const std::string& player,
		unsigned int recall_index) : scoped_wml_variable(var_name), player_(player),
		recall_index_(recall_index) {}
	void activate();
private:
	const std::string player_;
	unsigned int recall_index_;
};

/** Information on a WML variable. */
struct variable_info
{
typedef std::pair<std::vector<config*>::iterator, std::vector<config*>::iterator> array_range;
public:
	/**
	 * TYPE: the correct variable type should be decided by the user of the info structure
	 * Note: an Array can also be considered a Container, since index 0 will be used by default
	 */
	enum TYPE { TYPE_SCALAR,    //a Scalar variable resolves to a t_string attribute of *vars
	            TYPE_ARRAY,     //an Array variable is a series of Containers
	            TYPE_CONTAINER, //a Container is a specific index of an Array (contains Scalars)
	            TYPE_UNSPECIFIED };

	variable_info(const std::string& varname, bool force_valid=true,
		TYPE validation_type=TYPE_UNSPECIFIED);

	TYPE vartype; //default is TYPE_UNSPECIFIED
	bool is_valid;
	std::string key; //the name of the internal attribute or child
	bool explicit_index; //true if query ended in [...] specifier
	size_t index; //the index of the child
	config *vars; //the containing node in game_state::variables

	/**
	 * Results: after deciding the desired type, these methods can retrieve the result
	 * Note: first you should force_valid or check is_valid, otherwise these may fail
	 */
	t_string& as_scalar();
	config& as_container();
	array_range as_array(); //range may be empty
};

#endif
