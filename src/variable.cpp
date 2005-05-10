/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Copyright (C) 2005 by Philippe Plantier <ayin@anathas.org>
 
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "variable.hpp"
#include "gamestatus.hpp"
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

bool vconfig::null() const
{
	return cfg_ == NULL;
}

const config& vconfig::get_config() const
{
	return *cfg_;
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

const t_string& vconfig::operator[](const std::string& key) const
{
	return expand(key);
}

const t_string& vconfig::expand(const std::string& key) const
{
	const t_string& val = (*cfg_)[key];

	if(repos != NULL && !val.str().empty() && val.str()[0] == '$') {
		return repos->get_variable(val.str().substr(1));
	} else {
		return val;
	}
}

const t_string& vconfig::get_attribute(const std::string& key) const
{
	return (*cfg_)[key];
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

