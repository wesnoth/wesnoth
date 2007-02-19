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

#include "global.hpp"

#include "variable.hpp"
#include "config.hpp"
#include <iostream>

namespace
{
	// FIXME: the variable repository should be a class of variable.hpp,
	// and not the game_state.
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

const t_string &vconfig::operator[](const std::string& key) const
{
	return expand(key);
}

const t_string &vconfig::expand(const std::string& key) const
{
	const t_string& val = (*cfg_)[key];

        if(repos != NULL && !val.str().empty() && val.str()[0] == '$') {
                 const t_string &tmp = repos->get_variable(val.str().substr(1));
		 // if variable was not found, we return it "as is"
		 if(!tmp.empty()) return tmp;
	}
	return val;
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

scoped_wml_variable::scoped_wml_variable(const std::string var_name,const config&var_value):
	var_name_(var_name)

{
	previous_val_= repos->variables;
	repos->variables.clear_children(var_name);
	repos->variables.add_child(var_name,var_value);
}
scoped_wml_variable::~scoped_wml_variable()
{
	repos->variables.clear_children(var_name_);
	config::child_list old_val =previous_val_.get_children(var_name_);
	for(config::child_list::iterator j=old_val.begin(); j != old_val.end() ; j++){
		repos->variables.add_child(var_name_,**j);
	}
}
