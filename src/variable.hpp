/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Copyright (C) 2005 by Philippe Plantier <ayin@anathas.org>

   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef VARIABLE_H_INCLUDED
#define VARIABLE_H_INCLUDED

#include <string>
#include <vector>

#include "gamestatus.hpp"
class config;
class t_string;

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

	const t_string& operator[](const std::string&) const;
	const t_string& expand(const std::string&) const; /** < Synonym for operator[] */
	const t_string& get_attribute(const std::string& key) const { return (*cfg_)[key]; }

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

// Here should go a class which servers as a variable repository
#if 0
class repository
{
public:
private:
	config variables_;
}
#endif

#endif
