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
	scoped_wml_variable(const std::string var_name,const config&var_value);
	~scoped_wml_variable();
private:
	config previous_val_;
	std::string var_name_;
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
